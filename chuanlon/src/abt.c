#include "../include/simulator.h"
#include <stdlib.h>
#include <string.h>

#define MAX_STACK_SIZE 1000

int seqnum = 0;
struct pkt last_packet_sent;
int waiting_for_ack = 0;
int e = 0;
struct msg *buffer[MAX_STACK_SIZE];
int buffer_top = -1;

// Calculate the checksum of a packet
int calculate_checksum(struct pkt packet) {
    int sum = packet.seqnum + packet.acknum;
    for (int i = 0; i < 20; ++i) {
        sum += packet.payload[i];
    }
    return sum;
}

// Send a packet from A to B
void A_output(struct msg message) {
    if (waiting_for_ack || seqnum == 1) {
        buffer[++buffer_top] = malloc(sizeof(struct msg));
        memcpy(buffer[buffer_top]->data, message.data, 20);
        return;
    }
    // Construct packet with message data
    struct pkt packet;
    memset(packet.payload, '\0', 20);
    strncpy(packet.payload, message.data, 20);
    packet.seqnum = seqnum;
    packet.checksum = calculate_checksum(packet);
    // Send packet to layer 3
    last_packet_sent = packet;
    tolayer3(0, packet);
    // Start timer and wait for ACK
    waiting_for_ack = 1;
    starttimer(0, 60);
    // Flip sequence number for next packet
    seqnum = !seqnum;
}

// Receive an ACK packet at A
void A_input(struct pkt packet) {
    // Check if ACK is for the last packet we sent and if it's not corrupted
    if (packet.acknum == last_packet_sent.seqnum && calculate_checksum(packet) == packet.checksum) {
        stoptimer(0);
        waiting_for_ack = 0;
        if (buffer_top >= 0) {
            // Send next buffered message
            struct msg next_message = *buffer[0];
            free(buffer[0]);
            for (int i = 0; i < buffer_top; i++) {
                buffer[i] = buffer[i + 1];
            }
            buffer_top--;
            A_output(next_message);
        }
    }
}

// Handle timer interrupt at A
void A_timerinterrupt() {
    tolayer3(0, last_packet_sent);
    starttimer(0, 60);
}

// Receive a packet at B
void B_input(struct pkt packet) {
    // Check if packet is not corrupted
    if (calculate_checksum(packet) != packet.checksum) {
        return;
    }
    // Construct and send ACK packet
    struct pkt ack_packet;
    ack_packet.acknum = packet.seqnum;
    ack_packet.checksum = ack_packet.acknum;
    tolayer3(1, ack_packet);
    if (packet.seqnum == e) {
        // Send message to layer 5 and update expected sequence number
        tolayer5(1, packet.payload);
        e = !e;
    }
}

void A_init() {}

void B_init() {}
