/*
  Optical Heart Rate Detection and Blood Oxygen Levels
  By: Mark Wottreng
  Date: 24 January 2022

  References:
  https://github.com/sparkfun/MAX30105_Breakout
  https://github.com/aromring/MAX30102_by_RF
  https://github.com/Protocentral/Pulse_MAX30102


  This is a demo to show the reading of heart rate and blood oxygen level
  This code is unverified and not calibrated, USE FOR DEVELOPEMENT ONLY

  It is best to attach the sensor to your finger using a rubber band or other tightening
  device. Humans are generally bad at applying constant pressure to a thing. When you
  press your finger against the sensor it varies enough to cause the blood in your
  finger to flow differently which causes the sensor readings to go wonky.

  Hardware Connections (Breakoutboard to ESP8266 NodeMCU):
  -3.3V power
  -GND
  -SDA = D1
  -SCL = D2
  -INT = D6

  * NOTE: if reading are not consistent, some calibration may be required
          see maxim_max30102_init() in /lib/max30102/max30102.cpp
*/

//#include <Wire.h>
#define SDA_PIN 5 //D1
#define SCL_PIN 4 //D2
#define int_pin 12 //D6

#include <max30102.h>
#include <SPI.h>
#include <algorithmRF.h>

long samplesTaken = 0; //Counter for calculating the Hz or read rate
//
uint32_t elapsedTime,timeStart;

uint32_t aun_ir_buffer[BUFFER_SIZE]; //infrared LED sensor data
uint32_t aun_red_buffer[BUFFER_SIZE];  //red LED sensor data
float old_n_spo2;  // Previous SPO2 value
uint8_t uch_dummy,k;

//
void millis_to_hours(uint32_t ms, char* hr_str)
{
  char istr[6];
  uint32_t secs,mins,hrs;
  secs=ms/1000; // time in seconds
  mins=secs/60; // time in minutes
  secs-=60*mins; // leftover seconds
  hrs=mins/60; // time in hours
  mins-=60*hrs; // leftover minutes
  itoa(hrs,hr_str,10);
  strcat(hr_str,":");
  itoa(mins,istr,10);
  strcat(hr_str,istr);
  strcat(hr_str,":");
  itoa(secs,istr,10);
  strcat(hr_str,istr);
}
//
void setup()
{
  //Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(int_pin, INPUT);

  Serial.begin(115200);
  Serial.println("Initializing...");


  // Initialize sensor
  if (!maxim_max30102_init()) // I2C port defined in max30102.cpp init(), 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  uint8_t uch_dummy;
  maxim_max30102_read_reg(REG_REV_ID, &uch_dummy);
  Serial.print("Rev ID: "); // sensor revision, code is targeted at Rev 2+
  Serial.println(uch_dummy);

  old_n_spo2=0.0;
  /*
   while(Serial.available()==0)  //wait until user presses a key
  {
    Serial.println(F("Press any key to start conversion"));
    delay(2000);
  }
  uch_dummy=Serial.read();
  */
  Serial.print(F("Time[s]\tSpO2\tHR\tClock\tTemp[C]"));



  //startTime = millis();
  timeStart=millis();
}

void loop()
{
  float n_spo2,ratio,correl;  //SPO2 value
  int8_t ch_spo2_valid;  //indicator to show if the SPO2 calculation is valid
  int32_t n_heart_rate; //heart rate value
  int8_t  ch_hr_valid;  //indicator to show if the heart rate calculation is valid
  int32_t i;
  char hr_str[10];
     
  //buffer length of BUFFER_SIZE stores ST seconds of samples running at FS sps
  //read BUFFER_SIZE samples, and determine the signal range
  for(i=0;i<BUFFER_SIZE;i++)
  {
    while(digitalRead(int_pin)==1);  //wait until the interrupt pin asserts
    maxim_max30102_read_fifo((aun_red_buffer+i), (aun_ir_buffer+i));  //read from MAX30102 FIFO
    delay(2); // need time for int pin to reset
    // Serial.print("");
    // Serial.print(i, DEC);
    //  Serial.print(F("\t"));
    //  Serial.print(aun_red_buffer[i], DEC);
    //  Serial.print(F("\t"));
    //  Serial.print(aun_ir_buffer[i], DEC);
    //  Serial.println("");
  }

  //calculate heart rate and SpO2 after BUFFER_SIZE samples (ST seconds of samples) using Robert's method
  rf_heart_rate_and_oxygen_saturation(aun_ir_buffer, BUFFER_SIZE, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid, &ratio, &correl); 
  elapsedTime=millis()-timeStart;
  millis_to_hours(elapsedTime,hr_str); // Time in hh:mm:ss format
  elapsedTime/=1000; // Time in seconds

  // Read the _chip_ temperature in degrees Celsius
  int8_t integer_temperature;
  uint8_t fractional_temperature;
  maxim_max30102_read_temperature(&integer_temperature, &fractional_temperature);
  float temperature_C = integer_temperature + (((float)fractional_temperature)/16.0);
  float temperature_F = (temperature_C * 1.8) + 32; // convert to F
  //

  Serial.println("------");
  Serial.print(elapsedTime);
  Serial.print("\t");
  Serial.print(n_spo2);
  Serial.print("\t");
  Serial.print(n_heart_rate, DEC);
  Serial.print(" BPM\t");
  Serial.print(hr_str);
  Serial.print("\t");
  Serial.print(temperature_F);
  Serial.println(" F");
  Serial.println("------");
}




