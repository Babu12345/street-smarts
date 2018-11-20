/*
This code is what will be running on the single lamp
ESP32 module. 

Basically, it should perform a GET request every 3 seconds,
checking to see if its status has been updated on the 
server to one of the emergency states, and update the
lamp color based on that information.
*/

#include <U8g2lib.h>
#include <WiFi.h>

#define SCREEN U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI
#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 128

// LIGHTSTATE variables
#define OFF 0
#define DIM 1
#define ON 2
#define RED 3
#define BLUE 4
#define ORANGE 5

// STATE variables
#define IDLE 0
#define MOTION 3
#define RECORDING 4
#define FIRE 5
#define SAFETY 6
#define INJURY 7
#define CANCEL 8
#define HELP_INCOMING 9
#define HELP_CLOSE 10
#define HELP_ARRIVED 11
#define RESET 12

// lamp ID in database
int ID = 4;

const int response_timeout = 6000; //ms to wait for response from host
const int button_pin = 15;
unsigned long get_timer = millis();
int get_delay = 5000 ;
int STATE = 0;
int LIGHTSTATE = 0;
String type = "";
String message;
String status;

SCREEN oled(U8G2_R0, 5, 17, 16);


class Button {
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
        t_of_state_2 = millis();       //init
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
      if (state == 0) {
          if (button_pressed) {
              state = 1;
              t_of_button_change = millis();
          }
          return flag;
      } else if (state == 1) {
          if (button_pressed && millis() - t_of_button_change >= debounce_time) {
              state = 2;
              t_of_state_2 = millis();
          } else if (!button_pressed) {
              state = 0;
              t_of_button_change = millis();
          }
      } else if (state == 2) {
          if (!button_pressed){
              state = 4;
              t_of_button_change = millis();
          } else if (button_pressed && millis() - t_of_state_2 >= long_press_time){
              state = 3;
          }
      } else if (state == 3) {
          if (!button_pressed){
              state = 4;
              t_of_button_change = millis();
          }
      } else if (state == 4) {
          if (button_pressed && millis() - t_of_state_2 < long_press_time) {
              state = 2;
              t_of_button_change = millis();
          } else if (button_pressed && millis() - t_of_state_2 >= long_press_time) {
              state = 3;
              t_of_button_change = millis();
          } else if (!button_pressed && millis() - t_of_button_change >= debounce_time) {
              state = 0;
              if (millis() - t_of_state_2 <= long_press_time) {
                  flag = 1;
              } else if (millis() - t_of_state_2 >= long_press_time) {
                  flag = 2;
              }
          }
      }
      return flag;
    }
};


Button button(15); //declare button object


void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  oled.setFont(u8g2_font_5x7_tf); //set font on oled
  oled.begin();
  WiFi.begin("MIT");      //attempt to connect to wifi
  oled.setCursor(0, 15);  //set cursor
  oled.print("Starting"); //print starting message
  oled.sendBuffer();      //send it (so it displays)
  start_WiFi();
  Serial.println("Started.");
  status = do_get();
}


void loop() {
  oled.clearBuffer();
  int bv = button.update(); //get button value
  switch (STATE) {
    case IDLE:
      message = "Long press to POST";
      if (bv == 2){
        STATE = FIRE;
      }
      break;
    case FIRE: {
      if (bv == 2){
        String output = do_post("IDLE");
        STATE = IDLE;
        LIGHTSTATE = ON;//Delay and then dim
      } else {
        String output = do_post("FIRE");
        message = output;
        LIGHTSTATE = RED;
      }
      break;
      }
    }
  switch (LIGHTSTATE) {
    case OFF:
      oled.setCursor(64, 6);
      oled.print("Lamp: OFF");
      break;
    case DIM:
      oled.setCursor(64, 6);
      oled.print("Lamp: DIM");
      break;
    case ON:
      oled.setCursor(64, 6);
      oled.print("Lamp: ON");
      break;
    case RED:
      oled.setCursor(64, 6);
      oled.print("Lamp: RED");
      break;
    case BLUE:
      oled.setCursor(64, 6);
      oled.print("Lamp: BLUE");
      break;
    case ORANGE:
      oled.setCursor(64, 6);
      oled.print("Lamp: ORANGE");
      break;
  }
  // update the status if we haven't checked in a while
  if (millis() - get_timer > get_delay) {
    String get = do_get();
    status = get;
    get_timer = millis();
    if (status == "FIRE"){
      LIGHTSTATE = RED;
    } else if (status == "IDLE"){
      LIGHTSTATE = ON;
    } else if (status == "ACCEPTED") {
      LIGHTSTATE = BLUE;
    }
  }
  // draw lines for the "status bar"
  oled.drawLine(0, 7, 128, 7);
  oled.drawLine(11, 0, 11, 7);
  oled.drawLine(61, 0, 61, 7);
  // print the main message at center screen
  pretty_print(0, 30, message, 5, 7, 1, oled);
  // print the status bar message along the top
  pretty_print(0,6,"#" + String(ID) + String(" WiFi: ") + String(WiFi.SSID()),5,7,1,oled);
  // print the "last updated" at the bottom
  pretty_print(0, 58,String("updated ") + String((millis()-get_timer)/1000) + String(" seconds ago"),5,7,1,oled);
  oled.sendBuffer();
}


