#include "../include/simulator.h"
#include "string.h"
#include "stdlib.h"

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
#define MAX_SEQ_NUM 8  // 取值范围是[0,7]
#define WINDOW_SIZE 4

int base = 0;
int nextseqnum = 0;
int unacked = 0;

struct pkt window[WINDOW_SIZE];
struct msg buffer[WINDOW_SIZE];
int last_in_buffer = -1;

int AddCheckSum(struct pkt packet){
    int sum = packet.seqnum + packet.acknum;
    for (int i = 0; i < 20; ++i) {
        sum += packet.payload[i];
    }
    return sum;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
        struct msg message;
{
    if (nextseqnum < base + WINDOW_SIZE) {
        // send packet
        struct pkt packet;
        packet.seqnum = nextseqnum;
        packet.acknum = 0;
        memset(packet.payload, '\0', 20);
        strncpy(packet.payload, message.data, 20);
        packet.checksum = AddCheckSum(packet);
        window[nextseqnum % WINDOW_SIZE] = packet;

        tolayer3(0, packet);
        unacked++;
        if (base == nextseqnum) {
            starttimer(0, 20.0f);
        }
        nextseqnum++;
    } else {
        // buffer the message
        buffer[++last_in_buffer] = message;
    }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
        struct pkt packet;
{
    if (AddCheckSum(packet) == packet.checksum && packet.acknum >= base) {
        base = packet.acknum + 1;
        unacked -= (base - window[base % WINDOW_SIZE].seqnum);
        if (unacked == 0) {
            stoptimer(0);
        } else {
            starttimer(0, 20.0f);
        }
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    for (int i = base; i < nextseqnum; ++i) {
        tolayer3(0, window[i % WINDOW_SIZE]);
    }
    starttimer(0, 20.0f);
}
/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{

}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
    if (AddCheckSum(packet) == packet.checksum && packet.seqnum >= base && packet.seqnum < base + WINDOW_SIZE) {
        // send ACK
        struct pkt ack;
        ack.acknum = packet.seqnum;
        ack.checksum = AddCheckSum(ack);
        tolayer3(1, ack);
        // deliver data
        struct msg message;
        strncpy(message.data, packet.payload, 20);
        tolayer5(1, message.data);

        // update base
        int old_base = base;
        base = packet.seqnum + 1;

        // check buffer
        while (last_in_buffer >= 0 && nextseqnum < base + WINDOW_SIZE) {
            // send packet
            struct msg msg = buffer[last_in_buffer--];
            struct pkt packet;
            packet.seqnum = nextseqnum;
            packet.acknum = 0;
            memset(packet.payload, '\0', 20);
            strncpy(packet.payload, msg.data, 20);
            packet.checksum = AddCheckSum(packet);
            window[nextseqnum % WINDOW_SIZE] = packet;

            tolayer3(0, packet);
            unacked++;
            if (nextseqnum == base) {
                starttimer(0, 20.0f);
            }
            nextseqnum++;
        }
        // stop timer if necessary
        if (base != old_base) {
            if (unacked == 0) {
                stoptimer(0);
            } else {
                starttimer(0, 20.0f);
            }
        }
    } else {
        // send duplicate ACK
        struct pkt ack;
        ack.acknum = (base > 0) ? base - 1 : 0;
        ack.checksum = AddCheckSum(ack);
        tolayer3(1, ack);
    }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    base = 0;
    nextseqnum = 0;
    unacked = 0;
    last_in_buffer = -1;
}
