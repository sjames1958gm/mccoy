// based on exerciseall-ver1
// 3.24.2018 1645hrs
// first use of device ver3 using: 220rpm, 3hall effect, 1 3-axis accel
// pins have been changed from ver1

// Code for SPI interface to 8266
#include <SPI.h>

#define RCVCMD 1
#define RCVLEN 2
#define RCVDATA 3
#define RCVCOMP 4
#define SENDCMD 5
#define SENDLEN 6
#define SENDDATA 7
#define WAIT 8

#define STATUS_IDLE 1
#define STATUS_RUNNING 2

#define DEBUG 1
// Set to 1 to indeicate no H/W is available
#define NOHW 1

char sendBuffer[256];
unsigned char rcvBuffer[150];
volatile char status;
volatile int index;
volatile char state;
volatile unsigned char command;
volatile unsigned char sendCommand;
volatile unsigned char lastCommand;
volatile int length;
volatile int lengthNdx;

bool sendOnSpi = false;
volatile char *outMsg;
volatile int outLength;
String idleResp = "idle";
String runResp = "run";
String initResponse = "Ready";
volatile int pollCount = 0;

int counter = 0;

#define PINGCMD 2
#define POLL 3
#define MSG 7
#define ZVIBES 30
#define RUNCMD 20
#define GETHITDATA 21
#define ACK 0x7F


const int greenRelay = 24;
const int redRelay = 28;
const int yellowRelay = 26; 
const int Lenable3and4 = 6;
const int LIN3 = 7;
const int LIN4 = 5;
const int Hallupper = 47;
const int Hallmid = 45;
const int Halllower = 43;
const int analogPinx = 5;
const int analogPiny = 6;
const int analogPinz = 7;
int maxuptime = 550;
int maxdowntime = 475;
int targetholdtime = 1000;
int hitdiff = 50;
char pgmname[] = "exerciseall-ver2";
int i, j, loopcount, deploycount, deploy, hitcount, hit, dir, olddir, lowerlimit, upperlimit, goingup, goingdown, movecycles, moveerror, itemnumber, tm, tmh, jobnumber, prtctl;
unsigned long lowermicros, uppermicros,startmicros, startmillis, stopmicros, runtime,testmillis, starttest, prevhittime;
unsigned long hittime, hitinterval, startuptime, startdowntime, curdowntime, curuptime, targetholdstart, vibereadstart, vibereadend, vibereadtime;
unsigned long uploopcount, reportmills, reportintervalmills, hallreportstartmicros, hallreadmicros, stoptime, motorelaxdone;
int hallU, hallM, hallL;
int UPsteplength, DOWNsteplength;
int dohallML, dohallMH, dohalftimereport;
int xval;
int yval;
int zval;
int xwork, xrest, xposthreshold, xnegthreshold, oldhighxval;
int ywork, yrest, yposthreshold, ynegthreshold, oldhighyval;
int zwork, zrest, zposthreshold, znegthreshold, oldhighzval;
unsigned long gethitstart, microswork, zhittime, zpintime;

void setpins(){
  pinMode(greenRelay, OUTPUT);
  pinMode(redRelay, OUTPUT);
  pinMode(yellowRelay, OUTPUT);
  digitalWrite(greenRelay,LOW);
  digitalWrite(yellowRelay,LOW);
  digitalWrite(redRelay,LOW);
  pinMode(Lenable3and4, OUTPUT);
  pinMode(LIN3, OUTPUT);
  pinMode(LIN4, OUTPUT);
  pinMode(Halllower, INPUT_PULLUP);
  pinMode(Hallmid, INPUT_PULLUP);
  pinMode(Hallupper, INPUT_PULLUP);
  digitalWrite(Lenable3and4, LOW);
  } // end setpins()

void getmasterdata(){
 masterdatatop:
 Serial.println("MASTER DATA:  to change enter the item number OR 99 to accept");
 Serial.println("ITEM    Value   Discreption");
 Serial.println("1.  "+String(maxuptime)+"  maxuptime.");
 Serial.println("2.  "+String(maxdowntime)+"  maxdowntime.");
 Serial.println("3.  "+String(targetholdtime)+"  targetholdtime.");
 Serial.println("4.  "+String(hitdiff)+"  hitdiff.");
 while (!Serial.available()){ ;}
  delay(50);
  while(Serial.available())
  {
    itemnumber = Serial.parseInt();
    if(Serial.read() != '\n'){Serial.println("changing item "+String(itemnumber)+"  enter new value.");}
  } 
  switch (itemnumber) {
    case 1:
    while (!Serial.available()){ ;}
    delay(50);
    while(Serial.available())
    {
    maxuptime = Serial.parseInt();
    if(Serial.read() != '\n'){Serial.println("maxuptime = "+String(maxuptime)+".");}
    } 
    break;
    case 2:
      while (!Serial.available()){ ;}
      delay(50);
      while(Serial.available())
      {
      maxdowntime = Serial.parseInt();
      if(Serial.read() != '\n'){Serial.println("maxuptime = "+String(maxuptime)+".");}
      } 
      break;
    case 3:
    while (!Serial.available()){ ;}
      delay(50);
      while(Serial.available())
      {
      targetholdtime = Serial.parseInt();
      if(Serial.read() != '\n'){Serial.println("targetholdtime = "+String(targetholdtime)+".");}
      } 
      break;
     case 4:
    while (!Serial.available()){ ;}
      delay(50);
      while(Serial.available())
      {
      hitdiff = Serial.parseInt();
      if(Serial.read() != '\n'){Serial.println("hitdiff = "+String(hitdiff)+".");}
      } 
      break;
    case 99:
      goto masterdatagood;
      break;
    default: 
       break;
  } // end of switch
  goto masterdatatop;
  masterdatagood:
  Serial.println("DONE in getmasterdata().");
} // end getmasterdata()


