
 
void setup()
{
  // Setup serial monitor wait for available.
  Serial.begin(115200);
  while(!Serial); 

  // Open serial communications and wait for port to open:
  //serial 2 is to esp8266 
  Serial2.begin(115200);
  Serial2.setTimeout(4000);
  while(!Serial2);
  // Not sure if these two are needed . . .
  Serial2.flush();
  while(Serial2.available() > 0) Serial2.read();

  if (!sendCmdAndRead(Serial2, "AT", "OK", "")) {
    Serial.println("Hmm, not responding");
    while(1);
  }
  
  if (!sendCmdAndRead(Serial2, "AT+IPR=9600", "OK", "")) {
    Serial.println("Hmm, not responding");
    while(1);
  }

  Serial2.end();

  Serial2.begin(9600);
  while(!Serial2);
  // Not sure if these two are needed . . .
  Serial2.flush();
  while(Serial2.available() > 0) Serial2.read();

  if (!sendCmdAndRead(Serial2, "AT", "OK", "")) {
    Serial.println("Hmm, not responding");
    while(1);
  }
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
   delay(10000);
}



