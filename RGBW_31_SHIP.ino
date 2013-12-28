/*********************---IR Decoder Credit---*****************************
 *  This is an example for the Adafruit NEC Remote Control               *
 *                                                                       *
 *  Designed specifically to work with the Adafruit NEC Remote Control   *
 *	----> http://www.adafruit.com/products/389                       *
 *  and IR Receiver Sensor                                               *
 *	----> http://www.adafruit.com/products/157                       *
 *                                                                       *
 *  These devices use IR to communicate, 1 pin is required to            *
 *  interface                                                            *
 *  Adafruit invests time and resources providing this open source code, *
 *  please support Adafruit and open-source hardware by purchasing       *
 *  products from Adafruit!                                              *
 *                                                                       *
 *  Written by Limor Fried/Ladyada for Adafruit Industries.              *
 *  BSD license, all text above must be included in any redistribution   *
 * ***********************************************************************
 * 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
 * IR Commands:                                           *
 * Remote Key           NEC Code       Command            *
 * -------------------------------------------------------*
 * vol-			0              rate -             *
 * play/pause		1              cycle start/stop   *
 * vol+			2              rate +             *
 * setup		4              white toggle       *
 * up			5              bright             *
 * stop/mode		6              all off            *
 * left			8              white down         *
 * enter/save		9              save to memory     *
 * right		10             white up           *
 * 0/+10		12             all on             *
 * down			13             dim                *
 * repeat		14             memory recall      *
 * 1			16             red up             *
 * 2			17             green up           *
 * 3			18             blue up            *
 * 4			20             red toggle         *
 * 5			21             green toggle       *
 * 6			22             blue toggle        *
 * 7			24             red down           *
 * 8			25             green down         *
 * 9			26             blue down          *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
 * 
 * http://www.thecustomgeek.com
 * RGBW LED Controller v 3.1IR
 * Jeremy Saglimbeni - 2013
 */
#include <EEPROM.h>
#include "Adafruit_NECremote.h"
#define IRpin         2
Adafruit_NECremote remote(IRpin);
char datain[11];
int i;
int stay; // delay in seconds to stay on each color in cycle mode
int rate; // rate of color cycle
int cyclego = 0;
int ramp;
int btn = 4; // button 1 input
int btn2 = 8; // button 2 input
int btnv;
int btnv2;
int mode; // manual mode
int w = 3; // pin 3 PWM output
int r = 9; // pin 9 PWM output
int g = 10; // pin 10 PWM output
int b = 11; // pin 11 PWM output
int whtlev; // current white level
int redlev; // current red level
int grnlev; // current green level
int blulev; // current blue level
int index;
int targetw; // desired white level
int targetr; // desired red level
int targetg; // desired green level
int targetb; // desired blue level
char digit1[1];
char digit2[1];
char digit3[1];
int redwake;
int grnwake;
int bluwake;
int whtwake;
int whiteout;
int redout;
int greenout;
int blueout;
int btnpressone;
int btnpresstwo;
int shortpresstwo = 500; // this defines the time for short vs long button press in ms
int shortpressone = 500; // this defines the time for short vs long button press in ms
int serialtime = 45; // how often serial checks are made
unsigned long prvserialtime;
unsigned long currenttime;
unsigned long btntwotime;
unsigned long btnonetime;
unsigned long buttontime;
int remval;
int oldc;
int irmode;
int lastcode = -1; // for ir library
int whitego;
int cyclepause;
String mystring;
String oldstring;