void getjob(){
  getjobtop:
 Serial.println("GETJOB():  enter the item number to run");
 Serial.println("ITEM    function   Discreption");
 Serial.println("0,  getLocalStatus() get local status");
 Serial.println("1.  getmasterdata()  change values of constants");
 Serial.println("2.  restvibes()   establish rest values of accelarometers");
 Serial.println("3.  testvibes()   gather 3 axis data for user specified time");
 Serial.println("4.  zvibes()      gather z axis data for user specified time");
 Serial.println("5.  testhall()    gather data from Hall sensors for user specified time");
 Serial.println("6.  testmove()    target moves up-and-down user specified number of times and schedule");
 Serial.println("7.  testmovehit() target moves up user specified number of times and down when HIT");
 Serial.println("8.  motogotime()  motor runs user specified direction & time");
 Serial.println("9.  motogosteps()  motor runs up and down in steps of user defined time, reporting sensor data all the way.");
 Serial.println("10. lightsout()  turns off any lights on.");
 Serial.println("11. 8266-test1()  don't know what it does today");
 Serial.println("99. getjob() EXIT");
 while (!Serial.available()){ 
  handleSpi();
 }
  delay(50);
  while(Serial.available())
  {
    jobnumber = Serial.parseInt();
    if(Serial.read() != '\n'){Serial.println("going to "+String(jobnumber));}
  } 
  switch (jobnumber) {
    case 0:
      getLocalStatus();
      break;
    case 1:
    getmasterdata(); 
    break;
    case 2:
      restvibes(); 
      break;
    case 3:
      testvibes();
      break;
     case 4:
      zvibes(); 
      break;
    case 5:
      testhall(); 
      break;
    case 6:
      testmove(); 
      break;
    case 7:
      testmovehit(); 
      break;
    case 8:
      case8error:
      Serial.println("ENTER: milliseconds of runtime ,dir 0 = down, dir 1 = up");
      Serial.println("OR 9,9 to exit motogotime().");
      while (!Serial.available()){ ;}
      delay(50);
      while(Serial.available())
      {
      runtime = Serial.parseInt();
      dir = Serial.parseInt();
      if(Serial.read() != '\n'){Serial.println("IN getjob() dir, runtime = " +String(dir)+","+String(runtime));}
      }
      if(runtime == 9 || dir == 9) goto getjobgood;
      if(dir < 0 || dir > 1) goto case8error;
      if(runtime < 10  || runtime > 800) goto case8error;
      motogotime();
      goto case8error; 
      break;
    case 9:
      motogosteps();
      break;
    case 10:
      lightsout();
      break;
    case 99:
      goto getjobgood;
      break;
    default: 
       goto getjobtop;;
  } // end of switch
  getjobgood:
  Serial.println("DONE in getjob().");
} // end getjob()

void lightsout(){
  digitalWrite(redRelay, LOW);digitalWrite(yellowRelay, LOW);digitalWrite(greenRelay, LOW);
} // end of lights out

void getLocalStatus() {
  Serial.println("========== Local Status ===========");
  if (sendOnSpi) Serial.println("Send on SPI is true?");
  Serial.println(String("Status is ") + String((int)status));
  Serial.println(String("Poll Count is ") + String(pollCount));
  Serial.println("========== Local Status ===========");
}

void setup() {
  Serial.begin(9600);
  Serial.println(pgmname);
  setpins();
  loopcount = 0;
  runtime = 0;
  upperlimit = 1;
  lowerlimit = 1;
  digitalWrite(greenRelay,HIGH);
  delay(1000);
  digitalWrite(greenRelay,LOW);
  digitalWrite(yellowRelay,HIGH);
  delay(1000);
  digitalWrite(yellowRelay,LOW);
  digitalWrite(redRelay,HIGH);
  delay(1000);
  digitalWrite(redRelay,LOW);
  Serial.println("DONE w/ setup()");
  restvibes();

  // setup of SPI interface
  
  SPCR |= bit(SPE);      /* Enable SPI */
  pinMode(MISO, OUTPUT); /* Make MISO pin as OUTPUT (slave) */

  resetState();

  SPI.attachInterrupt(); /* Attach SPI interrupt */
  Serial.println("setup complete");

  status = STATUS_IDLE;

  }  // end setup

void loop() {
  loopcount++;
  getjob();
}  // end of loop()

void testwork(){
  Serial.println("DOING testwork()");
  getmasterdata();
  restvibes();
  testvibes();
  zvibes();
  testhall();
  testmove();
  testmovehit();
  Serial.println("DONE testwork()");
} // end of testwork()

