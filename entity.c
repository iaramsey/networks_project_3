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
    readyToSend,
    waitingForAck
};

struct Sender {
    enum SenderState state;
    int index;
    float estimated_rtt;
    struct pkt last_packet;
} A;

struct Receiver {
    int expectedSeq;

} B;


//GLOBAL VARIABLES FOR SENDER, A
struct pkt packetBuffer[1000]; //array of messages that are added from layer 5 (aka test1.txt)
int bufferIndex;
struct pkt window[3]; //array of packets representing the sender's window
int base; //index for packetBuffer[] for the first message in the window to be sent, i.e. start of the window
int sequence_number;




/**** A ENTITY ****/

void A_init(int window_size) {
    A.index = 0;
    bufferIndex = 0;
    base = 0;
    sequence_number = 0;


}

struct pkt* create_packet(struct msg *message) {
    // 1. new packet created (use malloc)
    struct pkt *packet = (struct pkt *)malloc(sizeof(struct pkt));
    // 2. determine the sequence number
    packet->seqnum = bufferIndex;
    // 3. determine the ack number
    packet->acknum = bufferIndex;
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

//    last_pkt = packet;
//    total_packet_number++;
    return packet;
}

void A_output(struct msg message) {
    //sendWindow(base, windowsize)
//    printf("message.data: %s\n", message.data);
    struct pkt packet = *create_packet(&message);

    packetBuffer[bufferIndex] = packet;

//    printf("Here's what's in the buffer at index %d: %s\n", bufferIndex, packetBuffer[bufferIndex].payload);
    bufferIndex++;

    if (A.state == readyToSend) {
        printf("sending packet %d from A\n", base);
//        printf("packet payload: %s\n", packet.payload);
        tolayer3_A(packetBuffer[base]);
        starttimer_A(100.0);
        A.state = waitingForAck;
        A.last_packet = packet;
    }
}




void A_input(struct pkt packet) {


//    printf("A.index: %d, B.acknum: %d\n", A.index, packet.acknum);
    if (base == packet.acknum) {
        printf("we did it baby, same index value %d\n", base);
    }
    A.state = readyToSend;
    base++;
//    printf("base value: %d\n", base);
    printf("bufferIndex value: %d\n", bufferIndex);
    if (base >= bufferIndex) {
        printf("we've gone through all of the messages in the buffer\n");
        return;
    }
    stoptimer_A();
    printf("sending packet %d from A\n", base);
    tolayer3_A(packetBuffer[base]);
    starttimer_A(1000.0);
    A.state = waitingForAck;

}

void A_timerinterrupt() {
    tolayer3_A(A.last_packet);
}


/**** B ENTITY ****/

void B_init(int window_size) {
    B.expectedSeq = 0;
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

//    printf("seqnum being recieved from B: %d\n", packet.seqnum);
//    if (packet.seqnum != B.index) {
//        printf("Wrong seqnum\n");
//        return;
//    }
    if (packet.seqnum != B.expectedSeq)
        return;
    printf("receiving packet %d from B\n", packet.seqnum);
    send_ack(B.expectedSeq);
    B.expectedSeq++;
    struct msg message;
    message.length = packet.length;
    memmove(message.data, packet.payload, packet.length);
    tolayer5_B(message);
}

void B_timerinterrupt() { }