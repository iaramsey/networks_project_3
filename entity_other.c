/******************************************************************************/
/*                                                                            */
/* ENTITY IMPLEMENTATIONS                                                     */
/*                                                                            */
/******************************************************************************/

// Student names: Anubhav Acharya, Ramie Katan
// Student computing IDs: aa9xu, rgk6pa
//
//
// This file contains the actual code for the functions that will implement the
// reliable transport protocols enabling entity "A" to reliably send information
// to entity "B".
//
// This is where you should write your code, and you should submit a modified
// version of this file.
//
// Notes:
// - One way network delay averages five time units (longer if there are other
//   messages in the channel for GBN), but can be larger.
// - Packets can be corrupted (either the header or the data portion) or lost,
//   according to user-defined probabilities entered as command line arguments.
// - Packets will be delivered in the order in which they were sent (although
//   some can be lost).
// - You may have global state in this file, BUT THAT GLOBAL STATE MUST NOT BE
//   SHARED BETWEEN THE TWO ENTITIES' FUNCTIONS. "A" and "B" are simulating two
//   entities connected by a network, and as such they cannot access each
//   other's variables and global state. Entity "A" can access its own state,
//   and entity "B" can access its own state, but anything shared between the
//   two must be passed in a `pkt` across the simulated network. Violating this
//   requirement will result in a very low score for this project (or a 0).
//
// To run this project you should be able to compile it with something like:
//
//     $ gcc entity.c simulator.c -o myproject
//
// and then run it like:
//
//     $ ./myproject 0.0 0.0 10 500 3 8 test1.txt
//
// Of course, that will cause the channel to be perfect, so you should test
// with a less ideal channel, and you should vary the random seed. However, for
// testing it can be helpful to keep the seed constant.
//
// The simulator will write the received data on entity "B" to a file called
// `output.dat`.

#include <stdio.h>
#include <stdlib.h>
#include "simulator.h"

// custom vector implementation
// ensures memory safety
typedef struct {
    int size;
    // The capacity is a private field
    int capacity;
    struct msg *data;
    // Private field
    struct msg *head;
} Vector;

// Function prototypes
void restructure(Vector *);

// Public interaction functions
int initialize(Vector *vector) {
    vector->size = 0;
    vector->capacity = 1;
    vector->data = (struct msg*)(calloc(vector->capacity, sizeof(struct msg)));
    vector->head = vector->data;
    return 0;
}

int push(Vector *vector, struct msg *data) {
    restructure(vector);
    memcpy((struct msg *)(vector->data+vector->size), data, sizeof(struct msg));
    vector->size++;
    return 0;
}

struct msg *pop(Vector *vector) {
    if (vector->size == 0) {
        return NULL;
    }
    struct msg *retval = malloc(sizeof(struct msg)), *old = vector->data;
    memcpy(retval, old, sizeof(struct msg));
    vector->size--;
    vector->capacity--;
    vector->data = &vector->data[1];
    // decrease(vector);
    return retval;
}
struct msg *peek(Vector *vector) {
    if (vector->size == 0) {
        return NULL;
    }
    return vector->data;
}

// Private functions
void restructure(Vector *vector) {
    if (((vector->data - vector->head) == vector->size) || (vector->size == vector->capacity)) {
        int oldCapacity = vector->capacity;
        if (vector->capacity == 0)
            vector->capacity = 1;
        vector->capacity*= 2;
        struct msg *temp = (struct msg *)(malloc(vector->capacity * sizeof(struct msg)));
        memcpy(temp, vector->data, oldCapacity * sizeof(struct msg));
        free(vector->head);
        vector->head = vector->data = temp;
    }
    return;
}

/******************************************************************************/
/*                                                                            */
/*                                ENTITY A                                    */
/*                                                                            */
/******************************************************************************/

#define TIMEOUT 1000

typedef enum {
    acknowledged,
    unacknowledged,
    timed_out
} state;

int expected_seq_num = -1;
int expected_ack_num = -1;
int current_seq_num = -1;
int current_ack_num = -1;
state last_pkt_state = acknowledged;
struct pkt *last_pkt = NULL;
Vector *buffer = NULL;
int window_A;

