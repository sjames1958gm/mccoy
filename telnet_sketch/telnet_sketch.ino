/*
  DHCP-based IP printer
 
 This sketch uses the DHCP extensions to the Ethernet library
 to get an IP address via DHCP and print the address obtained.
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 
 created 12 April 2011
 modified 9 Apr 2012
 by Tom Igoe
 
 */

#include <SPI.h>
#include <EthernetV2_0.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte MAC[] = {  
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
  
//IPAddress SERVER(192, 168, 1, 91); 
IPAddress SERVER(52, 15, 194, 28);

uint16_t PORT = 17218;

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;
#define W5200_CS  10
#define SDCARD_CS 4

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  pinMode(SDCARD_CS,OUTPUT);
  digitalWrite(SDCARD_CS,HIGH);//Deselect the SD card

  // this check is only needed on the Leonardo:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // start the Ethernet connection:
  if (Ethernet.begin(MAC) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  
  // print your local IP address:
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();
  
  Serial.println("connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(SERVER, PORT)) {
    Serial.println("connected");
  } 
  else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}

void loop() {
  // if there are incoming bytes available 
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // as long as there are bytes in the serial queue,
  // read them and send them out the socket if it's open:
  while (Serial.available() > 0) {
    char inChar = Serial.read();
    if (client.connected()) {
      client.print(inChar); 
    }
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    // do nothing:
    while(true);
  }
}


