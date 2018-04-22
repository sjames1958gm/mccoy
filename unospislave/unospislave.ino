#include <SPI.h>

#define RCVCMD 1
#define RCVLEN 2
#define RCVDATA 3
#define RCVCOMP 4
#define SENDCMD 5
#define SENDLEN 6
#define SENDDATA 7
#define ACK 0x7F

#define DEBUG 1

char buff[1024];
volatile byte index;
volatile int state;  /* use reception complete flag */
volatile int command;
int length;
int lengthNdx;
int count;

#define PING 2
#define CMDFILE 3
#define HITLOG 4
#define DBGLOG 5
#define STATUS 6

void setup (void)
{
  Serial.begin (115200);
  SPCR |= bit(SPE);         /* Enable SPI */
  pinMode(MISO, OUTPUT);    /* Make MISO pin as OUTPUT */

  resetState();
//  count = 0;
  
  SPI.attachInterrupt();    /* Attach SPI interrupt */
  Serial.println("setup complete");
}

void loop (void)
{
  if (state == RCVCOMP)          /* Check and print received buffer if any */
  {
    buff[index] = 0;
    debugMsgInt("Command: ", command);
    debugMsgInt("Length: ", length);
    command = ACK;
    state  = SENDCMD;
    if (length > 0) {
      Serial.println(buff);
    }
  }
  delay(1);
}

// SPI interrupt routine
ISR (SPI_STC_vect)
{
  uint8_t oldsrg = SREG;
  cli();
  char c = SPDR;
  switch (state) {
    case RCVCMD:
      command = c;
      break;
    case RCVLEN:
      if (!receiveLength(c, &length, &lengthNdx)) {
        if (length > 0) {
          state = RCVDATA;
        }
        else {
          state = SENDCMD;
        }
      }
    case RCVDATA:
      if (index < sizeof(buff) - 1) {
        buff[index++] = c;
        if (index == length) {
          state = RCVCOMP;
        }
      }
      break;
    case SENDCMD:
      SPDR = ACK;
      state = SENDLEN;
      break;
    case SENDLEN:
      SPDR = 0;
      state = SENDLEN;
      resetState();
      break;
    default:
      break;
  }
  SREG = oldsrg;
}

bool receiveLength(unsigned char c, int* length, int* lengthNdx) {
  int ndx = (*lengthNdx)++;
  int l = *length;
  *length = ((c & 0x7f) << (7 * ndx)) + l;
  return c >= 128;
}

void resetState() {
  state = RCVCMD;
  index = 0;
  length = 0;
  lengthNdx = 0;
}

void debugMsgInt(const char* msg, int value) {
  if (DEBUG) {
    Serial.print(msg);
    Serial.println(value);
  }
}
