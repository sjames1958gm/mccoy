
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>

#include <SPI.h>

// Enables debug print outs
#define DEBUG 1
#define DISABLE_RESET 0

// Configure IP/Gateway - fixed IP 192.168.2.101
// WiFi SSID / password

IPAddress ip(192, 168, 2, 101);
IPAddress gateway(192, 168, 2, 254);
IPAddress netmask(255, 255, 255, 0);
const char* ssid = "ML-guest";
const char* password = "mlguest538!";
//const char *ssid = "ATTVMb9amS";
//const char *password = "xmcpmjhvr7u7";

// Commands between MCU and Arduino
#define RESETCMD 1
#define PINGCMD 2
#define POLLCMD 3
#define MSG 7
#define RUNCMD 20
#define HITDATA 21
#define ACK 0x40

// Delay after resetting Arduino
#define resetDelay 4000

int resetPin = D1;
int serialPollRate = 100;
int serialPollLast = millis();
unsigned char slaveState = 0;
int pollCount = 0;
int webCount = 0;
String hitData;

ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!");
}

void handleStatus() {
  webCount++;
  String resp = "{ \"status\": \"";
  switch (slaveState) {
    case 1:
      resp += "ready\"}";
      break; 
    case 2:
      resp += "running\"}";
      break; 
    case 3:
      resp += "complete\"}";
      break; 
    default: 
      resp += "unknown\"}";
  }
  server.send(200, "application/json", resp);
}

void handleStart() {
  webCount++;
  server.send(200, "application/json", "{}");
  sendCommandWithoutData(RUNCMD, "run");
}

void handleReset() {
  webCount++;
  server.send(200, "application/json", "{}");
  resetSlave();
  hitData = "";
}

void handleGetHitData() {
  webCount++;
  if (hitData.length() == 0) {
    sendCommandWithoutData(HITDATA, "get hit data");
    server.send(200, "application/json", "{\"data\":\"\"}");
    }
  else {
    String json = "{\"data\":\"";
    json += hitData + String("\"}");
    server.send(200, "application/json", json);
  }
}

void handleNotFound() {
   server.send(404, "application/json", "{\"message\": \"Not Found\"}");
}

void setup()
{
  Serial.begin(115200);
  delay(10);
  
  SPI.begin();

  resetSlave();

  delay(resetDelay);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi .mode(WIFI_STA);
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
  
  if (MDNS.begin("esp8266")) {
    Serial1.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/start", handleStart);
  server.on("/reset", handleReset);
  server.on("/hitData", handleGetHitData);
  
  server.onNotFound(handleNotFound);
  
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

  
  server.begin();
  Serial.println("HTTP server started");
}

void getJobMenu() {
 Serial.println("GETJOB():  enter the item number to run");
 Serial.println("ITEM    function   Description");
 Serial.println("0. display menu");
 Serial.println("1. get local status");
 Serial.println("2. run exercise");
 Serial.println("3. get hit data");
 Serial.println("4. reset slave");
 Serial.println("99. EXIT");
}

// Get action to perform - if no action just monitor interfaces
void getJob(){
  int jobNumber;
  getJobMenu();
  bool done = false;
  while (!done) {
    while (!Serial.available()){
      // Poll interfaces while waiting for user input
      server.handleClient();
      handleSerial();
    }
    delay(50);
    while(Serial.available())
    {
      jobNumber = Serial.parseInt();
      if (Serial.read() != '\n') { Serial.println("going to "+String(jobNumber)); }
    } 
  
    switch (jobNumber) {
      case 0:
        getJobMenu();
        break;
      case 1:
        getLocalStatus();
        break;
      case 2:
        sendCommandWithoutData(RUNCMD, "run"); 
      break;
      case 3:
        sendCommandWithoutData(HITDATA, "get hit data");
        break;
      case 4:
        resetSlave();
        break;
      case 99:
        done = true;
        break;
    } // end of switch
  }
  Serial.println("DONE in getJob().");
}

void loop()
{
  getJob();
}

void getLocalStatus() {
  Serial.println("===================================================");
  Serial.println(String("Poll count is ") + String(pollCount));
  Serial.println(String("Peer status is ") + String(slaveState));
  Serial.println(String("Web count is ") + String(webCount));
  Serial.println(String("Local hit data is ") + hitData);
  Serial.println("===================================================");
}

void sendCommandWithoutData(unsigned char cmd, String cmdName) {
  
  SPI.transfer((char)cmd);
  debugMsgStr("sending command: ", cmdName, true);

  // No length
  SPI.transfer((char)0);

  delay(100);

  unsigned char recvCommand;
  String response = recvSerial(&recvCommand);
  debugMsgInt("Command rcv: ", (int)recvCommand, true);
  Serial.println(response);
  
  handleCommand(recvCommand, response);
}

/*
void handleWifi() {
  // Check if a client has connected
  if (!client.connected())
  {
    client = server.available();
    if (!client)
    {
      return;
    }
    Serial.println("new client");
  }

  // Return if no data is available
  if (!client.available())
  {
    return;
  }

  // Read command and length
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
    // Change this Don't send directly to Arduino
    return;
//    SPI.transfer((char)cmd);
//    // TODO: this needs to be send var int
//    SPI.transfer((char)request.length());
//    for (int i = 0; i < request.length(); i++)
//    {
//      SPI.transfer(request[i]);
//    }
//
//    // Wait a short bit and receive response
//    delay(10);
//    
//    unsigned char rcvCmd;
//    String response = recvSerial(&rcvCmd);
//    debugMsgStr("Rcv Command data: ", response, true);
//  
//    handleSerialMessage(&client, rcvCmd, response);
  }
}
*/
void handleSerial() {
  int now = millis();
  if (now - serialPollLast > serialPollRate) {
    
    SPI.transfer((char)POLLCMD);
    
    SPI.transfer((char)0);
  
    delay(2);
    unsigned char recvCommand;
    String recvData = recvSerial(&recvCommand);
    serialPollLast = millis();
    
    handleCommand(recvCommand, recvData);
  }
}

void handleCommand(unsigned char command, String& data) {
    switch (command) {
    case POLLCMD:
    {
      pollCount++;
      unsigned char newState = data[0];
      if (newState != slaveState) {
        Serial.println(String("slave state change, new state ") + String((int)newState));
        slaveState = newState;
      }
    }
    break;
    case HITDATA:
    {
      hitData = data;
      Serial.println(String("Hit Data Received:") + data);
    }
    break;
  }
}

String recvSerial(unsigned char* cmd) {  
  *cmd = SPI.transfer(0xFF);
  while (*cmd == 0xff) {
     delay(2);
     *cmd = SPI.transfer(0xFF);
  }
  
  debugMsgInt("Rcv Command: ", (int)*cmd, *cmd > POLLCMD);
  
  delay(2);
  unsigned char len = SPI.transfer(0xFF);
  debugMsgInt("Length: ", len, *cmd > POLLCMD);
  
  String resp = "";
  while (len > 0) {
    delay(2);
    resp += (char)SPI.transfer(0xFF);
    len--;
  }
  return resp;
}

/*
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
*/

void resetSlave()
{
  if (!DISABLE_RESET) {
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, LOW);
    delay(500);
    digitalWrite(resetPin, HIGH);
  }
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