void setup() {
  pinMode(w, OUTPUT); // white output
  pinMode(r, OUTPUT); // red output
  pinMode(g, OUTPUT); // green output
  pinMode(b, OUTPUT); // blue output
  pinMode(btn, INPUT); // button 1
  pinMode(btn2, INPUT); // button 2
  digitalWrite(btn, HIGH); // turn on internal pull up resistor
  digitalWrite(btn2, HIGH); // turn on internal pull up resistor
  irmode = EEPROM.read(14); // read the last stored value before powering off
  if (irmode == 255) {
    irmode = 0;
  }
  if ((digitalRead(btn) == LOW) && (digitalRead(btn2) == LOW)) {
    if (irmode == 0) {
      irmode = 1;
      EEPROM.write(14, irmode);
      iron();
    }
    else {
      irmode = 0;
      EEPROM.write(14, irmode);
      iroff();
    }
  }
  ramp = EEPROM.read(1);
  rate = EEPROM.read(2);
  redwake = EEPROM.read(3);
  grnwake = EEPROM.read(4);
  bluwake = EEPROM.read(5);
  whtwake = EEPROM.read(6);
  mode = EEPROM.read(7);
  cyclego = EEPROM.read(8);
  cyclepause = EEPROM.read(9);

  if (rate == 255) {
    rate = 4;
    EEPROM.write(2, rate);
  }
  if (ramp == 255) {
    ramp = 4;
    EEPROM.write(1, ramp);
  }
  if (mode == 255) {
    mode = 0;
    EEPROM.write(7, mode);
  }
  if (cyclego == 255) {
    cyclego = 0;
    EEPROM.write(8, cyclego);
  }
  if (cyclepause == 255) {
    cyclepause = 1;
    EEPROM.write(9, cyclepause);
  }
  Serial.begin(9600); // enable serial communication at 9600 baud
  Serial.println(F("RGBW v3.1"));
  red(redwake);
  green(grnwake);
  blue(bluwake);
  white(whtwake);
}
void loop() {
    currenttime = millis();
    buttons();
    if (irmode == 1) {
    hearir(); // check the ir
  }
    if(currenttime - prvserialtime > serialtime) {
      prvserialtime = currenttime;
      serialchk();
    }
    if (cyclego == 1) {
      cycle();
      cyclepause = 0;
    }
}
void serialchk() {
  index = 0;
  while(Serial.available() > 0 && index < 9)
  {
    char aChar = Serial.read();
    datain[index] = aChar;
    index++;
    datain[index] = '\0'; // Keep the string NULL terminated   
  }
  String mystring = datain;
  if (mystring != oldstring) {

    if (mystring.startsWith("white")) {
    digit1[1] = datain[5];
    digit2[1] = datain[6];
    digit3[1] = datain[7];
    targetw = atoi(digit2);
    whiteout = targetw;
    targetw = targetw * 2.5;
    Serial.print(F("White: ")); 
    Serial.print(whiteout); 
    Serial.println(F("%"));
    if (targetw >= 250) {
      targetw = 255;
    }
    halt();
    white(targetw);
  }
    if (mystring.startsWith("red")) {
    digit1[1] = datain[3];
    digit2[1] = datain[4];
    digit3[1] = datain[5];
    targetr = atoi(digit2);
    redout = targetr;
    targetr = targetr * 2.5;
    Serial.print(F("Red: ")); 
    Serial.print(redout); 
    Serial.println(F("%"));
    if (targetr >= 250) {
      targetr = 255;
    }
    halt();
    red(targetr);
  }
  if (mystring.startsWith("green")) {
    digit1[1] = datain[5];
    digit2[1] = datain[6];
    digit3[1] = datain[7];
    targetg = atoi(digit2);
    greenout = targetg;
    targetg = targetg * 2.5;
    Serial.print(F("Green: ")); 
    Serial.print(greenout); 
    Serial.println("%");
    if (targetg >= 250) {
      targetg = 255;
    }
    halt();
    green(targetg);
  }
  if (mystring.startsWith("blue")) {
    digit1[1] = datain[4];
    digit2[1] = datain[5];
    digit3[1] = datain[6];
    targetb = atoi(digit2);
    blueout = targetb;
    targetb = targetb * 2.5;
    Serial.print(F("Blue: ")); 
    Serial.print(blueout); 
    Serial.println(F("%"));
    if (targetb>= 250) {
      targetb = 255;
    }
    halt();
    blue(targetb);
  }
    
    if (mystring.startsWith("goblue")) {
      Serial.println(F("Blue"));
      halt();
      blue(255);
      red(0);
      green(0);
    }
    
    if (mystring.startsWith("gogreen")) {
      Serial.println(F("Green")); 
      halt();
      green(255);
      red(0);
      blue(0);
    }
    
    if (mystring.startsWith("gored")) {
      Serial.println(F("Red"));
      halt();
      red(255);
      green(0);
      blue(0);
    }

    if (mystring.startsWith("cycle")) {
      Serial.println(F("Cycle Started!"));
      green(0);
      blue(0);
      red(255);
      cyclego = 1;
      EEPROM.write(8, cyclego);
    }
    if (mystring.startsWith("magenta")) {
      Serial.println(F("Magenta"));
      halt();
      magenta();
    }
    if (mystring.startsWith("cyan")) {
      Serial.println(F("Cyan"));
      halt();
      cyan();
    }
    if (mystring.startsWith("gold")) {
      Serial.println(F("Gold"));
      halt();
      gold();
    }
    if (mystring.startsWith("rgbwhite")) {
      Serial.println(F("RGB White"));
      halt();
      rgbwhite();
    }
    if (mystring.startsWith("orange")) {
      Serial.println(F("Orange"));
      halt();
      orange();
    }
    if (mystring.startsWith("ltblue")) {
      Serial.println(F("Light Blue"));
      halt();
      ltblue();
    }
    if (mystring.startsWith("ltgreen")) {
      Serial.println(F("Light Green"));
      halt();
      ltgreen();
    }
    if (mystring.startsWith("violet")) {
      Serial.println(F("Violet"));
      halt();
      violet();
    }
    if (mystring.startsWith("pink")) {
      Serial.println(F("Pink"));
      halt();
      pink();
    }
    if (mystring.startsWith("rgbww")) {
      Serial.println(F("Warm White"));
      halt();
      rgbww();
    }

    if (mystring.startsWith("rate")) {
      digit1[1] = datain[4];
      digit2[1] = datain[5];
      digit3[1] = datain[6];
      rate = atoi(digit2);
      Serial.print(F("Rate: ")); 
      Serial.println(rate);
      EEPROM.write(2, rate);
    }
    
    if (mystring.startsWith("pause")) {
        cyclego = 0;
        EEPROM.write(8, cyclego);
        cyclepause = 1;
        EEPROM.write(9, cyclepause);
        mode = 8;
        Serial.println(F("Cycle Paused."));
        EEPROM.write(3, redlev);
        EEPROM.write(4, grnlev);
        EEPROM.write(5, blulev);
        EEPROM.write(6, whtlev);
    }
    
    if (mystring.startsWith("stay")) {
      digit1[1] = datain[4];
      digit2[1] = datain[5];
      digit3[1] = datain[6];
      stay = atoi(digit2) * 1000;
      Serial.print(F("Stay: ")); 
      Serial.print(stay / 1000); 
      Serial.println(F(" Seconds"));
    }
    if (mystring.startsWith("ramp")) {
      digit1[1] = datain[4];
      digit2[1] = datain[5];
      digit3[1] = datain[6];
      ramp = atoi(digit2);
      Serial.print(F("Ramp: ")); 
      Serial.println(ramp);
      EEPROM.write(1, ramp);
    }
    if (mystring.startsWith("bright")) {
      targetr = (redlev + 10);
      targetg = (grnlev + 10);
      targetb = (blulev + 10);
      if (targetr >= 250) {
        targetr = 255;
      }
      if (targetg >= 250) {
        targetg = 255;
      }
      if (targetb >= 250) {
        targetb = 255;
      }
      Serial.print(F("Brighten: "));
      Serial.print(F("Red:"));
      Serial.print(targetr);
      Serial.print(F(" Green:")); 
      Serial.print(targetg);
      Serial.print(F(" Blue:")); 
      Serial.println(targetb);
      red(targetr);
      green(targetg);
      blue(targetb);
    }
    if (mystring.startsWith("dim")) {
      targetr = (redlev - 10);
      targetg = (grnlev - 10);
      targetb = (blulev - 10);
      if (targetr <= 0) {
        targetr = 0;
      }
      if (targetg <= 0) {
        targetg = 0;
      }
      if (targetb <= 0) {
        targetb = 0;
      }
      Serial.print(F("Dim: "));
      Serial.print(F("Red:")); 
      Serial.print(targetr);
      Serial.print(F(" Green:")); 
      Serial.print(targetg);
      Serial.print(F(" Blue:")); 
      Serial.println(targetb);
      red(targetr);
      green(targetg);
      blue(targetb);
    }
    /*
if (mystring.startsWith("more")) {
     stay = stay + 1000;
     }
     if (mystring.startsWith("less")) {
     stay = stay - 1000;
     if (stay < 0) {
     stay = 0;
     }
     }
     if (mystring.startsWith("fast")) {
     rate++;
     }
     if (mystring.startsWith("slow")) {
     rate--;
     if (rate < 0) {
     rate = 0;
     }
     }
     */
    if (mystring.startsWith("alloff")) {
      halt();
      Serial.println(F("All LED's Off!"));
      red(0);
      green(0);
      blue(0);
      white(0);
    }

    if (mystring.startsWith("stat")) {
      if (cyclego == 1) {
        Serial.println(F("Cycle Mode"));
       }
      else {
        Serial.print(F("Red Level = "));
        Serial.print(redlev);
        Serial.print(F(" Green Level = "));
        Serial.print(grnlev);
        Serial.print(F(" Blue Level = "));
        Serial.print(blulev);
        Serial.print(F(" White Level = "));
        Serial.print(whtlev);
        Serial.print(F(" Ramp Rate =  "));
        Serial.println(ramp);
      }
    }

    datain[1] = 0;
  }
  mystring = oldstring;
}


