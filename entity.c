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

/** Global variable for total sequence value **/
int totalSequenceBytes = 0;
enum SenderState {
    WAIT_LAYER5,
    WAIT_ACK
};

struct Sender {
    enum SenderState state;
    int seq;
    float estimated_rtt;
    struct pkt last_packet;
} A;

struct Receiver {
    int seq;
} B;


int get_checksum(struct pkt *packet) {
    int checksum = 0;
    checksum += packet->seqnum;
    checksum += packet->acknum;
    for (int i = 0; i < packet->length; i++)
        checksum += packet->payload[i];
    return checksum;
}

/**** A ENTITY ****/

void A_init(int window_size) {
    A.state = WAIT_LAYER5;
    A.seq = 0;
    A.estimated_rtt = 1000.0;
}


void A_output(struct msg message) {
    //Need to send
    if (A.state != WAIT_LAYER5) {
        printf("  A_output: not yet acked. drop the message: %s\n", message.data);
        return;
    }
    printf("  A_output: send packet: %s\n", message.data);
    struct pkt packet;
    packet.seqnum = A.seq;
    packet.length = message.length; //is length needed??
    packet.checksum = get_checksum(&packet);
    A.last_packet = packet;

//    packet.acknum = 0;
    A.state = WAIT_ACK;

    memmove(packet.payload, message.data, message.length);

    tolayer3_A(packet);
    starttimer_A(A.estimated_rtt);



}

void A_input(struct pkt packet) {
    if (A.state != WAIT_ACK) {
        printf("  A_input: A->B only. drop.\n");
        return;
    }
    if (packet.checksum != get_checksum(&packet)) {
        printf("  A_input: packet corrupted. drop.\n");
        return;
    }

    if (packet.acknum != A.seq) {
        printf("  A_input: not the expected ACK. drop.\n");
        return;
    }
    printf("  A_input: acked.\n");
    stoptimer_A(0);
    A.seq = 1 - A.seq;
    A.state = WAIT_LAYER5;
}

void A_timerinterrupt() {
    if (A.state != WAIT_ACK) {
        printf("  A_timerinterrupt: not waiting ACK. ignore event.\n");
        return;
    }
    printf("  A_timerinterrupt: resend last packet: %s.\n", A.last_packet.payload);
    tolayer3_A(A.last_packet);
    starttimer_A(A.estimated_rtt);

}


/**** B ENTITY ****/

void B_init(int window_size) {
    B.seq = 0;
}


void send_ack(int AorB, int ack) {
    struct pkt packet;
    packet.acknum = ack;
    packet.checksum = get_checksum(&packet);
    tolayer3_B(packet);
}

void B_input(struct pkt packet) {
    struct msg message;
//
//  if (packet.checksum != get_checksum(&packet)) {
//        printf("  B_input: packet corrupted. send NAK.\n");
//        send_ack(1, 1 - B.seq);
//        return;
//    }
//    if (packet.seqnum != B.seq) {
//        printf("  B_input: not the expected seq. send NAK.\n");
//        send_ack(1, 1 - B.seq);
//        return;
//    }
    printf("is this reached --------------------------");
    printf("  B_input: recv message: %s\n", pagitcket.payload);
    printf("  B_input: send ACK.\n");
    send_ack(1, B.seq);
    memmove(message
    .data, packet.payload, packet.length);
    message.length = packet.length;
    B.seq = 1 - B.seq;
    tolayer5_B(message);
}

void B_timerinterrupt() { }
