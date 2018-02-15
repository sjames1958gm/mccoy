#include <BlockDriver.h>
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <SysCall.h>

#define BAUD_RATE 9600
#define METHOD_GET 1
#define METHOD_POST 2
#define METHOD_HEAD 3
#define METHOD_OTHER 4

#define exercisesDir "exercises"

boolean sdCardInitialized = false;
SdFat SD;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial); 

  pinMode(LED_BUILTIN, OUTPUT);
  
  // Open serial communications and wait for port to open:
  Serial2.begin(BAUD_RATE);
  while(!Serial2);

  if (SD.begin()) {
    sdCardInitialized = true;
  }
  else {
    Serial.println("SD Card initialization failed!");
  }
  
  Serial.println("init complete");
}

void loop() {
  // put your main code here, to run repeatedly:
  boolean respond = false;
  boolean hasBody;
  String requestLine;
  int bodyLength;
  if (readHeaders(Serial2, hasBody, requestLine, bodyLength)) {
    if (hasBody) readBody(Serial2);
    int method;
    String path;
    processRequestLine(requestLine, method, path);
    Serial.println(method);
    Serial.println(path);
    switch (method) {
      case METHOD_GET:
        respond = handleGet(Serial2, path);
      break;
      case METHOD_POST:
        respond = true;
      break;
      case METHOD_HEAD:
        respond = true;
      break;
      default:
        respond = true;
    }
  }

  if (respond) {
    Serial.println("=====");
    Serial2.println("HTTP/1.1 200 OK");
    Serial2.println("Connection: close");
    Serial2.println();
  }
}

boolean readHeaders(Stream& stream, boolean& hasBody, String& requestLine, int& bodyLength)
{
  if (!stream.available()) return false;
  Serial.println("readHeaders");
  requestLine = "";
  String hdr;
  hasBody = false;
  do 
  {
    hdr = readLine(stream);
    Serial.print("H: ");
    Serial.println(hdr + " : " + hdr.length() );
    // If HEAD this doesn't count.
    if (requestLine.length() == 0) requestLine = hdr;
    hdr.toLowerCase();
    if (hdr.indexOf("content-length") != -1) {
      hasBody = true;
      bodyLength = processContentLength(hdr);
    }
  }
  while (hdr.length() != 0);
  
  Serial.println("end");
  return true;
}

boolean readBody(Stream& stream)
{
  Serial.println("readBody");
  String body;
  do 
  {
    body = readLine(stream);
    Serial.print("B: ");
    Serial.println(body);
  }
  while (body.length() != 0);
  
  Serial.println("end");
  return true;
}

String readLine(Stream& stream)
{
  digitalWrite(LED_BUILTIN, HIGH);
  String result = "";
  while (true) {
    int av = stream.available();
    while (av-- > 0) {
      char c = stream.read();
      Serial.print("C: ");
      Serial.println(c, HEX);
      if (c == '\r') {
        if (stream.peek() == '\n') stream.read();
        return result;
      }
      // Getting sporadic zeros on serial interface
      if (c != 0) result += c;
//      if (c == 0 && result.length() == 0) return "";
    }
    delay(10);
  }
}

void processRequestLine(String requestLine, int& method, String& path)
{
  method = METHOD_OTHER;
  int ndx = requestLine.indexOf(" ");
  if (ndx == -1) return;
  method = methodToEnum(requestLine.substring(0, ndx));

  ndx = requestLine.indexOf('/', ndx);
  if (ndx == -1) return;
  int endndx = requestLine.indexOf(' ', ndx);
  if (endndx == -1) return;

  path = requestLine.substring(ndx, endndx);
}

int methodToEnum(String method) 
{
  if (method.equalsIgnoreCase("get")) return METHOD_GET;
  if (method.equalsIgnoreCase("get")) return METHOD_POST;
  if (method.equalsIgnoreCase("get")) return METHOD_HEAD;
  return METHOD_OTHER;  
}

boolean handleGet(Stream& stream, String& path) 
{
  // Return true if caller should respond;

  if (!sdCardInitialized) {
    send500Response(stream);
    return false;
  }

  if (path.equalsIgnoreCase("/exercises")) {
    String data = readDirectory(exercisesDir);
    Serial.println(data);
    if (data.length() == 0) {
      send500Response(stream);
    }
    else {  
      send200Response(stream, data);
    }
    return false;
  }
  return true;
}

String readDirectory(String path) {
  Serial.print("List Dir: ");
  Serial.println(path);
  String response = "";
  File dir = SD.open(path);
  if (!dir) {
    Serial.print("Could not open directory: ");
    Serial.println(path);
  }
  else 
  {
    while (true) {
      File entry =  dir.openNextFile();
      if (! entry) {
        // no more files
        break;
      }
      if (response.length() > 0) response += ", ";
      char name[24];
      entry.getName(name, sizeof(name));
      response += name;
      entry.close();
    }
    dir.close();
  }
  return response;
}

int processContentLength(String hdr) {
  return 0;
}

void send200Response(Stream& stream, String data) {
  Serial.print("Send : "); Serial.println(data);
  stream.write(data.length());
  stream.print(data);
//  stream.println("HTTP/1.1 200 OK");
//  stream.println("Connection: close");
//  stream.println("Content-type: text/plain");
//  stream.println();
//  stream.println(data);
//  stream.println();
}

void send500Response(Stream& stream) {
  stream.println("HTTP/1.1 500 Server Error");
  stream.println("Connection: close");
  stream.println();
}

