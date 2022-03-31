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
// A side - sending side
enum SenderState {
    WAIT_DATA,
    WAIT_ACK
};
enum SenderState state;
int sequence_number;
int counter; //ignore this
int total_packet_number = 0; //ignore this

struct pkt last_pkt;

// B side
int expected_seqnum;

// // A side global variables
// struct Sender {
//     int sequence_number;    // gives the sequence number of the given packet
//     // save packet in global variable so that it can be resent with interrupt
//     struct pkt* last_pkt;
// } A;

// // B side global variables
// struct Receiver {
//     int expected_seqnum;
// } B;

/**** A ENTITY ****/

void A_init(int window_size) {
    // window_size only matters for Go-Back-N protocol
    sequence_number = 0;
    //counter = 0;
    state = WAIT_DATA;

}

int get_checksum(struct pkt *packet) {
    int checksum = 0;
    checksum ^= packet->seqnum;
    checksum ^= packet->acknum;
    for (int i = 0; i < 20; ++i)
        checksum ^= packet->payload[i];
    return checksum;
}

void A_output(struct msg message) {
    printf("****ENTER A_OUTPUT****\n");
    printf("====== Total packet number is (A_output): %d ======\n", total_packet_number);
    struct pkt packet;
    if (state != WAIT_DATA) {
        printf("A_output: not yet acked. drop the message: %s\n", message.data);
        return;
    }

    packet.seqnum = sequence_number;
    packet.acknum = sequence_number;
    packet.length = message.length;
    memcpy(packet.payload, message.data, message.length);
    
    packet.checksum = get_checksum(&packet);
    
    printf("In A_output, packet message = %s", packet.payload);
    
    last_pkt = packet;
    state = WAIT_ACK;
    tolayer3_A(packet);

    starttimer_A(100.0);
    printf("****EXIT A_OUTPUT****\n");
}

/*
 * Code for A_input should only handle:
 * 1. ACK's sent from B to A if they are corrupted or lost
 */
void A_input(struct pkt packet) {
    printf("****ENTER A_INPUT****\n");
    printf("\n");
    printf("-------ack received from B, Counter: %d-------\n", sequence_number);
    
    // 1. if packet corrupted, drop it
    if(packet.checksum != packet.acknum){
        printf("CHECKSUM INDICATES CORRUPTION\n");
        // return from A_input w/o doing anything
        return;
    }
    // 2. if it isnt corrupted, and ack is not for the packet that you sent, drop it
    if(packet.acknum != sequence_number){
        printf("ACK IS NOT FOR PACKET THAT WAS SENT\n");
        // return from A_input w/o doing anything
        return;
    }
    // 3. If recieved properly
    sequence_number = (sequence_number + 1) % 2; // to alternate between 0 and 1
    // stop the timer and return
    printf("TIMER HAS BEEN STOPPED\n");
    stoptimer_A();
    printf("****EXIT A_INPUT****\n");
}

void A_timerinterrupt() {
    printf("****ENTER TIMER_INTERRUPT****\n");
    // go here due to timeout
    // use to resend packet
    printf("TIMEOUT OCURRED, RESEND LAST PACKET WITH SEQUENCE NUMBER: %d\n", last_pkt.seqnum);
    tolayer3_A(last_pkt);
    // restart the timer
    starttimer_A(100.0);
    printf("****EXIT TIMER_INTERRUPT****\n");
}


/**** B ENTITY ****/

void B_init(int window_size) {
    expected_seqnum = 0;
}

void send_ack(int ack) {
    printf("****ENTER SEND_ACK****\n");
    printf("ACK %d SENT\n", ack);
    struct pkt ack_packet;
    // set ack value to passed in ack
    ack_packet.acknum = ack;
    // filler value for checksum, needs to be computed
    ack_packet.checksum = ack_packet.acknum;
    tolayer3_B(ack_packet);
    printf("****EXIT SEND_ACK****\n");
}

void B_input(struct pkt packet) {
    printf("Enter B_input");
    //1. check if packet is valid based on checksum
    int pass_checksum = 0;
    int pass_seqnum = 0;
    int expected_checksum = packet.seqnum ^ packet.acknum ^ packet.length;
    for(int i = 0; i < packet.length; i++){
        expected_checksum ^= packet.payload[i];
    }
    if(expected_checksum == packet.checksum){
        pass_checksum = 1;
    }
    else{
        printf("B_input checksum doesn't match expected_checksum");
    }
    printf("B_input expected_checksum = %d, packet_checksum = %d\n", expected_checksum, packet.checksum);
    //2. check if sequence number matches expected sequence number
    if(packet.seqnum == expected_seqnum){
        pass_seqnum = 1;
    }
    else{
        printf("B_input seqnum doesn't match expected_seqnum");
    }
    //3. strip message and length from packet and send to layer 5
    if((pass_checksum == 1) && (pass_seqnum == 1)){
        // only increment expected_seqnum if conditions met
        expected_seqnum = (expected_seqnum + 1) % 2;
        //check incoming packet for sequence 
        struct msg message;
        message.length = packet.length;
        for(int i = 0; i < packet.length; i++){
            message.data[i] = packet.payload[i];
        }
        tolayer5_B(message);
        //4. create new packet that acknowledges current packet and send to layer 3
        send_ack(packet.seqnum); // the ack value should match the sequence number
    }
    printf("Exit B_input");
}

void B_timerinterrupt() { }
