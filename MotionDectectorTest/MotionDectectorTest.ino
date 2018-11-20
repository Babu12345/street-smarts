#include <U8g2lib.h>
#include <adp5350.h>
#include "Arduino.h"
#include <driver/dac.h>
#define OFF 0
#define DIM 1
#define ON  2


int motionPin = 4;                  // choose the input pin (for PIR sensor)
int ledPin = 0;                     // variable for led pin
int motion = 0;                     // variable for reading the pin status
int state = 1;                      // start at DIM state

unsigned long t_motion;

void setup() {
  Serial.begin(115200);             // set up serial port
  pinMode(motionPin, INPUT);        // declare sensor as input
  pinMode(ledPin, OUTPUT);          // declare led as output
  dac_output_enable(DAC_CHANNEL_1); // set up the DAC
}


void loop(){
  motion = digitalRead(motionPin);
  switch (state){
    case OFF:
      dac_output_voltage(DAC_CHANNEL_1, 0);
      break;
    case DIM:
      dac_output_voltage(DAC_CHANNEL_1, 195);
      if (motion){
        t_motion = millis();
        state = ON;
      }
      break;
    case ON:
      dac_output_voltage(DAC_CHANNEL_1, 255);
      if (millis() - t_motion > 1000){
        state = DIM;
      }
      break;
  }
}
