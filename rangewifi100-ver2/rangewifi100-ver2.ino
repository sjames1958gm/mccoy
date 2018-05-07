#include <ESP8266WiFi.h>
#include <SPI.h>

#define DEBUG 1

#define RESETCMD 1
#define PINGCMD 2
#define POLLCMD 3
#define MSG 7
#define ZVIBES 20
#define ACK 0x7f

IPAddress ip(192, 168, 2, 100);
IPAddress gateway(192, 168, 2, 254);
IPAddress netmask(255, 255, 255, 0);

#define nop asm volatile("nop\n\t")

const char* ssid = "ML-guest";
const char* password = "mlguest538!";
//const char *ssid = "ATTVMb9amS";
//const char *password = "xmcpmjhvr7u7";

int ledPin = LED_BUILTIN;
int resetPin = D1;
int serialPollRate = 100;
int serialPollLast = millis();
WiFiServer server(9000);
WiFiClient client;

void setup()
{
  Serial.begin(115200);
  delay(10);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  SPI.begin();

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

  resetSlave();

  delay(2000);
  String testString = "SPI Interface Initialized\n";
  SPI.transfer((char)MSG);
//  debugMsgInt("sending command: ", MSG);
  SPI.transfer((char)testString.length());
//  debugMsgInt("sending length: ", testString.length());

  for (int i = 0; i < testString.length(); i++)
  {
    SPI.transfer(testString[i]);
  }
  delay(100);
  unsigned char cmd = SPI.transfer(0xFF);
  while (cmd == 0xff) {
     cmd = SPI.transfer(0xFF);
  }
//  debugMsgInt("Rcv Command: ", cmd);
  delay(1);
  unsigned char len = SPI.transfer(0xFF);
//  debugMsgInt("Length: ", len);
  while (len > 0) {
    delay(1);
    Serial.print((char)SPI.transfer(0xFF));
    len--;
  }
  Serial.println();
}

