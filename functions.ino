// 1) Sets the date and time on the ds1307
// 2) Starts the clock
// 3) Sets hour mode to 24 hour clock
// Assumes you're passing in valid numbers ##################

void setDateDs1307(byte second,        // 0-59
byte minute,        // 0-59
byte hour,          // 1-23
byte dayOfWeek,     // 1-7
byte dayOfMonth,    // 1-28/29/30/31
byte month,         // 1-12
byte year)          // 0-99
{
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.write(decToBcd(second));    // 0 to bit 7 starts the clock
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));      // If you want 12 hour am/pm you need to set
  // bit 6 (also need to change readDateDs1307)
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(dayOfMonth));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.endTransmission();
}

// Gets the date and time from the ds1307 *************************
void getDateDs1307(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *dayOfMonth,
byte *month,
byte *year)
{
  // Reset the register pointer
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write((uint8_t) 0x00); //this replaces Wire.write(0);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_I2C_ADDRESS, 7);

  // A few of these need masks because certain bits are control bits
  *second     = bcdToDec(Wire.read() & 0x7f);
  *minute     = bcdToDec(Wire.read());
  *hour       = bcdToDec(Wire.read() & 0x3f);  // Need to change this if 12 hour am/pm
  *dayOfWeek  = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month      = bcdToDec(Wire.read());
  *year       = bcdToDec(Wire.read());
}


// Get current data from real time clock ##################
int gettime(int activeClock) {

  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  int decsecond = int(second);
  int scnd = decsecond % 10;
  int tenscnd = decsecond / 10;
  int decminute = int(minute);
  int mnt = decminute % 10;
  int tenmnt = decminute / 10;
  int dechour = int(hour);
  int hr = dechour % 10;
  int tenhr = dechour /10;

  // Write hours and minutes to the LED display
  // starting with the rightmost digit
  
  // CLOCK 0 ******************************************************
  if (activeClock == 0){
    lc.setDigit(0,3,tenhr,false);
    lc.setDigit(0,2,hr,true);
    lc.setDigit(0,1,tenmnt,false);
    lc.setDigit(0,0,mnt,false);
  }
  
  // CLOCK 1 ******************************************************
  else if (activeClock == 1){
    lc.setDigit(0,7,tenhr,false);
    lc.setDigit(0,6,hr,true);
    lc.setDigit(0,5,tenmnt,false);
    lc.setDigit(0,4,mnt,false);
  }
  
  // CLOCK 2 ******************************************************
  else if (activeClock == 2){
    lc.setDigit(1,3,tenhr,false);
    lc.setDigit(1,2,hr,true);
    lc.setDigit(1,1,tenmnt,false);
    lc.setDigit(1,0,mnt,false);
  }

  // CLOCK 3 ******************************************************
  else if (activeClock == 3){
    lc.setDigit(1,7,tenhr,false);
    lc.setDigit(1,6,hr,true);
    lc.setDigit(1,5,tenmnt,false);
    lc.setDigit(1,4,mnt,false);
  }
  // CLOCK 4 ******************************************************
    else if (activeClock == 4){
    lc.setDigit(2,3,tenhr,false);
    lc.setDigit(2,2,hr,true);
    lc.setDigit(2,1,tenmnt,false);
    lc.setDigit(2,0,mnt,false);
  }

  // CLOCK 5 ******************************************************  
    else if (activeClock == 5){
    lc.setDigit(2,7,tenhr,false);
    lc.setDigit(2,6,hr,true);
    lc.setDigit(2,5,tenmnt,false);
    lc.setDigit(2,4,mnt,false);
  }

  int bigtime = ((tenhr*10 + hr) * 100) + ((tenmnt*10 + mnt)*100/60);
  return bigtime;
}

// Set all clocks to 00:00  ##################
void resetClocks() {
	byte second = 0;
	byte minute = 0;
	byte hour = 0;
	byte dayOfWeek = 1;
	byte dayOfMonth = 1;
	byte month = 1;
	byte year = 12;
for (byte i=0; i<6; i++){	// set the clock address, from 0 to 5
	if (bitRead(i,0) == 1) { 
		digitalWrite(Apin,1);} 
	else {
		digitalWrite(Apin,0);
	}
	if (bitRead(i,1) == 1) { 
		digitalWrite(Bpin,1);} 
	else {
		digitalWrite(Bpin,0);
	}
	if (bitRead(i,2) == 1) { 
		digitalWrite(Cpin,1);} 
	else {
		digitalWrite(Cpin,0);
	}
	Wire.beginTransmission(DS1307_I2C_ADDRESS);
	Wire.write(0);
	Wire.write(decToBcd(second));    	// 0 to bit 7 starts the clock
	Wire.write(decToBcd(minute));
	Wire.write(decToBcd(hour));      	// If you want 12 hour am/pm you need to set
										// bit 6 (also need to change readDateDs1307)
	Wire.write(decToBcd(dayOfWeek));
	Wire.write(decToBcd(dayOfMonth));
	Wire.write(decToBcd(month));
	Wire.write(decToBcd(year));
	Wire.endTransmission();
  }
}


// Convert normal decimal numbers to binary coded decimal ##################
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}

// Convert binary coded decimal to normal decimal numbers ##################
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}

