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

typedef struct {
  String sensorName;
  int sensorPin;
} Sensor;

Sensor sensors[] = { {"sensor1", A0}, {"sensor2", A1} };
int sensorsLength = sizeof(sensors) / sizeof(Sensor);

// Define exercises directory name
#define exercisesDir "exercises"

// Handler function typedefs for Route table
typedef String (*GetHandlerFunction)(Stream& stream, String path, String extra);
typedef String (*PostHandlerFunction)(Stream& stream, String path, String extra, String body);

// Forward defs for route handlers
// These are GET handlers
String getDir(Stream&, String, String);
String getFile(Stream&, String, String);
String getExercises(Stream&, String, String);
String getExercise(Stream&, String, String);
String getSensors(Stream&, String, String);
String getSensor(Stream&, String, String);
String getLed(Stream&, String, String);

// These are the POST handlers
String postDir(Stream&, String, String, String);
String postFile                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             (Stream&, String, String, String);

// GET Route table structure
typedef struct {
  String path;
  GetHandlerFunction fn;
} GetRoute;

GetRoute getroutes[] = {
  {"/dir", getDir},
  {"/file", getFile},
  {"/exercises", getExercises}, // Get list of exercises
  {"/exercise", getExercise}, // Get exercise file
  {"/sensors", getSensors},
  {"/sensor", getSensor},
  {"/led", getLed}, // test led on/off
  {"", NULL}  
};

// POST Route table structure
typedef struct {
  String path;
  PostHandlerFunction fn;
} PostRoute;

PostRoute postroutes[] = {
  {"/mkdir", postDir},
  {"/mkfile", postFile},
  {"", NULL}  
};

boolean sdCardInitialized = false;
SdFat SD;

void setup() {
  // Initialize monitor
  Serial.begin(9600);
  while(!Serial); 

  // Test LEDs set for output
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  
  // Serial communication to Wifi chip
  Serial2.begin(BAUD_RATE);
  while(!Serial2);

  // Initialized the SD card.
  if (SD.begin()) {
    sdCardInitialized = true;
  }
  else {
    Serial.println("SD Card initialization failed!");
  }
  
  Serial.println("init complete");
}

void checkWifi() {
  // Default response needed?
  boolean respond = false;
  // Does message have body
  boolean hasBody;
  // Request line from message
  String requestLine;
  int bodyLength;
  String body;

  // Read haders from serial interface to Wifi chip
  if (readHeaders(Serial2, hasBody, requestLine, bodyLength)) {
    // Read body if there is one.
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
        respond = handlePost(Serial2, path, body);
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

void loop() {
  checkWifi();
}

boolean readHeaders(Stream& stream, boolean& hasBody, String& requestLine, int& bodyLength)
{
  // If no data or marker not read return
  if (!stream.available()) return false;
  if (!readMarker(stream)) return false;
  
  Serial.println("readHeaders");
  
  requestLine = "";
  String hdr;
  hasBody = false;

  // Read header lines until an empty line is read.
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

// Ready body, for bodylength bytes
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

// Read a line until \r\n
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
    }
    delay(10);
  }
}

int methodToEnum(String method) 
{
  if (method.equalsIgnoreCase("get")) return METHOD_GET;
  if (method.equalsIgnoreCase("post")) return METHOD_POST;
  if (method.equalsIgnoreCase("head")) return METHOD_HEAD;
  return METHOD_OTHER;  
}

// Extract info from request line.
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

