/*
This code is what will be running on the single lamp
ESP32 module. 
Basically, it should perform a GET request every 3 seconds,
checking to see if its status has been updated on the 
server to one of the emergency states, and update the
lamp color based on that information.
*/

#include <U8g2lib.h>
#include <base64.h>
#include <WiFiClientSecure.h>

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
int LAST_STATE;

const int response_timeout = 6000; //ms to wait for response from host
const int button_pin = 15;
unsigned long get_timer = millis();
unsigned long put_timer = millis();
int put_delay = 10000;
int get_delay = 5000;
int STATE = 0;
int LIGHTSTATE = 0;
String type = "";
String message;
String status;

SCREEN oled(U8G2_R0, 5, 17, 16);

#define DELAY 1000
#define SAMPLE_FREQ 8000                           // Hz, telephone sample rate
#define SAMPLE_DURATION 5                         // duration of fixed sampling
#define NUM_SAMPLES SAMPLE_FREQ*SAMPLE_DURATION    // number of of samples
#define ENC_LEN (NUM_SAMPLES + 2 - ((NUM_SAMPLES + 2) % 3)) / 3 * 4  // Encoded length of clip

/* CONSTANTS */
//Prefix to POST request:
const String PREFIX = "{\"config\":{\"encoding\":\"MULAW\",\"sampleRateHertz\":8000,\"profanityFilter\":true,\"languageCode\": \"en-US\"}, \"audio\": {\"content\":\"";
const String SUFFIX = "\"}}"; //suffix to POST request
const int AUDIO_IN = A0; //pin where microphone is connected
const int BUTTON_PIN = 15; //pin where button is connected
const String API_KEY = "AIzaSyC2nT5F69sBBaldwhMkcf_nLxzpexAMslg";


/* Global variables*/
int button_state; //used for containing button state and detecting edges
int old_button_state; //used for detecting button edges
unsigned long time_since_sample;      // used for microsecond timing
String speech_data; //global used for collecting speech data
const char* ssid     = "6s08";     // 6s08 your network SSID (name of wifi network)
const char* password = "iesc6s08"; // iesc6s08 your network password
const char*  server = "speech.google.com";  // Server URL

WiFiClientSecure client; //global WiFiClient Secure object

//Below is the ROOT Certificate for Google Speech API authentication (we're doing https so we need this)
//don't change this!!
const char* root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G\n" \
"A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNp\n" \
"Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1\n" \
"MDgwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEG\n" \
"A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n" \
"hvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8omUVCxKs+IVSbC9N/hHD6ErPL\n" \
"v4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7SqbKSaZeqKeMWhG8\n" \
"eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQBoZfXklq\n" \
"tTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd\n" \
"C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pa\n" \
"zq+r1feqCapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCB\n" \
"mTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IH\n" \
"V2ccHsBqBt5ZtJot39wZhi4wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5n\n" \
"bG9iYWxzaWduLm5ldC9yb290LXIyLmNybDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG\n" \
"3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEAmYFThxxol4aR7OBKuEQLq4Gs\n" \
"J0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkIk7mpM0sYmsL4h4hO\n" \
"291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRDLenVOavS\n" \
"ot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd\n" \
"AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7\n" \
"TBj0/VLZjmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==\n" \
"-----END CERTIFICATE-----\n";

