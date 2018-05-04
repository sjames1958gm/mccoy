#include <ESP8266WiFi.h>
#include <SPI.h>

#define DEBUG 1

#define RESETCMD 1
#define PINGCMD 2
#define MSG 7
#define ACK 0x7f

#define nop asm volatile("nop\n\t")

const char *ssid = "ATTVMb9amS";
const char *password = "xmcpmjhvr7u7";

int ledPin = LED_BUILTIN;
int resetPin = D1;
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
  debugMsgInt("sending command: ", MSG);
  SPI.transfer((char)testString.length());
  debugMsgInt("sending length: ", testString.length());

  for (int i = 0; i < testString.length(); i++)
  {
    SPI.transfer(testString[i]);
  }
  delay(100);
  unsigned char cmd = SPI.transfer(0xFF);
  while (cmd == 0xff) {
     cmd = SPI.transfer(0xFF);
  }
  debugMsgInt("Rcv Command: ", cmd);
  delay(1);
  unsigned char len = SPI.transfer(0xFF);
  debugMsgInt("Length: ", len);
  while (len > 0) {
    delay(1);
    Serial.print((char)SPI.transfer(0xFF));
    len--;
  }
  Serial.println();
}

void loop()
{
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
    delay(1);
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

  // TODO - send to TCP client
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
  if (DEBUG)
  {
    Serial.print("Varint: ");
    Serial.println(value);
  }
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
