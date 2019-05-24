#include <Wire.h>
#include <Nextion.h>

#define CELCIUS true
#define FAHRENHEIT false

NexCheckbox cCelcius = NexCheckbox(1,7, "cCelcius");
NexCheckbox cFahrenheit = NexCheckbox(1,6, "cFahrenheit");
NexPage Home = NexPage(0,0,"Home");
NexPage Settings = NexPage(1,0,"Settings");
NexPage Water = NexPage(4,0,"Water");

NexTouch *nex_listen_list[] = {
  &cCelcius,
  &cFahrenheit,
  &Home,
  &Settings,
  &Water,
  NULL
};

int instruction = 0;
boolean instructionSent = false;

void setup() {

  Wire.begin(1); // Slave ID = 1
  nexInit();

  Wire.onReceive(receiveEvent);  // Attach a function to trigger when something is received.
  Wire.onRequest(requestEvent);

  cCelcius.attachPop(cCelciusPopCallback, &cCelcius);
  cFahrenheit.attachPop(cFahrenheitPopCallback, &cFahrenheit);
  Home.attachPop(HomePopCallback);
  Settings.attachPop(SettingsPopCallback);
  Water.attachPop(WaterPopCallback);
}

void loop() {
  nexLoop(nex_listen_list);
}

void receiveEvent(int bytes) {
 int test = Wire.read();    // read one character from the I2C
}

void requestEvent(){
  uint8_t buffer[2];
  buffer[0] = instruction >> 8;
  buffer[1] = instruction & 0xff;
  Wire.write(buffer, 2);
  instruction = 0;
}


void cCelciusPopCallback(void *ptr) {
  instruction = 3;
}

void cFahrenheitPopCallback(void *ptr) {
  instruction = 4;
}

void HomePopCallback(void *ptr){
  digitalWrite(13,HIGH);
  instruction = 1;
}

void SettingsPopCallback(void *ptr){
  digitalWrite(13,LOW);
  instruction = 2;
}

void WaterPopCallback(void *ptr){
  digitalWrite(13,LOW);
  instruction = 5;
}
