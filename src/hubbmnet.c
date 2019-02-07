/********************************************
 * @author : Gökhan ÖZELOĞLU                *
 *                                          *
 * @date : 07.11.2018 - 25.11.2018          *
 *                                          *
 * @email : b21627557@cs.hacettepe.edu.tr   *
 ********************************************
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

typedef struct {
    int log_entry;
    char **log_time;
    char *message;
    int hop_number;
    int frame_number;
    char *sender_ID;
    char *receiver_ID;
    char **activity;
    char **success;
    int reach;
} Log;

typedef struct {    /** Stores the some of client informations to print out in message part*/
    int sender_client_index;
    int receiver_client_index;
    char *sender_client_ID;
    char *receiver_client_ID;
    char *sender_client_MAC;
    char *receiver_client_MAC;
    char *next_client_MAC;
    char *next_client_name;
} Client_Infos;

typedef struct {    /** Holds the Physical Layer informations*/
    char *receiver_MAC;
    char *sender_MAC;
} PhysicalLayer;

typedef struct {    /** Holds the Network Layer informations*/
    char *receiver_IP;
    char *sender_IP;
} NetworkLayer;

typedef struct {    /** Holds the Transport Layer informations*/
    char *receiver_port_number;
    char *sender_port_number;
} TransportLayer;

typedef struct {    /** Holds the Application Layer informations*/
    char *receiver_ID;
    char *sender_ID;
    char *message_chunk;
} ApplicationLayer;

typedef struct {
    int top;
    void *frame[4];
} Stack;

typedef struct {
    int front;
    int rear;
    int frame_num;
    Stack **queue_array;
} Queue;

typedef struct {    /** Clients informations.*/
    char *client_name;
    char *IP;
    char *MAC;
    Queue *incoming_queue;
    Queue *outgoing_queue;
    Log *log;
} Clients;

void push(void *layer, Stack* stack) {
    stack->frame[++stack -> top] = layer;
}

void* pop(Stack *stack) {
    return stack->frame[stack->top--];
}

/** @details: Controls the queue is empty or not
 *  @return: If the queue is empty returns true
 *  If queue is not empty false
 * */
bool isEmpty(Queue* queue) {
    if (queue -> rear == -1) return true;
    else return false;
}

void enqueue(Stack *frame, Queue *queue) {
    queue -> rear++;
    queue -> queue_array[queue->rear] = frame;  /*toDO : THERE IS A PROBLEM*/
}

/** @param: queue is a Queue object
 *  @details: Front variable increments by one and returns the layer which in Stack type
 *  @return: Layer in Stack type
 * */
Stack *dequeue(Queue* queue) {
    return queue->queue_array[queue -> front++];
}

/** @param: queue is a Queue object
 *  @param frame_num represents the number of frame in queue
 *  @details: Just prints out the layer informations
 * */
void print_frames(Queue *queue, int frame_num) {
    int i;

    for (i = 0 ; i < frame_num ; i++) {
        printf("FRAME #%d\n", i+1);
        printf("Sender MAC address: %s, Receiver MAC address: %s\n", (*(PhysicalLayer *)(queue -> queue_array[i]) -> frame[3]).sender_MAC, (*(PhysicalLayer *)(queue -> queue_array[i])->frame[3]).receiver_MAC);
        printf("Sender IP address: %s, Receiver IP address: %s\n", (*(NetworkLayer *)(queue -> queue_array[i]) -> frame[2]).sender_IP, (*(NetworkLayer *)(queue -> queue_array[i])->frame[2]).receiver_IP);
        printf("Sender port number: %s, Receiver port number: %s\n", (*(TransportLayer *)(queue -> queue_array[i]) -> frame[1]).sender_port_number, (*(TransportLayer *)(queue -> queue_array[i]) -> frame[1]).receiver_port_number);
        printf("Sender ID: %s, Receiver ID: %s\n", (*(ApplicationLayer *)(queue -> queue_array[i]) -> frame[0]).sender_ID, (*(ApplicationLayer *)(queue -> queue_array[i]) -> frame[0]).receiver_ID);
        printf("Message chunk carried: %s\n", (*(ApplicationLayer *)(queue -> queue_array[i]) -> frame[0]).message_chunk);
        printf("--------\n");
    }
}

/** @param: clients_file is a FILE pointer which points to client data file
 *  @param: clients_array is a pointer array which its type is Clients
 *  @details: This function reads the clients file and stores the data into a pointer array which its type Clients(struct)
 *  @return: clients_array after filling the client data
 * */
Clients* read_client_data(FILE *clients_file, Clients* clients_array) {
    int j = 0;
    char *token, buffer[512];  /* For reading the data line by line with fgets */

    while (fgets(buffer, sizeof(buffer), clients_file)) {
        token = strtok(buffer, " \n");
        clients_array[j].client_name = (char *) malloc(sizeof(char) * strlen(token));
        strcpy(clients_array[j].client_name, token);

        token = strtok(NULL, " \n");
        clients_array[j].IP = (char *) malloc(sizeof(char) * strlen(token));
        strcpy(clients_array[j].IP, token);

        token = strtok(NULL, " \n");
        clients_array[j].MAC = (char *) malloc(sizeof(char) * strlen(token));
        strcpy(clients_array[j].MAC, token);

        j++;
    }

    return clients_array;
}

/** @param: message is the whole message read from the command data
 *  @param: max_msg_size represents the length of the available for one message chunk
 *  @param: parsed message is a 2D char pointer to store each message chunks dynamically
 *  @param: remainder is a number that represents the final message chunk's length
 *  @details: Firstly, program allocates the memory for parsed_message for each chunk dynamically.
 *  Then, controls the remainder's value if it is zero or not. If it is not zero, dynamically memory allocation is done
 *  for final message chunk. After that, dividing message operation is done with for loop by using `strncat` function.
 *  If remainder is not zero, the final remaining message is added to parsed_message array.
 * */
