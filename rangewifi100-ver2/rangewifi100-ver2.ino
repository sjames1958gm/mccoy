// 
// May 12 2018 version. TCP connections to host, SPI connection to UNO
//

#include <ESP8266WiFi.h>

#include <SPI.h>

#define DEBUG 1

#define RESETCMD 1
#define PINGCMD 2
#define POLLCMD 3
#define MSG 7
#define RUNCMD 20
#define GETHITDATA 21
#define ACK 0x7f
#define resetDelay 4000

//IPAddress ip(192, 168, 2, 100);
//IPAddress gateway(192, 168, 2, 254);
IPAddress ip(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 254);
IPAddress netmask(255, 255, 255, 0);

#define nop asm volatile("nop\n\t")

//const char* ssid = "ML-guest";
//const char* password = "mlguest538!";
const char *ssid = "ATTVMb9amS";
const char *password = "xmcpmjhvr7u7";

int ledPin = LED_BUILTIN;
int resetPin = D1;
int serialPollRate = 100;
int serialPollLast = millis();
WiFiServer server(9000);
WiFiClient client;
unsigned char slaveState = 0;

void setup()
{
  Serial.begin(115200);
  delay(10);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  SPI.begin();


  resetSlave();

  delay(resetDelay);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.config(ip, gateway, netmask);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this IP to connect: ");
  Serial.println(WiFi.localIP());

  String testString = "SPI Interface Initialized\n";
  SPI.transfer((char)MSG);
  debugMsgInt("sending command: ", MSG, true);
  SPI.transfer((char)testString.length());
  debugMsgInt("sending length: ", testString.length(), true);
  debugMsgStr("Sending message: ", testString, true);

  for (int i = 0; i < testString.length(); i++)
  {
    SPI.transfer(testString[i]);
  }
  delay(100);
  unsigned char cmd = SPI.transfer(0xFF);
  while (cmd == 0xff) {
     cmd = SPI.transfer(0xFF);
  }
  debugMsgInt("Rcv Command: ", cmd, false);
  delay(1);
  unsigned char len = SPI.transfer(0xFF);
  debugMsgInt("Length: ", len, false);
  while (len > 0) {
    delay(1);
    Serial.print((char)SPI.transfer(0xFF));
    len--;
  }
  Serial.println();
}

void getjobMenu() {
 Serial.println("GETJOB():  enter the item number to run");
 Serial.println("ITEM    function   Description");
 Serial.println("0. display menu");
 Serial.println("1. run exercise");
 Serial.println("2. get hit data");
 Serial.println("3. reset slave");
 Serial.println("99. getjob() EXIT");
}

void getjob(){
  int jobnumber;
  getjobMenu();
  bool done = false;
  while (!done) {
    while (!Serial.available()){
      // Poll interfaces while waiting for user input
      handleWifi();
      handleSerial();
    }
    delay(50);
    while(Serial.available())
    {
      jobnumber = Serial.parseInt();
      if (Serial.read() != '\n') { Serial.println("going to "+String(jobnumber)); }
    } 
  
    switch (jobnumber) {
      case 0:
        getjobMenu();
        break;
      case 1:
        sendRunCommand(); 
      break;
      case 2:
        getHitData(); 
        break;
      case 3:
        resetSlave();
        break;
      case 99:
        done = true;
        break;
    } // end of switch
  }
  Serial.println("DONE in getjob().");
} // end getjob()

void loop()
{
  getjob();
}

void sendRunCommand() {
  
  SPI.transfer((char)RUNCMD);
  debugMsgInt("sending command: ", RUNCMD, true);

  // No length
  SPI.transfer((char)0);

  delay(100);

  unsigned char recvCommand;
  String response = recvSerial(&recvCommand);
  debugMsgInt("Command rcv: ", (int)recvCommand, true);
  Serial.println(response);
}

void getHitData() {
  
}

//void zvibes(){
//  unsigned int testmillis;
//  Serial.println("IN zvibes() ENTER: milliseconds to test");
//  while (!Serial.available()){ 
//    handleWifi();
//  }
//  delay(50);
//  while(Serial.available())
//  {
//    testmillis = Serial.parseInt();
//    if(Serial.read() != '\n'){Serial.println("Entered testmillis = "+String(testmillis));}
//  }
//  
//  SPI.transfer((char)ZVIBES);
//  debugMsgInt("sending command: ", ZVIBES, true);
//  debugMsgInt("milliseconds: ", testmillis, true);
//  
//  SPI.transfer((char)4);
//  for (int i = 0; i < 4; i++) {
//    SPI.transfer((char)(testmillis & 0xFF));
//    testmillis  = testmillis >> 8;
//  }
//
//  delay(100);
//
//  // Wait for response - it will likely not be ZVIBES, but POLL
//  unsigned char recvCommand;
//  String response = recvSerial(&recvCommand);
//  debugMsgInt("Command rcv: ", (int)recvCommand, true);
//  Serial.println(response);
//  
//}

