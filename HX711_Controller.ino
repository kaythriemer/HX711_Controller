/*
   -------------------------------------------------------------------------------------
   HX711_ADC
   Arduino library for HX711 24-Bit Analog-to-Digital Converter for Weight Scales
   Olav Kallhovd sept2017
   -------------------------------------------------------------------------------------
*/


#include <HX711_ADC.h>

//************************************************
// Diese Werte können angepasst werden
//************************************************
// Power definitions
const int KRAFT1 = 20;
const int KRAFT2 = 200;
const int KRAFT3 = 500;
const int KRAFT4 = 1000;
const int KRAFT5 = 1500;

//readout frequency / time between two measures in ms  HX711 fastest is 10HZ(100ms)
const int rate = 100;

// blink frequency in ms for KRAFT between KRAFT4 and KRAFT5
const long interval = 100;

//activate debug
const bool DEBUG = true;

//activate tare during startup
const bool TARE_AT_START = true;
//*********************************************

//pins:
const int HX711_dout = A4;  //mcu > HX711 dout pin
const int HX711_sck = A5;   //mcu > HX711 sck pin

// Hint: PWM can be used to mix colors, e.g. yellow = red(255) + green(160)
const int REDPIN = 11;
const int REDPWM = 255;
int redState = LOW;

const int GREENPIN = 10;
const int GREENPWM = 160;

const int BLUEPIN = 9;
const int BLUEPWM = 255;
int blueState = LOW;

const int WHITEPIN = 6;
const int WHITEPWM = 255;

const int ALARMPIN = 5;
const int ALARMPWM = 255;

unsigned long previousMillis = 0;

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_calVal_eepromAdress = 0;
unsigned long t = 0;

float i = 0;

void setup() {

  pinMode(REDPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);


  analogWrite(REDPIN, 0);
  analogWrite(GREENPIN, 0);
  analogWrite(BLUEPIN, 0);
  analogWrite(WHITEPIN, 0);

  analogWrite(ALARMPIN, 0);

  Serial.begin(9600);
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  float calibrationValue;    // calibration value
  calibrationValue = 696.0;  // uncomment this if you want to set this value in the sketch

  LoadCell.begin();
  //LoadCell.setReverseOutput();
  unsigned long stabilizingtime = 2000;  // tare preciscion can be improved by adding a few seconds of stabilizing time
  boolean _tare = true;                  //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  } else {
    LoadCell.setCalFactor(calibrationValue);  // set calibration factor (float)
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update())
    ;
  Serial.print("Calibration value: ");
  Serial.println(LoadCell.getCalFactor());
  Serial.print("HX711 measured conversion time ms: ");
  Serial.println(LoadCell.getConversionTime());
  Serial.print("HX711 measured sampling rate HZ: ");
  Serial.println(LoadCell.getSPS());
  Serial.print("HX711 measured settlingtime ms: ");
  Serial.println(LoadCell.getSettlingTime());
  Serial.println("Note that the settling time may increase significantly if you use delay() in your sketch!");
  if (LoadCell.getSPS() < 7) {
    Serial.println("!!Sampling rate is lower than specification, check MCU>HX711 wiring and pin designations");
  } else if (LoadCell.getSPS() > 100) {
    Serial.println("!!Sampling rate is higher than specification, check MCU>HX711 wiring and pin designations");
  }

  // Tare
  if (TARE_AT_START) {
    LoadCell.tare();
    // check if tare operation is complete:
    if (LoadCell.getTareStatus() == true) {
      Serial.println("Tare complete");
    }
  }
}

void loop() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = rate;  

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      i = LoadCell.getData();
      if (DEBUG) {
        Serial.print("Load_cell output val: ");
        Serial.println(i);
      }
      newDataReady = 0;
      t = millis();
    }
  }


  // set color values
  if (i < KRAFT1) {
    analogWrite(REDPIN, 0);
    analogWrite(GREENPIN, 0);
    analogWrite(BLUEPIN, 0);
    analogWrite(ALARMPIN, 0);
    analogWrite(WHITEPIN, WHITEPWM);
  } else if (i < KRAFT2) {
    analogWrite(BLUEPIN, 0);
    analogWrite(WHITEPIN, 0);
    analogWrite(ALARMPIN, 0);
    //mix yellow
    analogWrite(REDPIN, REDPWM);
    analogWrite(GREENPIN, GREENPWM);
  } else if (i < KRAFT3) {
    analogWrite(REDPIN, 0);
    analogWrite(BLUEPIN, 0);
    analogWrite(WHITEPIN, 0);
    analogWrite(ALARMPIN, 0);
    analogWrite(GREENPIN, GREENPWM);
  } else if (i < KRAFT4) {
    analogWrite(GREENPIN, 0);
    analogWrite(BLUEPIN, 0);
    analogWrite(WHITEPIN, 0);
    analogWrite(ALARMPIN, 0);
    analogWrite(REDPIN, REDPWM);
  } else if (i < KRAFT5) {  // TODO BLINK RED or RED/BLUE
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      // if the LED is off turn it on and vice-versa:
      if (redState == LOW) {
        redState = HIGH;
        blueState = LOW;
      } else {
        redState = LOW;
        blueState = HIGH;
      }

      // set the LED with the ledState of the variable:
      digitalWrite(REDPIN, redState);
      digitalWrite(BLUEPIN, blueState);
    }
  } else {
    analogWrite(REDPIN, 0);
    analogWrite(GREENPIN, 0);
    analogWrite(BLUEPIN, 0);
    analogWrite(WHITEPIN, 0);
    analogWrite(ALARMPIN, ALARMPWM);
  }
}