void parse_message(char *message, int max_msg_size, char **parsed_message, int remainder) {
    size_t chunk_size = strlen(message) / max_msg_size; /* For allocating memory for each chunk */

    int i;
    for (i = 0; i < chunk_size ; i++) {
        *(parsed_message + i) = (char *) malloc(sizeof(char) * max_msg_size);
    }
    if (remainder != 0) {
        *(parsed_message + i) = (char *) malloc(sizeof(char) * remainder);
    }

    for (i = 0 ; i < chunk_size ; i++) {
        strncat(*(parsed_message + i), message, max_msg_size);  /* Dividing operation in given size */
        message += max_msg_size;    /* Pointer goes ahead in maximum message size */
    }

    if (remainder != 0) {   /* If there is still message to divide and store into array */
        strncat(*(parsed_message + i), message, remainder);
    }
}

/** @param clients_array stores the client informations
 *  @param sender_client represents the sender client's name
 *  @param receiver_client represents the client name that we want to reach
 *  @param routing_array stores the routing informations as a string
 *  @param client_num represents the number of client
 *  @return next neighbor
 *  @details Firstly, finds the index of our sender client in client_array
 *  Stores the index in tmp variable
 *  i variable redefines and points to starting index in routing_array
 *  While loop searches the routing_array to find receiver(neighbor) client
 * */
char *find_next_neighbor(Clients* clients_array, char *sender_client_MAC, char *receiver_client_IP, char **routing_array, int client_num) {
    int i, sender_client_index = 0;
    char *next_client_MAC = malloc(sizeof(char));
    for (i = 0 ; i < client_num ; i++) {
        if (strcmp(clients_array[i].MAC, sender_client_MAC) == 0) { /*Sender's name matches in client_array */
            sender_client_index = i;
            break;
        }
    }

    int receiver_client_index = 0;

    for (i = 0 ; i < client_num; i++) {     /*Finds the receiver client's index in client array by using receiver_client_IP parameter*/
        if (strcmp(clients_array[i].IP, receiver_client_IP) == 0) {
            receiver_client_index = i;
            break;
        }
    }

    i = 0;

    int receive_rout_ind = 0;
    while (i < (client_num - 1) * 2) {
        if (clients_array[receiver_client_index].client_name[0] == routing_array[sender_client_index][i]) {   /*Finds the intended client ID (E.g. client "E"*/
            receive_rout_ind = i;
            break;
        }
        i += 2;
    }

    char next_client_ID = routing_array[sender_client_index][receive_rout_ind+1];

    if (next_client_ID == '-') {    /*Drop situation*/
        next_client_MAC = realloc(next_client_MAC, sizeof(char) * strlen("-"));
        strcpy(next_client_MAC, "-");
    } else {
        for (i = 0 ; i < client_num ; i++) {    /*Searches the next client's MAC address in client array */
            if (clients_array[i].client_name[0] == routing_array[sender_client_index][receive_rout_ind+1]) {
                next_client_MAC = realloc(next_client_MAC, sizeof(char) * strlen(clients_array[i].MAC));
                strcpy(next_client_MAC, clients_array[i].MAC);
                break;
            }
        }
    }

    return next_client_MAC;    /*Returns MAC Address*/
}

char *next_client_ID(Clients *clients_array, char *MAC_address, int client_num) {
    char *next_client_ID = malloc(sizeof(char));
    bool found = false;
    int i = 0;

    while (i < client_num || found != true) {
        if (strcmp(clients_array[i].MAC, MAC_address) == 0) {
            next_client_ID = realloc(next_client_ID, sizeof(char) * strlen(clients_array[i].client_name));
            strcpy(next_client_ID, clients_array[i].client_name);

            found = true;
        }
        i++;
    }

    return next_client_ID;
}
/** @param: clients_array stores the client's informations like id, ip, name, etc
 *  @param: client_num is number of clients
 *  @details: The function allocates the memory in dynamically by using for loop.
 *  Loop iterates the client_num times.
 *  For each clients outgoing and incoming queues are declared here.
 * */
void declare_client_queues(Clients *clients_array, int client_num, int frame_num) {
    int i;

    for (i = 0 ; i < client_num ; i++) {
        clients_array[i].outgoing_queue = malloc(sizeof(Stack) * frame_num);
        clients_array[i].outgoing_queue -> rear = -1;
        clients_array[i].outgoing_queue -> front = 0;
        clients_array[i].outgoing_queue->frame_num = 0;

        clients_array[i].incoming_queue = malloc(sizeof(Stack) * frame_num);
        clients_array[i].incoming_queue -> rear = -1;
        clients_array[i].incoming_queue -> front = 0;
        clients_array[i].incoming_queue->frame_num = 0;
    }
}

void getTime(char *buff) {
    time_t now = time(NULL);
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
}

/** @param: outgoing_queue is sender queue which sends the frames to other client's incoming queue
 *  @param: clients_array stores the client's inforamation
 *  @param: routing_array holds the client's neighbor relationship
 *  @param: sender_MAC represents the sender client's MAC address to compare
 *  @param: receiver_IP represents the intended client's IP address
 *  @param: frame_num represents the number of frames
 *  @param: client_num represents the number of clients
 *  @param: hop_num represents the number of hops
 *  @details: This function is a recursive function that calls itself for each hop. Make compare the client's, and if frames do not reach the
 *  intended client, then finds the next client.
 * */