void testmove(){
  tmerror1:
  Serial.println("IN testmove() ENTER: direction to start(MUST be 1 or 9), number of move cycles:");
  while (!Serial.available()){ ;}
  delay(50);
  while(Serial.available())
  {
    dir = Serial.parseInt();
    movecycles = Serial.parseInt();
    if(Serial.read() != '\n'){Serial.println("Entered dir, cycles = "+String(dir)+" , "+String(movecycles)+".");}
  }
  if(dir == 9) {Serial.println("LEAVING testmove() w/ dir = 9."); return;}
  if(dir != 1){Serial.println("ERROR! slide MUST be at bottom to start this function!"); goto tmerror1;}
  if(movecycles < 1  || movecycles > 10) goto tmerror1;
  lowerlimit = digitalRead(Halllower);
  upperlimit = digitalRead(Hallupper);
  if(lowerlimit == 0 || upperlimit == 0) goto tmerror1;
  for(tm = 0; tm < movecycles; tm++){
    motogolimit(); // goes up, then down
    if(moveerror == 1){moveerror = 0; Serial.print("MOVE ERROR! "); delay(3000); }
  }
  Serial.println("DONE with testmove().");
  goto tmerror1;
} // end of testmove()

void testmovehit(){
  tmherror1:
  Serial.println("IN testmovehit() ENTER: direction to start(MUST be 1 or 9), number of move cycles:");
  while (!Serial.available()){ ;}
  delay(50);
  while(Serial.available())
  {
    dir = Serial.parseInt();
    movecycles = Serial.parseInt();
    if(Serial.read() != '\n'){Serial.println("Entered dir, cycles = "+String(dir)+" , "+String(movecycles)+".");}
  }
  if(dir == 9) {Serial.println("LEAVING testmovehit() w/ dir = 9."); return;}
  if(dir != 1){Serial.println("ERROR! slide MUST be at bottom to start this function!"); goto tmherror1;}
  if(movecycles < 1  || movecycles > 10) goto tmherror1;
  lowerlimit = digitalRead(Halllower);
  upperlimit = digitalRead(Hallupper);
  if(lowerlimit == 0 || upperlimit == 0) goto tmherror1;
  for(tmh = 0; tmh < movecycles; tmh++){
    motogolimithit(); // goes up, then down
    if(moveerror == 1){moveerror = 0; Serial.print("MOVE ERROR! "); delay(3000); }
  }
  Serial.println("DONE with testmovehit().");
  goto tmherror1;
} // end of testmovehit()

void motogosteps(){
  motogostepserror1:
  Serial.println("IN motogosteps() ENTER: direction to start(MUST be 1 or 9), UP step length in millis, DOWN step length in millis:");
  Serial.println("UP step MUST BE between 10 and 100.");
  Serial.println("DOWN step MUST BE between 10 and 50.");
  while (!Serial.available()){ ;}
  delay(50);
  while(Serial.available())
  {
    dir = Serial.parseInt();
    UPsteplength = Serial.parseInt();
    DOWNsteplength = Serial.parseInt();
    if(Serial.read() != '\n'){Serial.println("Entered dir, UPsteplength, DOWNsteplength = "+String(dir)+" , "+String(UPsteplength)+", "+ String(DOWNsteplength)+".");}
  }
  if(dir == 9) {Serial.println("LEAVING motogosteps() w/ dir = 9."); return;}
  if(dir != 1){Serial.println("ERROR! slide MUST be at bottom to start this function!"); goto motogostepserror1;}
  if(UPsteplength < 10  || UPsteplength > 500) goto motogostepserror1;
  if(DOWNsteplength < 10  || DOWNsteplength > 50) goto motogostepserror1;
  lowerlimit = digitalRead(Halllower);
  upperlimit = digitalRead(Hallupper);
  if(lowerlimit == 0 || upperlimit == 0) goto motogostepserror1;
    vibereport();
    hallreport();
    digitalWrite(Lenable3and4,LOW);
    if(dir == 1){
      moveerror = 0;
      digitalWrite(LIN3, LOW); //make it go up
      digitalWrite(LIN4, HIGH);
      Serial.println();
      Serial.println("IN motogosteps() STARTING dir = "+String(dir)+ "(U,L) at millis = ("+ String(upperlimit)+","+String(lowerlimit)+") "+ String(millis()));
      digitalWrite(Lenable3and4,HIGH);  //motor starts here
      startuptime = millis();
      // setup of loop of moving up
      uploopcount = 0;
      upperlimit = digitalRead(Hallupper);
      // dohallML = 1;
      // dohallMH = 1;
      dohalftimereport = 1;
      // actual loop of moving up
      while(upperlimit != 0 && (millis() - startuptime) < UPsteplength){
       upperlimit = digitalRead(Hallupper);
       uploopcount++;
       upperlimit = digitalRead(Hallupper);
      } // end of loop moving up
      stoptime = millis();
      digitalWrite(Lenable3and4, LOW); // stop the motor
      motorelax(); //put on the brakes
      motorelaxdone = millis();
      Serial.println("DONE moving up in motogosteps(): startuptime, uploopcount, motor off , motorelax() done at millis = "+String(startuptime)+","+String(uploopcount)+","+String(stoptime)+","+String(motorelaxdone));
      Serial.println("dohallML, dohallMH, dohalftimereport: "+String(dohallML)+","+String(dohallMH)+","+String(dohalftimereport)+".");
      vibereport();
      hallreport();
    }
}  // end of motogosteps() FOR TESTING SUNDAY
 

