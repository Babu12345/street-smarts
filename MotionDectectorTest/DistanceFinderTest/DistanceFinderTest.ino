/**
 * HC-SR04 Demo
 * Demonstration of the HC-SR04 Ultrasonic Sensor
 * Date: August 3, 2016
 * 
 * Description:
 *  Connect the ultrasonic sensor to the Arduino as per the
 *  hardware connections below. Run the sketch and open a serial
 *  monitor. The distance read from the sensor will be displayed
 *  in centimeters and inches.
 * 
 * Hardware Connections:
 *  Arduino | HC-SR04 
 *  -------------------
 *    5V    |   VCC     
 *    7     |   Trig     
 *    8     |   Echo     
 *    GND   |   GND
 *  
 * License:
 *  Public Domain
 */
#include <U8g2lib.h>

#define SCREEN U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI
#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 128
#define BUTTON_PIN 15

SCREEN oled(U8G2_R0, 5, 17,16); //declare oled with shorter SCREEN class name


/*
//Copy this...
// Pins
const int TRIG_PIN = 12;
const int ECHO_PIN = 14;
//The Button pin is now pin 15

// Anything over 400 cm (23200 us pulse) is "out of range"
//~54.24 cm or 21 inches (3146 us pulse)
const unsigned int MAX_DIST = 4146;
#define TIMEOUT_PERIOD 3000
int state=1;
unsigned long timer;


int isDetected(void){



  unsigned long t1;
  unsigned long t2;
  unsigned long pulse_width;
  float cm;
  float inches;

  // Hold the trigger pin high for at least 10 us
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Wait for pulse on echo pin
  while ( digitalRead(ECHO_PIN) == 0 );

  // Measure how long the echo pin was held high (pulse width)
  // Note: the micros() counter will overflow after ~70 min
  t1 = micros();
  while ( digitalRead(ECHO_PIN) == 1);
  t2 = micros();
  pulse_width = t2 - t1;

  // Calculate distance in centimeters and inches. The constants
  // are found in the datasheet, and calculated from the assumed speed 
  //of sound in air at sea level (~340 m/s).
  cm = pulse_width / 58.0;
  inches = pulse_width / 148.0;

  // Print out results
  if ( pulse_width > MAX_DIST ) {
    return 0;
  } else {
    return 1;
    //Serial.print(cm);
    //Serial.print(" cm \t");
    //Serial.print(inches);
    //Serial.println(" in");
  }
  
  // Wait at least 60ms before next measurement
  delay(60);
}

*/

void pretty_print(int startx, int starty, String input, int fwidth, int fheight, int spacing, SCREEN &display){
  
int x = startx;
  int y = starty;
  String temp = "";
  for (int i=0; i<input.length(); i++){
     if (fwidth*temp.length()<= (SCREEN_WIDTH-fwidth -x)){
        if (input.charAt(i)== '\n'){
          display.setCursor(x,y);
          display.print(temp);
          y += (fheight + spacing);
          temp = "";
          if (y>SCREEN_HEIGHT) break;
        }else{
          temp.concat(input.charAt(i));
        }
     }else{
      display.setCursor(x,y);
      display.print(temp);
      temp ="";
      y += (fheight + spacing);
      if (y>SCREEN_HEIGHT) break;
      if (input.charAt(i)!='\n'){
        temp.concat(input.charAt(i));
      }else{
          display.setCursor(x,y);
          y += (fheight + spacing);
          if (y>SCREEN_HEIGHT) break;
      } 
     }
     if(i==input.length()-1){
        display.setCursor(x,y);
        display.print(temp);
     }
  }

}

class Button{
    public:
  unsigned long t_of_state_2;
  unsigned long t_of_button_change;    
  unsigned long debounce_time;
  unsigned long long_press_time;
  int pin;
  int flag;
  bool button_pressed;
  int state; // This is public for the sake of convenience
  Button(int p) {
  flag = 0;  
    state = 0;
    pin = p;
    t_of_state_2 = millis(); //init
    t_of_button_change = millis(); //init
    debounce_time = 10;
    long_press_time = 1000;
    button_pressed = 0;
  }
  void read() {
    int button_state = digitalRead(pin);  
    button_pressed = !button_state;
  }
  int update() {

  read();
  flag = 0;
  if (state==0) { // Unpressed, rest state
    if (button_pressed) {
      state = 1;
      t_of_button_change = millis();
    }
  } else if (state==1) { //Tentative pressed
    if (!button_pressed) {
      state = 0;
      t_of_button_change = millis();
    } else if (millis()-t_of_button_change >= debounce_time) {
      state = 2;
      t_of_state_2 = millis();
    }
  } else if (state==2) { // Short press
    if (!button_pressed) {
      state = 4;
      t_of_button_change = millis();
    } else if (millis()-t_of_state_2 >= long_press_time) {
      state = 3;
    }
  } else if (state==3) { //Long press
    if (!button_pressed) {
      state = 4;
      t_of_button_change = millis();
    }
  } else if (state==4) { //Tentative unpressed
    if (button_pressed && millis()-t_of_state_2 < long_press_time) {
      state = 2; // Unpress was temporary, return to short press
      t_of_button_change = millis();
    } else if (button_pressed && millis()-t_of_state_2 >= long_press_time) {
      state = 3; // Unpress was temporary, return to long press
      t_of_button_change = millis();
    } else if (millis()-t_of_button_change >= debounce_time) { // A full button push is complete
      state = 0;
      if (millis()-t_of_state_2 < long_press_time) { // It is a short press
        flag = 1;
      } else {  // It is a long press
        flag = 2;
      }
    }
  }
  return flag;


  }
};

Button button(BUTTON_PIN);
void setup() {

  // The Trigger pin will tell the sensor to range find
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);


  oled.begin();     // initialize the OLED
  oled.clearBuffer();    //clear the screen contents
  oled.setFont(u8g2_font_5x7_tf);  //set font on oled  
  oled.setCursor(0,15); //set cursor
  oled.print("Starting"); //print starting message
  oled.sendBuffer(); //send it (so it displays)
  Serial.begin(9600);
}

void loop() {

int button_state = button.update();

if(button_state==1){
  oled.clearBuffer();
  oled.setCursor(0,20);
  oled.print("Short Button Press"); //print starting message
  oled.sendBuffer(); //send it (so it displays)
  delay(2000);

}else if(button_state==2){
  oled.clearBuffer();
  oled.setCursor(0,10);
  oled.print("Long Button Press"); //print starting message
  oled.sendBuffer(); //send it (so it displays)
  delay(2000);
  
}else{
  oled.clearBuffer();
  oled.setCursor(0,10);
  oled.print("No Button Press"); //print starting message
  oled.sendBuffer(); //send it (so it displays)
  
}


if(state==1){

  Serial.println("Woo the light is on.");
  timer=millis();
  
}else if(state==0){

  Serial.println("Timeout for the light");
  
}


state=isDetected();



}



