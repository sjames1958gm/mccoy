#include <SPI.h>

char buff[1024];
volatile byte index;
volatile bool receivedone;  /* use reception complete flag */
int length;
int lengthNdx;
bool needLength;
bool test;
int count;

#define RESET 1
#define HELLO 2
#define CMDFILE 3
#define HITLOG 4
#define DBGLOG 5
#define STATUS 6

void setup (void)
{
  Serial.begin (115200);
  SPCR |= bit(SPE);         /* Enable SPI */
  pinMode(MISO, OUTPUT);    /* Make MISO pin as OUTPUT */

  receivedone = false;
  length = 0;
  lengthNdx = 0;
  needLength = true;
  test = true;
  count = 0;
  
  SPI.attachInterrupt();    /* Attach SPI interrupt */
  Serial.println("setup complete");
}

void loop (void)
{
//  if (!needLength && test) {
//    Serial.println(length);
//    Serial.println(lengthNdx);
//    test = false;
//  }
  if (receivedone)          /* Check and print received buffer if any */
  {
    buff[index] = 0;
    Serial.print("Command: ");
    Serial.println((int)buff[0]);
    Serial.println(&buff[1]);
    index = 0;
    receivedone = false;
    length = 0;
    lengthNdx = 0;
    needLength = true;
    count = 0;
    if ((int)buff[0] == HELLO) {
      
    }
  }
}

// SPI interrupt routine
ISR (SPI_STC_vect)
{
  uint8_t oldsrg = SREG;
  cli();
  char c = SPDR;
  if (needLength) {
    needLength = receiveLength(c, &length, &lengthNdx);
  }
  else if (index < sizeof(buff) - 1) {
    buff[index++] = c;
    if (++count == length) {
      receivedone = true;
    }
  }
  SREG = oldsrg;
}

bool receiveLength(unsigned char c, int* length, int* lengthNdx) {
  int ndx = (*lengthNdx)++;
  int l = *length;
  *length = ((c & 0x7f) << (7 * ndx)) + l;
  return c >= 128;
}