void buttons() {
  buttontime = millis();
  btnv = digitalRead(btn);
  btnv2 = digitalRead(btn2);

  if (btnv == LOW) {
    if (btnpressone == 0) {
      btnonetime = millis();
      btnpressone = 1;
    }
    delay(20);
  }

  if (cyclego == 1) {
    if (btnv == HIGH && btnpressone == 1) {
      if ((buttontime - btnonetime) < shortpressone) { // short press
        if (cyclepause == 0) {
          cyclego = 0;
          EEPROM.write(8, cyclego);
          cyclepause = 1;
          EEPROM.write(9, cyclepause);
          btnonetime = 0;
          btnpressone = 0;
          mode = 7;
          EEPROM.write(3, redlev);
          EEPROM.write(4, grnlev);
          EEPROM.write(5, blulev);
          EEPROM.write(6, whtlev);
        }
      }
    }
    if (btnv == HIGH && btnpressone == 1) {
      if ((buttontime - btnonetime) > shortpressone) { // long press
        cyclego = 0;
        EEPROM.write(8, cyclego);
        cyclepause = 0;
        EEPROM.write(9, cyclepause);
        red(0);
        green(0);
        blue(0);
        redlev = 0;
        grnlev = 0;
        blulev = 0;
        btnonetime = 0;
        btnpressone = 0;
        mode = 0;
        EEPROM.write(7, mode);
      }
    }
  }

  if (cyclego == 0) {
    if (btnv == HIGH && btnpressone == 1) {
      if ((buttontime - btnonetime) < shortpressone) { // short press
        if (cyclepause == 1) {
          cyclego = 1;
          cyclepause = 0;
          mode = 7;
          EEPROM.write(7, mode);
          EEPROM.write(8, cyclego);
          EEPROM.write(9, cyclepause);
        }
        else {
          manual();
          delay(50);
          mode++;
          if (mode == 10) {
            mode = 0;
          }
          EEPROM.write(7, mode);
          btnonetime = 0;
          btnpressone = 0;
        }
      }
    }
    if (btnv == HIGH && btnpressone == 1) {
      if ((buttontime - btnonetime) > shortpressone) { // long press
        red(0);
        green(0);
        blue(0);
        mode = 0;
        EEPROM.write(7, mode);
        btnonetime = 0;
        btnpressone = 0;
      }
    }
  }
  if (btnv2 == LOW) {
    if (btnpresstwo == 0) {
      btntwotime = millis();
      btnpresstwo = 1;
    }
    delay(20);
  }

  if (cyclego == 1) {
    if (btnv2 == HIGH && btnpresstwo == 1) {
      if ((buttontime - btntwotime) < shortpresstwo) { // short press
        rate++;
        btnpresstwo = 0;
        btntwotime = 0;
        if (rate == 10) {
          rate = 0;
        }
      }
      if (btnv2 == HIGH && btnpresstwo == 1) {
        if ((buttontime - btntwotime) > shortpresstwo) { // long press
          rate = 4;
          btntwotime = 0;
          btnpresstwo = 0;

          EEPROM.write(2, rate);
        }
      }
    }
  }

  if (cyclego == 0) {
    if (btnv2 == HIGH && btnpresstwo == 1) {
      if ((buttontime - btntwotime) < shortpresstwo) { // short press
        whitego = whtlev + 25;
        if (whitego >= 256) {
          whitego = 255;
        }
        white(whitego);
        btntwotime = 0;
        btnpresstwo = 0;
      }
    }
    if (btnv2 == HIGH && btnpresstwo == 1) {
      if ((buttontime - btntwotime) > shortpresstwo) { // long press
        if (whtlev == 0) {
          white(255);
        }
        else {
          white(0);
        }
        btntwotime = 0;
        btnpresstwo = 0;
      }
    }
  }
}

