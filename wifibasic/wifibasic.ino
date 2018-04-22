#include <ESP8266WiFi.h>
#include <SPI.h>

#define DEBUG 1

const char* ssid = "ATTVMb9amS";
const char* password = "xmcpmjhvr7u7";
 
int ledPin = LED_BUILTIN;
int resetPin = D3;
WiFiServer server(9000);
WiFiClient client;

void setup() {
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
 
  while (WiFi.status() != WL_CONNECTED) {
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

  
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  delay(500);
  digitalWrite(resetPin, HIGH);
  delay(2000);
  String testString = "SPI Interface Initialized\n";
  SPI.transfer((char)testString.length() + 1);
  SPI.transfer((char)0);
  for (int i = 0; i < testString.length(); i++) {
    SPI.transfer(testString[i]);
  }
}
 
void loop() {
  // Check if a client has connected
  if (!client.connected()) {
    client = server.available();
    if (!client) {
      delay(10);
      return;
    }
    Serial.println("new client");
  }
 
  // Wait until the client sends some data
  if (!client.available()){
    delay(1);
    return;
  }

  int size = readVarint(&client);

  String request = readFor(&client, size);
  Serial.println(&request[1]);
  
  // Match the request
 
//  int value = LOW;
//  if (request.indexOf("/LED=ON") != -1)  {
//    digitalWrite(ledPin, LOW);
//    value = HIGH;
//  }
//  if (request.indexOf("/LED=OFF") != -1)  {
//    digitalWrite(ledPin, HIGH);
//    value = LOW;  
//  }

  SPI.transfer((char) request.length());
  for (int i = 0; i < request.length(); i++) {
    SPI.transfer(request[i]);
  }
 
//  // Return the response
//  String resp = "HTTP/1.1 200 OK\r\n";
//  resp += String("Content-Type: text/html\r\n\r\n");
//  resp += String("<!DOCTYPE HTML>\r\n");
//  resp += String("<html>\r\n");
// 
//  resp += String("Led pin is now: ");
// 
//  if(value == HIGH) {
//    resp += String("On\r\n");
//  } else {
//    resp += String("Off\r\n");
//  }
//  resp += String("<br><br>\r\n");
//  resp += String("<a href=\"/LED=ON\"\"><button>Turn On </button></a>\r\n");
//  resp += String("<a href=\"/LED=OFF\"\"><button>Turn Off </button></a><br />\r\n");  
//  resp += String("</html>\r\n");

  
  delay(1); 
}

int readVarint(WiFiClient* client) {
  unsigned int value = 0;
  int len = 0;
  while (true) {
    while(!client->available()) {
      delay(1);
    }
    unsigned char c = client->read();
    value = ((c & 0x7f) << (len * 7)) + value;
    if (c < 128) break;
  }
  if (DEBUG) { Serial.print("Value: "); Serial.println(value); }
  return value;
}

void sendVarint(WiFiClient* client, unsigned int value) {
  while (value > 0) {
    unsigned char c = value &0x7f;
    value = value >> 7;
    if (value > 0) c = c | 0x80;
    client->write(c);
  }
}

String readFor(WiFiClient* client, unsigned int len) {
  String buff = "";
  while (len-- > 0) {
    while(!client->available()) {
      delay(1);
    }
    char c = client->read();
    buff += c;
  }
  return buff;
}

void writeFor(WiFiClient* client, String buffer) {
  int ndx = 0;
  while (ndx < buffer.length()) {
    client->write(buffer[ndx++]);
  }
}

