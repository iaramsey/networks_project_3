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
#include <stdlib.h>
#include "simulator.h"

/*
 * GLOBAL VARIABLES
 */
int sequence_number;
int counter;


// struct Sender {
//     int index;              // gives the sequence number of the given packet
//     struct pkt last_packet; // last packet sent 
// } A;

// struct Receiver {
//     int index;
// } B;

/**** A ENTITY ****/

void A_init(int window_size) {
    // window_size only matters for Go-Back-N protocol
    sequence_number = 0;
    counter = 0;
}

// returns pointer to a packet
struct pkt* create_packet(struct msg *message) {
    // 1. new packet created (use malloc)
    struct pkt *packet = (struct pkt *)malloc(sizeof(struct pkt));
    // 2. determine the sequence number
    packet->seqnum = sequence_number;
    // 3. determine the ack number
    packet->acknum = sequence_number;
    // 4. determine length of data in the packet
    packet->length = message->length;
    // 5. use memcopy to copy data into the packet
    memcpy(packet->payload, message->data, message->length);
    // 6. calculate the checksum and set it to packet->checksum
    packet->checksum = packet->seqnum ^ packet->acknum ^ packet->length;
    for(int i = 0; i < sizeof(packet->payload); i++){
        packet->checksum ^= packet->payload[i];
    }
    // 7. increment the sequence number (and modulo by SEQUENCE = 1024 to keep within 1024)
    sequence_number = (sequence_number + 1) % 1024;

    return packet;
}

void A_output(struct msg message) {

    // create and send packet in the output. When do you add to queue of messages? In the output
    // 1. create packet (dereference pointer to created packet)
    struct pkt packet = *create_packet(&message);
    // 2. put into queue

    // 3. send packet from queue

    /* -- old implementation
    struct pkt packet;
    packet.seqnum = A.index;
    packet.length = message.length; //is length needed??
    packet.checksum = 0;
    packet.acknum = 0;
    memmove(packet.payload, message.data, message.length);
    */

    // send the packet to the network layer
    tolayer3_A(packet);

    // start the timer
    starttimer_A(1000.0);



}

/*
 * Code for A_input should only handle:
 * 1. ACK's sent from B to A if they are corrupted or lost
 * 2. Resend packets that were corrupted or lost
 */
void A_input(struct pkt packet) {
    //A.index++; // may be redundant due to inclusion in packet struct
    printf("-----------------------ack received from B, Counter: %d\n", sequence_number);

}

void A_timerinterrupt() {

}


/**** B ENTITY ****/

void B_init(int window_size) {

}

void send_ack(int ack) {
    struct pkt ack_packet;
    // set ack value to passed in ack
    ack_packet.acknum = ack;
    // filler value for checksum, needs to be computed
    ack_packet.checksum = 0;
    tolayer3_B(ack_packet);
}


void B_input(struct pkt packet) {
    struct msg message;
    message.length = packet.length;
    memmove(message.data, packet.payload, packet.length);
    // filler value, need to determine how to compute ACK number
    send_ack(0);
    tolayer5_B(message);
}

void B_timerinterrupt() { }
