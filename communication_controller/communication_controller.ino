



// Includes
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>
#include <Time.h>  
#include <DS1307RTC.h>

// Pin for LEDs
#define IRPIN 9

// Objects 
IRrecv irrecv(IRPIN);


LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
char buffer[20];



// mode and state defines
#define STATE_CLOCK 0
#define STATE_AMBIE_RAIN 1
#define STATE_AMBIE_MAN 2

uint8_t mode = STATE_CLOCK;

#define I2C_COMM 1
#define I2C_LED 2
uint8_t bytes[16];

#define CMD_STATE 0
#define CMD_AMBIE 1

// Variables
uint8_t ambie_color[3];

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
  Wire.begin(I2C_COMM);
  Wire.onReceive(receiveData); 
  
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

  switch(mode) {
    case STATE_CLOCK: 
      printClockLCD();      
      clock_ir();
      break;
    case STATE_AMBIE_RAIN: 
      lcd.setCursor(1,0); 
      lcd.print("Rainbow Mode        ");
      rain_ir();
      break;
    case STATE_AMBIE_MAN: 
      lcd.setCursor(1,0); 
      lcd.print("Ambient Mode        ");
      ambie_ir();
      break;

  }
 }

void clock_ir(){
 if (irrecv.decode(&results)) {
    switch(results.value){
      case 0x77E150DA: 
        // +
        //mode = 1;
        break; 
      case 0x77E160DA: 
        // >>
        //mode = 2;
        break; 
      case 0x77E190DA: 
        // << 
        //mode = 3;
        break; 
      case 0x77E130DA: 
        // -
        //mode = 4;
        break; 
      case 0x77E1A0DA: 
        // >||
        //mode = 5;
        break; 
      case 0x77E1C0DA: 
        // menu
        updateMode();
        break; 
      default: 
        //mode = 8;
        break;
  }
  
  irrecv.resume(); // Receive the next value
  } 
}

void rain_ir(){
 if (irrecv.decode(&results)) {
    switch(results.value){
      case 0x77E150DA: 
        // +
        //mode = 1;
        break; 
      case 0x77E160DA: 
        // >>
        //mode = 2;
        break; 
      case 0x77E190DA: 
        // << 
        //mode = 3;
        break; 
      case 0x77E130DA: 
        // -
        //mode = 4;
        break; 
      case 0x77E1A0DA: 
        // >||
        //mode = 5;
        break; 
      case 0x77E1C0DA: 
        // menu
        updateMode();
        break;  
      default: 
        //mode = 8;
        break;
  }
  
  irrecv.resume(); // Receive the next value
  } 
}

uint8_t last_color = 0;
void ambie_ir(){
  lcd.setCursor(0,1); lcd.print(" Rot  Blau Gruen");
  sprintf(buffer," %03i  %03i  %03i",ambie_color[0],ambie_color[1],ambie_color[2]);
  lcd.setCursor(0,2); lcd.print(buffer);
  lcd.setCursor(5*last_color,2);lcd.print(">");    
  if (irrecv.decode(&results)) {
      switch(results.value){
      case 0x77E150DA: 
        // +
        ambie_color[last_color]++;
        updateColor();
        break; 
      case 0x77E160DA: 
        // >>
        if(last_color != 2 ) last_color++;
        break; 
      case 0x77E190DA: 
        // << 
        if(last_color != 0) last_color--;
        break; 
      case 0x77E130DA: 
        // -
        ambie_color[last_color]--;
        updateColor();
        break; 
      case 0x77E1A0DA: 
        // >||
        //mode = 5;
        break; 
      case 0x77E1C0DA: 
        // menu
        updateMode();
        break; 
      default: 
        //mode = 8;
        break;
  }
  
  irrecv.resume(); // Receive the next value
  } 
}

void updateColor(){
 bytes[0] = CMD_AMBIE & 0xff;
 bytes[1] = ambie_color[0];
 bytes[2] = ambie_color[1];
 bytes[3] = ambie_color[2];
 Wire.beginTransmission(I2C_LED);
 Wire.write(bytes,4);
 Wire.endTransmission();     // stop transmitting

 
}

void updateMode() {
  mode++;
  if(mode==3) mode = 0;
  bytes[0] = CMD_STATE & 0xff;
  bytes[1] = (mode) & 0xff;
  Wire.beginTransmission(I2C_LED);
  Wire.write(bytes,2);
  Wire.endTransmission();     // stop transmitting

}

void printClockLCD(){
  lcd.setCursor(1,0);
  lcd.print("Time: ");if(hour24<10) lcd.print(0); lcd.print(hour24, DEC);
  lcd.print(" : ");   if(curr_minute<10) lcd.print(0); lcd.print(curr_minute, DEC);
  lcd.print(" : ");   if(currentsecond<10) lcd.print(0); lcd.print(currentsecond, DEC);	
}

void setTime(){
   t = now();
   currenttime = millis();
   hour24 = hour(t);
    
   hour12 = (hour24>12) ? hour24-12 : hour24;
   curr_minute =  minute(t);
   currentsecond = second(t);
}






void receiveData(int numBytes)
{
  int command = Wire.read();
  switch (command) {
  //case CMD_CLK: break;
  //case CMD_TIM: break;
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