void send(Queue *outgoing_queue, Clients *clients_array, char **routing_array, char *sender_MAC, char *receiver_IP, int frame_num, int client_num, int hop_num) {
    int i;

    int incoming_index = 0;
    /*Finds the next client's index by using IP address.*/
    for (i = 0; i < client_num; i++) {
        if (strcmp(clients_array[i].IP, receiver_IP) == 0) {
            incoming_index = i;
            break;
        }
    }

    int log_entry = clients_array[incoming_index].log->log_entry;
    clients_array[incoming_index].log->activity[log_entry] = malloc(sizeof(char *) * strlen("Message Received"));
    strcpy(clients_array[incoming_index].log->activity[log_entry], "Message Received");

    clients_array[incoming_index].log->success[log_entry] = malloc(sizeof(char *) * strlen("Yes"));
    strcpy(clients_array[incoming_index].log->success[log_entry], "Yes");

    clients_array[incoming_index].log->log_time[log_entry] = malloc(sizeof(char *) * 21);
    getTime(clients_array[incoming_index].log->log_time[log_entry]);

    clients_array[incoming_index].log->hop_number++;
    clients_array[incoming_index].log->log_entry++;

    int j;
    /*Allocates the memory for next client's incoming queue.*/
    clients_array[incoming_index].incoming_queue->queue_array = (Stack **) malloc(frame_num * sizeof(Stack *));

    outgoing_queue->frame_num = 0;
    for (j = 0; j < frame_num; j++) { /*Transfer operation*/
        enqueue(dequeue(outgoing_queue), clients_array[incoming_index].incoming_queue);  /*Transfer from X's outgoing to Y's incoming*/
        outgoing_queue->queue_array[j] = NULL;
        free(outgoing_queue->queue_array[j]);
    }
    clients_array[incoming_index].incoming_queue->frame_num = frame_num;

    /*Frames are transfered to next client's incoming queue's*/

    int front = clients_array[incoming_index].incoming_queue->front;
    PhysicalLayer physicalLayer = (*(PhysicalLayer *) pop(clients_array[incoming_index].incoming_queue->queue_array[front]));  /*Pops the physical layer*/
    NetworkLayer networkLayer = (*(NetworkLayer *) pop(clients_array[incoming_index].incoming_queue->queue_array[clients_array[incoming_index].incoming_queue->front])); /*pops the network layer*/

    char *intended_MAC = malloc(sizeof(char));
    for (i = 0; i < client_num; i++) {
        if (strcmp(networkLayer.receiver_IP, clients_array[i].IP) == 0) {    /*Finds the intended IP client's IP address.*/
            intended_MAC = realloc(intended_MAC, sizeof(char) * strlen(clients_array[i].MAC));
            strcpy(intended_MAC, clients_array[i].MAC);
            break;
        }
    }
    int new_clients_index = 0;

    char *next_client_MAC = find_next_neighbor(clients_array, physicalLayer.receiver_MAC, networkLayer.receiver_IP, routing_array, client_num);

    if (strcmp(next_client_MAC, "-") == 0) {    /*Drop situation*/
        printf("A message received by client %s, but intended for client %s. Forwarding...!\n", physicalLayer.receiver_MAC , intended_MAC);
        printf("Error: Unreachable destination. Packets are dropped after %d hops!\n", hop_num);

        log_entry = clients_array[incoming_index].log->log_entry;
        clients_array[incoming_index].log->log_entry++;
        clients_array[incoming_index].log->hop_number++;

        clients_array[incoming_index].log->reach = 1;
        clients_array[incoming_index].log->success[log_entry] = malloc(sizeof(char *) * strlen("No"));
        strcpy(clients_array[incoming_index].log->success[log_entry], "No");

        getTime(clients_array[incoming_index].log->log_time[log_entry]);
        clients_array[incoming_index].log->activity[log_entry] = malloc(sizeof(char) * strlen("Message Forwarded"));
        strcpy(clients_array[incoming_index].log->activity[log_entry], "Message Forwarded");

        clients_array[incoming_index].log->reach = 1;

        return;
    } else if (strcmp(physicalLayer.receiver_MAC, intended_MAC) == 0) {
        pop(clients_array[incoming_index].incoming_queue->queue_array[0]);
        ApplicationLayer applicationLayer = (*(ApplicationLayer *) pop(clients_array[incoming_index].incoming_queue->queue_array[0]));

        char *message = malloc(sizeof(char) * strlen(applicationLayer.message_chunk) * frame_num);
        strcpy(message, applicationLayer.message_chunk);

        clients_array[incoming_index].log->reach = 1;

        for (i = 1 ; i < frame_num ; i++) {
            pop(clients_array[incoming_index].incoming_queue->queue_array[i]);
            pop(clients_array[incoming_index].incoming_queue->queue_array[i]);
            pop(clients_array[incoming_index].incoming_queue->queue_array[i]);
            applicationLayer = (*(ApplicationLayer *)pop(clients_array[incoming_index].incoming_queue->queue_array[i]));
            strncat(message, applicationLayer.message_chunk, strlen(applicationLayer.message_chunk));
        }

        printf("A message received by client %s from client %s after a total number of %d hops.\n",
               applicationLayer.receiver_ID, applicationLayer.sender_ID, hop_num);

        printf("Message: %s\n", message);
        free(message);
        return;
    } else {


        char *received_client_ID = malloc(sizeof(char));
        char *intended_client_ID = malloc(sizeof(char));
        int counter;
        for (i = 0, counter = 0 ; i < client_num && counter < 2; i++) {

            if (strcmp(clients_array[i].IP, networkLayer.receiver_IP) == 0) {
                intended_client_ID = realloc(intended_client_ID, sizeof(char) * strlen(clients_array[i].client_name));
                strcpy(intended_client_ID, clients_array[i].client_name);
                counter++;
            }

            else if (strcmp(clients_array[i].MAC, physicalLayer.receiver_MAC) == 0) {
                received_client_ID = realloc(received_client_ID, sizeof(char) * strlen(clients_array[i].client_name));
                strcpy(received_client_ID, clients_array[i].client_name);
                counter++;
            }
        }

        /*Swap operation
         *Updates the new sender and receiver MAC addresses*/

        strcpy(physicalLayer.sender_MAC, physicalLayer.receiver_MAC);
        strcpy(physicalLayer.receiver_MAC, next_client_MAC);

        printf("A message received by client %s, but intended for client %s. Forwarding...\n", received_client_ID, intended_client_ID);
        for (i = 0; i < frame_num; i++) {
            printf("\tFrame #%d MAC address change: New sender MAC %s, new receiver MAC %s\n", i + 1, physicalLayer.sender_MAC, physicalLayer.receiver_MAC);
        }

        free(received_client_ID);
        free(intended_client_ID);

        /*Pushes back*/
        push(&networkLayer, clients_array[incoming_index].incoming_queue->queue_array[0]);
        push(&physicalLayer, clients_array[incoming_index].incoming_queue->queue_array[0]);

        for (i = 0; i < client_num; i++) {
            if (strcmp(next_client_MAC, clients_array[i].MAC) == 0) {
                new_clients_index = i;
                break;
            }
        }

        /*This for loop transfers the frames from incoming queue to it's outgoing queue to send next client's incoming queue*/
        clients_array[incoming_index].incoming_queue->frame_num = 0;
        clients_array[incoming_index].outgoing_queue->queue_array = (Stack **) malloc(frame_num * sizeof(Stack *));
        for (j = 0; j < frame_num; j++) { /*Transfer operation*/
            enqueue(dequeue(clients_array[incoming_index].incoming_queue), clients_array[incoming_index].outgoing_queue);  /*Transfer from X's outgoing to Y's incoming*/
            clients_array[incoming_index].incoming_queue->queue_array[j] = NULL;
            clients_array[incoming_index].incoming_queue->queue_array[j] = NULL;
            clients_array[incoming_index].incoming_queue->queue_array[j] = NULL;
            clients_array[incoming_index].incoming_queue->queue_array[j] = NULL;
            free(clients_array[incoming_index].incoming_queue->queue_array[j]);
            free(clients_array[incoming_index].incoming_queue->queue_array[j]);
            free(clients_array[incoming_index].incoming_queue->queue_array[j]);
            free(clients_array[incoming_index].incoming_queue->queue_array[j]);
        }
        clients_array[incoming_index].outgoing_queue->frame_num = frame_num;
        log_entry = clients_array[incoming_index].log->log_entry;
        clients_array[incoming_index].log->log_entry++;
        clients_array[incoming_index].log->hop_number++;

        clients_array[incoming_index].log->success[log_entry] = malloc(sizeof(char *) * strlen("Yes"));
        strcpy(clients_array[incoming_index].log->success[log_entry], "Yes");

        getTime(clients_array[incoming_index].log->log_time[log_entry]);
        clients_array[incoming_index].log->activity[log_entry] = malloc(sizeof(char) * strlen("Message Forwarded"));
        strcpy(clients_array[incoming_index].log->activity[log_entry], "Message Forwarded");

    }

    hop_num++;

    return send(clients_array[incoming_index].outgoing_queue, clients_array, routing_array, clients_array[incoming_index].MAC, clients_array[new_clients_index].IP, frame_num, client_num, hop_num);
}


