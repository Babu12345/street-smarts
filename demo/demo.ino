#include <SPI.h>
#include <U8g2lib.h>
#include <WiFi.h>

int ID[31] = {13,12,11,10,5,0,0,0,0,0,-1};

int ID_main = 11;

int ID_to_Pin[31]={46,42,41,37,39,45,19,44,22,21,18,
                    3,23,27,47,48,32,1,4,5,15,25,33,
                    34,36,43,40,35,26,12,38};
                    //Id number is the index and the pin will become the value
                    
int increment=0;

unsigned timer=0;
WiFiClient client;

int count=0;
//Start up WiFi.
void start_WiFi(){
  int count = 0; //count used for Wifi check times
  while (WiFi.status() != WL_CONNECTED && count<6) {
    delay(500);
    //Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    //Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  } else { //if we failed to connect just ry again.
    //Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP
  }
}

void setupLED(void){

  pinMode(13, INPUT);
  pinMode(14, INPUT);
  pinMode(2, INPUT);
  pinMode(16, INPUT);
  pinMode(17, INPUT);

}

void setup() { 

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
  
  //Serial.begin(115200);               // Set up serial port

  int count = 0; //count used for Wifi check times
 
  WiFi.begin("6s08","iesc6s08");
  //WiFi.begin("MIT","");
  start_WiFi();
  client.connect("iesc-s1.mit.edu", 80);//ALways keep connected
  
  for(int i = 0; i < 31; i++){
    ID[i] = -1;
  }
  
}

void loop() {

  String numbers="";
  
  if(millis()-timer > 5000){
      timer = millis();

      client.println("GET /608dev/sandbox/linnear/emergencyRequest.py?type=bigboi HTTP/1.1");
      client.println("Host: iesc-s1.mit.edu");
      client.print("\r\n");
      client.connected();
      count=millis();
      while (client.connected()) {

        //Keeps the LEDs on here in the loop

        LedsIncrement(increment);
        increment++;
        String line = client.readStringUntil('\n');
        if (line == "\r") {\
          break;
        }
        if (millis()-count>6000) break;
      }

      while (client.available()) {
        LedsIncrement(increment);
        increment++;
        
        numbers+=(char)client.read();
        //Keep the LEDs on in this loop
      }
      //client.stop();

      numbers+=" ";
      String number="";

      ID[0]=-1;
      int index = 0; //the index to populate
      for(int i = 0; i < numbers.length(); i++){

        if(numbers[i] == ' ' and i!=0){
        
          ID[index] = number.toInt();
          //Serial.println(ID[index]);
        
          index++;
          number = "";
        }else{
          number = number + numbers[i];
        }
        
        
        LedsIncrement(increment);
        increment++;
        //Keeps the LEDs looping over the numbers
      }

      
      LedReset();
      setupLED();
      ID[index]=-1;
      
       
  }


  //Turns the LEDs on in a loop
  LedsIncrement(increment);
  increment++;
  
}

//Increments to keep the circuit blocking
void LedsIncrement(int i){
//Lights up the LEDs

if(ID[0]!=-1){//Display nothing if array is empty
  
if(ID[i]== -1){
   increment=0;
   i=0;
}
//Serial.println(ID[i]);

if(ID_to_Pin[ID[i]]<34){
  
  //Light up normal pins
  digitalWrite(ID_to_Pin[ID[i]],HIGH);
  
}else{
  //Light up multiplexer pins
  
  whichLED(ID_to_Pin[ID[i]]);
 
}

}

}

void LedReset(){
//Resets the ON LEDs

for(int i=0;i<31;i++){
 

if(ID_to_Pin[i]<34){
  //Light up normal pins
  digitalWrite(ID_to_Pin[i],LOW);

}

//if(ID[i]== -1){
//  break;
//}

}

}

//Multiplexer function
void whichLED(int led_num){

  switch(led_num){
    case 34:
      setupLED();
      pinMode(13, OUTPUT);
      pinMode(14, OUTPUT);
      
      digitalWrite(14,LOW);
      digitalWrite(13,HIGH);

      break;

    case 35:
      setupLED();
      pinMode(2, OUTPUT);
      pinMode(14, OUTPUT);
      
      digitalWrite(14,LOW);
      digitalWrite(2,HIGH);

      break;
    case 36:
      setupLED();
      pinMode(16, OUTPUT);
      pinMode(14, OUTPUT);
      
      digitalWrite(14,LOW);
      digitalWrite(16,HIGH);

      break;
    case 37:
      setupLED();
      pinMode(2, OUTPUT);
      pinMode(13, OUTPUT);
      
      digitalWrite(2,LOW);
      digitalWrite(13,HIGH);


      break;
    case 38:
      setupLED();
      pinMode(2, OUTPUT);
      pinMode(14, OUTPUT);
      
      digitalWrite(2,LOW);
      digitalWrite(14,HIGH);


      break;
    case 39:
      setupLED();
      pinMode(2, OUTPUT);
      pinMode(16, OUTPUT);
      
      digitalWrite(2,LOW);
      digitalWrite(16,HIGH);
      break;          
    case 40:
      setupLED();
      pinMode(2, OUTPUT);
      pinMode(17, OUTPUT);
      
      digitalWrite(2,LOW);
      digitalWrite(17,HIGH);
      break;
    case 41:
      setupLED();
      pinMode(16, OUTPUT);
      pinMode(13, OUTPUT);
      
      digitalWrite(16,LOW);
      digitalWrite(13,HIGH);
      break;      
    case 42:
      setupLED();
      pinMode(16, OUTPUT);
      pinMode(14, OUTPUT);
      
      digitalWrite(16,LOW);
      digitalWrite(14,HIGH);
      break;

    case 43:
      setupLED();
      pinMode(16, OUTPUT);
      pinMode(2, OUTPUT);
      
      digitalWrite(16,LOW);
      digitalWrite(2,HIGH);
      break;
    case 44:
      setupLED();
      pinMode(16, OUTPUT);
      pinMode(17, OUTPUT);
      
      digitalWrite(16,LOW);
      digitalWrite(17,HIGH);
      break;
    case 45:
      setupLED();
      pinMode(17, OUTPUT);
      pinMode(13, OUTPUT);
      
      digitalWrite(17,LOW);
      digitalWrite(13,HIGH);
      break;
    case 46:
      setupLED();
      pinMode(17, OUTPUT);
      pinMode(14, OUTPUT);
      
      digitalWrite(17,LOW);
      digitalWrite(14,HIGH);
      break;
    case 47:
      setupLED();
      pinMode(17, OUTPUT);
      pinMode(2, OUTPUT);
      
      digitalWrite(17,LOW);
      digitalWrite(2,HIGH);
      break;
    case 48:
      setupLED();
      pinMode(17, OUTPUT);
      pinMode(16, OUTPUT);
      
      digitalWrite(17,LOW);
      digitalWrite(16,HIGH);
                  
      break;
    
  

  
  }
  
}