void white(int targetw)  { // white control
  if (whtlev < targetw) { // if the desired level is higher than the current level
    for (int x = whtlev; x <= targetw; x++) { // start ramping at the current level and brighten to the desired level
      analogWrite(w, x); // write the value to the pin
      delay(ramp); // delay for 8ms
    }
  }
  if (whtlev > targetw) { // if the desired level is lower than the current level
    for (int x = whtlev; x >= targetw; x--) { // start ramping at the current level and darken or dim to the desired level
      analogWrite(w, x); // write the value to the pin
      delay(ramp); // delay 8ms
    }
  }
  whtlev = targetw; // the desired level is now the current level
  EEPROM.write(6, whtlev);
}

void red(int targetr)  { // red control
  if (redlev < targetr) {
    for (int x = redlev; x <= targetr; x++) {
      analogWrite(r, x);
      delay(ramp);
    }
  }
  if (redlev > targetr) {
    for (int x = redlev; x >= targetr; x--) {
      analogWrite(r, x);
      delay(ramp);
    }
  }
  redlev = targetr;
  EEPROM.write(3, redlev);
}

void green(int targetg)  { // green control
  if (grnlev < targetg) {
    for (int x = grnlev; x <= targetg; x++) {
      analogWrite(g, x);
      delay(ramp);
    }
  }
  if (grnlev > targetg) {
    for (int x = grnlev; x >= targetg; x--) {
      analogWrite(g, x);
      delay(ramp);
    }
  }
  grnlev = targetg;
  EEPROM.write(4, grnlev);
}

