/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
int led = 1;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.

  pinMode(1, OUTPUT);
  pinMode(2, INPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  //pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, INPUT);
  pinMode(14, INPUT);
  pinMode(15, OUTPUT);
  pinMode(16, INPUT);
  pinMode(17, INPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(20, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(24, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(28, OUTPUT);
  pinMode(29, OUTPUT);
  pinMode(30, OUTPUT);
  pinMode(31, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(34, OUTPUT);
  pinMode(35, OUTPUT);
  pinMode(36, OUTPUT);
  pinMode(37, OUTPUT);
  pinMode(38, OUTPUT);
  pinMode(39, OUTPUT);
  
//Right to Left
//13,14,2,16,17
//digitalWrite(13,HIGH);//Pin1
//digitalWrite(14,HIGH);//Pin2
//digitalWrite(2,HIGH);//Pin3
//digitalWrite(16,HIGH);//Pin4
//digitalWrite(17,HIGH);//Pin5

}

void setupLED(void){

  pinMode(13, INPUT);
  pinMode(14, INPUT);
  pinMode(2, INPUT);
  pinMode(16, INPUT);
  pinMode(17, INPUT);

}
void whichLED(int led_num){

  switch(led_num){
    case 1:
      setupLED();
      pinMode(13, OUTPUT);
      pinMode(14, OUTPUT);
      
      digitalWrite(14,LOW);
      digitalWrite(13,HIGH);

      break;

    case 2:
      setupLED();
      pinMode(2, OUTPUT);
      pinMode(14, OUTPUT);
      
      digitalWrite(14,LOW);
      digitalWrite(2,HIGH);

      break;
    case 3:
      setupLED();
      pinMode(16, OUTPUT);
      pinMode(14, OUTPUT);
      
      digitalWrite(14,LOW);
      digitalWrite(16,HIGH);

      break;
    case 4:
      setupLED();
      pinMode(2, OUTPUT);
      pinMode(13, OUTPUT);
      
      digitalWrite(2,LOW);
      digitalWrite(13,HIGH);


      break;
    case 5:
      setupLED();
      pinMode(2, OUTPUT);
      pinMode(14, OUTPUT);
      
      digitalWrite(2,LOW);
      digitalWrite(14,HIGH);


      break;
    case 6:
      setupLED();
      pinMode(2, OUTPUT);
      pinMode(16, OUTPUT);
      
      digitalWrite(2,LOW);
      digitalWrite(16,HIGH);
      break;          
    case 7:
      setupLED();
      pinMode(2, OUTPUT);
      pinMode(17, OUTPUT);
      
      digitalWrite(2,LOW);
      digitalWrite(17,HIGH);
      break;
    case 8:
      setupLED();
      pinMode(16, OUTPUT);
      pinMode(13, OUTPUT);
      
      digitalWrite(16,LOW);
      digitalWrite(13,HIGH);
      break;      
    case 9:
      setupLED();
      pinMode(16, OUTPUT);
      pinMode(14, OUTPUT);
      
      digitalWrite(16,LOW);
      digitalWrite(14,HIGH);
      break;

    case 10:
      setupLED();
      pinMode(16, OUTPUT);
      pinMode(2, OUTPUT);
      
      digitalWrite(16,LOW);
      digitalWrite(2,HIGH);
      break;
    case 11:
      setupLED();
      pinMode(16, OUTPUT);
      pinMode(17, OUTPUT);
      
      digitalWrite(16,LOW);
      digitalWrite(17,HIGH);
      break;
    case 12:
      setupLED();
      pinMode(17, OUTPUT);
      pinMode(13, OUTPUT);
      
      digitalWrite(17,LOW);
      digitalWrite(13,HIGH);
      break;
    case 13:
      setupLED();
      pinMode(17, OUTPUT);
      pinMode(14, OUTPUT);
      
      digitalWrite(17,LOW);
      digitalWrite(14,HIGH);
      break;
    case 14:
      setupLED();
      pinMode(17, OUTPUT);
      pinMode(2, OUTPUT);
      
      digitalWrite(17,LOW);
      digitalWrite(2,HIGH);
      break;
    case 15:
      setupLED();
      pinMode(17, OUTPUT);
      pinMode(16, OUTPUT);
      
      digitalWrite(17,LOW);
      digitalWrite(16,HIGH);
                  
      break;
    
  

  
  }
  
}

void loop() {



  for(int x=1;x<=15;x++){
    //digitalWrite(x, HIGH);   // turn the LED on (HIGH is the voltage level)  
    whichLED(1);
    //delay(50);

  }
  
  /*for(int x=0;x<=39;x++){
    digitalWrite(x, LOW);   // turn the LED on (HIGH is the voltage level)

  }*/


  

}