/** @param: clients_array each client informations as an array
 *  @param: frame_num represents number of frame
 *  @param: client_num represents number of client
 *  @param: message represents command file's message
 *  @param: sender_ID the first client's ID
 *  @param: receiver_ID the intended client's ID
 *  @details: The function allocates the memory for each client by using for loop
 *  Some of informations are declared for each client
 *  These informations do not change
 * */
void activate_logs(Clients *clients_array, int frame_number, int client_num, char *message, char *sender_ID, char *receiver_ID) {

    int i;
    for (i = 0 ; i < client_num ; i++) {
        clients_array[i].log = malloc(sizeof(Log)); /*Memory allocation for each client*/
        clients_array[i].log->message = malloc(sizeof(char) * strlen(message)); /*Message is added to log*/
        strcpy(clients_array[i].log->message, message);

        clients_array[i].log->sender_ID = malloc(sizeof(char) * strlen(sender_ID)); /*Sender ID is added to log*/
        strcpy(clients_array[i].log->sender_ID, sender_ID);

        clients_array[i].log->receiver_ID = malloc(sizeof(char) * strlen(receiver_ID)); /*Receiver ID is added to log*/
        strcpy(clients_array[i].log->receiver_ID, receiver_ID);

        clients_array[i].log->hop_number = 0;
        clients_array[i].log->frame_number = frame_number;
        clients_array[i].log->log_entry = 0;
        clients_array[i].log->reach = 0;
        clients_array[i].log->activity = (char **) malloc(sizeof(char *));
        clients_array[i].log->log_time = (char **) malloc(sizeof(char *));
        clients_array[i].log->success = (char **) malloc(sizeof(char *));
    }
}


/** @param: command_file is the file that stores the commands
 *  @param: max_msg_size represents the size of the one message chunk
 *  @param: sender_port_number represents the sender client's port number
 *  @param: receiver_port represents the intended client's port number
 *  @param: clients_array stores the all clients informations
 *  @param: client_num represents the number of client
 *  @param: routing_array stores the routing map data
 *  @return: Nothing
 *  @details: This function reads the command file line by line and makes the operations, calls the functions
 * */
