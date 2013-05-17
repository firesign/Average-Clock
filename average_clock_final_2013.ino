/* 
 Average Clock Mod 1
 Six separate DS1307s using three MAX7219's shown on red displays. 
 Each clock is individually adjustable by Minutes and Hours. 
 A fourth MAX7219 drives a seventh, larger green display that shows the 
 average time of all six clocks.
 
 Average time can be hidden; it implements a button 
 which, when pressed, displays this value. User configuration for this
 function is via a configuration switch. When the switch is LOW, average time displays
 normally. When HIGH, average time is displayed only when the button attached
 to digital pin 4 is depressed. An automatic adjustment for LED intensity 
 based on ambient lighting is included.
 
 This version eliminates all the Hour and Minute adjustment buttons except
 for two, which control all of the clocks. Setup is performed in three steps:
 
 1. The Master Reset button is pressed, zeroing all six clocks.
 2. The Hour and Minute adjust buttons add hours and minutes to all clocks
	simultaneously.
 3. The Randomize button will add or remove up to 5 minutes randomly from
	each clock.
 
 
 by Michael B. LeBlanc
 NSCAD University
 http://generaleccentric.net
 */


//We always have to include the library
#include "LedControl.h"
#include "Wire.h"
#define DS1307_I2C_ADDRESS 0x68
#define hourPin 9         		// Pin 9 is the pin that adds an HOUR    
#define minutePin 8      		// Pin 8 is the pin that adds a MINUTE
#define Apin 5      			// Pin 5 goes to the A pin on 4051
#define Bpin 6      			// Pin 6 goes to the B pin on 4051
#define Cpin 7      			// Pin 7 goes to the C pin on 4051
#define masterResetButton A1 	        // Pin Analog 1 goes to the MASTER RESET
#define randomizeButton A2 		// Pin Analog 2 goes to the RANDOM
int activeClock = 0; 			// which clock is active
int rdm;
unsigned long delayStart, tgt;

int avgHundredhr, avgTenhr, avgHr, avgTens, avgTenmin, avgMin;

// The colon needs to driven by a separate pin ***************
int colonPin = 4;
// Colon timing: changes state every second
boolean colonState = LOW;
long previousMillis = 0; 	// Initialize colon timer
long interval = 1000; 		// interval for state changes

// Average Display Control ***********************************
boolean averageButtonEnabled;	// 
int averageConfigPin = 2;	// VIEW ON/OFF is connected to pin 2

boolean averageState;		// VIEW the average value (if VIEW ON)
int averagePin = 3;

int lightLevelPin = A0;		// a light cell for adjusting LED brilliance
int intensity = 0;		// the intensity of the LEDs

boolean randomizer;
boolean masterReset;		

/*
 Now we need a LedControl to work with.
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have four MAX72XXs.
 */
LedControl lc=LedControl(12,11,10,4);  // last digit is the number of MAX7219 chips in use

/* we always wait a bit between updates of the display */
unsigned long delayTime = 200;

void setup() {

	randomSeed(analogRead(3));

	// set up the average display
	pinMode(averagePin, INPUT);
	digitalWrite(averagePin, HIGH);
	
	// set up config switch
	pinMode(averageConfigPin, INPUT);
	digitalWrite(averageConfigPin, HIGH);
 
	// set the colon pin as output:
	pinMode(colonPin, OUTPUT);
	
	// set up the Master Reset button
	pinMode(masterResetButton, INPUT);
	digitalWrite(masterResetButton, HIGH);
	
	// set up the Randomize button
	pinMode(randomizeButton, INPUT);
	digitalWrite(randomizeButton, HIGH);
	
	// set up the "User: Please Wait" button
	// pinMode(waitLED, OUTPUT);


	//Serial.begin(9600);
	pinMode(Apin, OUTPUT);
	pinMode(Bpin, OUTPUT);
	pinMode(Cpin, OUTPUT);

  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */

  lc.shutdown(0,false);
  lc.shutdown(1,false);
  lc.shutdown(2,false);
  lc.shutdown(3,false);
  /* Set the initial brightness */
  lc.setIntensity(0,2);
  lc.setIntensity(1,2);
  lc.setIntensity(2,2);
  lc.setIntensity(3,2);
  lc.setScanLimit(3,3);
  /* and clear the display */
  lc.clearDisplay(0);
  lc.clearDisplay(1);
  lc.clearDisplay(2);
  lc.clearDisplay(3);
  

  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  pinMode(hourPin,INPUT);        // The add hour pin
  digitalWrite(hourPin, HIGH);   // turn on pullup resistor
  pinMode(minutePin,INPUT);      // The add minutes pin
  digitalWrite(minutePin, HIGH); // turn on pullup resistor
  Wire.begin();
}


