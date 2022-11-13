#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char * ssid = "*****";
const char * password = "*****";

WiFiClientSecure client;

static const uint8_t D0 = 16;
static const uint8_t D1 = 5;
static const uint8_t D2 = 4;
static const uint8_t D3 = 0;
static const uint8_t D4 = 2;
static const uint8_t D5 = 14;
static const uint8_t D6 = 12;
static const uint8_t D7 = 13;
static const uint8_t D8 = 15;

int latchPin = D5;
int clockPin = D6;
int dataPin = D4;
int comPin[] = {D3, D2, D1, D0};// Common pin (anode) of 4 digit 7-segment display
// Pin connected to ST_CP of 74HC595(Pin12) // Pin connected to SH_CP of 74HC595(Pin11) // Pin connected to DS of 74HC595(Pin14)
// Define the encoding of characters 0-F of the common-anode 7-Segment Display
byte num[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0x88, 0x83, 0xc6, 0xa1, 0x86, 0x8e};


int day = 1234;
byte bit[4];
int update = 20000;


void setup() {
  // set pins to output
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(comPin[i], OUTPUT);
  }

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Ready to go");

}

void loop() {

  bit[0] = day % 10;
  bit[1] = day / 10 % 10;
  bit[2] = day / 100 % 10;
  bit[3] = day / 1000 % 10;

  for (int i = 0; i < 4; i++) {
    // Select a single 7-segment display
    chooseCommon(i);

    writeData(num[bit[3 - i]]);
    delay(5);
    writeData(0xff);
  }
  update ++;
  if (update == 30000) {
    update = 0;
    spreadsheet_comm();
  }
}

void spreadsheet_comm(void) {
  HTTPClient http;
  String url_read = "https://script.google.com/macros/s/******/exec?read=read";
  String url_write = "https://script.google.com/macros/s/******/exec?write=update_0";

  // write/update
  client.setInsecure(); //the magic line, use with caution
  http.begin(client, url_write.c_str()); //Specify the URL and certificate
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.end();

  // read
  client.setInsecure(); //the magic line, use with caution
  http.begin(client, url_read.c_str()); //Specify the URL and certificate
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  String payload;
  if (httpCode > 0) { //Check for the returning code
    payload = http.getString();
    Serial.println(httpCode);
    Serial.println(payload);
    day = payload.toInt();
  }
  else {
    Serial.println("Error on HTTP request");
    Serial.println(httpCode);
  }

  Serial.println(day);
  http.end();
}

void chooseCommon(byte com) {
  // Close all single 7-segment display
  for (int i = 0; i < 4; i++) {
    digitalWrite(comPin[i], LOW);
  }
  // Open the selected single 7-segment display
  digitalWrite(comPin[com], HIGH);
}

void writeData(int value) {
  // Make latchPin output low level
  digitalWrite(latchPin, LOW);
  // Send serial data to 74HC595
  shiftOut(dataPin, clockPin, LSBFIRST, value);
  // Make latchPin output high level, then 74HC595 will update the data to parallel output
  digitalWrite(latchPin, HIGH);
}