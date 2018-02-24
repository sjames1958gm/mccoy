#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define BAUD_RATE 9600

const char* ssid = "range";
const char* password = "range1234";
ESP8266WebServer server(80);

IPAddress localIP(192,168,2,2);
IPAddress gatewayIP(192,168,2,1);
IPAddress subnet(255,255,255,0);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(led, 0);
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void handleSerial() {
  String request = (server.method() == HTTP_GET)?"GET ":"POST ";
  request += server.uri();
  request += " \r\n\r\n";
  Serial.print(request);

  int status;
  String response = readResponse(Serial, status);
  server.sendHeader("access-control-allow-origin", "*");
  if (response.length() > 0) {
    server.send(status, "text/plain", response);
  }
  else {
    server.send(status);
    Serial1.print("Send response: ");
    Serial1.print(status);
    Serial1.print(" - ");
    Serial1.println(response);
  }
}

void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(BAUD_RATE);
  Serial1.begin(9600);
  Serial1.println("Setup begin");
  WiFi.mode(WIFI_AP_STA);
  Serial1.print("SoftAP Config: ");
  Serial1.println(WiFi.softAPConfig(localIP, gatewayIP, subnet) ? "success" : "fail");

  Serial1.print("SoftAP: ");
  Serial1.println(WiFi.softAP(ssid, password) ? "success" : "fail");

  Serial1.print("SoftAP - IP:");
  Serial1.println(WiFi.softAPIP());
  Serial1.println("");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial1.print(".");
  }

  if (MDNS.begin("esp8266")) {
    Serial1.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/test", [](){
    server.send(200, "text/plain", "test passed");
  });

  server.onNotFound(handleSerial);

  server.begin();
  Serial.println("HTTP server started");
  Serial.flush();
  Serial.swap();
}

void loop(void){
  server.handleClient();
}

String readResponse(Stream& stream, int& status) {
  // Need some sort of timeout here?
  Serial1.println("readResponse");
  String response = "";
  unsigned int length = readStream(stream);
  Serial1.println(length, HEX);
  length = (length << 8) + readStream(stream);
  Serial1.println(length, HEX);
  while (length-- > 0) {
    response += (char)readStream(stream);
  }
  Serial1.println(response);

  status = 200;
  int ndx = response.indexOf(":");
  if (ndx != -1) {
    status = atoi(response.substring(0, ndx).c_str());
    response = response.substring(ndx + 1);
  }
  return response;
}

unsigned char readStream(Stream& stream) {
  while (!stream.available()) delay(10);
  unsigned char c = stream.read();
  Serial1.println((int)c, HEX);
  return c;
}  

