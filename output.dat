/******************************************************************************/
/*                                                                            */
/* ENTITY IMPLEMENTATIONS                                                     */
/*                                                                            */
/******************************************************************************/

// Student names: James Weeden, Iain Ramsey
// Student computing IDs: jmw7dc, iar5vt
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
#include <string.h>
#include "simulator.h"


enum SenderState {
    WAIT_LAYER5,
    WAIT_ACK
};

struct Sender {
    enum SenderState state;
    int index;
    float estimated_rtt;
    struct pkt last_packet;
} A;

struct Receiver {
    int index;
} B;






/**** A ENTITY ****/

void A_init(int window_size) {
    A.index = 0;
}


int get_checksum(struct pkt *packet) {
    int checksum = 0;
    checksum += packet->seqnum;
    checksum += packet->acknum;
    for (int i = 0; i < packet->length; i++)
        checksum += packet->payload[i];
    return checksum;
}

void A_output(struct msg message) {
    struct pkt packet;
    //seqnum and acknum are the same when the packet is sent from the sender
    packet.seqnum = A.index;
    packet.acknum = A.index;
    packet.length = message.length;
    memmove(packet.payload, message.data, message.length);
    packet.checksum = get_checksum(&packet);
    tolayer3_A(packet);
    starttimer_A(1000.0);
}

void A_input(struct pkt packet) {
//    if (A.state == WAIT_LAYER5) {
//        printf("currently waiting from layer5 input form A\n");
//        return;
//    }
//    //need to check for incorrect checksum
//
//    //need to check for seqnum being wrong
//    if (A.index != packet.acknum){
//        printf("incorrect acknum\n");
//        return;
//    }
//
//    printf("-----------------------ack succesfully received");

    printf("-----------------A.index: %d, B.index: %d\n", A.index, B.index);
    if (A.index == packet.acknum) {
        printf("we did it baby, same index value %d\n", A.index);
    }
    A.state = WAIT_LAYER5;
    A.index++;
    stoptimer_A();

}

void A_timerinterrupt() {

}


/**** B ENTITY ****/

void B_init(int window_size) {
    B.index = 0;
}


void send_ack(int ack) {
    struct pkt packet;
    packet.acknum = ack;
    packet.seqnum = 0;
    char myArray[32] = {0};
    memmove(packet.payload, myArray, 32);
    packet.length = 0;

//    packet.checksum = get_checksum(&packet);
    tolayer3_B(packet);
}


void B_input(struct pkt packet) {
//    if (packet.checksum != get_checksum(&packet)) {
//        printf("  B_input: packet corrupted. send NAK.\n");
//        send_ack(1 - B.index);
//        return;
//    }
//    if (packet.seqnum != B.index) {
//        printf("  B_input: not the expected seq. send NAK.\n");
//        send_ack(1 - B.index);
//        return;
//    }
//    printf("  B_input: recv message: %s\n", packet.payload);
//    printf("  B_input: send ACK.\n");

//    printf("------------------B.index: %d\n", B.index);
    send_ack(B.index);
    B.index++;
    struct msg message;
    message.length = packet.length;
    memmove(message.data, packet.payload, packet.length);
    tolayer5_B(message);
}

void B_timerinterrupt() { }