void handleWifi() {
  // Check if a client has connected
  if (!client.connected())
  {
    client = server.available();
    if (!client)
    {
      delay(10);
      return;
    }
    Serial.println("new client");
  }

  // Wait until the client sends some data
  if (!client.available())
  {
    return;
  }

  unsigned char cmd = readVarint(&client);
  int len = readVarint(&client);

  debugMsgInt("Command: ", cmd, false);
  debugMsgInt("Length: ", len, false);

  String request;
  if (len > 0)
  {
    request = readFor(&client, len);
  }

  if (cmd == RESETCMD)
  {
    resetSlave();
  }
  else
  {
    SPI.transfer((char)cmd);
    // TODO: this needs to be send var int
    SPI.transfer((char)request.length());
    for (int i = 0; i < request.length(); i++)
    {
      SPI.transfer(request[i]);
    }

    // Wait a short bit and receive response
    delay(10);
    
    unsigned char rcvCmd;
    String response = recvSerial(&rcvCmd);
    debugMsgStr("Rcv Command data: ", response, true);
  
    handleSerialMessage(&client, rcvCmd, response);
  }
}

void handleSerial() {
  int now = millis();
  if (now - serialPollLast > serialPollRate) {
    
    SPI.transfer((char)POLLCMD);
    
    SPI.transfer((char)0);
  
    delay(100);
    unsigned char recvCommand;
    String recvData = recvSerial(&recvCommand);
    serialPollLast = millis();
    switch (recvCommand) {
      case POLLCMD:
      {
        unsigned char newState = recvData[0];
        if (newState != slaveState) {
          Serial.println(String("slave state change, new state ") + String((int)newState));
          slaveState = newState;
        }
      }
      break;
    }
  }
}

String recvSerial(unsigned char* cmd) {
  
  *cmd = SPI.transfer(0xFF);
  while (*cmd == 0xff) {
     *cmd = SPI.transfer(0xFF);
  }
  
  debugMsgInt("Rcv Command: ", (int)*cmd, *cmd > POLLCMD);
  
  delay(1);
  unsigned char len = SPI.transfer(0xFF);
  debugMsgInt("Length: ", len, *cmd > POLLCMD);
  
  String resp = "";
  while (len > 0) {
    delay(1);
    resp += (char)SPI.transfer(0xFF);
    len--;
  }
  return resp;
}

void handleSerialMessage(WiFiClient* client, unsigned char cmd, String response) {
  client->write(cmd);
  client->write((unsigned char)response.length());
  for (int i = 0; i < response.length(); i++) {
    client->write((unsigned char)response[i]);
  }
}


int readVarint(WiFiClient *client)
{
  unsigned int value = 0;
  int len = 0;
  while (true)
  {
    while (!client->available())
    {
      delay(1);
    }
    unsigned char c = client->read();
    value = ((c & 0x7f) << (len * 7)) + value;
    if (c < 128)
      break;
  }
  debugMsgInt("Varint: ", value, false);

  return value;
}

void sendVarint(WiFiClient *client, unsigned int value)
{
  do
  {
    unsigned char c = value & 0x7f;
    value = value >> 7;
    if (value > 0)
      c = c | 0x80;
    client->write(c);
  } while (value > 0);
}

String readFor(WiFiClient *client, unsigned int len)
{
  String buff = "";
  while (len-- > 0)
  {
    while (!client->available())
    {
      delay(1);
    }
    char c = client->read();
    buff += c;
  }
  return buff;
}

void writeFor(WiFiClient *client, String buffer)
{
  int ndx = 0;
  while (ndx < buffer.length())
  {
    client->write(buffer[ndx++]);
  }
}

void resetSlave()
{
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  delay(500);
  digitalWrite(resetPin, HIGH);
}

void debugMsgInt(const char *msg, int value, bool filter)
{
  if (DEBUG && filter)
  {
    Serial.print(msg);
    Serial.println(value);
  }
}

void debugMsgStr(const char *msg, String data, bool filter)
{
  if (DEBUG && filter)
  {
    Serial.println(msg + data);
  }
}
