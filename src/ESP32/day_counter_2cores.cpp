#include <HTTPClient.h>
#include <WiFi.h>

// WiFi credentials
const char* ssid = "";
const char* password = "";

WiFiClientSecure client;

int latchPin = 17;
int clockPin = 16;
int dataPin = 18;
int comPin[] = {19, 21, 22, 23};// Common pin (anode) of 4 digit 7-segment display
// Pin connected to ST_CP of 74HC595(Pin12) // Pin connected to SH_CP of 74HC595(Pin11) // Pin connected to DS of 74HC595(Pin14)
// Define the encoding of characters 0-F of the common-anode 7-Segment Display
byte num[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0x88, 0x83, 0xc6, 0xa1, 0x86, 0x8e};


int day = 1234;
int save = 4321;
byte bit[4];

TaskHandle_t Task1;
TaskHandle_t Task2;

void setup() {
  // set pins to output
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(comPin[i], OUTPUT);
  }


  delay(1000);
  Serial.begin(115200);
  delay(1000);
  // connect to WiFi
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  Serial.flush();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Ready to go");

  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */
  delay(500);

  xTaskCreatePinnedToCore(
    Task2code,   /* Task function. */
    "Task2",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task2,      /* Task handle to keep track of created task */
    1);          /* pin task to core 1 */
  delay(500);

}

//Task1code: blinks an LED every 1000 ms
void Task1code( void * pvParameters ) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {

    if (day != save) {
      save = day;
      bit[0] = save % 10;
      bit[1] = save / 10 % 10;
      bit[2] = save / 100 % 10;
      bit[3] = save / 1000 % 10;
    }

    for (int i = 0; i < 4; i++) {
      // Select a single 7-segment display
      chooseCommon(i);

      writeData(num[bit[3 - i]]);
      delay(5);
      writeData(0xff);
    }
  }
}

//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters ) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    spreadsheet_comm();
  }
}

void loop() {
}

void spreadsheet_comm(void) {
  HTTPClient http;
  String url_read = "https://script.google.com/macros/s/***/exec?read=read";
  String url_write = "https://script.google.com/macros/s/***/exec?write=test";
  int httpCode;

  // write/update
  client.setInsecure(); //the magic line, use with caution
  http.begin(client, url_write.c_str()); //Specify the URL and certificate
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  httpCode = http.GET();
  http.end();

  // read
  client.setInsecure(); //the magic line, use with caution
  http.begin(client, url_read.c_str()); //Specify the URL and certificate
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  httpCode = http.GET();
  String payload;
  if (httpCode > 0) { //Check for the returning code
    payload = http.getString();
    //Serial.println(httpCode);
    //Serial.println(payload);
    day = payload.toInt();
  }
  else {
    Serial.println("Error on HTTP request");
    Serial.println(httpCode);
  }

  //Serial.println(day);
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