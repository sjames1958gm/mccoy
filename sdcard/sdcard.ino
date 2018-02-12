#include <BlockDriver.h>
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <SysCall.h>

#define LIST_DIR 'd'
#define CHG_DIR 'c'
#define SHOW_FILE 's'

String currDir = "/";

SdFat sd;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  if (!sd.begin()) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
}

void loop() {
  showMenu();
  String selection;
  char input = getInput(selection);
  switch (input) {
    case LIST_DIR:
      listDir(selection);
      break;
    case CHG_DIR:
      changeDir(selection);
      break;
    case SHOW_FILE:
      listFile(selection);
      break;
    default:
      Serial.println("Invalid Selection:");
  }
}

void showMenu() {
  Serial.println("No Menu yet:");
}

char getInput(String& selection) {
  while(!Serial.available()) {
    delay(50);
  }

  char c = Serial.read();
  switch (c) {
    case LIST_DIR:
      selection = currDir;
      break;
    case CHG_DIR:
      selection = "/";
      break;
    case SHOW_FILE:
      selection = "test.txt";
      break;
  }
  // Flush input
  while (Serial.available()) Serial.read();
  return c;
}

void listDir(String selection) {
  Serial.print("List Dir: ");
  Serial.println(selection);
  File dir = sd.open(selection);
  if (!dir) {
    Serial.print("Could not open directory: ");
    Serial.println(selection);
  }
  else 
  {
    int numTabs = 1;
    while (true) {

      File entry =  dir.openNextFile();
      if (! entry) {
        // no more files
        break;
      }
      for (uint8_t i = 0; i < numTabs; i++) {
        Serial.print('\t');
      }
      char name[24];
      entry.getName(name, sizeof(name));
      Serial.print(name);
      if (entry.isDirectory()) {
        Serial.println("/");
      } else {
        // files have sizes, directories do not
        Serial.print("\t\t");
        Serial.println(entry.size(), DEC);
      }
      entry.close();
    }
    dir.close();
  }
}

void changeDir(String selection) {
  Serial.print("Change Dir: ");
  Serial.println(selection);
  currDir = selection;
}

void listFile(String selection) {
  Serial.print("Show File: ");
  Serial.println(selection);
  File fn = sd.open(selection);
  if (!fn) {
    Serial.print("Could not open file: ");
    Serial.println(selection);
  }
  else 
  {
      // read from the file until there's nothing else in it:
     while (fn.available()) {
        Serial.write(fn.read());
     }
      // close the file:
      fn.close();
  }
}