void blue(int targetb)  { // blue control
  if (blulev < targetb) {
    for (int x = blulev; x <= targetb; x++) {
      analogWrite(b, x);
      delay(ramp);
    }
  }
  if (blulev > targetb) {
    for (int x = blulev; x >= targetb; x--) {
      analogWrite(b, x);
      delay(ramp);
    }
  }
  blulev = targetb;
  EEPROM.write(5, blulev);
}
void cyan() {
  blue(255);
  green(255);
  red(0);
}
void gold() {
  red(255);
  green(255);
  blue(0);
}
void magenta() {
  blue(255);
  red(255);
  green(0);
}
void pink() {
  red(255);
  blue(51);
  green(0);
}
void violet() {
  blue(255);
  red(166);
  green(0);
}
void ltgreen() {
  red(0);
  green(255);
  blue(25);
}
void ltblue() {
  blue(255);
  green(191);
  red(0);
}
void orange() {
  red(255);
  green(38);
  blue(0);
}
void rgbwhite() {
  red(255);
  green(255);
  blue(255);
}
void rgbww() {
  blue(12);
  green(115);
  red(242);
}

void cycle() { // cycle routine
  if (cyclego == 1) {
    serialchk();
    for(i = 0 ; i <= 255; i+=1) {  // blue on
      if (cyclego == 1) {
        analogWrite(b, i);
        buttons();
        delay(rate);
        blulev = i;
      }
    }
  }
  delay(stay);
  if (cyclego == 1) {
    serialchk();
    for(i = 255 ; i >= 0; i-=1) { // red off
      if (cyclego == 1) {
        analogWrite(r, i);
        buttons();
        delay(rate);
        redlev = i;
      }
    }
  }
  delay(stay);
  if (cyclego == 1) {
    serialchk();
    for(i = 0 ; i <= 255; i+=1) { // green on
      if (cyclego == 1) {
        analogWrite(g, i);
        buttons();
        delay(rate);
        grnlev = i;
      }
    }
  }
  delay(stay);
  if (cyclego == 1) {
    serialchk();
    for(i = 255 ; i >= 0; i-=1) { // blue off
      if (cyclego == 1) {
        analogWrite(b, i);
        buttons();
        delay(rate);
        blulev = i;
      }
    }
  }
  delay(stay);
  if (cyclego == 1) {
    serialchk();
    for(i = 0 ; i <= 255; i+=1) { // red on
      if (cyclego == 1) {
        analogWrite(r, i);
        buttons();
        delay(rate);
        redlev = i;
      }
    }
  }
  delay(stay);
  if (cyclego == 1) {
    serialchk();
    for(i = 255 ; i >= 0; i-=1) { // green off
      if (cyclego == 1) {
        analogWrite(g, i);
        buttons();
        delay(rate);
        grnlev = i;
      }
    }
  }
  delay(stay);
  /*
  EEPROM.write(6, whtlev);
  EEPROM.write(3, redlev);
  EEPROM.write(4, grnlev);
  EEPROM.write(5, blulev);
  */
}




