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

// LEDs for testing
#define LED1 8
#define LED2 9
#define LED3 10

// MARKER is the first character for a new message (sync character?)
#define MARKER '#'

// Define exercises directory name
#define exercisesDir "exercises"

// Handler function typedefs for Route table
typedef String (*GetHandlerFunction)(Stream& stream, String path, String extra);
typedef String (*PostHandlerFunction)(Stream& stream, String path, String extra, String body);

// Forward defs for route handlers
String getExercises(Stream&, String, String);
String getExercise(Stream&, String, String);
String getLed(Stream&, String, String);

// Get Route table structure
typedef struct {
  String path;
  GetHandlerFunction fn;
} GetRoute;

GetRoute getroutes[] = {
  {"/exercises", getExercises}, // Get list of exercises
  {"/exercise", getExercise}, // Get exercise file
  {"/led", getLed}, // test led on/off
  {"", NULL}  
};

// Get Route table structure
typedef struct {
  String path;
  PostHandlerFunction fn;
} PostRoute;

PostRoute postroutes[] = {
  {"", NULL}  
};

boolean sdCardInitialized = false;
SdFat SD;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial); 

  // Test LEDs set for output
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  
  // Open serial communications and wait for port to open:
  // Serial communication to Wifi chip
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
  // Default response needed?
  boolean respond = false;
  // Does message have body
  boolean hasBody;
  // Request line from message
  String requestLine;
  int bodyLength;
  String body;
  
  if (readHeaders(Serial2, hasBody, requestLine, bodyLength)) {
    if (hasBody) readBody(Serial2, bodyLength, body);
    
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
        // Not implemented
        respond = handlePost(Serial2, path);
      break;
      case METHOD_HEAD:     
        // Not implemented
        respond = true;
      break;
      default:
        respond = true;
    }
  }

  if (respond) {
    sendResponse(Serial2, 404, "Not Found");
  }
}

boolean readHeaders(Stream& stream, boolean& hasBody, String& requestLine, int& bodyLength)
{
  if (!stream.available()) return false;
  if (!readMarker(stream)) return false;
  
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

boolean readBody(Stream& stream, int bodyLength, String& body)
{
  Serial.println("readBody");
  String data;
  body = "";
  do 
  {
    data = readLine(stream);
    Serial.print("B: ");
    Serial.println(data);
    body += data;
  }
  while (body.length() < bodyLength);
  
  Serial.println("end");
  return true;
}

String readLine(Stream& stream)
{
  String result = "";
  while (true) {
    int av = stream.available();
    while (av-- > 0) {
      char c = stream.read();
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
    sendResponse(stream, 500, "SD Card not initialized");
    return false;
  }

  String route = path;
  String extra = "";
  int ndx = path.indexOf("/", 1);
  if (ndx != -1) {
    route = path.substring(0, ndx);
    extra = path.substring(ndx + 1);
  }

  GetRoute* r = &getroutes[0];
  while (r->fn != NULL) {
    if (r->path.equals(route)) {
      (*r->fn)(Serial2, route, extra);
      return;
    }
    *r++;
  }

  sendResponse(Serial2, 404, "Not Found");
  
  return false;
}

boolean handlePost(Stream& stream, String& path) 
{
  sendResponse(Serial2, 404, "Not Found");
  return false;
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

String readFile(String directory, String file) {
  Serial.print("Read file: ");
  String path = directory + "/" + file;
  Serial.println(path);
  String response = "";
  if (!SD.chdir(directory.c_str())) {
    Serial.println("Could not change to directory: ");
    Serial.println(directory);
  }
  else {
    File fn = SD.open(file);
    if (!fn) {
      Serial.print("Could not open file: ");
      Serial.println(file);
    }
    else 
    {
      // read from the file until there's nothing else in it:
      while (fn.available()) {
        response += (char)fn.read();
      }
      // close the file:
      fn.close();
      Serial.println(response);
    }
  }
  return response;
}

int processContentLength(String hdr) {
  return 0;
}

void sendResponse(Stream& stream, int status, String data)
{
  Serial.print("Send : "); Serial.print(data.length()); Serial.println(data);
  char buff[10];
  sprintf(buff, "%d", status);
  
  int length = data.length() + strlen(buff) + 1;
  Serial.println(length >> 8);
  stream.write(length >> 8);
  Serial.println(length & 0xFF);
  stream.write(length & 0xFF);
  for (int i = 0; i < strlen(buff); i++) {
    stream.print(buff[i]);
  }
  stream.print(':');
  char* datac = data.c_str();
  for (int i = 0; i < data.length(); i++) {
    stream.print(datac[i]);
  }
}

String getSensorList() {
  return "sensor1, sensor2";
}

String getExercises(Stream& stream, String path, String extra) {
  String data = readDirectory(exercisesDir);
  Serial.println(data);
  if (data.length() == 0) {
    sendResponse(stream, 500, "Server Error");
  }
  else {  
    sendResponse(stream, 200, data);
  }
  return "";
}

String getExercise(Stream& stream, String path, String extra) {
  String data = readFile(exercisesDir, extra);
  Serial.println(data);
  if (data.length() == 0) {
    sendResponse(stream, 500, "Server Error");
  }
  else {  
    sendResponse(stream, 200, data);
  }
  return "";
}

String getLed(Stream& stream, String path, String extra) {
  Serial.println(path + " path");
  Serial.println(extra + " extra");
  int ndx = extra.indexOf("/");
  if (ndx == -1) {
    sendResponse(stream, 404, "Not Found");
  } 
  else {
 
    int state = extra.substring(0, ndx).equals("on") ? HIGH : LOW;
    int led = atoi(extra.substring(ndx + 1).c_str());
    Serial.println(state);
    Serial.println(led);
    switch (led) {
      case 1:
        digitalWrite(LED1, state);
        sendResponse(stream, 200, "OK");
        break;
      case 2:
        digitalWrite(LED2, state);
        sendResponse(stream, 200, "OK");
        break;
      case 3:
        digitalWrite(LED3, state);
        sendResponse(stream, 200, "OK");
        break;
      default:
        sendResponse(stream, 404, "Not Found");
    }
    
  }
  
  return "";
}

bool readMarker(Stream& stream) {
  char c = stream.read();
  if (c == MARKER) return true;
  return false;
}

