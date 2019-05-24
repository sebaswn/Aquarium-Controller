#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <Nextion.h>
#include <SPI.h>
#include <SD.h>

#define WaterTemperatureSensor 10
#define DHTPIN 11
#define DHTTYPE DHT22
#define relayHeaterPin 2

#define CELCIUS true
#define ON true
#define FAHRENHEIT false
#define OFF false

#define WaterTempGoalC 23
#define WaterTempGoalF 77

#define delayTime 50
unsigned long previousTime = 0;

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(WaterTemperatureSensor);
DallasTemperature WaterSensor(&oneWire);

//PageID, ID, Name
NexText tAirTemp = NexText(0, 6, "tAirTemp");
NexText tAirHum = NexText(0, 7, "tAirHum");
NexText tWaterTemp = NexText(0, 12, "tWaterTemp");
NexCheckbox cCelcius = NexCheckbox(1, 7, "cCelcius");
NexCheckbox cFahrenheit = NexCheckbox(1, 6, "cFahrenheit");
NexText tWaterMeasure = NexText(0, 21, "tWaterMeasure");
NexText tAirMeasure = NexText(0, 19, "tAirMeasure");
NexText tWaterTemp2 = NexText(4, 5, "tWaterTemp");
NexText tWaterMeasure2 = NexText(4, 9, "tWaterMeasure");
NexWaveform sWaterTemp = NexWaveform(4, 5, "sWaterTemp");


//Temperatures
float waterTemp, airTemp, airHum;
boolean temperatureSetting = CELCIUS;
int currentPage = 0;
boolean heaterStatus = OFF;

//char buffer[100] = {0};

void setup() {
  Serial.begin(9600);
  nexInit();
  WaterSensor.begin();
  dht.begin();
  Wire.begin();

  pinMode(relayHeaterPin, OUTPUT);
}

void loop() {

  Wire.beginTransmission(1);
  Wire.requestFrom(1, 2);    // request 6 bytes from slave device #1
  int receivedValue = Wire.read() << 8 | Wire.read();
  Wire.endTransmission();

  if (receivedValue == 1) {
    currentPage = 0;
    updateSensorsOnMainScreen();
  } else if (receivedValue == 2) {
    currentPage = 1;
    updateSettingsPage();
  } else if (receivedValue == 3) {
    temperatureSetting = CELCIUS;
    updateGraphicsMainPage();
  } else if (receivedValue == 4) {
    temperatureSetting = FAHRENHEIT;
    updateGraphicsMainPage();
  } else if (receivedValue == 5) {
    for (int i = 0; i < 325; i++) {
      sWaterTemp.addValue(0, 50);
    }
    currentPage = 4;
  }

  if (delayTime <= millis() - previousTime) {
    checkRelays();

    if (currentPage == 0) {
      updateSensorsOnMainScreen();
    } else if (currentPage == 1) {
      //updateSettingsPage();
    } else if (currentPage == 4) {

      updateWaterPage();
    }

    previousTime = millis();
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

  sWaterTemp.addValue(0,   tempWaterTemp);

  static char waterTempChar[5];
  dtostrf(waterTemp, 2, 2, waterTempChar);
  tWaterTemp2.setText(waterTempChar);

  if (temperatureSetting == CELCIUS) {
    tWaterMeasure2.setText("C");
  } else if (temperatureSetting == FAHRENHEIT) {
    tWaterMeasure2.setText("F");
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
    cFahrenheit.setValue(0);
    cCelcius.setValue(1);
  } else {
    cCelcius.setValue(0);
    cFahrenheit.setValue(1);
  }
}

void updateSensorsOnMainScreen() {
  getWaterSensorsReadings();
  getAirSensorsReadings();

  static char hum[4];
  dtostrf(airHum, 2, 1, hum);
  tAirHum.setText(hum);

  static char airTempChar[5];
  dtostrf(airTemp, 2, 2, airTempChar);
  tAirTemp.setText(airTempChar);

  static char waterTempChar[5];
  dtostrf(waterTemp, 2, 2, waterTempChar);
  tWaterTemp.setText(waterTempChar);

}

void updateGraphicsMainPage(){
  if (temperatureSetting == CELCIUS) {
    tWaterMeasure.setText("C");
    tAirMeasure.setText("C");
  } else if (temperatureSetting == FAHRENHEIT) {
    tWaterMeasure.setText("F");
    tAirMeasure.setText("F");
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