void A_init(int window_size) {
    // No need to implement this for alternating bit protocol
    window_A = window_size;
    return;
}

struct pkt *createPacket(struct msg *message, struct pkt *last_pkt) {
    // create a new packet
    struct pkt *packet = (struct pkt *)malloc(sizeof(struct pkt));

    if (last_pkt != NULL) {
        // set the sequence number
        packet->seqnum = current_seq_num;

        // set the ack number
        packet->acknum = current_ack_num;// doesn't matter
    }
    else {
        // set the sequence number
        packet->seqnum = 0;

        // set the ack number
        packet->acknum = 0;
    }

    // set the length of data in the packet
    packet->length = message->length;

    // copy the data into the packet
    memcpy(packet->payload, message->data, message->length);

    // calculate and set the checksum
    packet->checksum = (((0 ^ packet->seqnum) ^ packet->acknum) ^ packet->length);
    for (int i = 0; i < 32; ++i) {
        packet->checksum^= packet->payload[i];
    }

    // increment the sequence number
    current_seq_num = (packet->seqnum + 1) % window_A;
    expected_seq_num = packet->seqnum;

    // update the ack number
    current_ack_num = (packet->acknum + 1) % window_A;
    expected_ack_num = packet->acknum;

    return packet;
}

void A_output(struct msg message) {
    // Initialize the output buffer
    if (buffer == NULL) {
        printf("A: First call, initializing the buffer\n");
        buffer = (Vector *)calloc(1, sizeof(Vector));
        initialize(buffer);
    }

    // If last packet has not been acknowledged yet, do not send another packet
    if ((last_pkt_state == unacknowledged) || (buffer->size != 0)) {
        printf("A: Pending acknowledgment for last packet or packet waiting in buffer. Message appended to buffer.\n");
        push(buffer, &message);
        return;
    }
    else {
        printf("A: Last packet acknowledged. Sending the message now.\n");
    }

    // Create packet from the message
    struct pkt *packet = createPacket(&message, last_pkt);

    printf("A: Sending packet with sequence number %d and ack number %d\n", packet->seqnum, packet->acknum);

    if (last_pkt != NULL) {
        free(last_pkt);
        last_pkt = NULL;
    }

    // save this packet if retransmission is needed
    last_pkt = packet;

    // set the last packet to unacknowledged
    last_pkt_state = unacknowledged;

    // start the timer
    starttimer_A(TIMEOUT);

    // send the packet
    tolayer3_A(*packet);

    printf("A: Awaiting acknowledgment for packet with sequence number %d and ack number %d\n", packet->seqnum, packet->acknum);

    return;
}

void A_input(struct pkt packet) {
    // check if the packet is corrupted
    // calculate the checksum
    int checksum = (((0 ^ packet.seqnum) ^ packet.acknum) ^ packet.length);
    for (int i = 0; i < 32; ++i) {
        checksum^= packet.payload[i];
    }
    if (packet.checksum != checksum) {
        // corrupted packet detected, exit without doing anything
        printf("A: corrupted packet detected\n");
        return;
    }

    // At this point, the packet is not corrupt and is not duplicate
    // so, we process the packet
    // Expected acknowledgement sequence number matching
    if (packet.acknum != expected_seq_num) {
        printf("A: Acknowledgment sequence mismatch. Expected %d got %d.\n", expected_seq_num, packet.acknum);
        return;
    }

    // sanity check
    if (packet.seqnum != current_seq_num) {
        printf("A: Expected sequence mismatch. Expected %d got %d.\n", current_seq_num, packet.seqnum);
        return;
    }

    // stop the timer
    stoptimer_A();

    // update the last ack number and state
    last_pkt_state = acknowledged;
    printf("A: Received acknowledgment %d\n", expected_seq_num);

    // Check the buffer and send the next packet in the buffer
    if (buffer->size != 0) {
        printf("A: Sending the next packet from the buffer\n");
        // pop the message from the buffer
        struct msg *message = pop(buffer);

        // Create packet from the message
        struct pkt *Packet = createPacket(message, last_pkt);

        printf("A: Sending packet with sequence number %d and ack number %d\n", Packet->seqnum, Packet->acknum);

        if (last_pkt != NULL) {
            free(last_pkt);
            last_pkt = NULL;
        }

        // save this packet if retransmission is needed
        last_pkt = Packet;

        // set the last packet to unacknowledged
        last_pkt_state = unacknowledged;

        // send the packet
        tolayer3_A(*Packet);

        // start the timer
        starttimer_A(TIMEOUT);

        printf("A: Waiting for acknowledgment %d\n", expected_seq_num);
    }
    return;
}