void motogolimit(){
  digitalWrite(Lenable3and4,LOW);
    if(dir == 1){
      moveerror = 0;
      digitalWrite(LIN3, LOW); //make it go up
      digitalWrite(LIN4, HIGH);
      Serial.println();
      Serial.println("IN motogolimit() STARTING dir = "+String(dir)+ "(U,L) at millis = ("+ String(upperlimit)+","+String(lowerlimit)+") "+ String(millis()));
      digitalWrite(Lenable3and4,HIGH);  //motor starts here
      startuptime = millis();
      prevhittime = startuptime;
      prtctl = 0;
      vibereport();
      hallreport();
      reportmills = millis();
      // loop of moving up
      uploopcount = 0;
      reportintervalmills = 50;
      while(upperlimit != 0){
       uploopcount++;
       if((millis() - reportmills) > reportintervalmills){
        vibereport();
        hallreport();
        reportmills = millis();
       }
       lowerlimit = digitalRead(Halllower);
       upperlimit = digitalRead(Hallupper);
       curuptime = millis() - startuptime;
       if(curuptime > maxuptime){ 
       moveerror = 1;
       Serial.println("ERROR moveup in motogolimit()= timedout at "+String(curuptime)+" mills of up time.");
       break;
       }
       Serial.println("DONE loop of moving up.  uploopcount = "+String(uploopcount)+".");
       // end of loop moving up
      zval = analogRead(analogPinz);
      zpintime = millis();
      if(zval > zposthreshold || zval < znegthreshold) {hit = 1; Serial.print("z"); digitalWrite(greenRelay,HIGH);}
      if(zval > (zposthreshold + 50) || zval < (znegthreshold - 50)) {hit = 1; Serial.print("Z"); digitalWrite(greenRelay,HIGH);}
      if(hit == 1){
        hittime = zpintime;
        hitinterval = hittime - prevhittime;
        prevhittime = hittime;
        hit = 0;
        Serial.print(" zdiff,hitinterval ("+String(zval - zposthreshold)+", ");
        Serial.println(String(hitinterval)+")");
      }
       if((millis() - hittime) > 50) digitalWrite(greenRelay, LOW);
       Serial.print(String(upperlimit));
       prtctl++;
       if((prtctl + 1) == 20){prtctl = 0; Serial.println();}
       }
     digitalWrite(Lenable3and4,LOW);
     motorelax();
     if(moveerror == 1)Serial.println("ERROR moveup in motogolimit() = timedout at "+String(curuptime)+" mills of up time.");
     if(moveerror == 0)Serial.println("IN motogolimit() UPPER Hall good.");
     dir = 0;
     targetholdstart = millis();
     Serial.println("IN motogolimit() targethold.");
     digitalWrite(redRelay,HIGH);  // why do this?
     while( (millis() - targetholdstart) < targetholdtime) {
      zval = analogRead(analogPinz);
      zpintime = millis();
      if(zval > zposthreshold || zval < znegthreshold) {hit = 1; Serial.print("z"); digitalWrite(greenRelay,HIGH);}
      if(zval > (zposthreshold + 50) || zval < (znegthreshold - 50)) {hit = 1; Serial.print("Z"); digitalWrite(greenRelay,HIGH);}
      if(hit == 1){
        hittime = zpintime;
        hitinterval = hittime - prevhittime;
        prevhittime = hittime;
        hit = 0;
        Serial.print(" zdiff,hitinterval ("+String(zval - zposthreshold)+", ");
        Serial.println(String(hitinterval)+")");
      }
      if((millis() - hittime) > 50) digitalWrite(greenRelay, LOW);
      } // end of target hold while()
      digitalWrite(redRelay,LOW);  // per red above, the light is on during targetholdup period
      Serial.println("OUT of target hold.");
    }
    if(dir == 0){
      moveerror = 0;
      digitalWrite(LIN3, HIGH); // make it go down
      digitalWrite(LIN4, LOW);
      Serial.println();
      Serial.println("IN motogolimit() STARTING dir = "+String(dir)+ "(U,L) at millis = ("+ String(upperlimit)+","+String(lowerlimit)+") "+ String(millis()));
      digitalWrite(Lenable3and4,HIGH);
      startdowntime = millis();
      prtctl = 0;
      vibereport();
      while(lowerlimit != 0){
       lowerlimit = digitalRead(Halllower);
       upperlimit = digitalRead(Hallupper);
       curdowntime = millis() - startdowntime;
       if(curdowntime > maxdowntime){ 
       moveerror = 1;
       break;
       }
      zval = analogRead(analogPinz);
      zpintime = millis();
      if(zval > zposthreshold || zval < znegthreshold) {hit = 1; Serial.print("z"); digitalWrite(greenRelay,HIGH);}
      if(zval > (zposthreshold + 50) || zval < (znegthreshold - 50)) {hit = 1; Serial.print("Z"); digitalWrite(greenRelay,HIGH);}
      if(hit == 1){
        hittime = zpintime;
        hitinterval = hittime - prevhittime;
        prevhittime = hittime;
        hit = 0;
        Serial.print(" zdiff,hitinterval ("+String(zval - zposthreshold)+", ");
        Serial.println(String(hitinterval)+")");
      }
      if((millis() - hittime) > 50) digitalWrite(greenRelay, LOW); 
       Serial.print(String(lowerlimit));
       prtctl++;
       if((prtctl + 1) == 20){prtctl = 0; Serial.println();}
       }
     digitalWrite(Lenable3and4,LOW);
     motorelax();
     if(moveerror == 1)Serial.println("ERROR movedown in motogolimit() = timedout at "+String(curdowntime)+" mills of up time.");
     if(moveerror == 0)Serial.println("IN motogolimit LOWER Hall good.");
     dir = 1;
     delay(50);  
    }
Serial.println("DONE in motogolimit().");  
} // end motogolimit()

