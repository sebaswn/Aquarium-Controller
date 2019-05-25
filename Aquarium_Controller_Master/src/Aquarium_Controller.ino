#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
//#include <Nextion.h>
#include <SPI.h>
//#include <SD.h>

#define WaterTemperatureSensor 10
#define DHTPIN 11
#define DHTTYPE DHT22
#define relayHeaterPin 2

#define CELCIUS true
#define ON true
#define FAHRENHEIT false
#define OFF false

#define HOMEPAGE 0
#define SETTINGSPAGE 1
#define WATERPAGE 4


#define WaterTempGoalC 23
#define WaterTempGoalF 77

#define delayTime 100
unsigned long previousTime = 0;

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(WaterTemperatureSensor);
DallasTemperature WaterSensor(&oneWire);

//PageID, ID, Name
//NexText tAirTemp = NexText(0, 6, "tAirTemp");
//NexText tAirHum = NexText(0, 7, "tAirHum");
//NexText tWaterTemp = NexText(0, 12, "tWaterTemp");
//NexCheckbox cCelcius = NexCheckbox(1, 7, "cCelcius");
//NexCheckbox cFahrenheit = NexCheckbox(1, 6, "cFahrenheit");
//NexText tWaterMeasure = NexText(0, 21, "tWaterMeasure");
//NexText tAirMeasure = NexText(0, 19, "tAirMeasure");
//NexText tWaterTemp2 = NexText(4, 5, "tWaterTemp");
//NexText tWaterMeasure2 = NexText(4, 9, "tWaterMeasure");


//Temperatures
float waterTemp, airTemp, airHum;
boolean temperatureSetting = CELCIUS;
int currentPage = HOMEPAGE;
boolean heaterStatus = OFF;

//char buffer[100] = {0};

void setup() {
  Serial.begin(9600);
  //nexInit();
  WaterSensor.begin();
  dht.begin();
  Wire.begin();


  pinMode(relayHeaterPin, OUTPUT);
}

void loop() {
  checkForInstructions();

  if (delayTime <= millis() - previousTime) {
    checkRelays();

    if (currentPage == HOMEPAGE) {
      updateSensorsOnMainScreen();
    } else if (currentPage == SETTINGSPAGE) {
      //updateSettingsPage();
    } else if (currentPage == WATERPAGE) {

      updateWaterPage();
    }

    previousTime = millis();
  }
}



void checkForInstructions(){
  Wire.beginTransmission(1);
  Wire.requestFrom(1, 2);    // request 6 bytes from slave device #1
  int receivedValue = Wire.read() << 8 | Wire.read();
  Wire.endTransmission();

  if(receivedValue != 0){

    if (receivedValue == 1) {
      currentPage = HOMEPAGE;
      updateSensorsOnMainScreen();
      updateGraphicsMainPage();
    } else if (receivedValue == 2) {
      currentPage = SETTINGSPAGE;
      updateSettingsPage();
    } else if (receivedValue == 3) {
      temperatureSetting = CELCIUS;
    } else if (receivedValue == 4) {
      temperatureSetting = FAHRENHEIT;
    } else if (receivedValue == 5) {
      currentPage = WATERPAGE;
    }

  }
}

void updateWaterPage() {
  getWaterSensorsReadings();
  int tempWaterTemp = waterTemp;
  if (temperatureSetting == CELCIUS) {
    tempWaterTemp = map(tempWaterTemp, WaterTempGoalC - 5, WaterTempGoalC + 5, 0, 100);
  } else {
    tempWaterTemp = map(tempWaterTemp, WaterTempGoalF - 10, WaterTempGoalF + 10, 0, 100);
  }

  //sWaterTemp.addValue(0,   tempWaterTemp);
  writeToScreen("add ", "5,0," + String(tempWaterTemp));

  writeToScreen("tWaterTemp.txt=\"", String(waterTemp) + "\"");

  if (temperatureSetting == CELCIUS) {

    writeToScreen("tWaterMeasure2.txt=\"", "C\"");
  } else if (temperatureSetting == FAHRENHEIT) {
      writeToScreen("tWaterMeasure2.txt=\"", "F\"");
  }

  if (heaterStatus == ON) {
    Serial.print("p4.pic=29");
    Serial.write(0xFF);
    Serial.write(0xFF);
    Serial.write(0xFF);
  } else {
    Serial.print("p4.pic=28");
    Serial.write(0xFF);
    Serial.write(0xFF);
    Serial.write(0xFF);
  }
}

void checkRelays() {
  if (waterTemp < WaterTempGoalC) {
    heaterStatus = ON;
    digitalWrite(relayHeaterPin, LOW);
  } else {
    heaterStatus = OFF;
    digitalWrite(relayHeaterPin, HIGH);
  }
}

void updateSettingsPage() {
  if (temperatureSetting == CELCIUS) {
    writeToScreen("cFahrenheit.val=", 0);
    writeToScreen("cCelcius.val=", 1);

  } else {
    writeToScreen("cFahrenheit.val=", 1);
    writeToScreen("cCelcius.val=", 0);
  }

}



void updateGraphicsMainPage(){
  if (temperatureSetting == CELCIUS) {
    writeToScreen("tWaterMeasure.txt=\"", "C\"");
    writeToScreen("tAirMeasure.txt=\"", "C\"");

  } else if (temperatureSetting == FAHRENHEIT) {
    writeToScreen("tWaterMeasure.txt=\"", "F\"");
    writeToScreen("tAirMeasure.txt=\"", "F\"");
  }

  if (heaterStatus == ON) {
    writeToScreen("p4.pic=", "29");
  } else {
    writeToScreen("p4.pic=", "28");
  }
}

void getWaterSensorsReadings() {
  WaterSensor.requestTemperatures();
  if (temperatureSetting == CELCIUS) {
    waterTemp = WaterSensor.getTempCByIndex(0);
  } else {
    waterTemp = WaterSensor.getTempFByIndex(0);
  }
}

void getAirSensorsReadings() {
  airHum = dht.readHumidity();
  if (temperatureSetting == CELCIUS) {
    airTemp = dht.readTemperature();
  } else {
    airTemp = dht.readTemperature(true);
  }
}

void updateSensorsOnMainScreen() {
  getWaterSensorsReadings();
  getAirSensorsReadings();

  writeToScreen("tAirHum.txt=\"", String(airHum) + "\"");
  writeToScreen("tAirTemp.txt=\"", String(airTemp) + "\"");
  writeToScreen("tWaterTemp.txt=\"", String(waterTemp) + "\"");

  updateGraphicsMainPage();

}

void writeToScreen(String instruction, String message){
  Serial.print(instruction);
  Serial.print(message);
  Serial.write(0xFF);
  Serial.write(0xFF);
  Serial.write(0xFF);
}

void writeToScreen(String instruction, int value){
  Serial.print(instruction);
  Serial.print(value);
  Serial.write(0xFF);
  Serial.write(0xFF);
  Serial.write(0xFF);
}
