/*
This code is what will be running on the emergency
responder-side ESP32 module. 

Basically, it does a get request every 3 seconds, then
prints the relevant message on the OLED. If it receives an
emergency request, it can accept it and post that to the
server, changing the status of the lights.
*/

#include <U8g2lib.h>
#include <WiFi.h>

#define SCREEN U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI
#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 128
#define IDLE 0
#define CALL 1
#define ACCEPTED 2

const int response_timeout = 20000; //ms to wait for response from host
const int button_pin = 15;
unsigned long get_timer = millis();
int get_delay = 5000;
int STATE = 0;
String type = "";
String message;
String timing_msg;

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
  //WiFi.begin("MIT");      //attempt to connect to wifi
  WiFi.begin("6s08","iesc6s08");
  //WiFi.begin("MIT","");
  
  oled.setCursor(0, 15);  //set cursor
  oled.print("Starting"); //print starting message
  oled.sendBuffer();      //send it (so it displays)
  start_WiFi();
  Serial.println("Started.");
}


void loop() {
  oled.clearBuffer();
  timing_msg = String("updated ") + String((millis()-get_timer)/1000) + String(" seconds ago");
  int bv = button.update();
  // ability to 'accept' a request with a post request
  if (message.substring(0, 4) == "Fire" && bv == 2) {
    message = message.substring(0, message.indexOf("L"));
    timing_msg = "accepting...";
    draw();
    message += do_post("ACCEPTED", message.substring(message.indexOf("["), message.indexOf("]")+1));
    get_timer = millis();
  }
  // update the status if we haven't checked in a while
  if (millis() - get_timer > get_delay) {
    timing_msg = "updating...";
    draw();
    String get = do_get();
    message = get;  
    if (message.substring(0, 4) == "Fire") {
      message += String("\nLong press to accept");
    }
    get_timer = millis();
  }
  draw();
}


void draw() {
  // draw lines for the "status bar"
  oled.drawLine(0, 7, 128, 7);
  oled.drawLine(61, 0, 61, 7);
  // print the main message at center screen
  pretty_print(0,16,message,5,7,3,oled);
  // print the status bar message along the top
  pretty_print(0,6,"Responder   " + String(" WiFi: ") + String(WiFi.SSID()),5,7,1,oled);
  // print the "last updated" at the bottom
  pretty_print(0, 58, timing_msg,5,7,1,oled);
  oled.sendBuffer();
}


String do_get(){
  String out = "";
  WiFiClient client; //instantiate a client object
  if (client.connect("iesc-s1.mit.edu", 80)) { //try to connect to iesc-s1.mit.edu
    client.println("GET /608dev/sandbox/linnear/emergencyRequest.py?type=responder HTTP/1.1");
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
      if (millis()-count>response_timeout) break;
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


String do_post(String status, String path){
  String out = "";
  WiFiClient client; //instantiate a client object
  if (client.connect("iesc-s1.mit.edu", 80)) { //try to connect to iesc-s1.mit.edu
    String body = "{\"type\":\"responder\", \"status\":\""+status+"\", \"path\":\""+path+"\"}";
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
      if (millis()-count>response_timeout) break;
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
