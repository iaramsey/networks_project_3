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


struct Sender {
    int index;
    struct pkt last_packet;
} A;

struct Receiver {
    int index;
} B;

int counter;




/**** A ENTITY ****/

void A_init(int window_size) {
    A.index = 0;
    counter = 0;
}


void A_output(struct msg message) {

    struct pkt packet;
    packet.seqnum = A.index;
    packet.length = message.length; //is length needed??
    packet.checksum = 0;
    packet.acknum = 0;
    memmove(packet.payload, message.data, message.length);

    tolayer3_A(packet);
    starttimer_A(1000.0);



}

void A_input(struct pkt packet) {
    A.index++;
    printf("-----------------------ack received from B, Counter: %d\n", A.index);

}

void A_timerinterrupt() {

}


/**** B ENTITY ****/

void B_init(int window_size) {

}


void send_ack() {
    struct pkt packet;
//    packet.payload = "ack";
    packet.length = 3;
    tolayer3_B(packet);
}


void B_input(struct pkt packet) {
    struct msg message;
    message.length = packet.length;
    memmove(message.data, packet.payload, packet.length);
    send_ack();
    tolayer5_B(message);
}

void B_timerinterrupt() { }