String do_get(){
  String out = "";
  WiFiClient client; //instantiate a client object
  if (client.connect("iesc-s1.mit.edu", 80)) { //try to connect to iesc-s1.mit.edu
    client.println("GET /608dev/sandbox/linnear/emergencyRequest.py?ID="+String(ID)+"&type=lamp HTTP/1.1");
    client.println("Host: iesc-s1.mit.edu");
    client.print("\r\n");
    unsigned long count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      String line = client.readStringUntil('\n');
      Serial.println(line);
      if (line == "\r") { //found a blank line!
        //headers have been received! (indicated by blank line)
        break;
      }
      if (millis()-count>6000) break;
    }
    count = millis();
    String op; //create empty String object
    while (client.available()) { //read out remaining text (body of response)
      op+=(char)client.read();
    }
    out += op;
    Serial.println(op);
    client.stop();
    Serial.println();
    Serial.println("-----------");
  }else{
    Serial.println("connection failed");
    Serial.println("wait 0.5 sec...");
    client.stop();
    delay(300);
  }
  return out;
}


String do_post(String inp){
  String out = "";
  WiFiClient client; //instantiate a client object
  if (client.connect("iesc-s1.mit.edu", 80)) { //try to connect to iesc-s1.mit.edu
    String body = "{\"type\":\"lamp\",\"status\":\""+String(inp)+"\",\"ID\":"+String(ID)+"}";
    client.println("POST /608dev/sandbox/linnear/emergencyRequest.py HTTP/1.1");
    client.println("Host: iesc-s1.mit.edu");
    client.println("Content-Type: application/json");
    client.println("Content-Length: " + String(body.length()));
    client.print("\r\n");
    client.print(body);
    unsigned long count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      String line = client.readStringUntil('\n');
      Serial.println(line);
      if (line == "\r") { //found a blank line!
        //headers have been received! (indicated by blank line)
        break;
      }
      if (millis()-count>6000) break;
    }
    count = millis();
    String op; //create empty String object
    while (client.available()) { //read out remaining text (body of response)
      op+=(char)client.read();
    }
    out += op;
    Serial.println(op);
    client.stop();
    Serial.println();
    Serial.println("-----------");
  }else{
    Serial.println("connection failed");
    Serial.println("wait 0.5 sec...");
    client.stop();
    delay(300);
  }
  return out;
}

void pretty_print(int startx, int starty, String input, int fwidth, int fheight, int spacing, SCREEN &display){
  int y = starty;
  int input_ind = 0;
  int width = (SCREEN_WIDTH - startx)/fwidth;
  while (input_ind < input.length()){
    if (y > SCREEN_HEIGHT){
      break;
      }
    oled.setCursor(startx,y);
    String str = input.substring(input_ind, input_ind+width);
    if (str.indexOf("\n") != -1){
      str = str.substring(0, str.indexOf("\n") + 1);
      input_ind = input_ind + str.length();
      }
    else {
      input_ind = input_ind + width;
      }
    oled.print(str);
    y = y + fheight + spacing;
    oled.setCursor(startx, y);
    }
}


//Start up WiFi.
void start_WiFi(){
  int count = 0; //count used for Wifi check times
  while (WiFi.status() != WL_CONNECTED && count<6) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  } else { //if we failed to connect just ry again.
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP
  }
}