void motogolimithit(){
  digitalWrite(Lenable3and4,LOW);
    if(dir == 1){
      moveerror = 0;
      digitalWrite(LIN3, LOW); //make it go up
      digitalWrite(LIN4, HIGH);
      Serial.println();
      Serial.println("IN motogolimithit() STARTING dir = "+String(dir)+ "  (U,L) at millis = ("+ String(upperlimit)+","+String(lowerlimit)+") "+ String(millis()));
      digitalWrite(Lenable3and4,HIGH);
      startuptime = millis();
      prevhittime = startuptime;
      prtctl = 0;
      vibereport();
      while(upperlimit != 0){
       lowerlimit = digitalRead(Halllower);
       upperlimit = digitalRead(Hallupper);
       curuptime = millis() - startuptime;
       if(curuptime > maxuptime){ 
       moveerror = 1;
       Serial.println("ERROR moveup in motogolimithit()= timedout at "+String(curuptime)+" mills of up time.");
       break;
       }
      zval = analogRead(analogPinz);
      zpintime = millis();
      if(zval > zposthreshold || zval < znegthreshold) {hit = 1; Serial.print("z"); digitalWrite(greenRelay,HIGH);}
      if(zval > (zposthreshold + 50) || zval < (znegthreshold - 50)) {hit = 1; Serial.print("Z"); digitalWrite(greenRelay,HIGH);}
      if(hit == 1){
        hittime = zpintime;
        hitinterval = hittime - prevhittime;
        prevhittime = hittime;
        hit = 0;
        Serial.print(" zdiff,hitinterval ("+String(zval - zrest)+", ");
        Serial.println(String(hitinterval)+")");
        goto gothit;
      }
       if((millis() - hittime) > 50) digitalWrite(greenRelay, LOW);
       Serial.print(String(upperlimit));
       prtctl++;
       if((prtctl + 1) == 40){prtctl = 0; Serial.println();}
       }
     digitalWrite(Lenable3and4,LOW);
     motorelax();
     if(moveerror == 1)Serial.println("ERROR in motogolimithit() moveup = timedout at "+String(curuptime)+" mills of up time.");
     if(moveerror == 0)Serial.println("IN motogolimithit UPPER Hall good.");
     targetholdstart = millis();
     Serial.println("IN targethold.");
     digitalWrite(redRelay,HIGH);
     while( (millis() - targetholdstart) < targetholdtime) {
      zval = analogRead(analogPinz);
      zpintime = millis();
      if(zval > zposthreshold || zval < znegthreshold) {hit = 1; Serial.print("z"); digitalWrite(greenRelay,HIGH);}
      if(zval > (zposthreshold + 50) || zval < (znegthreshold - 50)) {hit = 1; Serial.print("Z"); digitalWrite(greenRelay,HIGH);}
      if(hit == 1){
        hittime = zpintime;
        hitinterval = hittime - prevhittime;
        prevhittime = hittime;
        hit = 0;
        Serial.print(" zdiff,hitinterval ("+String(zval - zrest)+", ");
        Serial.println(String(hitinterval)+")");
        goto gothit;
      }
      if((millis() - hittime) > 50) digitalWrite(greenRelay, LOW);
      } // end of target hold while()
      digitalWrite(redRelay,LOW);
      Serial.println("OUT of target hold.");
      gothit:
      dir = 0;
    }
    if(dir == 0){
      moveerror = 0;
      digitalWrite(LIN3, HIGH); // make it go down
      digitalWrite(LIN4, LOW);
      Serial.println();
      Serial.println("IN motogolimithit() STARTING dir = "+String(dir)+ "(U,L) at millis = ("+ String(upperlimit)+","+String(lowerlimit)+") "+ String(millis()));
      digitalWrite(Lenable3and4,HIGH);
      startdowntime = millis();
      prtctl = 0;
      vibereport();
      while(lowerlimit != 0){
       lowerlimit = digitalRead(Halllower);
       upperlimit = digitalRead(Hallupper);
       curdowntime = millis() - startdowntime;
       if(curdowntime > maxdowntime){ 
       moveerror = 1;
       break;
       }
      zval = analogRead(analogPinz);
      zpintime = millis();
      if(zval > zposthreshold || zval < znegthreshold) {hit = 1; Serial.print("z"); digitalWrite(greenRelay,HIGH);}
      if(zval > (zposthreshold + 50) || zval < (znegthreshold - 50)) {hit = 1; Serial.print("Z"); digitalWrite(greenRelay,HIGH);}
      if(hit == 1){
        hittime = zpintime;
        hitinterval = hittime - prevhittime;
        prevhittime = hittime;
        hit = 0;
        Serial.print(" zdiff,hitinterval ("+String(zval - zrest)+", ");
        Serial.println(String(hitinterval)+")");
      }
      if((millis() - hittime) > 50) digitalWrite(greenRelay, LOW); 
       Serial.print(String(lowerlimit));
       prtctl++;
       if((prtctl + 1) == 40){prtctl = 0; Serial.println();}
       }
     digitalWrite(Lenable3and4,LOW);
     motorelax();
     if(moveerror == 1)Serial.println("ERROR movedown in motogolimithit() = timedout at "+String(curdowntime)+" mills of up time.");
     if(moveerror == 0)Serial.println("IN motogolimithit() LOWER Hall good.");
     dir = 1;
     delay(50);  
    }
Serial.println("DONE in motogolimithit()IN .");  
} // end motogolimithit()

