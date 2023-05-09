#include "../include/simulator.h"
#include "string.h"
#include "stdlib.h"
#include <stdio.h>



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

/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/
#define MAX_STACK_SIZE 1000


int s = 1;
struct pkt H_packet;
int rev = 1;
int e = 0;

int top = -1;
struct msg** buf;

int isEmpty() {
    return top == -1;
}

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
    if (!rev){
//        push(message.data);
        memset(buf[++top]->data, '\0', 20);
        memcpy(buf[top]->data, message.data, 20);
        return;
    }
        struct pkt p;
        memset(p.payload, '\0', 20);
        strncpy(p.payload, message.data,20);
        p.seqnum = !s;
        p.checksum = AddCheckSum(p);

        s = p.seqnum;
        H_packet = p;

        starttimer(0,20.0f);
        tolayer3(0,H_packet);
        rev = 0;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
    int checksum = AddCheckSum(packet);
    if (checksum != packet.checksum || packet.acknum != H_packet.seqnum) return;
    stoptimer(0);
    rev = 1;
    if (!isEmpty()){
        struct pkt p;
        p.seqnum = !s;
        p.acknum = 0;
        memset(p.payload, '\0', 20);
        strncpy(p.payload, buf[top]->data,20);
        p.checksum = AddCheckSum(p);

        s = p.seqnum;
        H_packet = p;

        starttimer(0,20.0f);
        tolayer3(0,H_packet);
        rev = 0;
        top--;
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    //处理超时
    starttimer(0,20.0f);
    tolayer3(0,H_packet);

}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    buf = malloc(MAX_STACK_SIZE * sizeof (struct msg*));
    for (int i = 0; i < MAX_STACK_SIZE; ++i) {
        buf[i] = malloc(sizeof (struct msg));
    }
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
    int checksum = AddCheckSum(packet);
    if (checksum != packet.checksum) return;

    if(checksum == packet.checksum){
        struct pkt ack;
        ack.acknum = packet.seqnum;
        ack.checksum = ack.acknum;
        if (e == packet.seqnum){
            tolayer5(1,packet.payload);
            e = !e;
        }
        tolayer3(1, ack);
    }
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{

}
