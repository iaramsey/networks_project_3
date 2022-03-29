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
// A side
int sequence_number;
int counter;
// save packet in global variable so that it can be resent with interrupt
struct pkt* last_pkt = NULL;

// B side
int expected_seqnum;


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
    packet->checksum = 0 ^ packet->seqnum ^ packet->acknum ^ packet->length;
    for(int i = 0; i < sizeof(packet->payload); i++){
        packet->checksum ^= packet->payload[i];
    }
    // 7. increment the sequence number (and modulo by SEQUENCE = 1024 to keep within 1024)
    sequence_number = (sequence_number + 1) % 2; // to alternate between 0 and 1

    last_pkt = packet;
    return packet;
}

void A_output(struct msg message) {

    // create and send packet in the output. When do you add to queue of messages? In the output
    // 1. create packet (dereference pointer to created packet)
    struct pkt packet = *create_packet(&message);
    // 2. put into queue

    // 3. send packet from queue

    // 4. send the packet to the network layer
    tolayer3_A(packet);

    // 5. start the timer
    starttimer_A(1000.0);
}

/*
 * Code for A_input should only handle:
 * 1. ACK's sent from B to A if they are corrupted or lost
 */
void A_input(struct pkt packet) {
    printf("-----------------------ack received from B, Counter: %d\n", sequence_number);

    // 1. if packet corrupted, drop it
    if(packet.checksum != (0^packet.acknum)){
        // return from A_input w/o doing anything
        return;
    }
    // 2. if it isnt corrupted, and ack is not for the packet that you sent, drop it
    else if(packet.acknum != sequence_number){
        // return from A_input w/o doing anything
        return;
    }
    // 3. If recieved properly
    else{
        // stop the timer and return
        stoptimer_A();
        return;
    }    

}

void A_timerinterrupt() {
    // go here due to timeout
    // use to resend packet
    tolayer3_A(*last_pkt);
}


/**** B ENTITY ****/

void B_init(int window_size) {
    expected_seqnum = 0;
}

void send_ack(int ack) {
    struct pkt *ack_packet = (struct pkt *)malloc(sizeof(struct pkt));
    // set ack value to passed in ack
    ack_packet->acknum = ack;
    // filler value for checksum, needs to be computed
    ack_packet->checksum = 0^ack_packet->acknum;
    tolayer3_B(*ack_packet);
    free(ack_packet);
}


void B_input(struct pkt packet) {
    //1. check if packet is valid based on checksum
    int pass_checksum = 0;
    int pass_seqnum = 0;
    int expected_checksum = 0 ^ packet.seqnum ^ packet.acknum ^ packet.length;
    for(int i = 0; i < sizeof(packet.payload); i++){
        expected_checksum ^= packet.payload[i];
    }
    if(expected_checksum == packet.checksum){
        pass_checksum = 1;
    }
    //2. check if sequence number matches expected sequence number
    expected_seqnum = (expected_seqnum + 1) % 2;;
    if(packet.seqnum == expected_seqnum){
        pass_seqnum = 1;
    }
    //3. strip message and length from packet and send to layer 5
    if(pass_checksum == 1 && pass_seqnum == 1){
        //check incoming packet for sequence 
        struct msg message;
        message.length = packet.length;
        memmove(message.data, packet.payload, packet.length);
        tolayer5_B(message);
        //4. create new packet that acknowledges current packet and send to layer 3
        send_ack(packet.seqnum); // the ack value should match the sequence number
    }
}

void B_timerinterrupt() { }