void loop() { 
  
  // set the correct intensity for LEDs according to light conditions
  intensity = setIntensity();
	for (int i=0; i<3; i++){
		lc.setIntensity(i,(intensity / 2));
		}
	lc.setIntensity(3,intensity);		// the main display needs more juice

  digitalWrite(Apin, LOW);
  digitalWrite(Bpin, LOW);  			// connect to clock 0 
  digitalWrite(Cpin, LOW);
  activeClock = 0;  
  int aTime = gettime(activeClock);  	// main routine to display the time

  digitalWrite(Apin, HIGH);
  digitalWrite(Bpin, LOW);  			// connect to clock 1 
  digitalWrite(Cpin, LOW);
  activeClock = 1;
  int bTime = gettime(activeClock);  	// main routine to display the time

  digitalWrite(Apin, LOW);
  digitalWrite(Bpin, HIGH);  			// connect to clock 2   
  digitalWrite(Cpin, LOW); 
  activeClock = 2;
  int cTime = gettime(activeClock);  	// main routine to display the time

  digitalWrite(Apin, HIGH);
  digitalWrite(Bpin, HIGH);  			// connect to clock 3   
  digitalWrite(Cpin, LOW); 
  activeClock = 3;
  int dTime = gettime(activeClock);  	// main routine to display the time
  
  digitalWrite(Apin, LOW);
  digitalWrite(Bpin, LOW);  			// connect to clock 4   
  digitalWrite(Cpin, HIGH); 
  activeClock = 4;
  int eTime = gettime(activeClock);  	// main routine to display the time
  
  digitalWrite(Apin, HIGH);
  digitalWrite(Bpin, LOW);  			// connect to clock 5   
  digitalWrite(Cpin, HIGH); 
  activeClock = 5;
  int fTime = gettime(activeClock);  	// main routine to display the time


  // Calculate the average and then display it as a seventh clock
  float averageTime = ((aTime + bTime + cTime + dTime + eTime + fTime) / 6) + 1.85;
  //Serial.print("Average Time: ");
  //Serial.println(averageTime);
  avgHundredhr = averageTime / 100;
  avgTenhr = avgHundredhr / 10;
  avgHr = avgHundredhr % 10;

  avgTens = averageTime - (avgHundredhr * 100);
  avgTens = (avgTens * 6)/ 10; 					// convert from decimal to minutes

  avgTenmin = avgTens / 10;
  avgMin = (avgTens % 10); 
  
  averageButtonEnabled = digitalRead(averageConfigPin);
  averageState = digitalRead(averagePin);		// shall we display the average time?

    //if ((averageButtonEnabled == LOW) || ((averageButtonEnabled == HIGH) && (averageState == HIGH))){
   //if ((averageButtonEnabled == HIGH) || ((averageButtonEnabled == LOW) && (averageState == LOW))){
     if ((averageButtonEnabled == LOW) || ((averageButtonEnabled == HIGH) && (averageState == LOW))){
       //if (averageButtonEnabled == LOW){
	// The button is pressed or config switch is on
	// Display the average time to the main display
	lc.setDigit(3,3,avgTenhr,false);
        lc.setDigit(3,2,avgHr,true);
        lc.setDigit(3,1,avgTenmin,false);
        lc.setDigit(3,0,avgMin,false); 
	
	checkColon();

  } else {
	// The button is not pressed.
	lc.clearDisplay(3);
	digitalWrite(colonPin, LOW);
	}
	
	masterReset = digitalRead(masterResetButton); 	// Reset clocks to 00:00
	if (masterReset == LOW) {
		resetClocks();
	}
	
	randomizer = digitalRead(randomizeButton);		// Randomly add or subtract 5 minutes
	if (randomizer == LOW) {
		randomizeTime();
	}
	
	// Check to see if the minutes or hours adjust buttons are pressed
	boolean minuteSense = digitalRead(minutePin);
	boolean hourSense = digitalRead(hourPin);
	
	if (minuteSense == LOW || hourSense == LOW) {
		//Serial.println("setClocks!");
		setClocks();
	}  
}