void getjob(){
  int jobnumber;
  bool done = false;
  while (!done) { 
 Serial.println("GETJOB():  enter the item number to run");
 Serial.println("ITEM    function   Discreption");
 Serial.println("1.  getmasterdata()  change values of constants");
 Serial.println("2.  restvibes()   establish rest values of accelarometers");
 Serial.println("3.  testvibes()   gather 3 axis data for user specified time");
 Serial.println("4.  zvibes()      gather z axis data for user specified time");
 Serial.println("5.  testhall()    gather data from Hall sensors for user specified time");
 Serial.println("6.  testmove()    target moves up-and-down user specified number of times and schedule");
 Serial.println("7.  testmovehit() target moves up user specified number of times and down when HIT");
 Serial.println("8.  motogotime()  motor runs user specified direction & time");
 Serial.println("9.  motogosteps()  motor runs up and down in steps of user defined time, reporting sensor data all the way.");
 Serial.println("10. lightsout()  turns off any lights on.");
 Serial.println("99. getjob() EXIT");
 while (!Serial.available()){
  handleWifi();
  handleSerial();
 }
  delay(50);
  while(Serial.available())
  {
    jobnumber = Serial.parseInt();
    if(Serial.read() != '\n'){Serial.println("going to "+String(jobnumber));}
  } 
  switch (jobnumber) {
    case 1:
//    getmasterdata(); 
    break;
    case 2:
//      restvibes(); 
      break;
    case 3:
//      testvibes();
      break;
     case 4:
      zvibes(); 
      break;
    case 5:
//      testhall(); 
      break;
    case 6:
//      testmove(); 
      break;
    case 7:
//      testmovehit(); 
      break;
    case 8:
//      case8error:
//      Serial.println("ENTER: milliseconds of runtime ,dir 0 = down, dir 1 = up");
//      Serial.println("OR 9,9 to exit motogotime().");
//      while (!Serial.available()){ ;}
//      delay(50);
//      while(Serial.available())
//      {
//      runtime = Serial.parseInt();
//      dir = Serial.parseInt();
//      if(Serial.read() != '\n'){Serial.println("IN getjob() dir, runtime = " +String(dir)+","+String(runtime));}
//      }
//      if(runtime == 9 || dir == 9) goto getjobgood;
//      if(dir < 0 || dir > 1) goto case8error;
//      if(runtime < 10  || runtime > 800) goto case8error;
//      motogotime();
//      goto case8error; 
      break;
    case 9:
//      motogosteps();
      break;
    case 10:
//      lightsout();
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

void zvibes(){
  unsigned int testmillis;
  Serial.println("IN zvibes() ENTER: milliseconds to test");
  while (!Serial.available()){ 
    handleWifi();
  }
  delay(50);
  while(Serial.available())
  {
    testmillis = Serial.parseInt();
    if(Serial.read() != '\n'){Serial.println("Entered testmillis = "+String(testmillis));}
  }
  
  SPI.transfer((char)ZVIBES);
  debugMsgInt("sending command: ", ZVIBES);
  debugMsgInt("milliseconds: ", testmillis);
  
  SPI.transfer((char)4);
  for (int i = 0; i < 4; i++) {
    SPI.transfer((char)(testmillis & 0xFF));
    testmillis  = testmillis >> 8;
  }
//  debugMsgInt("sending length: ", testString.length());
//
//  for (int i = 0; i < testString.length(); i++)
//  {
//    SPI.transfer(testString[i]);
//  }
  delay(100);
  unsigned char cmd = SPI.transfer(0xFF);
  while (cmd == 0xff) {
     cmd = SPI.transfer(0xFF);
  }
//  debugMsgInt("Rcv Command: ", cmd);
  delay(1);
  unsigned char len = SPI.transfer(0xFF);
//  debugMsgInt("Length: ", len);
  while (len > 0) {
    delay(1);
    Serial.print((char)SPI.transfer(0xFF));
    len--;
  }
  Serial.println();
  
}

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

  debugMsgInt("Command: ", cmd);
  debugMsgInt("Length: ", len);

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
    cmd = SPI.transfer(0xFF);
    while (cmd == 0xff) {
       cmd = SPI.transfer(0xFF);
    }
    debugMsgInt("Rcv Command: ", cmd);
    
    delay(1);
    unsigned char inLen = SPI.transfer(0xFF);
    debugMsgInt("Length: ", inLen);
    
    String response;
    while (inLen > 0) {
      delay(1);
      char c = SPI.transfer(0xFF);    
      response += c;
      inLen--;
    }
    debugMsgStr("Rcv Command data: ", response);
  
    handleSerialMessage(&client, cmd, response);
  }
}

void handleSerial() {
  int now = millis();
  if (now - serialPollLast > serialPollRate) {
  SPI.transfer((char)POLLCMD);
  
  SPI.transfer((char)0);

  delay(100);
  unsigned char cmd = SPI.transfer(0xFF);
  while (cmd == 0xff) {
     cmd = SPI.transfer(0xFF);
  }
//  debugMsgInt("Rcv Command: ", cmd);
  delay(1);
  unsigned char len = SPI.transfer(0xFF);
//  debugMsgInt("Length: ", len);
  unsigned int resp = 0;
  while (len > 0) {
    delay(1);
    resp = (resp << 8) && (char)SPI.transfer(0xFF);
    len--;
  }
  if (cmd == ZVIBES) {
    debugMsgInt("Received zvibes data", resp);
  }
    serialPollLast = millis();
  }
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
  debugMsgInt("Varint: ", value);

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

void debugMsgInt(const char *msg, int value)
{
  if (DEBUG)
  {
    Serial.print(msg);
    Serial.println(value);
  }
}

void debugMsgStr(const char *msg, String data)
{
  if (DEBUG)
  {
    Serial.println(msg + data);
  }
}