// Flash the colon character on the main clock once every second ##################
void checkColon() {
    unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis > interval) {
    // save the last time you blinked the LED 
    previousMillis = currentMillis;   

    // if the LED is off turn it on and vice-versa:
    if (colonState == LOW)
      colonState = HIGH;
    else
      colonState = LOW;

    // set the LED with the ledState of the variable:
    digitalWrite(colonPin, colonState);
  }
}


// Set the intensity of all displays according to ambient light level ##################
int setIntensity() {
	int lightlevel = (analogRead(lightLevelPin) / 64);  // scale result to values from 0-15
	return lightlevel;
}

// Time randomizer routine         ##################
void randomizeTime() {		// routine to randomly add or subtract time for each clock
	byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
	
	digitalWrite(Apin,0);
	digitalWrite(Bpin,0);
	digitalWrite(Cpin,0);
	
for (int i=5; i>=0; i--){	// set the clock address, from 0 to 5
	//Serial.print("CLOCK ");
	//Serial.println(i);
	if (bitRead(i,0) == 1) { 
		digitalWrite(Apin,1);} 
	else { digitalWrite(Apin,0); }
	if (bitRead(i,1) == 1) { digitalWrite(Bpin,1);} 
	else { digitalWrite(Bpin,0); }
	if (bitRead(i,2) == 1) { 
		digitalWrite(Cpin,1);} 
	else { digitalWrite(Cpin,0); }
	
	getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
	
	int decminute = int(minute);
	int dechour = int(hour);
	
	int randomMinutes = random(5);		// up to 5 minutes,
	int polarity = random(2);			// give or take
	//Serial.print("This clock changes by ");
	if (polarity == 1) {
		//Serial.print("+"); 
		}
	else {
		//Serial.print("-");
	}
	//Serial.print(randomMinutes);
	//Serial.println(" minutes");
	//Serial.println(" ");
	/* Serial.print("for Clock ");
	Serial.print(i);
	Serial.print("   randomMinutes: ");
	Serial.print(randomMinutes);
	Serial.print("   polarity: ");
	Serial.print(polarity); 
	Serial.print("    HOUR: ");
	Serial.print(hour);
	Serial.print("    MINUTE: ");
	Serial.println(minute); */
	
	if (randomMinutes > 0) {
		if (polarity == 0) { 				// subtract time
			 for (int m = randomMinutes; m >= 0; m--){
				if (decminute > 0) {
					decminute--; 
				} else { decminute = 59;
					if (dechour > 0) {
						dechour--; }
					else { dechour = 23; }
				}
			} 
		}  else {							// add time
			 for (int m = 0; m <= randomMinutes; m++){
				if (decminute < 59) {
					decminute++; } 
				else { decminute = 0;
					if (dechour < 24) {
						dechour++; }
					else { dechour = 0; }
				}
			} 
		} 
		setDateDs1307(second, decminute, dechour, dayOfWeek, dayOfMonth, month, year );	
		//Serial.print("time set for clock ");
		//Serial.println(i);
	}
  }
}


// routine to add minutes and hours to all 6 clocks ##################
void setClocks() {	

 	byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
	
	getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
	
	digitalWrite(Apin,0);
	digitalWrite(Bpin,0);
	digitalWrite(Cpin,0);
	
	int decminute = int(minute);
	int dechour = int(hour);
	
	// Routine to adjust time
    boolean hourpressed = digitalRead(hourPin);
    boolean minutepressed = digitalRead(minutePin);
	
	if (hourpressed == LOW)         // add one hour
    { 
	//Serial.print("setClocks: hourButton!");
		if (dechour < 23){
			dechour++;}
		else {
			dechour = 0;}
		//byte newhour = byte(dechour);
		//setDateDs1307(second, minute, dechour, dayOfWeek, dayOfMonth, month, year );
		 
		for (byte i=0; i<6; i++)	// set each clock
		{
			if (bitRead(i,0) == 1) { 
				digitalWrite(Apin,1); } 
			else { digitalWrite(Apin,0); }
			if (bitRead(i,1) == 1) { 
				digitalWrite(Bpin,1); } 
			else { digitalWrite(Bpin,0); }
			if (bitRead(i,2) == 1) { 
				digitalWrite(Cpin,1); } 
			else { digitalWrite(Cpin,0); }	
		setDateDs1307(second, minute, dechour, dayOfWeek, dayOfMonth, month, year );}
		
	}
    else if (minutepressed == LOW)         // add one minute
	{ 
		//Serial.print("setClocks: minuteButton!");
		if (decminute < 59) {
			decminute++; }
	else {
			decminute = 0; }
		//byte newminute = byte(decminute);
		//setDateDs1307(second, decminute, hour, dayOfWeek, dayOfMonth, month, year );
		}	
	
	for (byte i=0; i<6; i++){	// set each clock
		if (bitRead(i,0) == 1) { 
			digitalWrite(Apin,1);} 
		else { digitalWrite(Apin,0); }
		if (bitRead(i,1) == 1) { 
			digitalWrite(Bpin,1);} 
		else { digitalWrite(Bpin,0); }
		if (bitRead(i,2) == 1) { 
			digitalWrite(Cpin,1);} 
		else { digitalWrite(Cpin,0); }
		setDateDs1307(second, decminute, dechour, dayOfWeek, dayOfMonth, month, year );
	}	
	delay(delayTime); 
}
