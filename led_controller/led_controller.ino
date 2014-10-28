// LEDs 
// rot Data --> Rot
// Grün 5V  --> Blau
// Schwarz Masse --> Schwarz


// Includes
#include <Wire.h> 
#include "FastLED.h"

#include <Time.h>  
#include <DS1307RTC.h>

// defines

//Number of LEEs in Strip
#define NUM_LEDS 60
// Pin for LEDs
#define LED_DATA_PIN 9
#define LIGHT_SENSOR A0//Grove - Light Sensor is connected to A0 of Arduino


// Define COLORS:
#define BASE_R 1
#define BASE_G 2
#define BASE_B 2
#define MARK_R 10
#define MARK_G 10
#define MARK_B 10
#define HOUR_R 0
#define HOUR_G 0
#define HOUR_B 255
#define MINU_R 0
#define MINU_G 255
#define MINU_B 0
#define SECC_R 255
#define SECC_G 0
#define SECC_B 0



// Objects 
CRGB leds[NUM_LEDS];

CRGB ledhour;
CRGB ledhourdim;
CRGB ledminu;
CRGB ledsecc;

// mode and state defines
#define STATE_CLOCK 0
#define STATE_AMBIE_RAIN 1
#define STATE_AMBIE_MAN 2
uint8_t mode = STATE_AMBIE_RAIN;

// Variables L.
const int numReadings = 20;
int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average


// globaler Helligkeitswert
uint8_t gvalue = 100;

// time cache

time_t t;
uint8_t hour12;
uint8_t hour24;
uint8_t curr_minute;
uint8_t currentsecond;

unsigned long currenttime = 0;
unsigned long lasttime = 0;
unsigned long alerttime = 0;

// last second
uint8_t lastsecond = 0;

// redraw flag
boolean redraw = 1;
// clock option show five minute dots
uint8_t coptionfivemin = 1;

// clock option invert colors
boolean coptioninvert = 0;

// clock option fade seconds
boolean coptionfade = 0;

// multiprupose counter
int counter = 0;


// Setup function
void setup()
{
  Wire.begin(2);
  Wire.onRequest(sendData);
  Wire.onReceive(receiveData); 
  Serial.begin(115200);

  setSyncProvider(RTC.get);
  setSyncInterval(1);
  lasttime = millis();
  currenttime = millis();
  lastsecond = second();
  
  FastLED.addLeds<WS2812, LED_DATA_PIN, GRB>(leds, NUM_LEDS);

  ledhour.setRGB(HOUR_R,HOUR_G,HOUR_B);
  ledhourdim.setRGB(HOUR_R/4,HOUR_G/4,HOUR_B/4);
  ledminu.setRGB(MINU_R,MINU_G,MINU_B);
  ledsecc.setRGB(SECC_R,SECC_G,SECC_B);

  int tmp = analogRead(LIGHT_SENSOR); 
  Serial.print("Init Value : ");       
  Serial.println(tmp);       

  for (int thisReading = 0; thisReading < numReadings; thisReading++) readings[thisReading] = tmp;          
  total = tmp*numReadings;
}

// Main Loop
void loop()
{
   // timing
  setTime();  
   if(currenttime - lasttime > 45) {
      switch(mode){
      case STATE_CLOCK: clockMode(); break;
      case STATE_AMBIE_RAIN: RainBowMode(); break;
      default: clockMode();
      }redraw = 1;
      lasttime = currenttime;
    } else {
      // if nothing to do average the ligth
      volatile int newligth = analogRead(LIGHT_SENSOR); 
      
      total= total - readings[index];
      //Serial.print("Index: ");       
      //Serial.print(index);           
      //Serial.print(" Old: ");       
      //Serial.print(readings[index]);       
      readings[index] = newligth; 
      total= total + readings[index];       
      index = index + 1;                    
      if (index >= numReadings) index = 0;           
      average = total / numReadings;    
      //Serial.print(" Akt: ");       
      //Serial.print(readings[index]);      
      //Serial.print(" Akt: ");       
      //Serial.print(newligth);       
      //Serial.print(" Tot: ");       
      //Serial.print(total);         
      //Serial.print(" Avg. ");             
      //Serial.println(average);       
    }
    
   //delay(1000);
   // redraw if needed
  if(redraw) {
    FastLED.show((average>>3)+5);
    redraw = 0;
  }
}

void setTime(){
   t = now();
   currenttime = millis();
   hour24 = hour(t);
    
   hour12 = (hour24>12) ? hour24-12 : hour24;
   curr_minute =  minute(t);
   currentsecond = second(t);
}


void drawClockBase(){
  fill_solid( &(leds[0]), 60 /*number of leds*/, CRGB( BASE_R, BASE_G, BASE_B) );
  //lightPixels(BASE_R, BASE_G,BASE_B);
  for (uint8_t i=0; i<60; i += 5) {
    leds[i].setRGB(MARK_R, MARK_G, MARK_B);
  }
}


// clock mode
void clockMode() {
 
  drawClockBase();
    
  uint8_t analoghour = hour12*5+(minute(t)/12);

  
  
  leds[pixelCheck(analoghour-1)] +=  (ledhour);  //.setRGB(70, 0, 0);
  leds[pixelCheck(analoghour)]   += ledhour;  //.setRGB  (255, 0, 0);
  leds[pixelCheck(analoghour+1)] +=  (ledhourdim); //.setRGB(70, 0, 0);
  
  leds[minute(t)]+=ledminu;//.setRGB(0, 0, 255);
  
  if (coptionfade) {
    // reset counter
    if(counter>25) {
      counter = 0;
    }
    else if (lastsecond != currentsecond) {
      lastsecond = second();
      counter = 0;  
    }
    leds[pixelCheck(second(t)+1)].setRGB(0, counter*10, 0);  
    leds[second(t)].setRGB(0, 255-(counter*10), 0);
    counter++;
  }
  else {
    leds[second(t)] += ledsecc; //.setRGB(0, 255, 0);
  }
  
  
}

void RainBowMode(){
  fill_rainbow( leds, NUM_LEDS, counter, 8);
  counter++;
  if(counter == 256) counter = 0;
}



// set the correct pixels
int pixelCheck(int i) {
  if (i>59) {
    i = i - 60;
  }
  if (i<0) {
    i = i +60;
  }
  return i;
}

// light all pixels with given values
void lightPixels(byte r,byte g,byte b) {
  for (uint8_t i=0; i<NUM_LEDS; i++) {
    //strip.setPixelColor(i,c);
    leds[i].setRGB( r, g, b);
  }
}


void sendData()
{
  // Byte Array for sending Data
  uint8_t bytes[2];
  // Get Value of Light
  bytes[0] = average & 0xff;
  bytes[1] = (average >> 8) & 0xff;
  // send Data
  Wire.write(bytes,2); 
}

#define CMD_CLK 1
#define CMD_TIM 2
void receiveData(int numBytes)
{
  int command = Wire.read();
  switch (command) {
  case CMD_CLK: break;
  case CMD_TIM: break;
  default: break;
  }
  
  while(1 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
}