void A_timerinterrupt() {
    // If this routine gets called, then a timeout has occured.
    // A retransmission of the last packet will be done.
    // If the last packet has already been acknowledged, then do nothing.
    if (last_pkt_state == unacknowledged) {
        printf("A: Timeout detected. Retransmitting the last packet. S:- %d; A:- %d\n", last_pkt->seqnum, last_pkt->acknum);
        // send the packet again
        tolayer3_A(*last_pkt);
        // start the timer again
        starttimer_A(TIMEOUT);
    }
}

/******************************************************************************/
/*                                                                            */
/*                                ENTITY B                                    */
/*                                                                            */
/******************************************************************************/

int first = -1;
int expected_seq_num_B = 0;
struct pkt *last_successful_pkt_B = NULL;
int window_B;

void B_init(int window_size) {
    // No need to implement this for alternating bit protocol
    window_B = window_size;
    return;
}

void B_input(struct pkt packet) {
    // check if the packet is corrupted
    // calculate the checksum
    int checksum = (((0 ^ packet.seqnum) ^ packet.acknum) ^ packet.length);
    for (int i = 0; i < 32; ++i) {
        checksum^= packet.payload[i];
    }
    if (packet.checksum != checksum) {
        // corrupted packet detected, exit without doing anything
        printf("B: corrupted packet detected\n");
        return;
    }

    // check if the packet is a duplicate
    if (first == -1){
        printf("B: first packet received\n");
        first = packet.seqnum;
    }
    else if (packet.seqnum == first) {
        // duplicate data packet, retransmit last ack and exit
        tolayer3_B(*last_successful_pkt_B);
        printf("B: duplicate data packet. Resending acknowledgement for S:- %d and A:- %d\n", last_successful_pkt_B->seqnum, last_successful_pkt_B->acknum);
        return;
    }

    // At this point, the packet is not corrupt and is not duplicate
    // so, we process the packet
    // clear the previous packet
    if (last_successful_pkt_B != NULL) {
        free(last_successful_pkt_B);
        last_successful_pkt_B = NULL;
    }

    printf("B: Received packet with sequence number %d and ack number %d\n", packet.seqnum, packet.acknum);

    // sanity check to ensure packet is in the correct order
    if (packet.seqnum != expected_seq_num_B) {
        printf("B: Packet sequence mismatch. Expected %d got %d\n", expected_seq_num_B, packet.seqnum);
        return;
    }

    // construct the message to send to layer5
    struct msg* message = (struct msg*) malloc(sizeof(struct msg));
    message->length = packet.length;
    memcpy(message->data, packet.payload, packet.length);

    // send the constructed message to layer5
    tolayer5_B(*message);
    printf("B: sent message to layer5\n");
    free(message);
    message = NULL;

    // construct the ack packet
    struct pkt* ack_packet = (struct pkt*) malloc(sizeof(struct pkt));
    ack_packet->seqnum = (packet.seqnum + 1) % window_B;
    ack_packet->acknum = packet.seqnum;
    ack_packet->length = 0;
    ack_packet->checksum = (((0 ^ ack_packet->seqnum) ^ ack_packet->acknum) ^ ack_packet->length);
    for (int i = 0; i < 32; ++i) {
        ack_packet->payload[i] = 0;
        ack_packet->checksum^= 0;
    }

    // save a backup if retransmission is needed
    last_successful_pkt_B = ack_packet;

    // update the last sequence number
    first = packet.seqnum;

    // update the expected sequence number
    expected_seq_num_B = (first + 1) % window_B;

    // send the ack packet
    tolayer3_B(*ack_packet);
    printf("B: sent ack packet %d\n", ack_packet->acknum);
}

void B_timerinterrupt() {
    // Not needed for alternating bit protocol
    return;
}
