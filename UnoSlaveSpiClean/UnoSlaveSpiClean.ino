
// Code for SPI interface to 8266
#include <SPI.h>

// Controls some debug output
#define DEBUG 1
// Set to 1 to indeicate no H/W is available
#define NOHW 1

//
// =========== SPI data =================
// SPI send/recv states
#define SPI_STATE_RCVCMD 1
#define SPI_STATE_RCVLEN 2
#define SPI_STATE_RCVDATA 3
#define SPI_STATE_RCVCOMP 4
#define SPI_STATE_SENDCMD 5
#define SPI_STATE_SENDLEN 6
#define SPI_STATE_SENDDATA 7
#define SPI_STATE_WAIT 8

// SPI Variables
volatile char spi_state;
char spi_sendBuffer[256];
unsigned char spi_rcvBuffer[150];
volatile int spi_rcvIndex;
volatile int spi_length;
volatile int spi_lengthNdx;
volatile unsigned char spi_rcvCommand;

volatile unsigned char spi_sendCommand;
volatile unsigned char spi_lastSentCommand;
volatile char *spi_sendMsg;
volatile int spi_sendLength;

// Status variable to debug whether we are getting polled
volatile int spi_pollCount = 0;

// Variable to prevent simulataneous sends
bool sendOnSpi = false;

#define PINGCMD 2
#define POLLCMD 3
#define MSG 7
#define RUNCMD 20
#define HITDATA 21
#define ACK 0x7F
//
// ===========  End of SPI data =================
//

// Local Run Status
volatile char status;
#define STATUS_IDLE 1
#define STATUS_RUNNING 2
#define STATUS_RUN_COMPLETE 3

char pgmname[] = "unoSpiSlaveNoAppCode";
int loopCount = 0;
unsigned long runTimer = millis();
String hitData;

void setup() {
  Serial.begin(9600);
  Serial.println(pgmname);

  setupSpi();

  Serial.println("setup complete");

  status = STATUS_IDLE;

  }  // end setup

void loop() {
  loopCount++;
  getjob();
}  // end of loop()

// =================== Application code ================
//
void getJobMenu() {
  Serial.println("GETJOB():  enter the item number to run");
  Serial.println("ITEM    function   Description");
  Serial.println("0, refresh menu");
  Serial.println("1, get local status");
  Serial.println("2, mark end of exercise");
  Serial.println("99. getjob() EXIT");
}

void getjob(){
  int jobNumber;
  bool done = false;
  getJobMenu();

  while (!done) {
    while (!Serial.available()){ 
      monitorSpi();
      checkRunStatus();
    }
    delay(50);
    
    while(Serial.available())
    {
      jobNumber = Serial.parseInt();
      if(Serial.read() != '\n'){Serial.println("going to " + String(jobNumber));}
    } 
    switch (jobNumber) {
    case 0:
      getJobMenu();
    break;
    case 1:
      getLocalStatus();
    break;
    case 2:
      if (status == STATUS_RUNNING) status = STATUS_RUN_COMPLETE;
    case 99:
      done = true;
    break;
    } // end of switch
  }
  Serial.println("DONE in getjob().");
} // end getjob()

void getLocalStatus() {
  Serial.println("========== Local Status ===========");
  if (sendOnSpi) Serial.println("Send on SPI is true?");
  Serial.println(String("Status is ") + String((int)status));
  Serial.println(String("Poll Count is ") + String(spi_pollCount));
  Serial.println("========== Local Status ===========");
}

void checkRunStatus() {
  unsigned long now = millis();
  if ((status == STATUS_RUNNING) && ((now - runTimer) > 10000)) {
    status = STATUS_RUN_COMPLETE;
    hitData = "Random hit data from Arduino";
    sendToSpiPeer(HITDATA, hitData.c_str(), hitData.length());
    Serial.println("Run Complete");
  }
}
// ======================================================
//

//
// SPI interface code
//
//
// Application interface routines
//
//
//
void setupSpi() {
  // setup of SPI interface
  
  SPCR |= bit(SPE);      /* Enable SPI */
  pinMode(MISO, OUTPUT); /* Make MISO pin as OUTPUT (slave) */

  resetSpiState();

  SPI.attachInterrupt(); /* Attach SPI interrupt */
}

//
// check for state of RCVCOMP - meaning command has been received for application
// 
void monitorSpi() {
  // State RCVCOMP means a command was received - process it
  if (spi_state == SPI_STATE_RCVCOMP)
  {
    unsigned char locCommand = spi_rcvCommand;
        
    spi_rcvBuffer[spi_rcvIndex] = 0;
    
    debugMsgInt("Command: ", locCommand);
    debugMsgInt("Length: ", spi_length);

    if (spi_length > 0)
    {
        debugMsgStr("Message data: ", (char*)spi_rcvBuffer);
    }

    // Simply reply with POLL message ? 
    spi_sendLength = 0;
    spi_sendCommand = POLLCMD;  
    spi_state = SPI_STATE_SENDCMD;

    switch (locCommand) {
      case RUNCMD:
        Serial.println("Run command received");
        status = STATUS_RUNNING;
        runTimer = millis();
      break;
      case HITDATA:
        Serial.println("Get hit data command received");
      break;
    }
  }
  delay(1);

}

