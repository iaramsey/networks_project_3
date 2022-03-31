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




enum SenderState {
    readyToSend,
    waitingForAck
};

struct Sender {
    enum SenderState state; //state of the sender
    struct pkt packetBuffer[1024]; //array of packets that are waiting to be sent to B
    int bufferIndex; //value for the next new message to be added to the buffer, should be ahead of A.base
    int base; //starting value for the sender window
    int bufferSize; //number of packets currently in the queue, ++ when A receives a message from layer 5, -- when A receives an ACK from B
    float timerValue;

} A;

struct Receiver {
    int expectedSeq; //expected sequence number B is looking for
} B;









/**** A ENTITY ****/

int get_checksum(struct pkt *packet) {
    int checksum = 0;
    checksum += packet->seqnum;
    checksum += packet->acknum;
    for (int i = 0; i < packet->length; ++i)
        checksum += packet->payload[i];
    return checksum;
}

void A_init(int window_size) {
    A.bufferIndex = 0;
    A.base = 0;
    A.state = readyToSend;
    A.bufferSize = 0;
    A.timerValue = 100.0;



}

struct pkt* create_packet(struct msg *message) {
    struct pkt *packet = (struct pkt *)malloc(sizeof(struct pkt));     // 1. new packet created (use malloc)
    packet->seqnum = A.bufferIndex;     // 2. determine the sequence number
    packet->acknum = A.bufferIndex;    // 3. determine the ack number
    packet->length = message->length;  // 4. determine length of data in the packet
    memcpy(packet->payload, message->data, message->length);    // 5. use memcopy to copy data into the packet
    packet->checksum = get_checksum(packet);
//    packet->checksum = 0 ^ packet->seqnum ^ packet->acknum ^ packet->length;     // 6. calculate the checksum and set it to packet->checksum
//    for(int i = 0; i < sizeof(packet->payload); i++){
//        packet->checksum ^= packet->payload[i];
//    }
    return packet;
}

void A_output(struct msg message) {

    struct pkt packet = *create_packet(&message); //create packet from message parameter
    A.packetBuffer[A.bufferIndex] = packet; //add packet to the queue
    A.bufferIndex++; //increment buffer index
    A.bufferSize++; //increment size

    if (A.state == readyToSend) { //only send packet if sender is ready
        printf("sending packet %d from A\n", A.base);
        tolayer3_A(A.packetBuffer[A.base]); //send the packet from base
        //EVENTUALLY WILL BE SEND WINDOW HERE
        starttimer_A(A.timerValue);
        A.state = waitingForAck; //change state because now we're waitng for an ack
    }
}

void A_input(struct pkt packet) {

//    if (A.state != waitingForAck) {
//        printf("not waiting for ack\n");
//        return;
//    }
//
//    if (packet.checksum != get_checksum(&packet)) {
//        printf("  A_input: packet corrupted. drop.\n");
//        return;
//    }
//
//    if (packet.acknum != A.base) {
//        printf("  A_input: not the expected ACK. drop.\n");
//        return;
//    }
//    if (A.base == packet.acknum) {
//        printf("we did it baby, same index value %d\n", A.base);
//    }
    A.state = readyToSend;
    A.base++;
//    printf("A.base value: %d\n", A.base);
//    printf("A.bufferIndex value: %d\n", A.bufferIndex);
    if (A.base >= A.bufferIndex) {
        printf("we've gone through all of the messages in the buffer\n");
        return;
    }

    stoptimer_A();
    printf("sending packet %d from A\n", A.base);
    tolayer3_A(A.packetBuffer[A.base]);
    starttimer_A(A.timerValue);
    A.state = waitingForAck;

}

void A_timerinterrupt() {
    if (A.state != waitingForAck) {
        printf("not waiting for ack, don't care\n");
        return;
    }
//    tolayer3_A(A.last_packet);
//    stoptimer_A();
    printf("sending packet %d from A\n", A.base);
    tolayer3_A(A.packetBuffer[A.base]);
    starttimer_A(A.timerValue);
    A.state = waitingForAck;
}


/**** B ENTITY ****/

void B_init(int window_size) {
    B.expectedSeq = 0;
}


void send_ack(int ack) {
    struct pkt packet;
    packet.acknum = ack;
    packet.seqnum = ack;
    char myArray[32] = {0};
    memmove(packet.payload, myArray, 32);
    packet.length = 0;

//    packet.checksum = get_checksum(&packet);
    tolayer3_B(packet);
}


void B_input(struct pkt packet) {

//    printf("seqnum being recieved from B: %d\n", packet.seqnum);
//    if (packet.seqnum != B.expectedSeq) {
//        printf("Wrong seqnum: %d\n", packet.seqnum);
//        return;
//    }
//    if (packet.checksum != get_checksum(&packet)) {
//        printf("wrong checksum\n");
//        return;
//    }
    printf("receiving packet %d from B\n", packet.seqnum);
    send_ack(B.expectedSeq);
    B.expectedSeq++;
    struct msg message;
    message.length = packet.length;
    memmove(message.data, packet.payload, packet.length);
    tolayer5_B(message);
}

void B_timerinterrupt() { }