void read_command_file(FILE *command_file, int max_msg_size, const char *sender_port_number, const char *receiver_port_number, Clients* clients_array, int client_num, char **routing_array) {
    int frame_num = 0;
    char *token, ch_command_size[6], buffer[4096];
    fgets(ch_command_size, sizeof(ch_command_size), command_file);
    Client_Infos *client_infos = malloc(sizeof(Client_Infos));

    client_infos->sender_client_ID = malloc(sizeof(char));
    client_infos->receiver_client_ID = malloc(sizeof(char));
    client_infos->sender_client_MAC = malloc(sizeof(char));
    client_infos->receiver_client_MAC = malloc(sizeof(char));
    client_infos->next_client_MAC = malloc(sizeof(char));
    client_infos->next_client_name = malloc(sizeof(char));

    char *next_client = malloc(sizeof(char));
    int sender_client_index = 0;

    while (fgets(buffer, sizeof(buffer), command_file)) {
        token = strtok(buffer, " ");

        if (strcmp(token, "MESSAGE") == 0) {    /*Command matches with "MESSAGE"*/

            token = strtok(NULL, " ");
            client_infos->sender_client_ID = realloc(client_infos->sender_client_ID, sizeof(char) * strlen(token));
            strcpy(client_infos->sender_client_ID, token);

            token = strtok(NULL, " ");
            client_infos->receiver_client_ID = realloc(client_infos->receiver_client_ID, sizeof(char) * strlen(token));
            strcpy(client_infos->receiver_client_ID, token);

            int i;
            for (i = 0 ; i < client_num ; i++) {
                if (strcmp(client_infos->receiver_client_ID, clients_array[i].client_name) == 0) {
                    client_infos->receiver_client_MAC = realloc(client_infos->receiver_client_MAC, sizeof(char) * strlen(clients_array[i].MAC));
                    strcpy(client_infos->receiver_client_MAC, clients_array[i].MAC);
                    client_infos->receiver_client_index = i;
                } else if (strcmp(client_infos->sender_client_ID, clients_array[i].client_name) == 0) {
                    client_infos->sender_client_index = i;
                }
            }
            token = strtok(NULL, "#");
            printf("---------------------------------------------------------------------------------------\n");
            printf("Command: MESSAGE %s %s #%s#\n", client_infos->sender_client_ID, client_infos->receiver_client_ID, token);
            printf("---------------------------------------------------------------------------------------\n");
            printf("Message to be sent: %s\n\n", token);
            int remainder = strlen(token) % max_msg_size;
            frame_num = strlen(token) / max_msg_size;

            if (remainder != 0) {
                frame_num++;
            }

            declare_client_queues(clients_array, client_num, frame_num);

            int j;
            char **parsed_message = (char **) malloc(sizeof(char *) * frame_num);
            char *message = (char *) malloc(sizeof(char) * strlen(token));

            strcpy(message, token);
            parse_message(token, max_msg_size, parsed_message, remainder);

            i = 0;

            char *receiver_IP_add = malloc(sizeof(char));
            char *sender_IP_add = malloc(sizeof(char));

            int counter = 0;
            sender_client_index = 0;

            for (j = 0; j < client_num, counter != 2; j++) {
                if (strcmp(clients_array[j].client_name, client_infos->receiver_client_ID) == 0) {
                    receiver_IP_add = realloc(receiver_IP_add, sizeof(char) * strlen(clients_array[j].IP));
                    strcpy(receiver_IP_add, clients_array[j].IP);

                    counter++;
                }
                else if (strcmp(clients_array[j].client_name, client_infos->sender_client_ID) == 0) {
                    sender_IP_add = realloc(sender_IP_add, sizeof(char) * strlen(clients_array[j].IP));
                    strcpy(sender_IP_add, clients_array[j].IP);

                    client_infos->sender_client_MAC = realloc(client_infos->sender_client_MAC, sizeof(char) * strlen(clients_array[j].MAC));
                    strcpy(client_infos->sender_client_MAC, clients_array[j].MAC);


                    client_infos->next_client_MAC = find_next_neighbor(clients_array, client_infos->sender_client_MAC, clients_array[client_infos->receiver_client_index].IP, routing_array, client_num);
                    next_client = realloc(next_client, sizeof(char) * strlen(client_infos->next_client_MAC));
                    strcpy(next_client, client_infos->receiver_client_MAC);
                    client_infos->next_client_name = next_client_ID(clients_array, client_infos->next_client_MAC, client_num);

                    counter++;
                    sender_client_index = j;
                }
            }

            clients_array[sender_client_index].outgoing_queue -> queue_array = (Stack **) malloc(frame_num * sizeof(Stack *));

            while (i < frame_num) {
                ApplicationLayer  *applicationLayer = malloc(sizeof(ApplicationLayer));
                TransportLayer *transportLayer = malloc(sizeof(TransportLayer));
                NetworkLayer *networkLayer = malloc(sizeof(NetworkLayer));
                PhysicalLayer *physicalLayer = malloc(sizeof(PhysicalLayer));

                Stack *stack1 = malloc(sizeof(Stack *));
                stack1 -> top = -1;

                clients_array[sender_client_index].outgoing_queue -> queue_array[i] = realloc(clients_array[sender_client_index].outgoing_queue -> queue_array[i], sizeof(Stack *));

                networkLayer->sender_IP = malloc(sizeof(char) * strlen(sender_IP_add));
                strcpy(networkLayer->sender_IP, sender_IP_add);

                networkLayer->receiver_IP = malloc(sizeof(char) * strlen(receiver_IP_add));
                strcpy(networkLayer->receiver_IP, receiver_IP_add);

                physicalLayer->sender_MAC = malloc(sizeof(char) * strlen(client_infos->sender_client_MAC));
                strcpy(physicalLayer->sender_MAC, client_infos->sender_client_MAC);

                physicalLayer->receiver_MAC = malloc(sizeof(char) * strlen(client_infos->next_client_MAC));
                strcpy(physicalLayer->receiver_MAC, client_infos->next_client_MAC);

                transportLayer->sender_port_number = malloc(sizeof(char) * strlen(sender_port_number));
                strcpy(transportLayer->sender_port_number, sender_port_number);

                transportLayer->receiver_port_number = malloc(sizeof(char) * strlen(receiver_port_number));
                strcpy(transportLayer->receiver_port_number, receiver_port_number);

                applicationLayer->sender_ID = malloc(sizeof(char) * strlen(client_infos->sender_client_ID));
                strcpy(applicationLayer->sender_ID, client_infos->sender_client_ID);

                applicationLayer->receiver_ID = malloc(sizeof(char) * strlen(client_infos->receiver_client_MAC));
                strcpy(applicationLayer->receiver_ID, client_infos->receiver_client_ID);

                applicationLayer->message_chunk = malloc(sizeof(char) * strlen(parsed_message[i]));
                strcpy(applicationLayer->message_chunk, parsed_message[i]);

                push(applicationLayer, stack1);
                push(transportLayer, stack1);
                push(networkLayer, stack1);
                push(physicalLayer, stack1);

                enqueue(stack1, clients_array[sender_client_index].outgoing_queue);
                i++;
            }

            clients_array[sender_client_index].outgoing_queue->frame_num = frame_num;   /*Shows how many frames are there in the outgoing queue.*/
            print_frames(clients_array[sender_client_index].outgoing_queue, frame_num);
            activate_logs(clients_array, frame_num, client_num, message, client_infos->sender_client_ID, client_infos->receiver_client_ID);

            free(receiver_IP_add);
            free(sender_IP_add);
            printf("free\n");
            for (j = 0 ; j < frame_num ; j++) {
                free(parsed_message[j]);
            }
            free(parsed_message);
            free(message);

        }
        else if (strcmp(token, "SHOW_FRAME_INFO") == 0) {
            char client_name[2], which_queue[4], frame_number[3];
            token = strtok(NULL, " ");
            strcpy(client_name, token);


            token = strtok(NULL, " ");
            strcpy(which_queue, token);

            token = strtok(NULL, " ");
            strcpy(frame_number, token);

            int which_frame = atoi(frame_number);


            printf("---------------------------------\n");
            printf("Command: SHOW_FRAME_INFO %s %s %s\n", client_name, which_queue, frame_number);

            if (which_frame <= 0 || which_frame > frame_num) {
                printf("No such frame.\n");
            } else {
                int count = 0, i;
                for (i = 0 ; i < client_num ; i++) {
                    if (strcmp(clients_array[i].client_name, client_name) == 0) {
                        count++;
                        break;
                    }
                }
                if ( count != 0 ) {
                    which_frame -= 1;
                    int client_index = -1;
                    bool found = false;
                    for (i = 0 ; i < client_num && found != true; i++) {
                        if (strcmp(clients_array[i].client_name, client_name) == 0) {
                            client_index = i;
                            found = true;
                        }
                    }

                    if (strcmp("out", which_queue) == 0) {
                        if (isEmpty(clients_array[client_index].outgoing_queue)) {
                            printf("No such frame\n");
                        } else if (atoi(frame_number) > frame_num) {
                            printf("No such frame.\n");
                        } else {
                            printf("Current Frame #%d on the outgoing queue of client %s\n", which_frame+1, client_name);
                            printf("Carried Message: \"%s\"\n", (*(ApplicationLayer *)(clients_array[client_index].outgoing_queue -> queue_array[which_frame]) -> frame[0]).message_chunk);
                            printf("Layer 0 info: Sender ID: %s, Receiver ID: %s\n", (*(ApplicationLayer *)(clients_array[client_index].outgoing_queue -> queue_array[which_frame]) -> frame[0]).sender_ID, (*(ApplicationLayer *)(clients_array[client_index].outgoing_queue -> queue_array[which_frame]) -> frame[0]).receiver_ID);
                            printf("Layer 1 info: Sender port number: %s, Receiver port number: %s\n", (*(TransportLayer *)(clients_array[client_index].outgoing_queue -> queue_array[which_frame]) -> frame[1]).sender_port_number, (*(TransportLayer *)(clients_array[client_index].outgoing_queue -> queue_array[which_frame]) -> frame[1]).receiver_port_number);
                            printf("Layer 2 info: Sender IP address: %s, Receiver IP address: %s\n", (*(NetworkLayer *)(clients_array[client_index].outgoing_queue -> queue_array[which_frame]) -> frame[2]).sender_IP, (*(NetworkLayer *)(clients_array[client_index].outgoing_queue -> queue_array[which_frame])->frame[2]).receiver_IP);
                            printf("Layer 3 info: Sender MAC address: %s, Receiver MAC address: %s\n", (*(PhysicalLayer *)(clients_array[client_index].outgoing_queue -> queue_array[which_frame]) -> frame[3]).sender_MAC, (*(PhysicalLayer *)(clients_array[client_index].outgoing_queue -> queue_array[which_frame])->frame[3]).receiver_MAC);

                            printf("Number of hops so far: %d\n", clients_array[client_index].log->hop_number);
                        }
                    } else if (strcmp("in", which_queue) == 0) {
                        if (isEmpty(clients_array[client_index].incoming_queue)) {
                            printf("No such frame\n");
                        } else if (atoi(frame_number) > frame_num) {
                            printf("No such frame.\n");
                        } else {
                            printf("Current Frame #%d on the incoming queue of client %s\n", which_frame+1, client_name);
                            printf("Carried Message: \"%s\"\n", (*(ApplicationLayer *)(clients_array[client_index].incoming_queue -> queue_array[which_frame]) -> frame[0]).message_chunk);
                            printf("Layer 0 info: Sender ID: %s, Receiver ID: %s\n", (*(ApplicationLayer *)(clients_array[client_index].incoming_queue -> queue_array[which_frame]) -> frame[0]).sender_ID, (*(ApplicationLayer *)(clients_array[client_index].incoming_queue -> queue_array[which_frame]) -> frame[0]).receiver_ID);
                            printf("Layer 1 info: Sender port number: %s, Receiver port number: %s\n", (*(TransportLayer *)(clients_array[client_index].incoming_queue -> queue_array[which_frame]) -> frame[1]).sender_port_number, (*(TransportLayer *)(clients_array[client_index].incoming_queue -> queue_array[which_frame]) -> frame[1]).receiver_port_number);
                            printf("Layer 2 info: Sender IP address: %s, Receiver IP address: %s\n", (*(NetworkLayer *)(clients_array[client_index].incoming_queue -> queue_array[which_frame]) -> frame[2]).sender_IP, (*(NetworkLayer *)(clients_array[client_index].incoming_queue -> queue_array[which_frame])->frame[2]).receiver_IP);
                            printf("Layer 3 info: Sender MAC address: %s, Receiver MAC address: %s\n", (*(PhysicalLayer *)(clients_array[client_index].incoming_queue -> queue_array[which_frame]) -> frame[3]).sender_MAC, (*(PhysicalLayer *)(clients_array[client_index].incoming_queue -> queue_array[which_frame])->frame[3]).receiver_MAC);

                            printf("Number of hops so far: %d\n", clients_array[client_index].log->hop_number);
                        }
                    }
                } else {
                    printf("No such frame.\n");
                }

            }
        }
        else if (strcmp(token, "SHOW_Q_INFO") == 0) {
            token = strtok(NULL, " ");
            char client_id[2];
            strcpy(client_id, token);

            token = strtok(NULL, " \n");
            char which_client[4];
            strcpy(which_client, token);

            int count = 0, i;

            for (i = 0 ; i < frame_num ; i++) {
                if (strcmp(clients_array[i].client_name, client_id) == 0) {
                    count++;
                    break;
                }
            }

            printf("--------------------------\n");
            printf("Command: SHOW_Q_INFO %s %s\n", client_id, which_client);
            printf("--------------------------\n");
            if (count == 0) {
                printf("Client %s Incoming Queue Status\n", client_id);
                printf("Current total number of frame: %d\n", 0);
            } else {
                if (strcmp(which_client, "in") == 0) {
                    bool found = false;
                    int ind = 0;
                    for (i = 0 ; i < client_num ; i++) {
                        if (strcmp(client_id, clients_array[i].client_name) == 0) {
                            if (isEmpty(clients_array[i].incoming_queue) == false) {
                                printf("Client %s Incoming Queue Status\n", clients_array[i].client_name);
                                printf("Current total number of frame: %d\n", clients_array[i].incoming_queue->frame_num);
                                found = true;
                                break;
                            } else {
                                ind = i;
                                break;
                            }
                        }
                    }
                    if (found == false) {
                        printf("Client %s Incoming Queue Status\n", clients_array[ind].client_name);
                        printf("Current total number of frame: %d\n", 0);
                    }
                } else if (strcmp(which_client, "out") == 0) {
                    bool found = false;
                    int ind = 0;
                    for (i = 0; i < frame_num; i++) {
                        if (strcmp(client_id, clients_array[i].client_name) == 0) {
                            if (isEmpty(clients_array[i].outgoing_queue) == false) {
                                printf("Client %s Outgoing Queue Status\n", clients_array[i].client_name);
                                printf("Current total number of frame: %d\n", clients_array[i].outgoing_queue->frame_num);
                                found = true;
                                break;
                            } else {
                                ind = i;
                                break;
                            }
                        }
                    }
                    if (found == false) {
                        printf("Client %s Incoming Queue Status\n", clients_array[ind].client_name);
                        printf("Current total number of frame: %d\n", 0);
                    }
                }
            }
        }
        else if (strcmp(token, "SEND") == 0) {
            token = strtok(NULL, " \n");
            printf("----------------\n");
            printf("Command: SEND %s\n", token);
            printf("----------------\n");
            int i = 0;
            int sender_client_ind = 0;
            int receiver_client_ind = 0;
            int next_client_ind = 0;

            while (i < client_num) {
                if (strcmp(clients_array[i].client_name, token) == 0) {
                    sender_client_ind = i;
                } else if (strcmp(clients_array[i].client_name, client_infos->receiver_client_ID) == 0) {
                    receiver_client_ind = i;
                } else if (strcmp(clients_array[i].client_name, client_infos->next_client_name) == 0) {
                    next_client_ind = i;
                }
                i++;
            }
            char *next_client_IP = malloc(sizeof(char));

            for (i = 0 ; i < client_num ; i++) {
                if (strcmp(client_infos->next_client_MAC, clients_array[i].MAC) == 0) {
                    next_client_IP = realloc(next_client_IP, sizeof(char) * strlen(clients_array[i].IP));
                    strcpy(next_client_IP, clients_array[i].IP);
                }
            }
            int log_entry = clients_array[sender_client_ind].log->log_entry;
            clients_array[sender_client_ind].log->activity[log_entry] = malloc(sizeof(char *) * strlen("Message Forwarded"));
            strcpy(clients_array[sender_client_ind].log->activity[log_entry], "Message Forwarded");

            clients_array[sender_client_ind].log->success[log_entry] = malloc(sizeof(char *) * strlen("Yes"));
            strcpy(clients_array[sender_client_ind].log->success[log_entry], "Yes");

            clients_array[sender_client_ind].log->log_time[log_entry] = malloc(sizeof(char *) * 21);
            getTime(clients_array[sender_client_ind].log->log_time[log_entry]);

            clients_array[sender_client_ind].log->hop_number++;
            clients_array[sender_client_ind].log->log_entry++;

            int hop_num = 1;

            send(clients_array[sender_client_ind].outgoing_queue, clients_array, routing_array, clients_array[sender_client_ind].MAC, next_client_IP, frame_num, client_num, hop_num);
        }
        else if (strcmp(token, "PRINT_LOG") == 0) {
            token = strtok(NULL, " \n");

            printf("---------------------\n");
            printf("Command: PRINT_LOG %s\n", token);
            printf("---------------------\n");
            printf("Client %s Logs:\n", token);
            printf("--------------\n");
            int i;
            for (i = 0 ; i < client_num ; i++) {
                if (strcmp(token, clients_array[i].client_name) == 0) {
                    if (clients_array[i].log->log_entry != 0) {     /*Controls the is there log entry or not*/
                        int j;
                        for (j = 0 ; j < clients_array[i].log->log_entry ; j++) {
                            printf("Log Entry #%d\n", j+1);
                            printf("Timestamp: %s\n", *clients_array[i].log->log_time);
                            printf("Message: %s\n", clients_array[i].log->message);
                            printf("Number of frames: %d\n", clients_array[i].log->frame_number);
                            printf("Number of hops: %d\n", clients_array[i].log->hop_number);
                            printf("Sender ID: %s\n", clients_array[i].log->sender_ID);
                            printf("Receiver ID: %s\n", clients_array[i].log->receiver_ID);
                            printf("Activity: %s\n", clients_array[i].log->activity[j]);
                            printf("Success: %s\n", clients_array[i].log->success[j]);
                            if ( j != clients_array[i].log->log_entry -1) {
                                printf("--------------\n");
                            }
                        }
                    }
                }
            }
        }
        else {
            printf("---------------------\n");
            printf("Command : %s\n", token);
            printf("---------------------\n");
            printf("Invalid command.\n");
        }
    }

    int i;
    for (i = 0; i < client_num ; i++) {  /*Frees the client array*/
        free(clients_array[i].IP);
        free(clients_array[i].MAC);
        free(clients_array[i].client_name);
        free(clients_array[i].incoming_queue);
        free(clients_array[i].outgoing_queue);
    }

    free(client_infos->next_client_name);
    free(client_infos->receiver_client_ID);
    free(client_infos->receiver_client_MAC);
    free(client_infos->sender_client_ID);
    free(client_infos->next_client_MAC);
    free(client_infos->sender_client_MAC);
    free(client_infos);

    free(next_client);

}

