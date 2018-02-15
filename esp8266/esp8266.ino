// How to make configurable?
#define SSID "ATTVMb9amS"
#define PASS "xmcpmjhvr7u7"
#define DST_IP "192.168.1.203"
#define PORT "9999"
#define RESET_PIN 8
const int ConnectRetries = 5;
#define BAUD_RATE 9600

#define wifi Serial3

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
  wifi.begin(BAUD_RATE);
  wifi.setTimeout(4000);
  while(!wifi);
  // Not sure if these two are needed . . .
  wifi.flush();
  while(wifi.available() > 0) wifi.read();

  Serial.println("Start up");
  
  if (!sendCmdAndRead(wifi, "AT", "OK", "")) {
    Serial.println("Hmm, not responding");
    while(1);
  }

//  Serial.println("Reset module S/W");
//  if (resetModule(wifi)) {
//    Serial.println("Module is ready");
//  }

  setAccessPointMode(wifi);

  sendCmdAndRead(wifi, "AT+GMR", "OK", "");
//   return;
  connected = connectWiFi(wifi, ConnectRetries);

  if (!connected){
    Serial.println("Failed to connect to Wifi");
  }

  Serial.println("Connected to WiFi");
//  sendCmdAndRead(wifi, "AT+CIFSR", "OK", "");

  // set the single connection mode
  Serial.println("");
  sendCmdAndWait(wifi, "AT+CIPMUX=0", "OK");
}


// Haven't gotten this to work
boolean resetModule(Stream& stream) {
  while(true) {
    if (sendCmdAndRead(wifi, "AT+RST", "ready", "")) {
      return true;
    }
    while(1);
  }
}

boolean connectWiFi(Stream& wifi, int retries)
{
  for(int i = 0; i < retries ; i++)
  {
    
    String cmd="AT+CWJAP=\"";
    cmd+=SSID;
    cmd+="\",\"";
    cmd+=PASS;
    cmd+="\"";

    if (sendCmdAndReadWDelay(wifi, cmd, 1500, "OK", "")) {
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

boolean setAccessPointMode(Stream& wifi) {
  return sendCmdAndRead(wifi, "AT+CWMODE=1", "OK", "");
}
  
bool sendCmdAndWait(Stream& stream, String cmd, String waitString) {
  Serial.println(cmd);
  stream.println(cmd + "\r\n");

  if (stream.find((char*)waitString.c_str())) return true;
  return false;
}

bool sendCmdAndRead(Stream& stream, String cmd, String waitString, String errorString) {
  sendCmdAndReadWDelay(stream, cmd, 100, waitString, errorString);
}

bool sendCmdAndReadWDelay(Stream& stream, String cmd, int delayTime, String waitString, String errorString) {
  Serial.print("C: ");
  Serial.println(cmd);
  stream.print(cmd + "\r\n");
  if (delayTime > 0) delay(delayTime);
  int retries = 10;

  String resp = "";
  String line = readLine(stream);
  while (true) {
    Serial.println(line);
    if (line.indexOf("OK")) {
      resp += "\n";
      resp += line;
      return resp.indexOf(waitString) != -1;
    }
    line = readLine(stream);
  }
}


void loop()
{
  if (!connected) return;
  do 
  {
    isConnected(wifi);
    
    if (!connectTcp(wifi, DST_IP, PORT)) {
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

    wifi.print("AT+CIPSEND=");
    wifi.println(data.length());
  
  if(wifi.find((char*)">"))
  {
    Serial.print(">");
  }
  else
  {
    wifi.println("AT+CIPCLOSE");
    Serial.println("command timeout");
    delay(10000);
    break;
  }

  if (sendCmdAndRead(wifi, data, "OK", ""))
  {
    while (!wifi.available());

    while (wifi.available())
    {
      char c = wifi.read();
      Serial.print(c);
      if(c=='\r') Serial.print('\n');
    }

    Serial.println("");

    Serial.println("====");
  }
  } while(false);
    delay(10000);
}

void cmdLoopMode(Stream& wifi) {
  
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

  sendCmdAndRead(wifi, readbuff, "OK", "");
    
}

String readLine(Stream& stream)
{
  String result = "";
  int timeout = 0;
  while (true) {
    int av = stream.available();
    while (av-- > 0) {
      timeout = 0;
      char c = stream.read();
      Serial.print(c, HEX);
      if (c == '\r') {
        if (stream.peek() == '\n') stream.read();
        Serial.println();
        return result;
      }
      result += c;
    }
    timeout += 10;
    if (timeout > 200) return result;
    delay(10);
  }
}
