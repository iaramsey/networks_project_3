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
    waitingForAck,
    waitingForLayer5
};

struct Sender {
    enum SenderState state; //state of the sender
    struct pkt packetBuffer[1024]; //array of packets that are waiting to be sent to B
    int bufferIndex; //value for the next new message to be added to the buffer, should be ahead of A.base
    int base; //starting value for the sender window
    int bufferSize; //number of packets currently in the queue, ++ when A receives a message from layer 5, -- when A receives an ACK from B
    float timerValue;

} A;

int indicator;

struct Receiver {
    int expectedSeq; //expected sequence number B is looking for
    struct pkt ack_packet;
    char zeroArray[32];
} B;









/**** A ENTITY ****/

int get_checksum(struct pkt *packet) {
    int checksum = 0;
    checksum += packet->seqnum;
    checksum += packet->acknum;
    if (packet->length > 32)
        packet->length = 32;
    for (int i = 0; i < packet->length; ++i)
        checksum += packet->payload[i];
    return checksum;
//    return 7;
}

void A_init(int window_size) {
    A.bufferIndex = 1;
    A.base = 1;
    A.state = readyToSend;
    A.bufferSize = 0;
    A.timerValue = 1000.0;
    indicator = 0;



}



struct pkt* create_packet(struct msg *message, int seqnum, int acknum) {
    struct pkt *packet = (struct pkt *)malloc(sizeof(struct pkt));     // 1. new packet created (use malloc)
    packet->seqnum = seqnum;     // 2. determine the sequence number
    packet->acknum = acknum;    // 3. determine the ack number
    packet->length = message->length;  // 4. determine length of data in the packet
    memcpy(packet->payload, message->data, message->length);    // 5. use memcopy to copy data into the packet
    packet->checksum = get_checksum(packet);
//    packet->checksum = 0 ^ packet->seqnum ^ packet->acknum ^ packet->length;     // 6. calculate the checksum and set it to packet->checksum
//    for(int i = 0; i < sizeof(packet->payload); i++){
//        packet->checksum ^= packet->payload[i];
//    }
    return packet;
}

struct pkt* create_ack_packet(int acknum) {
    struct pkt *packet = (struct pkt *)malloc(sizeof(struct pkt));     // 1. new packet created (use malloc)
    packet->seqnum = 0;     // 2. determine the sequence number
    packet->acknum = acknum;    // 3. determine the ack number
    packet->length = 32;  // 4. determine length of data in the packet
    memcpy(packet->payload, B.zeroArray, 32);    // 5. use memcopy to copy data into the packet
    packet->checksum = get_checksum(packet);
    return packet;
}


void A_output(struct msg message) {

    struct pkt packet = *create_packet(&message, A.bufferIndex, 1); //create packet from message parameter
    if (A.bufferIndex == A.base) {
        printf("A_output: sending packet %d from A with checksum %d\n", packet.seqnum, packet.checksum);
        tolayer3_A(packet); //send the packet from base
        //EVENTUALLY WILL BE SEND WINDOW HERE
        starttimer_A(A.timerValue);
        A.state = waitingForAck; //change state because now we're waitng for an ack
    }

    A.packetBuffer[A.bufferIndex] = packet; //add packet to the queue


    A.bufferIndex++; //increment buffer index
    A.bufferSize++; //increment size
//    struct pkt packet;
//    packet.seqnum = A.bufferIndex;
//    packet.acknum = A.bufferIndex;
//    packet.length = message.length;
//    memcpy(packet.payload, message.data, message.length);
//    packet.checksum = get_checksum(&packet);






//    if (A.state == readyToSend) { //only send packet if sender is ready
//        printf("A_output: sending packet %d from A with checksum %d\n", A.packetBuffer[A.base].seqnum, A.packetBuffer[A.base].checksum);
//
//        tolayer3_A(A.packetBuffer[A.base]); //send the packet from base
//        //EVENTUALLY WILL BE SEND WINDOW HERE
//        starttimer_A(A.timerValue);
//        A.state = waitingForAck; //change state because now we're waitng for an ack
//    }
}

void A_input(struct pkt packet) {

    printf("receiving ack from B with acknum: %d and checksum %d\n", packet.acknum, packet.checksum);
    printf("ack.payload: %s\n", packet.payload);
    //
//    printf("packet checksum: %d\n", get_checksum(&packet));
//packet.checksum == get_checksum(&packet) && GET_CHECKSUM IS CAUSING A SEG FAULT
//    printf("%d %d %s\n", packet.seqnum, packet.acknum, packet.payload);
    if (packet.checksum == get_checksum(&packet) && packet.acknum == A.base) {
        printf("ack succesfully received, increment base\n");
        A.base++;
        printf("-------------------------------------\n");
        printf("A.base: %d, A.bufferIndex: %d\n", A.base, A.bufferIndex);
        if (A.base >= A.bufferIndex)
            stoptimer_A();
        else
        starttimer_A(A.timerValue);
    }
    else
        printf("ack packet is corrupted\n");


}

void A_timerinterrupt() {
    if (A.state != waitingForAck) {
        printf("not waiting for ack, don't care about timer expiring\n");
        return;
    }
//    tolayer3_A(A.last_packet);
//    stoptimer_A();
    printf("packet dropped resending packet %d from A with seqnum %d and checksum %d\n", A.packetBuffer[A.base].seqnum, A.packetBuffer[A.base].seqnum, A.packetBuffer[A.base].checksum);

    tolayer3_A(A.packetBuffer[A.base]);
    starttimer_A(A.timerValue);
    A.state = waitingForAck;

//    exit(1);
}


/**** B ENTITY ****/

void B_init(int window_size) {
    B.expectedSeq = 1;
    B.ack_packet = *create_ack_packet(0);
}




void B_input(struct pkt packet) {
    if (packet.checksum == get_checksum(&packet) && packet.seqnum == B.expectedSeq) {
        printf("B received right packet and is incrementing expected value\n");
        struct msg message;
        message.length = packet.length;
        printf("is it reaching this\n");
        memcpy(message.data, packet.payload, packet.length); //THIS IS CAUSING A SEG FAULT
        tolayer5_B(message);
        B.ack_packet.acknum = B.expectedSeq;
        B.ack_packet.checksum = get_checksum(&B.ack_packet);
        B.expectedSeq++;
    }
    else
        printf("B didn't receive the desired packet. ExpectedSeq: %d, received seqnum: %d\n", B.expectedSeq, packet.seqnum);
    printf("B is sending ack packet with acknum: %d and checksum: %d\n", B.ack_packet.acknum, B.ack_packet.checksum);
    printf("B.ack_packet.payload: %s\n", B.ack_packet.payload);
    tolayer3_B(B.ack_packet);
//    exit(1);
}

void B_timerinterrupt() { }