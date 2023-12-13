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

This sketch is for controlling servo's, LED's, inputs(ie sensors) and outputs(ie relays, stepper motors etc)
Operates up to 16 servos at defineable speeds taking the close (Close - 0) and throw (Throw - 1) commands from JMRI.
PCA9685 board 1 is for servo's
  Thrown sets the turnout to the divergent route.
  Closed sets the turnout to the straight route.
PCA9685 board 1 is for LED's
  Thrown is ON.
  Closed is OFF.
20 outputs are defined
46 inputs are defined
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <CMRI.h>
#include <Auto485.h>

Adafruit_PWMServoDriver pwm0 = Adafruit_PWMServoDriver(0x40);                            // setup first board address for servo's
Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver(0x41);                            // setup the second board address for led's

#define CMRI_ADDR 1
#define DE_PIN  2
Auto485 bus(DE_PIN);
CMRI cmri(CMRI_ADDR, 64, 128, bus);

// LED VARIABLES
int LedOnVal[] = {4096,4096,4096,4096,4096,4096,4096,4096,4096,4096,4096,4096,4096,4096,4096,4096};
int LedOffVal[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// SERVO VARIABLES
int Close[] = {2000,1800,1900,2000,0,0,0,0,0,0,0,0,0,0,0,0};                             // table to define Close values for each servo
int Throw[] = {1000,1100,1200,1300,0,0,0,0,0,0,0,0,0,0,0,0};                             // table to define Throw values for each servo
int StepSize[] = {1,5,10,1,1,1,1,1,1,1,1,1,1,1,1,1};                                      // table to define servo travel increment. Step size should be 1 or even
int Status[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};                                        // table to store the current position of each turnout  0 = closed    1 = thrown
int CurrentPosition[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};                               // table to hold the current positions of each turnout
int DelayTime[] = {0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0};                                     // table to define the time delay between pulses. This controls the rotational speed

unsigned long previousMillis[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};                      // table to hold the previous movement time for each turnout

void MoveServo(int i){
unsigned long currentMillis = millis();

if(DelayTime[i] == 0){                                                                    // if delay time is 0 then move servo quickly to required state
  if (cmri.get_bit(i) == 1){pwm0.writeMicroseconds(i, Throw[i]); Status[i] = 1;}
  else {pwm0.writeMicroseconds(i, Close[i]); Status[i] = 0;}
  }
else{                                                                                     // delay time is greater than 0 so move servo slowly
  if ((currentMillis - previousMillis[i]) >= DelayTime[i]){                               // checks if sufficient time has passed since last movement
    previousMillis[i] = currentMillis;                                                    // update last movement time
    if(cmri.get_bit(i) == 1){                                                             // Throw servo
      if(Throw[i] > Close[i]){                                                            // Throw is greater than close therefore need to add the increment from StepSize table

        CurrentPosition[i] = CurrentPosition[i] + StepSize[i];                            // Update current position in table
        pwm0.writeMicroseconds(i, CurrentPosition[i]);                                     // move servo 1 step
        if(CurrentPosition[i] == Throw[i]){Status[i] = 1;}                                // if the servo has reached the thrown position than update status table
        }
      else{                                                                               // Throw is less than close therefore need to subtract the increment from StepSize table
        CurrentPosition[i] = CurrentPosition[i] - StepSize[i];                            // Update current position in table
        pwm0.writeMicroseconds(i, CurrentPosition[i]);                                     // move servo 1 step
        if(CurrentPosition[i] == Throw[i]){Status[i] = 1;}                                // if the servo has reached the thrown position than update status table
        }
      }

    if(cmri.get_bit(i) == 0){                                                             // Close servo
      if(Close[i] > Throw[i]){                                                            // close is greater than Throw therefore need to add the increment from StepSize table
        CurrentPosition[i] = CurrentPosition[i] + StepSize[i];                            // Update current position in table
        pwm0.writeMicroseconds(i, CurrentPosition[i]);                                     // move servo 1 step
        if(CurrentPosition[i] == Close[i]){Status[i] = 0;}                                // if the servo has reached the closed position than update status table
        }
      else{                                                                               // Throw is less than close therefore need to subtract the increment from StepSize table
        CurrentPosition[i] = CurrentPosition[i] - StepSize[i];                            // Update current position in table
        pwm0.writeMicroseconds(i, CurrentPosition[i]);                                     // move servo 1 step
        if(CurrentPosition[i] == Close[i]){Status[i] = 0;}                                // if the servo has reached the closed position than update status table
        }
      }
  }
  }
}

void setup()
{
for (int i=3; i<13; i++){pinMode(i, INPUT_PULLUP);}                                       // define sensor shield pins 3 to 12 as inputs
for (int i=14; i<20; i++){pinMode(i, INPUT_PULLUP);}                                     // define sensor shield pins 14 to 19 as inputs
for (int i=22; i<50; i++){pinMode(i, INPUT_PULLUP);}                                     // define sensor shield pins 22 to 49 as inputs
for (int i=50; i<70; i++){pinMode(i, OUTPUT);}                                           // define sensor shield pins 50 to 69 as outputs

Serial.begin(19200);
bus.begin(19200);
pwm0.begin();
pwm0.setPWMFreq(50);                                                                      // This is the PWM frequency for servo's
pwm1.begin();
pwm1.setPWMFreq(1000);                                                                    // This is the PWM frequency for led's

// set all servos to closed position

for(int i=0; i<16; i++){
  if(Close[i] != 0 && Throw[i] != 0){
    pwm0.writeMicroseconds(i, Close[i]);
    CurrentPosition[i] = Close[i];
    Status[i] = 0;
    }
  }
}

void loop()
{
cmri.process();

// PROCESS PCA9685 BOARDS
for (int i = 0; i < 16; i++){
  if (Status[i] != cmri.get_bit(i) && Close[i] != 0 && Throw[i] != 0){MoveServo(i);}                                        // PROCESS SERVO'S. Check if servo status has changed and throw and close values are not zero
  if (cmri.get_bit(i+16) == 1){pwm1.writeMicroseconds(i, LedOnVal[i]);} else {pwm1.writeMicroseconds(i, LedOffVal[i]);}     // PROCESS LED'S
  }

// PROCESS SENSORS
for (int i = 3; i < 13; i++){cmri.set_bit((i-1), !digitalRead(i));}                     // A sensor attached to pin 3 corresponds to address 1003 for this CMRI node
for (int i = 14; i < 20; i++){cmri.set_bit((i-1), !digitalRead(i));}                    // A sensor attached to pin 14 corresponds to address 1014 for this CMRI node
for (int i = 22; i < 50; i++){cmri.set_bit((i-1), !digitalRead(i));}                    // A sensor attached to pin 22 corresponds to address 1022 for this CMRI node

// PROCESS OUTPUTS                                                                      // Non servo outputs will start on bit 100, this will be address 1101 in CMRI
for (int i = 50; i < 70; i++){digitalWrite(i, !cmri.get_bit(i+50));}                    // An output attached to pin 50 corresponds to address 1101 for this CMRI node
}
