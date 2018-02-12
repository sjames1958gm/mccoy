// How to make configurable?
#define SSID "ATTVMb9amS"
#define PASS "xmcpmjhvr7u7"
#define DST_IP "192.168.1.203"
#define PORT "9999"
#define RESET_PIN 8
const int ConnectRetries = 5;
#define BAUD_RATE 115200

boolean connected = false;
 
void setup()
{
  // Setup serial monitor wait for available.
  Serial.begin(115200);
  while(!Serial); 

//  Serial.println("Reset Module H/W");
//  pinMode(RESET_PIN, OUTPUT);
//  digitalWrite(RESET_PIN, LOW);
//  delay(500);
//  digitalWrite(RESET_PIN, HIGH);
//  delay(1000);
  

  // Open serial communications and wait for port to open:
  //serial 2 is to esp8266 
  Serial2.begin(BAUD_RATE);
  Serial2.setTimeout(4000);
  while(!Serial2);
  // Not sure if these two are needed . . .
  Serial2.flush();
  while(Serial2.available() > 0) Serial2.read();

  if (!sendCmdAndRead(Serial2, "AT", "OK", "")) {
    Serial.println("Hmm, not responding");
    while(1);
  }

  Serial.println("Reset module S/W");
  if (resetModule(Serial2)) {
    Serial.println("Module is ready");
  }

  setAccessPointMode();

  sendCmdAndRead(Serial2, "AT+GMR", "OK", "");
   return;
  connected = connectWiFi(ConnectRetries);

  if (!connected){
    Serial.println("Failed to connect to Wifi");
  }

  Serial.println("Connected to WiFi");
  sendCmdAndRead(Serial2, "AT+CIFSR", "OK", "");

  // set the single connection mode
  Serial.println("");
  sendCmdAndWait(Serial2, "AT+CIPMUX=0", "OK");
}


// Haven't gotten this to work
boolean resetModule(Stream& stream) {
  while(true) {
    if (sendCmdAndRead(Serial2, "AT+RST", "ready", "")) {
      return true;
    }
    while(1);
  }
}

boolean connectWiFi(int retries)
{
  for(int i = 0; i < retries ; i++)
  {
    
    String cmd="AT+CWJAP=\"";
    cmd+=SSID;
    cmd+="\",\"";
    cmd+=PASS;
    cmd+="\"";

    if (sendCmdAndReadWDelay(Serial2, cmd, 1500, "OK", "")) {
      return true;
    } 
    delay(1000);
  }
  return false;
}

boolean connectTcp(Stream& stream, String ip, String port) {
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += ip;
  cmd += "\",";
  cmd += port;

  return sendCmdAndRead(stream, cmd, "OK", "ERROR");
}

boolean isConnected(Stream& stream) {
  return sendCmdAndRead(stream, "AT+CIPSTATUS", "OK", "ERROR");
}

boolean disconnectTcp(Stream& stream) {
  return sendCmdAndRead(stream, "AT+CIPCLOSE", "OK", "ERROR");
}

boolean setAccessPointMode() {
  return sendCmdAndRead(Serial2, "AT+CWMODE=1", "OK", "");
}
  
bool sendCmdAndWait(Stream& stream, String cmd, String waitString) {
  Serial.println(cmd);
  stream.println(cmd);

  if (stream.find((char*)waitString.c_str())) return true;
  return false;
}

bool sendCmdAndRead(Stream& stream, String cmd, String waitString, String errorString) {
  sendCmdAndReadWDelay(stream, cmd, 100, waitString, errorString);
}

bool sendCmdAndReadWDelay(Stream& stream, String cmd, int delayTime, String waitString, String errorString) {
  Serial.print("C: ");
  Serial.println(cmd);
  stream.println(cmd);
  if (delayTime > 0) delay(delayTime);
  int retries = 10;
  while (!stream.available()) {
    delay(10);
  }
  String match;
  String readbuff;
  int av = stream.available();
  
  while (av > 0) {
    Serial.println(av);
    while (av-- > 0) {
      char c = stream.read();
      Serial.write(c);
      readbuff += c;
    }
//    Serial.write("RB: ");
//    Serial.write(readbuff.c_str());
    match += readbuff;
    readbuff = "";
    if (match.indexOf(waitString) != -1) break;
    delay(500);
    av = stream.available();
  }

  Serial.println(match.c_str());
  Serial.println(match.indexOf(waitString) != -1 ? "match" : "no match");
  Serial.flush();
  delay(1000);
  return match.indexOf(waitString) != -1;
}


void loop()
{
  if (!connected) return;
  do 
  {
    isConnected(Serial2);
    
    if (!connectTcp(Serial2, DST_IP, PORT)) {
      Serial.println("Failed to connect");
      while(1);
    }

      uint32_t buffer[5] = {1, 2, 3, 4, 5};

    String data;
    int i;
    for (i = 0; i < 5; i++) 
    {
      data += buffer[i];
    }

    Serial2.print("AT+CIPSEND=");
    Serial2.println(data.length());
  
  if(Serial2.find((char*)">"))
  {
    Serial.print(">");
  }
  else
  {
    Serial2.println("AT+CIPCLOSE");
    Serial.println("command timeout");
    delay(10000);
    break;
  }

  if (sendCmdAndWait(Serial2, data, "OK"))
  {
    while (!Serial2.available());

    while (Serial2.available())
    {
      char c = Serial2.read();
      Serial.print(c);
      if(c=='\r') Serial.print('\n');
    }

    Serial.println("");

    Serial.println("====");
  }
  } while(false);
    delay(10000);
}

void cmdLoopMode() {
  int av = Serial.available();
  if (av == 0) return;
  Serial.println(av);
  String readbuff = "";
  while (av > 0) {
    while (av-- > 0) {
      char c = Serial.read();
      readbuff += c;   
    }
    delay(50);
    av = Serial.available();
  }

  readbuff += "\n";
  Serial.print("C: ");
  Serial.println(readbuff);

  sendCmdAndRead(Serial2, readbuff, "OK", "");
    
}

