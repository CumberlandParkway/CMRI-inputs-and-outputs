/*
Written by Adrian Garnham
Adapted from too many sources to list

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

This sketch combines servo turnout control, sensor inputs and other outputs for arduino mega
Pins 0, 1 and 2 are protected since these are used for communication if using Auto485
Pin 13 is the Arduino LED pin and should not be used as an input, but can be used as an output for some applications
Pins 20 and 21 are reserved for PWM servo control with PCA9685 boards

We will set the Arduino up to behave like a piece of CMRI hardware called a SUSIC with up to 64 slots
Each slot has 32 bit input/output card
We will set cards 0 and 1 to be the sensor inputs (up to 64 inputs) and cards 2 - 5  to support 128 outputs

INPUTS
	Pins 3 to 12 are available (address 1003 - 1012)
	Pins 14 to 19 are available (address 1014 - 1019)
	Pins 22 to 45 are available (address 1022 - 1045)
OUTPUTS
	Pins 46 to 69 are available (address 1146 - 1169)
*/
// Include libraries
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <CMRI.h>
#include <Auto485.h>
#include "ServoThrows_Cumberland_A.h"
//#include "ServoThrows_Cumberland_B.h"
//#include "ServoThrows_Cumberland_C.h"
//#include "ServoThrows_Poynters_lane.h"
//#include "ServoThrows_TestTrack.h"

// CMRI Settings
#define CMRI_ADDR 1 //CMRI node address in JMRI
#define DE_PIN 2

// Define the PCA9685 board addresses
Adafruit_PWMServoDriver pwm0 = Adafruit_PWMServoDriver(0x40); //setup the first board address
Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver(0x41); //setup the second board address

// Setup serial communication
Auto485 bus(DE_PIN); // Arduino pin 2 -> MAX485 DE and RE pins

// Define CMRI connection with 64 inputs and 128 outputs
CMRI cmri(CMRI_ADDR, 64, 128, bus);

void setup() {

  // SET PINS TO INPUT OR OUTPUT

    for (int i=3; i<20; i++)  {
           pinMode(i, INPUT_PULLUP);       // define sensor shield pins 3 to 19 as inputs
        }
        
    for (int i=22; i<46; i++)  {
           pinMode(i, INPUT_PULLUP);       // define sensor shield pins 22 to 45 as inputs
        }
    
    for (int i=46; i<70; i++)  {
           pinMode(i, OUTPUT);             // define sensor shield pins 46 to 69 as outputs
        }

  //INITIALISE PCA9685 BOARDS

Serial.begin(19200); //Baud rate of 19200, ensure this matches the baud rate in JMRI, using a faster rate can make processing faster but can also result in incomplete data
bus.begin(19200);
pwm0.begin();
pwm0.setPWMFreq(50);  // This is the maximum PWM frequency
pwm1.begin();
pwm1.setPWMFreq(1000);  // This is the maximum PWM frequency

// centre all servos

for(int i=0; i<numServos; i++){
int ServoCentre = (Throw[i]+Close[i])/2;
pwm0.writeMicroseconds(i, ServoCentre);
delay(500);}
}

void loop(){
cmri.process();

// PROCESS PCA9685 BOARDS
// Assume servos start on bit 0 which corresponds to output address 1001
// Assume Leds start on bit 16 which corresponds to output address 1017

for (int i = 0; i < 16; i++) {
//BOARD 1

Status[i] = (cmri.get_bit(i));
if (Status[i] == 1){pwm0.writeMicroseconds(i, Throw[i]);}
else {pwm0.writeMicroseconds(i, Close[i]);}

// BOARD 2

Status[i+16] = (cmri.get_bit(i+16));
if (Status[i+16] == 1){pwm1.writeMicroseconds(i, LedOnVal[i]);}
else {pwm1.writeMicroseconds(i, LedOffVal[i]);}
}

   // PROCESS SENSORS
   // Do not read pins 0, 1 or 2
   // A sensor attached to pin 3 corresponds to address 1003 for this CMRI node
     for (int i = 3; i < 13; i++) {
      cmri.set_bit((i-1), !digitalRead(i));
     }
   // Do not read pin 13.
   // A sensor attached to pin 14 corresponds to address 1014 for this CMRI node
     for (int i = 14; i < 20; i++) {
      cmri.set_bit((i-1), !digitalRead(i));
     }
   // Do not read pins 20 and 21.
   // A sensor attached to pin 22 corresponds to address 1022 for this CMRI node
     for (int i = 22; i < 46; i++) {
      cmri.set_bit((i-1), !digitalRead(i));
     }

    // PROCESS OUTPUTS
    // Non servo outputs will start on bit 100, this will be address 1101 in CMRI, bit 101 will be 1102, etc.
   // An output attached to pin 46 corresponds to address 1101 for this CMRI node
   // An output attached to pin 69 corresponds to address 1124 for this CMRI node


     for (int i = 46; i < 70; i++) {
        digitalWrite(i, !cmri.get_bit(i+54));
     }
}
