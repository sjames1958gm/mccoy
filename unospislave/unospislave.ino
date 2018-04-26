#include <SPI.h>

#define RCVCMD 1
#define RCVLEN 2
#define RCVDATA 3
#define RCVCOMP 4
#define SENDCMD 5
#define SENDLEN 6
#define SENDDATA 7
#define WAIT 8

#define STATUS_OK 1
#define STATUS_READY 2
#define STATUS_RUNNING 3
#define STATUS_DONE 4

#define DEBUG 1

char buff[1024];
volatile int index;
volatile int state;
volatile int command;
int sendCommand;
int length;
int lengthNdx;
int status;

char *outMsg;
int outLength;
String pingRsp = "pong";

int counter = 0;

#define PING 2
#define CMDFILE 3
#define HITLOG 4
#define DBGLOG 5
#define STATUS 6
#define ACK 0x7F

#define LED 13

void setup(void)
{
  Serial.begin(115200);
  Serial.println("setup start");
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  SPCR |= bit(SPE);      /* Enable SPI */
  pinMode(MISO, OUTPUT); /* Make MISO pin as OUTPUT (slave) */

  resetState();

  SPI.attachInterrupt(); /* Attach SPI interrupt */
  Serial.println("setup complete");
}

void loop(void)
{
  if (state == RCVCOMP) /* Check and print received buffer if any */
  {
    state = SENDCMD;

    buff[index] = 0;
    debugMsgInt("Command: ", command);
    debugMsgInt("Length: ", length);

    if (length > 0)
    {
      Serial.println(buff);
    }

    if (command == PING)
    {
      digitalWrite(LED, (counter++ % 2) == 0 ? HIGH : LOW);
    }
  }
  delay(1);
}

// SPI interrupt routine
ISR(SPI_STC_vect)
{
  uint8_t oldsrg = SREG;
  cli();
  char c = SPDR;
  switch (state)
  {
  case RCVCMD:
    command = c;
    // Out of sync if command = 0xff
    if (command == 0xff)
      break;
    state = RCVLEN;
    break;
  case RCVLEN:
    if (receiveLength(c, &length, &lengthNdx))
    {
      if (length > 0)
      {
        state = RCVDATA;
      }
      else
      {
        state = RCVCOMP;
        handleCommandISR();
      }
    }
    break;
  case RCVDATA:
    if (index < sizeof(buff) - 1)
    {
      buff[index++] = c;
      if (index == length)
      {
        state = RCVCOMP;
      }
    }
    break;
  case SENDCMD:
    SPDR = sendCommand;
    state = SENDLEN;
    break;
  case SENDLEN:
    SPDR = outLength;
    if (outLength > 0)
    {
      state = SENDDATA;
    }
    else
    {
      state = WAIT;
    }
    break;
  case SENDDATA:
    SPDR = *outMsg++;
    outLength--;
    if (outLength == 0)
    {
      state = WAIT;
    }
    break;
  case WAIT:
    // Ignore recv data as it was to trigger SENDLEN
    resetState();
    break;
  default:
    break;
  }
  SREG = oldsrg;
}

void handleCommandISR()
{
  switch (command)
  {
  case PING:
    sendCommand = PING;
    outLength = pingRsp.length();
    outMsg = pingRsp.c_str();
    state = SENDCMD;
    break;
  default:
    // Let loop() handle
    break;
  }
}

bool receiveLength(unsigned char c, int *length, int *lengthNdx)
{
  int ndx = (*lengthNdx)++;
  int l = *length;
  *length = ((c & 0x7f) << (7 * ndx)) + l;
  return c < 128;
}

void resetState()
{
  state = RCVCMD;
  index = 0;
  length = 0;
  lengthNdx = 0;
}

void debugMsgInt(const char *msg, int value)
{
  if (DEBUG)
  {
    Serial.print(msg);
    Serial.println(value);
  }
}