// Pins
const int TRIG_PIN = 12;
const int ECHO_PIN = 14;
//The Button pin is now pin 15
// Anything over 400 cm (23200 us pulse) is "out of range"
//~54.24 cm or 21 inches (3146 us pulse)
const unsigned int MAX_DIST = 4146;
#define TIMEOUT_PERIOD 3000
int state=1;
int last_on;
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
  // The Trigger pin will tell the sensor to range find
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  oled.setFont(u8g2_font_5x7_tf); //set font on oled
  oled.begin();
  WiFi.begin(ssid,password); //WiFi.begin(ssid,password);      //attempt to connect to wifi
  oled.setCursor(0, 15);  //set cursor
  speech_data.reserve(PREFIX.length()+ENC_LEN+SUFFIX.length());
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
      if (isDetected() == 1 and last_on != isDetected()){
        do_put("0.5");
        if (listen() == 1){
          STATE = FIRE;
        }
        put_timer = millis();
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
  // "put" if we don't see motion for a while
  if (isDetected() == 1 && millis() - put_timer > put_delay) {
    Serial.println("doing the PUT request");
    do_put("0.1");
    put_timer = millis();
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
  last_on = isDetected();
  oled.sendBuffer();
  LAST_STATE = STATE;
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


String do_put(String inp){
  String out = "";
  WiFiClient client; //instantiate a client object
  if (client.connect("iesc-s1.mit.edu", 80)) { //try to connect to iesc-s1.mit.edu
    String body = "{\"type\":\"lamp\",\"status\":"+String(inp)+",\"ID\":"+String(ID)+"}";
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

int listen(){
    int alert = 0;
    client.setCACert(root_ca);
    delay(200);
    Serial.println("listening...");
    oled.clearBuffer();    //clear the screen contents
    oled.drawStr(0,15,"listening...");
    oled.sendBuffer();     // update the screen
    record_audio();
    Serial.println("sending...");    
    oled.clearBuffer();    //clear the screen contents
    oled.drawStr(0,15,"sending...");
    oled.sendBuffer();     // update the screen
    Serial.print("\nStarting connection to server...");
    delay(300);
    bool conn = false;
    for (int i=0; i<10; i++){
      if (client.connect(server,443));{
        conn = true;
        break;
      }
      Serial.print(".");
      delay(300);
    }
    if (!conn){
        Serial.println("Connection failed!");
        return 0;
    }else {
      Serial.println("Connected to server!");
      // Make a HTTP request:
    delay(200);
    client.println("POST https://speech.googleapis.com/v1/speech:recognize?key="+API_KEY+"  HTTP/1.1");
    client.println("Host: speech.googleapis.com");
    client.println("Content-Type: application/json");
    client.println("Cache-Control: no-cache");
    client.println("Content-Length: " + String(speech_data.length()));
    client.print("\r\n");
    int len = speech_data.length();
    int ind = 0;
    int jump_size=3000;
    while (ind<len){
      delay(100);//experiment with this number!
      if (ind+jump_size<len) client.print(speech_data.substring(ind,ind+jump_size));
      else client.print(speech_data.substring(ind));
      ind+=jump_size;
    }
    //client.print("\r\n\r\n");
    unsigned long count = millis();
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      Serial.print(line);
      if (line == "\r") { //got header of response
        Serial.println("headers received");
        break;
      }
      if (millis()-count>4000) break;
    }
    Serial.println("Response...");
    count = millis();
    while (!client.available()) {
      delay(100);
      Serial.print(".");
      if (millis()-count>4000) break;
    }
    Serial.println();
    Serial.println("-----------");
    String op;
    while (client.available()) {
      op+=(char)client.read();
    }
    Serial.println(op);
    int trans_id = op.indexOf("transcript");
    if (trans_id != -1){
      int foll_coll = op.indexOf(":",trans_id);
      int starto = foll_coll+2; //starting index
      int endo = op.indexOf("\"",starto+1); //ending index
      oled.clearBuffer();    //clear the screen contents
      oled.setCursor(0,15);
      String message = op.substring(starto+1,endo);
      if(message == "fire fire fire"){
        alert = 1;
      }
      else {
        oled.print(message);
        
      }
      oled.sendBuffer();     // update the screen 
      delay(2000);
    }
    Serial.println("-----------");
    client.stop();
    Serial.println("done");
    
    }
    return alert;
}

void record_audio() {
  int sample_num = 0;    // counter for samples
  int enc_index = PREFIX.length()-1;    // index counter for encoded samples
  float time_between_samples = 1000000/SAMPLE_FREQ;
  int value = 0;
  uint8_t raw_samples[3];   // 8-bit raw sample data array
  String enc_samples;     // encoded sample data array
  unsigned long time_since_begin = millis();
  time_since_sample = micros();
  Serial.println(NUM_SAMPLES);
  speech_data = PREFIX;
  while (sample_num<NUM_SAMPLES and millis() - time_since_begin < 5000) {   //read in NUM_SAMPLES worth of audio data
    value = analogRead(AUDIO_IN);  //make measurement
    raw_samples[sample_num%3] = mulaw_encode(value-1241); //remove 1.0V offset (from 12 bit reading)
    sample_num++;
    if (sample_num%3 == 0) {
      speech_data+=base64::encode(raw_samples, 3);
    }

    // wait till next time to read
    while (micros()-time_since_sample <= time_between_samples); //wait...
    time_since_sample = micros();
  }
  speech_data += SUFFIX;
}

int8_t mulaw_encode(int16_t sample){
   const uint16_t MULAW_MAX = 0x1FFF;
   const uint16_t MULAW_BIAS = 33;
   uint16_t mask = 0x1000;
   uint8_t sign = 0;
   uint8_t position = 12;
   uint8_t lsb = 0;
   if (sample < 0)
   {
      sample = -sample;
      sign = 0x80;
   }
   sample += MULAW_BIAS;
   if (sample > MULAW_MAX)
   {
      sample = MULAW_MAX;
   }
   for (; ((sample & mask) != mask && position >= 5); mask >>= 1, position--)
        ;
   lsb = (sample >> (position - 4)) & 0x0f;
   return (~(sign | ((position - 5) << 4) | lsb));
}

