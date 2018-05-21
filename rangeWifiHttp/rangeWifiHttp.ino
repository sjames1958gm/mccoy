//
// ESP8266 Web Server with SPI interface to Arduino
//
#include <ESP8266WebServer.h> // https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer
#include <ESP8266mDNS.h> // https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266mDNS
#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi

#include <SPI.h> // https://github.com/esp8266/Arduino/tree/master/libraries/SPI

// Enables debug print outs
#define DEBUG 1
// Set to 1 to disable reset logic from NodeMCU (ESP8266 board)
#define DISABLE_RESET 0

//
// Configure IP/Gateway - fixed IP 192.168.1.100
//
IPAddress ip(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 254);
IPAddress netmask(255, 255, 255, 0);
//
// WiFi SSID / password
//
//const char* ssid = "ML-guest";
//const char* password = "mlguest538!";
const char *ssid = "ATTVMb9amS";
const char *password = "xmcpmjhvr7u7";

// Commands between NodeMCU and Arduino SPI slave
#define RESETCMD 1
#define PINGCMD 2
#define POLLCMD 3
#define MSG 7
#define RUNCMD 20
#define HITDATA 21
#define ACK 0x40

// Wait this delay (msec) after resetting arduino
#define resetDelay 4000
// This is the 
int resetPin = D1;
// Poll arduino only every serialPollRate msec
int serialPollRate = 100;
int serialPollLast = millis();

// State of the arduino
// 1 - ready, 2 - running, 3 - run complete
#define READY_STATE 1
#define RUNNING_STATE 2
#define RUN_COMPLETE_STATE 3
unsigned char slaveState = 0;

// Status counters - used to debug connection status
int pollCount = 0;
int webCount = 0;
// Last value of hitData received.
String hitData;

// Create the Web Server listening on port 80 (http)
ESP8266WebServer server(80);

// HTTP route handlers (see setup for mapping from URL to function
//
// Root route - http://<address>/
void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!");
}

// Status route - http://<address>/status
void handleStatus() {
  webCount++;
  String resp = "{ \"status\": \"";
  switch (slaveState) {
    case READY_STATE:
      resp += "ready\"}";
      break; 
    case RUNNING_STATE:
      resp += "running\"}";
      break; 
    case 3=RUN_COMPLETE_STATE:
      resp += "complete\"}";
      break; 
    default: 
      resp += "unknown\"}";
  }
  server.send(200, "application/json", resp);
}

// Start (run) route - http://<address>/start
void handleStart() {
  webCount++;
  server.send(200, "application/json", "{}");
  // Send RUN command to arduino
  sendCommandWithoutData(RUNCMD, "run");
}

// Reset route - http://<address>/reset
void handleReset() {
  webCount++;
  server.send(200, "application/json", "{}");
  // reset arduino device and clear current hitData
  resetSlave();
  hitData = "";
}

// Get hit data route - http://<address>/hitData
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

// Default route for all other routes
void handleNotFound() {
   server.send(404, "application/json", "{\"message\": \"Not Found\"}");
}

void setup()
{
  Serial.begin(115200);
  delay(10);

  // Initialize SPI  
  SPI.begin();

  // Reset the slave
  resetSlave();

  // Wait for configured delay to allow arduino to reset
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

  // Start the web (port 80) server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this IP to connect: ");
  Serial.println(WiFi.localIP());

  // Start DNS (not sure if this is needed)  
  if (MDNS.begin("esp8266")) {
    Serial1.println("MDNS responder started");
  }

  // Setup up the URL routing to handler functions above
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/start", handleStart);
  server.on("/reset", handleReset);
  server.on("/hitData", handleGetHitData);
  
  server.onNotFound(handleNotFound);
  
  // Send test string to arduino (put this is a function?)
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
  Serial.println("");

  Serial.println("Setup complete");
}

// Simple menu for various commands
void getJobMenu() {
 Serial.println("GETJOB():  enter the item number to run");
 Serial.println("ITEM    function   Description");
 Serial.println("0. display menu");
 Serial.println("1. get local status");
 Serial.println("2. run exercise");
 Serial.println("3. get hit data");
 Serial.println("4. reset arduino");
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

// Dump some local data for debugging
void getLocalStatus() {
  Serial.println("===================================================");
  Serial.println(String("Poll count is ") + String(pollCount));
  Serial.println(String("Peer status is ") + String(slaveState));
  Serial.println(String("Web count is ") + String(webCount));
  Serial.println(String("Local hit data is ") + hitData);
  Serial.println("===================================================");
}

// Send a simple command with no data (cmdName is for debug output)
void sendCommandWithoutData(unsigned char cmd, String cmdName) {
  
  SPI.transfer((char)cmd);
  debugMsgStr("sending command: ", cmdName, true);

  // No length
  SPI.transfer((char)0);

  delay(100);

  // Receive and process any response
  unsigned char recvCommand;
  String response = recvSerial(&recvCommand);
  debugMsgInt("Command rcv: ", (int)recvCommand, true);
  Serial.println(response);
  
  handleCommand(recvCommand, response);
}

// Poll the serial port if serialPollRate time has passed
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

// Handle command received from arduino
void handleCommand(unsigned char command, String& data) {
    switch (command) {
    case POLLCMD:
    {
      pollCount++;
      unsigned char newState = data[0];
      if (newState != slaveState) {
        Serial.println(String("arduino state change, new state ") + String((int)newState));
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

// Receive message from the arduino (in response to command (POLL, etc) from nodemcu)
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

// Simply toggle the reset pin to reset the arduino
void resetSlave()
{
  if (!DISABLE_RESET) {
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, LOW);
    delay(500);
    digitalWrite(resetPin, HIGH);
  }
}

// Function to print a debug message + int value
void debugMsgInt(const char *msg, int value, bool filter)
{
  if (DEBUG && filter)
  {
    Serial.print(msg);
    Serial.println(value);
  }
}

// Function to print a debug message + string value
void debugMsgStr(const char *msg, String data, bool filter)
{
  if (DEBUG && filter)
  {
    Serial.println(msg + data);
  }
}
