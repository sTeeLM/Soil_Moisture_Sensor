#include <STC8G.H>

#include "bt.h"
#include "debug.h"

#include "debug.h"
#include "bt.h"
#include "cext.h"
#include "delay.h"


#define    OLEN  32                             // size of serial transmission buffer
static unsigned   char  ostart;                 // transmission buffer start index
static unsigned   char  oend;                   // transmission buffer end index
static char idata outbuf[OLEN];                 // storage for transmission buffer

#define    ILEN  32                              // size of serial receiving buffer
static unsigned   char  istart;                 // receiving buffer start index
static unsigned   char  iend;                   // receiving buffer end index
static char idata inbuf[ILEN];                  // storage for receiving buffer

static bit sendfull;                                   // flag: marks transmit buffer full
static bit sendactive;                                 // flag: marks transmitter active

/*--------------------------------------------------------------------------------
 * Serial Interrupt Service Routine
 *------------------------------------------------------------------------------*/
static void bt_isr (void) interrupt 8 using 2
{
char c;

  //----- Received data interrupt. -----
  if (S2CON & 0x1) {
    c = S2BUF;                                   // read character
    S2CON &= ~0x1;                               // clear interrupt request flag
    if (istart + ILEN != iend) {
      inbuf[iend++ & (ILEN-1)] = c;             // put character into buffer
    }
  }

  //----- Transmitted data interrupt. -----
  if ((S2CON & 0x2) != 0) {
    S2CON &= ~0x2;                                     // clear interrupt request flag
    if (ostart != oend) {                       // if characters in buffer and
      S2BUF = outbuf[ostart++ & (OLEN-1)];       // transmit character
      sendfull = 0;                             // clear 'sendfull' flag
    }
    else {                                      // if all characters transmitted
      sendactive = 0;                           // clear 'sendactive'
    }
  }
}



void bt_initialize(void)
{
  CDBG("bt init");
}