void hallreport(){
  hallreportstartmicros = micros();
  hallU=digitalRead(Hallupper);
  hallM=digitalRead(Hallmid);
  hallL=digitalRead(Halllower);
  hallreadmicros = (micros() - hallreportstartmicros);
  Serial.println("IN hallreport() at micros: "+String(hallreportstartmicros)+ ".  U, M, L = "+String(hallU)+","+String(hallM)+","+String(hallL)+".  Read time micros = "+String(hallreadmicros)+"."); 
}


void vibereport(){
      vibereadstart = micros();
      xval = analogRead(analogPinx);
      yval = analogRead(analogPiny);
      zval = analogRead(analogPinz);
      vibereadend = micros();
      vibereadtime = vibereadend - vibereadstart;
      Serial.println();
      Serial.print("IN vibereport() vibes diff from rest & read micros:  ");
      Serial.print("("+String(xval - xrest)+" , ");
      Serial.print(String(yval - yrest)+" , ");
      Serial.print(String(zval - zrest)+") ");
      Serial.println(String(vibereadtime));
} // end vibereport()

void zvibes(){
  // Commands now come across serial from 8266
// / Serial.println("IN zvibes() ENTER: milliseconds to test");
//  while (!Serial.available()){ ;}
//  delay(50);
//  while(Serial.available())
//  {
//    testmillis = Serial.parseInt();
//    if(Serial.read() != '\n'){Serial.println("Entered testmillis = "+String(testmillis));}
//  }
//  Serial.println("test while redRelay is on!");
Serial.println("test length in millis:  "+String(testmillis));
  digitalWrite(redRelay,HIGH);
  starttest = millis();
  prevhittime = starttest;
  Serial.print("(zval minus rest zval)");
  Serial.println("START time zvibes() = "+String(prevhittime));
  bool anyHits = false;
  while(millis() - starttest < testmillis){
    if (NOHW) break;
      zval = analogRead(analogPinz);
      zpintime = millis();
      hit = 0;
      if(zval > zposthreshold || zval < znegthreshold) {hit = 1; Serial.print("z");}
      if(zval > (zposthreshold + 50) || zval < (znegthreshold - 50)) {hit = 1; Serial.print("Z");}
      if(hit == 1){
        anyHits = true;
        hittime = zpintime;
        hitinterval = hittime - prevhittime;
        prevhittime = hittime;
        hit = 0;
        String temp = String(zval - zrest)+", "+String(hitinterval)+"/n";
        Serial.println(temp);
        temp.toCharArray(sendBuffer, 256);
        sendToPeer(ZVIBES, sendBuffer, temp.length());
      }
   }
   if (!anyHits) {
    String none = "No hits";
    none.toCharArray(sendBuffer, 256);
    sendToPeer(ZVIBES, sendBuffer, none.length());
   }
  digitalWrite(redRelay, LOW);
  Serial.println("DONE zvibes().");
}// end zvibes()


void testvibes(){
  Serial.println("IN testvibes() ENTER: milliseconds to test");
  while (!Serial.available()){ ;}
  delay(50);
  while(Serial.available())
  {
    testmillis = Serial.parseInt();
    if(Serial.read() != '\n'){Serial.println("Entered testmillis = "+String(testmillis));}
  }
  Serial.println("test while redRelay is on!");
  digitalWrite(redRelay,HIGH);
  starttest = millis();
  prevhittime = starttest;
  Serial.print("(x,y,zvals minus rest val)");
  Serial.println("START time testvibes() = "+String(prevhittime));
  while(millis() - starttest < testmillis){
      xval = analogRead(analogPinx);
      yval = analogRead(analogPiny);
      zval = analogRead(analogPinz);
      hit = 0;
      if(xval > xposthreshold || xval < xnegthreshold) {hit = 1; Serial.print("x");}
      if(yval > yposthreshold || yval < ynegthreshold) {hit = 1; Serial.print("y");}
      if(zval > zposthreshold || zval < znegthreshold) {hit = 1; Serial.print("z");}
      if(hit == 1){
        hittime = millis();
        hitinterval = hittime - prevhittime;
        prevhittime = hittime;
        hit = 0;
        Serial.print("("+String(xval - xrest)+" , ");
        Serial.print(String(yval - yrest)+" , ");
        Serial.print(String(zval - zrest)+") ");
        Serial.println(String(hitinterval));
      }
   }
  digitalWrite(redRelay, LOW);
  Serial.println("DONE testvibes().");
}// end testvibes()

