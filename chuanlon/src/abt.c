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

/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/
int ackNum = 0;
struct pkt H_packet;
int rev = 0;
int seq = 0;
char * buffer;

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
        strncpy(H_packet.payload, message.data,20);
        strncpy(buffer,message.data,20);
        H_packet.acknum = ackNum;
        H_packet.seqnum = seq;
        H_packet.checksum = AddCheckSum(H_packet);
        starttimer(0,5);
        tolayer3(0,H_packet);
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
    stoptimer(1);
    if (packet.acknum != ackNum){

    }
    if (ackNum == 0) ackNum = 1;
    else ackNum = 0;

}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    //处理超时
    tolayer3(0,H_packet);


}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    ackNum = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
    stoptimer(0);
    int checksum = AddCheckSum(packet);
    if(checksum == packet.checksum){
        starttimer(1,5);
        tolayer3(1,H_packet);
        tolayer5(1,packet.payload);
    }
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{

}