boolean handleGet(Stream& stream, String& path) 
{
  // Return true if caller should respond;

  String route = path;
  String extra = "";
  int ndx = path.indexOf("/", 1);
  if (ndx != -1) {
    route = path.substring(0, ndx);
    extra = path.substring(ndx + 1);
  }

  Serial.println(String("handleGet, path: ") + path + String(", extra: ") + extra);
  
  // get matching route and call function
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

boolean handlePost(Stream& stream, String& path, String& body) 
{
  // Return true if caller should respond;

  String route = path;
  String extra = "";
  int ndx = path.indexOf("/", 1);
  if (ndx != -1) {
    route = path.substring(0, ndx);
    extra = path.substring(ndx + 1);
  }

  Serial.println(String("handlePost, path: ") + path + String(", extra: ") + extra);
  
  // get matching route and call function
  PostRoute* r = &postroutes[0];
  while (r->fn != NULL) {
    if (r->path.equals(route)) {
      (*r->fn)(Serial2, route, extra, body);
      return;
    }
    *r++;
  }

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

String readFile(String directory, String file, bool& error) {
  Serial.print("Read file: ");
  String path = directory + "/" + file;
  Serial.println(path);
  String response = "";
  error = true;
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
      error = false;
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

String getFile(Stream& stream, String path, String extra) 
{
  Serial.println(String("getFile ") + extra);
  String dir = "/";
  int ndx = extra.lastIndexOf('/');
  String file = extra;
  if (ndx != -1) {
    dir += extra.substring(0, ndx);
    file = extra.substring(ndx + 1);
  }

  if (file.length() == 0) {
    Serial.println("file name length 0");
    sendResponse(stream, 500, "Server Error");
    return "";
  }

  bool error;
  String data = readFile(dir, file, error);
  if (!error) {
    sendResponse(stream, 200, data);  
  }
  else {
    sendResponse(stream, 404, "Not Found");  
  }
  
  return "";
}

String getDir(Stream& stream, String path, String extra) 
{
  Serial.println(String("getDir ") + extra);

  String dir = String("/") + extra;

  String data = readDirectory(dir);

  if (data.length() > 0) {
    sendResponse(stream, 200, data);  
  }
  else {
    sendResponse(stream, 404, "Not Found");  
  }
  
  return "";
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
  bool error;
  String data = readFile(exercisesDir, extra, error);
  Serial.println(data);
  if (error) {
    sendResponse(stream, 500, "Server Error");
  }
  else {  
    sendResponse(stream, 200, data);
  }
  return "";
}

String getSensors(Stream& stream, String path, String extra) {
  Serial.println(path + " path");
  Serial.println(extra + " extra");
  String response = "";
  for (int i = 0; i < sensorsLength; i++) {
      response += sensors[i].sensorName + String(",");
  }
    
  sendResponse(stream, 200, response.substring(0, response.length() - 1));
}

String getSensor(Stream& stream, String path, String extra) {
  Serial.println(path + " path");
  Serial.println(extra + " extra");
  String response = "";
  
  if (extra.length() == 0) {
    for (int i = 0; i < sensorsLength; i++) {
      int sensorValue = analogRead(sensors[i].sensorPin);
      Serial.println("Sensor value: " + String(sensorValue));
      response += sensors[i].sensorName + String(":") + String(sensorValue) + String(",");
    }
    response = response.substring(0, response.length() - 1);
  }
  else {
    int ndx = atoi(extra.c_str());
    if (ndx >= 0 && ndx <= sensorsLength) {
      int sensorValue = analogRead(sensors[ndx].sensorPin);
      Serial.println("Sensor value: " + String(sensorValue));
      response += sensors[ndx].sensorName + String(":") + String(sensorValue);
    }
    else {
      sendResponse(stream, 404, "Not Found");
      return "";
    }
  }

  sendResponse(stream, 200, response);
  
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

String postDir(Stream& stream, String path, String extra, String body) {
  Serial.println(String("postDir ") + extra);
  if (extra.length() == 0) {
    sendResponse(stream, 500, "Server Error");
  }
  else {
    
    String dir = String("/") + extra;
    
    if (SD.mkdir(dir.c_str() )) {
      sendResponse(stream, 200, "");
    }
    else {
      sendResponse(stream, 500, "Server Error");
    }
  }
  return "";
}

String postFile(Stream& stream, String path, String extra, String body) {
  Serial.println(String("postFile ") + extra);

  String dir = "/";
  String file = "";
  int ndx = extra.lastIndexOf("/");
  if (ndx == -1) {
    file = extra;
  }
  else {
    dir += extra.substring(0, ndx);
    file = extra.substring(ndx + 1);
  }
    
  if (file.length() == 0) {
    Serial.println(String("error: no file specified: ") + extra);
    sendResponse(stream, 500, "Server Error");
  }
  else {   
    if (false) {
      sendResponse(stream, 200, "");
    }
    else {
      sendResponse(stream, 500, "Server Error");
    }
  }
  return "";
}

bool readMarker(Stream& stream) {
  char c = stream.read();
  if (c == MARKER) return true;
  return false;
}

