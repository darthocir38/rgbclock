// Includes
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>


#include <Time.h>  
#include <DS1307RTC.h>

// defines

// Pin for LEDs
#define IRPIN 9

// Objects 
IRrecv irrecv(IRPIN);
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// mode and state defines
#define STATE_CLOCK 0
#define STATE_AMBIE 1
uint8_t mode = STATE_CLOCK;

// Variables

decode_results results;


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
  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(4,0); lcd.print("Simple Clock");
  lcd.setCursor(1,3), lcd.print("MfG King of Chaos!");
  
  setSyncProvider(RTC.get);
  setSyncInterval(1);
  lasttime = millis();
  currenttime = millis();
  lastsecond = second();
  
  irrecv.enableIRIn(); // Start the receiver

}

// Main Loop
void loop()
{
  // timing
  setTime();
  if (irrecv.decode(&results)) {
    switch(results.value){
      case 0x77E150DA: 
        // +
        mode = 1;
        break; 
      case 0x77E160DA: 
        // >>
        mode = 2;
        break; 
      case 0x77E190DA: 
        // << 
        mode = 3;
        break; 
      case 0x77E130DA: 
        // -
        mode = 4;
        break; 
      case 0x77E1A0DA: 
        // >||
        mode = 5;
        break; 
      case 0x77E1C0DA: 
        // menu
        //mode++;
        //if(mode==2) mode = 0;
        mode = 6;
        break; 
      default: 
        mode = 8;
        break;
  }
  lcd.setCursor(0,0); lcd.print("Simple Clock "); lcd.print(results.value,16);
  irrecv.resume(); // Receive the next value
  }
  
  
  
 }


/*
void printClockLCD(){
  lcd.setCursor(0,1);
  lcd.print("Time: ");if(hour24<10) lcd.print(0); lcd.print(hour24, DEC);
  lcd.print(" : ");   if(curr_minute<10) lcd.print(0); lcd.print(curr_minute, DEC);
  lcd.print(" : ");   if(currentsecond<10) lcd.print(0); lcd.print(currentsecond, DEC);	
  lcd.setCursor(0,2); if(light<100) lcd.print(0);lcd.print(light, DEC);
}
*/
void setTime(){
   t = now();
   currenttime = millis();
   hour24 = hour(t);
    
   hour12 = (hour24>12) ? hour24-12 : hour24;
   curr_minute =  minute(t);
   currentsecond = second(t);
}