void manual() { // manual modes
  if (mode == 0) { // red
    red(255);
    green(0);
    blue(0);
  }
  if (mode == 1) { // green
    green(255);
    red(0);
    blue(0);
  }
  if (mode == 2) { // blue
    blue(255);
    red(0);
    green(0);
  }
  if (mode == 3) { // magenta
    red(255);
    blue(255);
    green(0);
  }
  if (mode == 4) { // teal
    green(255);
    blue(255);
    red(0);
  }
  if (mode == 5) { // yellow
    red(255);
    green(255);
    blue(0);
  }
  if (mode == 6) { // RGB white
    red(255);
    green(255);
    blue(255);
  }
  if (mode == 7) { // color cycle
    green(0);
    blue(0);
    cyclego = 1;
    EEPROM.write(8, cyclego);
    red(255);
  }
  if (mode == 8) { // stop
    cyclego = 0;
    EEPROM.write(8, cyclego);
  }
  if (mode == 9) { // stop
    red(0);
    green(0);
    blue(0);
  }
  if (mode == 11) {

    cyclego = 0;
    EEPROM.write(8, cyclego);
    mode = 0;
  } 
}
void alloff() {
  red(0);
  green(0);
  blue(0);
  white(0);
  mode = 7;
}
void halt() {
  cyclego = 0;
      mode = 8;
      ramp = EEPROM.read(1);
      EEPROM.write(8, cyclego);
}
void hearir() { // listen to IR sensor
  int c = remote.listen(1);
  remval = c;
  if (oldc != c) {
    remoteck();
  }
  c = oldc;
}
void iron() { // show IR mode is on
  for(int y = 0 ; y <= 30; y++) {
    digitalWrite(r, HIGH);
    delay(20);
    digitalWrite(r, LOW);
    delay(20);
  }
}
void iroff() { // show IR mode is off
  for(int y = 255 ; y >= 0; y--) {
    analogWrite(r, y);
    delay(12);
  }
}
// ir remote check
void remoteck() {
  if (remval == 20) { // key 4 - red toggle
    if (redlev == 0) {
      red(255);
    }
    else {
      red(0);
    }
  }
  if (remval == 21) { // key 5 - green toggle
    if (grnlev == 0) {
      green(255);
    }
    else {
      green(0);
    }
  }
  if (remval == 22) { // key 6 - blue toggle
    if (blulev == 0) {
      blue(255);
    }
    else {
      blue(0);
    }
  }
  if (remval == 4) { // key setup - white toggle
    if (whtlev == 0) {
      white(255);
    }
    else {
      white(0);
    }
  }
  if (remval == 6) { // stop - all off
    alloff();
  }
  if (remval == 16) { // key 1 - red up
    if (redlev <= 245) {
      red(redlev + 10);
    }
  }
  if (remval == 24) { // key 7 - red down
    if (redlev >= 10) {
      red(redlev - 10);
    }
  }
  if (remval == 17) { // key 2 - green up
    if (grnlev <= 245) {
      green(grnlev + 10);
    }
  }
  if (remval == 25) { // key 8 - green down
    if (grnlev >= 10) {
      green(grnlev - 10);
    }
  }
  if (remval == 18) { // key 3 - blue up
    if (blulev <= 245) {
      blue(blulev + 10);
    }
  }
  if (remval == 26) { // key 9 - blue down
    if (blulev >= 10) {
      blue(blulev - 10);
    }
  }
  if (remval == 10) { // key right - white up
    if (whtlev <= 245) {
      white(whtlev + 10);
    }
  }
  if (remval == 8) { // key left - white down
    if (whtlev >= 10) {
      white(whtlev - 10);
    }
  }
  if (remval == 9) { // key enter/save - save the current color
    EEPROM.write(10, redlev);
    EEPROM.write(11, grnlev);
    EEPROM.write(12, blulev);
    EEPROM.write(13, whtlev);
    red(0);
    green(0);
    blue(0);
    white(0);
    red(EEPROM.read(10));
    green(EEPROM.read(11));
    blue(EEPROM.read(12));
    white(EEPROM.read(13));
    delay(100);
  }
  if (remval == 14) { // key repeat - recall the saved color
    red(EEPROM.read(10));
    green(EEPROM.read(11));
    blue(EEPROM.read(12));
    white(EEPROM.read(13));
  }
  if (remval == 0) { // key vol- - rate down
    rate--;
    if (rate <= 0) {
      rate = 0;
    }
    EEPROM.write(2, rate);
    Serial.print("Rate: ");
    Serial.println(rate);
  }
  if (remval == 2) { // key vol+ - rate up
    rate++;
    if (rate >= 254) {
      rate = 254;
    }
    EEPROM.write(2, rate);
    Serial.print("Rate: ");
    Serial.println(rate);
  }
  if (remval == 1) { // key play/pause - pause/resume cycle
    if (cyclepause == 0) {
      cyclego = 0;
      EEPROM.write(8, cyclego);
      cyclepause = 1;
      EEPROM.write(9, cyclepause);
      mode = 8;
      Serial.println(F("Cycle Paused."));
    }
    else {
      cyclego = 1;
      EEPROM.write(8, cyclego);
      cyclepause = 0;
      EEPROM.write(9, cyclepause);
      mode = 7;
      Serial.print(F("Cycle Resumed."));
      red(255);
      green(0);
      blue(0);
    }
  }
  if (remval == 13) { // key down - RGB dim
    targetr = (redlev - 10);
    targetg = (grnlev - 10);
    targetb = (blulev - 10);
    if (targetr <= 0) {
      targetr = 0;
    }
    if (targetg <= 0) {
      targetg = 0;
    }
    if (targetb <= 0) {
      targetb = 0;
    }
    Serial.print(F("Dim: "));
    Serial.print(F("Red:")); 
    Serial.print(targetr);
    Serial.print(F(" Green:")); 
    Serial.print(targetg);
    Serial.print(F(" Blue:")); 
    Serial.println(targetb);
    red(targetr);
    green(targetg);
    blue(targetb);
  }
  if (remval == 5) { // key up - RGB bright
    targetr = (redlev + 10);
    targetg = (grnlev + 10);
    targetb = (blulev + 10);
    if (targetr >= 250) {
      targetr = 255;
    }
    if (targetg >= 250) {
      targetg = 255;
    }
    if (targetb >= 250) {
      targetb = 255;
    }
    Serial.print(F("Brighten: "));
    Serial.print(F("Red:")); 
    Serial.print(targetr);
    Serial.print(F(" Green:")); 
    Serial.print(targetg);
    Serial.print(F(" Blue:")); 
    Serial.println(targetb);
    red(targetr);
    green(targetg);
    blue(targetb);
  }
  if (remval == 12) { // key 0/+10 - all on
    red(255);
    green(255);
    blue(255);
    white(255);
  }
}