//
// Send message to peer
//
void sendToSpiPeer(unsigned char cmd, char* buffer, int len) {
  if (!sendOnSpi) {
    buffer[len] = 0;
    Serial.println(String("Send to peer: ") + buffer);
    spi_sendCommand = cmd;
    spi_sendMsg = buffer;
    spi_sendLength = len;
    sendOnSpi = true;
//    spi_state = SPI_STATE_SENDCMD;
  }
}

//
// SPI interrupt routine
// The send/receive logic goes through the following states
//
// RCVCMD -> RCVLEN -> RCVDATA(if  receive length > 0) -> RCVCOMP 
// - command receive is complete - process received command here or in application code
// SENDCMD -> SENDLEN -> SENDDATA(if send length > 0) -> WAIT
// WAIT state is to receive one last byte from master to send last byte of send command
//
ISR(SPI_STC_vect)
{
  // Save interrupt state
  uint8_t oldsrg = SREG;
  // Clear interrupts
  cli();

  // Recieve a character
  char c = SPDR;
  
  switch (spi_state)
  {
  case SPI_STATE_RCVCMD:
    spi_rcvCommand = c;
    // Out of sync if spi_rcvCommand = 0xff
    if (spi_rcvCommand == 0xff) {
      // Send NULL in response
      SPDR = 0;
      break; 
    }
      
    spi_state = SPI_STATE_RCVLEN;
    break;
  case SPI_STATE_RCVLEN:
    if (receiveLength(c, &spi_length, &spi_lengthNdx))
    {
      if (spi_length > 0)
      {
        spi_state = SPI_STATE_RCVDATA;
      }
      else
      {
        spi_state = handleCommandISR();
      }
    }
    break;
  case SPI_STATE_RCVDATA:
    if (spi_rcvIndex < sizeof(spi_rcvBuffer) - 1)
    {
      spi_rcvBuffer[spi_rcvIndex++] = c;
      if (spi_rcvIndex == spi_length)
      {
        spi_state = handleCommandISR();
      }
    }
    break;
    // If in receive complete state - send 0xFF - main loop hasn't processed last message
  case SPI_STATE_RCVCOMP:
    SPDR = 0xFF;
    break;
    
  case SPI_STATE_SENDCMD:
    SPDR = spi_sendCommand;
    spi_state = SPI_STATE_SENDLEN;
    spi_lastSentCommand = spi_sendCommand;
    break;
  case SPI_STATE_SENDLEN:
    SPDR = (unsigned char)spi_sendLength;
    if (spi_sendLength > 0)
    {
      spi_state = SPI_STATE_SENDDATA;
    }
    else
    {
      spi_state = SPI_STATE_WAIT;
      if (spi_lastSentCommand != POLLCMD) {
        sendOnSpi = false;
      }
    }
    break;
  case SPI_STATE_SENDDATA:
    SPDR = *spi_sendMsg++;
    spi_sendLength--;
    if (spi_sendLength == 0)
    {
      if (spi_lastSentCommand != POLLCMD) {
        sendOnSpi = false;
      }
      spi_state = SPI_STATE_WAIT;
    }
    break;
  case SPI_STATE_WAIT:
    // Ignore recv data as it was to trigger last SEND
    resetSpiState();
    break;
  default:
    break;
  }

  // restore interrupts
  SREG = oldsrg;
}

//
// handleCommandISR - handle complete command (called from ISR)
//
int handleCommandISR()
{
  // This function is called when the received command is complete
  SPDR = 0xFF;
  switch (spi_rcvCommand)
  {
  case PINGCMD:
    // Respond PING with PING
    spi_sendCommand = PINGCMD;
    spi_sendMsg = &status;
    spi_sendLength = 1;
    return SPI_STATE_SENDCMD;
  case POLLCMD:
    if (!sendOnSpi) {
      spi_pollCount++;
      // Send POLL response if not sending some other command
      spi_sendCommand = POLLCMD;
      spi_sendMsg = &status;
      spi_sendLength = 1;
    }
    return SPI_STATE_SENDCMD;
  default:
    // Let application code handle by transitioning to RCVCOMP
    break;
  }
  return SPI_STATE_RCVCOMP;
}

// Receive variable integer length
bool receiveLength(unsigned char c, volatile int *length, volatile int *lengthNdx)
{
  int ndx = (*lengthNdx)++;
  int l = *length;
  *length = ((c & 0x7f) << (7 * ndx)) + l;
  return c < 128;
}

// Reset state machine and receive variables
void resetSpiState()
{
  spi_state = SPI_STATE_RCVCMD;
  spi_rcvIndex = 0;
  spi_length = 0;
  spi_lengthNdx = 0;
}

void debugMsgInt(const char *msg, int value)
{
  if (DEBUG)
  {
    Serial.print(msg);
    Serial.println(value);
  }
}

void debugMsgStr(const char *msg, String value)
{
  if (DEBUG)
  {
    Serial.print(msg);
    Serial.println(value);
  }
}