void testhall(){
  errorhall1:
  Serial.println("IN testhall() ENTER: milliseconds for test");
  while (!Serial.available()){ ;}
  delay(50);
  while(Serial.available())
  {
    testmillis = Serial.parseInt();
    if(Serial.read() != '\n'){ Serial.println("ENTERED testmillis = "+String(testmillis));}
  }
  Serial.println("test uppwer while redRelay is on!");
  digitalWrite(redRelay,HIGH);
  starttest = millis();
  i = 0;
  while(millis() - starttest < testmillis){
      Serial.print(digitalRead(Hallupper));
      i++;
      if(i == 30){Serial.println(" "+String(millis())); i = 0;}
  }
  digitalWrite(redRelay, LOW);
  Serial.println("DONE testing upper");
Serial.println("test middle while yellowRelay is on!");
  digitalWrite(yellowRelay,HIGH);
  starttest = millis();
  i = 0;
  while(millis() - starttest < testmillis){
      Serial.print(digitalRead(Hallmid));
      i++;
      if(i == 30){Serial.println(" "+String(millis())); i = 0;}
  }
  digitalWrite(yellowRelay, LOW);
  Serial.println("DONE testing middle");
  Serial.println("test lower while greenRelay is on!");
  digitalWrite(greenRelay,HIGH);
  starttest = millis();
  i = 0;
  while(millis() - starttest < testmillis){
      Serial.print(digitalRead(Halllower));
      i++;
      if(i == 30){Serial.println(" "+String(millis())); i = 0;}
  }
  digitalWrite(greenRelay, LOW);
  Serial.println("DONE testing lower");
  Serial.println("DONE testhall().");
} // end of testhall()

void motoworktime(){
    digitalWrite(Lenable3and4,LOW);
    if(dir == 0){
      digitalWrite(LIN3, HIGH); //make it go down
      digitalWrite(LIN4, LOW);
    }
    if(dir == 1){
      digitalWrite(LIN3, LOW); // make it go up
      digitalWrite(LIN4, HIGH);
    }
    // make a header fior data give dir & runtime & key to data!!!
    Serial.println("IN motoworktime() direction, runtime = "+String(dir)+","+String(runtime));
    Serial.println("LOG DATA: timestamp, lowerval, upperval, xval, yval, zval"); 
    digitalWrite(greenRelay, HIGH);
    digitalWrite(Lenable3and4, HIGH); //start motor
    startmillis = millis();
    i = 0;
    while(millis() - startmillis < runtime){
    i++;
    if(digitalRead(Halllower) == 0) Serial.print("L");
    if(digitalRead(Hallupper) == 0) Serial.print("U");
      if(i == 5){ 
        Serial.print(String(millis())+" , ");
        Serial.print(String(digitalRead(Halllower))+" , ");
        Serial.print(String(digitalRead(Hallupper))+" , ");
        Serial.print(String(xrest - analogRead(analogPinx))+" , ");
        Serial.print(String(yrest - analogRead(analogPiny))+" , ");
        Serial.print(String(zrest - analogRead(analogPinz))+".");
        Serial.println();
        i = 0;}  
    }
    digitalWrite(Lenable3and4, LOW); //stop motor
    motorelax();
    digitalWrite(greenRelay, LOW);
}  // end motoworktime

void motorelax(){
  digitalWrite(Lenable3and4, LOW);
  runtime = 10;
  Serial.println("IN motorelax(). dir, runtime = "+String(dir)+","+String(runtime));
  if(dir == 0){ dir = 1; motogotime(); dir = 0;}
  if(dir == 1){dir = 0; motogotime(); dir = 1;}
  Serial.println("DONE motorelax.");
}
void motogoup(){
  Serial.println("IN motogoup().");
      digitalWrite(LIN3, LOW); // make it go up
      digitalWrite(LIN4, HIGH);
      upperlimit = 1;
      startmicros = micros();
      digitalWrite(Lenable3and4, HIGH); //start motor 
}

void motogodown(){
      digitalWrite(LIN3, HIGH); //make it go down
      digitalWrite(LIN4, LOW);
      lowerlimit = 1;
      startmicros = micros();
      digitalWrite(Lenable3and4, HIGH); //start motor 
}

 void motogotime(){
    digitalWrite(Lenable3and4,LOW);
    Serial.println("IN motogotime.  dir, runtime = "+String(dir)+","+String(runtime));
    if(dir == 0){
      digitalWrite(LIN3, HIGH); //make it go down
      digitalWrite(LIN4, LOW);
    }
    if(dir == 1){
      digitalWrite(LIN3, LOW); // make it go up
      digitalWrite(LIN4, HIGH);
    }
    digitalWrite(greenRelay, HIGH);
    digitalWrite(Lenable3and4, HIGH); //start motor
    delay(runtime);  
    digitalWrite(Lenable3and4, LOW); //stop motor
    digitalWrite(greenRelay, LOW);
    Serial.println("DONE in motogotime.");
}  // end motogotime

void restvibes(){
  digitalWrite(yellowRelay,HIGH);
  delay(250);  
  for(i = 0; i < 10; i++){
  xval = analogRead(analogPinx);
  yval = analogRead(analogPiny);
  zval = analogRead(analogPinz);
  xwork = xwork + xval;
  ywork = ywork + yval;
  zwork = zwork + zval;
  }
  digitalWrite(yellowRelay,LOW);
  xrest = xwork/10;
  yrest = ywork/10;
  zrest = zwork/10;
  xposthreshold = xrest + hitdiff;
  yposthreshold = yrest + hitdiff;
  zposthreshold = zrest + hitdiff;
  xnegthreshold = xrest - hitdiff;
  ynegthreshold = yrest - hitdiff;
  znegthreshold = zrest - hitdiff;
  xval = 0; yval = 0; zval = 0; xwork = 0; ywork = 0; zwork = 0;
  
  Serial.println("DONE restvibes(), xrest, xposthreshold, xnegthreshold = " + String(xrest) + " , " + String(xposthreshold)+" , "+String(xnegthreshold));
  Serial.println("DONE restvibes(), yrest, yposthreshold, ynegthreshold = " + String(yrest) + " , " + String(yposthreshold)+" , "+String(ynegthreshold));
  Serial.println("DONE restvibes(), zrest, zposthreshold, znegthreshold = " + String(zrest) + " , " + String(zposthreshold)+" , "+String(znegthreshold));
  }// end restvibes