/** @param: routing_file is a FILE pointer to read routing data
 *  @param: routing_array is a char pointer array that stores the routing data
 *  @details: Routing data reads with fgets function line by line.
 * */
void read_routing_data(FILE *routing_file, char **routing_array) {
    char *token, buffer[10];
    int i = 0, client = 0;

    while (fgets(buffer, sizeof(buffer), routing_file)) {
        token = strtok(buffer, " \n");
        if (strcmp(token, "-") == 0) {  /* Compares the hyphen(tire) character */
            client++;
            i = 0;
            continue;   /*Passes the hyphen character*/
        } else {
            strcpy(*(routing_array + client) + i, token);
            i++;

            token = strtok(NULL, " \n");
            strcpy(*(routing_array + client) + i, token);
            i++;
        }
    }
}

int main(int argc, char const *argv[]) {

    /* File pointers declaration*/
    FILE *clientFile;
    FILE *routingFile;
    FILE *commandFile;

    /* Files are opened*/
    clientFile = fopen(argv[1], "r");
    routingFile = fopen(argv[2], "r");
    commandFile = fopen(argv[3], "r");

    /* Integer command line arguments*/
    /* Converted strings to integers*/
    int max_msg_size = atoi(argv[4]);

    if (clientFile == NULL) {   /* Control for client data file */
        printf("Client data file could not open!\n");
        exit(EXIT_FAILURE); /* Terminates the program if cannot open the file*/
    }

    if (routingFile == NULL) {
        printf("Routing file could not open!\n");
        exit(EXIT_FAILURE);
    }

    if (commandFile == NULL) {
        printf("Command file could not open!\n");
        exit(EXIT_FAILURE);
    }

    int j;
    char client_size[5];
    fgets(client_size, sizeof(client_size), clientFile);

    Clients *clients_array = (Clients *) malloc(sizeof(Clients) * atoi(client_size));
    clients_array = read_client_data(clientFile, clients_array);

    char **routing_array = (char **) malloc(sizeof(char *) * atoi(client_size));
    for (j = 0 ; j < atoi(client_size) ; j++) {
        *(routing_array + j) = (char *) malloc(sizeof(char) * (atoi(client_size) - 1) * 2);
    }
    read_routing_data(routingFile, routing_array);

    read_command_file(commandFile, max_msg_size, argv[5], argv[6], clients_array, atoi(client_size), routing_array);

    /*Free operation*/
    fclose(clientFile);
    fclose(commandFile);
    fclose(routingFile);

    for (j = 0 ; j < atoi(client_size) ; j++) {
        free(routing_array[j]);
    }

    free(routing_array);
    free(clients_array);
    return 0;
}