/*
   -------------------------------------------------------------------------------------
   HX711_ADC
   Arduino library for HX711 24-Bit Analog-to-Digital Converter for Weight Scales
   Olav Kallhovd sept2017
   -------------------------------------------------------------------------------------
*/

/*
   Settling time (number of samples) and data filtering can be adjusted in the config.h file
   For calibration and storing the calibration value in eeprom, see example file "Calibration.ino"

   The update() function checks for new data and starts the next conversion. In order to acheive maximum effective
   sample rate, update() should be called at least as often as the HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS.
   If you have other time consuming code running (i.e. a graphical LCD), consider calling update() from an interrupt routine,
   see example file "Read_1x_load_cell_interrupt_driven.ino".

   This is an example sketch on how to use this library
*/

#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

//pins:
const int HX711_dout = A4;  //mcu > HX711 dout pin
const int HX711_sck = A5;   //mcu > HX711 sck pin

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


// Power definitions
const int KRAFT1 = 20;
const int KRAFT2 = 200;
const int KRAFT3 = 500;
const int KRAFT4 = 1000;
const int KRAFT5 = 1500;

//readout frequency / time between two measures in ms  HX711 fastest is 10HZ(100ms)
const int rate = 100;

// blink frequency in ms
const long interval = 100;

const bool DEBUG = false;


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
#if defined(ESP8266) || defined(ESP32)
  //EEPROM.begin(512); // uncomment this if you use ESP8266 and want to fetch this value from eeprom
#endif
  //EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch this value from eeprom

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
}

void loop() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = rate;  //increase value to slow down serial print activity

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

    // analogWrite(REDPIN, 0);
    // analogWrite(GREENPIN, 0);
    // analogWrite(WHITEPIN, 0);
    // analogWrite(ALARMPIN, 0);
    // analogWrite(BLUEPIN, BLUEPWM);
  } else {
    analogWrite(REDPIN, 0);
    analogWrite(GREENPIN, 0);
    analogWrite(BLUEPIN, 0);
    analogWrite(WHITEPIN, 0);
    analogWrite(ALARMPIN, ALARMPWM);
  }


  // receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  // check if last tare operation is complete:
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
}