void passes(){
  error1:
  Serial.println("ENTER: milliseconds of runtime ,dir 0 = down, dir 1 = up");
  Serial.println("OR");
  Serial.println("TO TEST: enter 888, any-int");
  while (!Serial.available()){ ;}
  delay(50);
  while(Serial.available())
  {
    runtime = Serial.parseInt();
    dir = Serial.parseInt();
    if(Serial.read() != '\n'){Serial.println("IN passes() dir, runtime = " +String(dir)+","+String(runtime));}
  }
  if(runtime == 888){
  Serial.println("IN passes(), 888 runtime test next.");
  goto donehere;
  }
 if(dir < 0 || dir > 1) goto error1;
 if(runtime < 10  || runtime > 700) goto error1;
 donehere:
 Serial.println("DONE w/ passes() dir , runtime  = "+String(dir)+" , "+String(runtime));
} // end passes() 

// SPI interrupt routine
ISR(SPI_STC_vect)
{
  uint8_t oldsrg = SREG;
  cli();
  char c = SPDR;
  switch (state)
  {
  case RCVCMD:
    command = c;
    // Out of sync if command = 0xff
    if (command == 0xff) {
      // Send NULL in response
      SPDR = 0;
      break; 
    }
      
    state = RCVLEN;
    break;
  case RCVLEN:
    if (receiveLength(c, &length, &lengthNdx))
    {
      if (length > 0)
      {
        state = RCVDATA;
      }
      else
      {
        state = handleCommandISR();
      }
    }
    break;
  case RCVDATA:
    if (index < sizeof(rcvBuffer) - 1)
    {
      rcvBuffer[index++] = c;
      if (index == length)
      {
        state = handleCommandISR();
      }
    }
    break;
    // If in receive complete state - send 0xFF - main loop hasn't processed last message
  case RCVCOMP:
    SPDR = 0xFF;
    break;
  case SENDCMD:
    SPDR = sendCommand;
    state = SENDLEN;
    lastCommand = sendCommand;
    break;
  case SENDLEN:
    SPDR = (unsigned char)outLength;
    if (outLength > 0)
    {
      state = SENDDATA;
    }
    else
    {
      state = WAIT;
      if (lastCommand != POLL) {
        sendOnSpi = false;
      }
    }
    break;
  case SENDDATA:
    SPDR = *outMsg++;
    outLength--;
    if (outLength == 0)
    {
      if (lastCommand != POLL) {
        sendOnSpi = false;
      }
      state = WAIT;
    }
    break;
  case WAIT:
    // Ignore recv data as it was to trigger last SEND
    resetState();
    break;
  default:
    break;
  }
  SREG = oldsrg;
}

int handleCommandISR()
{
  switch (command)
  {
  case PINGCMD:
    sendCommand = PINGCMD;
    outMsg = &status;
    outLength = 1;
    SPDR = 0xFF;
    return SENDCMD;
    break;
  case POLL:
    if (!sendOnSpi) {
      pollCount++;
      // Send POLL response if not sending some other command
      sendCommand = POLL;
      outMsg = &status;
      outLength = 1;
      SPDR = 0xFF;
    }
    return SENDCMD;
  default:
    // Let loop() handle
    break;
  }
  SPDR = 0xFF;
  return RCVCOMP;
}

bool receiveLength(unsigned char c, volatile int *length, volatile int *lengthNdx)
{
  int ndx = (*lengthNdx)++;
  int l = *length;
  *length = ((c & 0x7f) << (7 * ndx)) + l;
  return c < 128;
}

void resetState()
{
  state = RCVCMD;
  index = 0;
  length = 0;
  lengthNdx = 0;
}

void handleSpi() {
  // State RCVCOMP means a command was received - process it
  if (state == RCVCOMP)
  {
    unsigned char locCommand = command;
        
    rcvBuffer[index] = 0;
    
    debugMsgInt("Command: ", command);
    debugMsgInt("Length: ", length);

    if (length > 0)
    {
      if (locCommand == ZVIBES) {
        testmillis = 0;
        for (int i = 0; i < length; i++) {
          testmillis = testmillis | (rcvBuffer[i] << (8 * i));
        }
      }
      else {
        Serial.println((char*)rcvBuffer);
      }
    }

    outLength = 0;
    sendCommand = POLL;  
    state = SENDCMD;

    switch (locCommand) {
      case RUNCMD:
        Serial.println("Run command received");
        status = STATUS_RUNNING;
      break;
      case GETHITDATA:
        Serial.println("Get hit data command received");
      break;
    }
  }
  delay(1);

}

void sendToPeer(unsigned char cmd, char* buffer, int len) {
  if (!sendOnSpi) {
    buffer[len] = 0;
    Serial.println(String("Send to peer: ") + buffer);
    sendCommand = cmd;
    outMsg = buffer;
    outLength = len;
    sendOnSpi = true;
    state = SENDCMD;
  }
}

#define DEBUG 1
void debugMsgInt(const char *msg, int value)
{
  if (DEBUG)
  {
    Serial.print(msg);
    Serial.println(value);
  }
}
