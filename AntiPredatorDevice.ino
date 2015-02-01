// APD version 1.2.0
// 2/1/15
// Brian Tice
//


#include <Arduino.h>
#include <Wire.h>            // Need this for I2C support
#include <SPI.h>             // Need this for SPI communication Support
#include <SD.h>              // Need this for microSD card read and write
// Note: SDFAT.h is supposedly faster and supports two  SD cards
//       Sticking with SD.h for now mostly because the Adafruit
//       Library for VS1053 mp3 player is written using SD.h.

#include "BlinkM_funcs.h"    // Need this for BlinkM routines for RGB led clusters.
// This 'library' is a little different that the others in that its
// functions are housed in a file associated with main sketch called BlinkM_funcs.h

#include <avr/pgmspace.h>    // for progmem stuff
#include <RTClib.h>          // Need this for ChronoDot real time clock library functions
#include <RTC_DS3231.h>      // Need this for the particular Chip used on the ChronoDot 2.0
#include <SFE_TSL2561.h>     // Need this for Luminosity sensor support. changed for more extensive library
// from Mike Grusin. The Adafruit library didn't have functions
// to set Interrupts and HI/LOW light thresholds for Interrupt. This is better.
#include <Adafruit_VS1053.h> // Need this for music player library functions
#include "LiquidCrystal.h"   // Need this for I2C backback and LCD display
// Note this is latest version of this library from
// adafruit that includes I2C support

#include <MenuBackend.h>     //MenuBackend library - need this to run LCD menu routine
// Compliments of Alexander-m  Brevig

// IMPORTANT: to use the menubackend library by Alexander Brevig download it at
// http://www.arduino.cc/playground/uploads/Profiles/MenuBackend_1-4.zip
// and add the next code at line 195
//  void toRoot() {
//       setCurrent( &getRoot() );
//  }


// These are the pins used for the breakout example
#define BREAKOUT_RESET       9     // VS1053 reset pin (output)
#define BREAKOUT_CS         10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS         8     // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_CS            7     // VS1053 chip select pin (output)
#define SHIELD_DCS           6     // VS1053 Data/command select pin (output)


#define PIEZO_SOUNDER_PIN    22    // Create constant for Piezo Sounder HiLo alarm
#define MICRO_SD_CHIP_SELECT 53    // Chip select constant for MicroSD datalogger


// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

// Create Constants for Keypad items
#define BUTTON_PIN_1         30
#define BUTTON_PIN_2         31
#define BUTTON_PIN_3         32
#define BUTTON_PIN_4         33
#define BUTTON_PIN_5         34



#define PIR_A_LED_PIN         40
#define PIR_B_LED_PIN         41
#define DAY_NIGHT_ISR_PIN      2
#define PIR_B_SIGNAL_PIN      18
#define SOUND_SWITCH_PIN      12
#define PIR_CALIBRATION_TIME   5

#define MUTE_AUDIO_PIN         45

#define STATE_DAYTIME_IDLE              2
#define STATE_MENU_ISR                  3
#define STATE_MENU                      4
#define STATE_PREPARE_FOR_DAYTIME_IDLE  5
#define STATE_DAY_NIGHT_ISR             6
#define STATE_ARMED_STATE               7


// Create constants for LED pattern types:
#define RED_LED_PATTERN_TYPE       1
#define GREEN_LED_PATTERN_TYPE     2
#define BLUE_LED_PATTERN_TYPE      3
#define RANDOM_LED_PATTERN_TYPE    4

#define MAX9744_I2CADDR 0x4B // 0x4B is the default i2c address for MAX 9744 Class D Amp
#define STARTUP_VOLUME 53

//#define VS1053_FILEPLAYER_PIN_INT 3

#define LOW_LIGHT_THRESHOLD 300 // This is the threshold that determines the transition from daytime idle to armed state.
// Once in armed state,


int8_t thevol = 53;          // We'll track the volume level in this variable.
// Range: 0 - 63. 0 Low, 63 Loudest

// Class declarations for system

SFE_TSL2561 light;                  // Need this for Luminosity sensor. Default I2C address is: 0x09 = TSL2561_ADDR_FLOAT
// This can be changed by soldering jumpers on the sensor
// The address will be different depending on whether you let
// the ADDR pin float (addr 0x39), or tie it to ground or vcc. In those cases
// use TSL2561_ADDR_LOW (0x29) or TSL2561_ADDR_HIGH (0x49) respectively

LiquidCrystal lcd(0);               // Need this to initialize the I2C backpack / LCD combo
// Connect via i2c, default address #0 (A0-A2 not jumpered)
// I2C address for LCD is 0x00 without soldering jumper headers
// I did not jump A0-A2 which is why lcd is initialized as lcd(0)

// This is a special version of LiquidCrystal.h with overloaded
// constructor, if only one argument is given, the I2C backpack kicks in

RTC_DS3231 RTC;                     // Need this to create an instance of the RTC_DS3231 class for real time clock
// located in RTC_DS.3231.h


Adafruit_VS1053_FilePlayer musicPlayer =     // Need this to create instance of music player class and File Player

// create breakout-example object!

Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
// create shield-example object!
//Adafruit_VS1053_FilePlayer(SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

File dataFile;                      // Need this to declare a File instance for use in datalogging.



MenuBackend menu = MenuBackend(menuUsed,menuChanged);  //Menu variables


MenuItem menu1Item1 =         MenuItem("Select Alarm Modes");           //initialize menuitems
MenuItem menuItem1SubItem1 =  MenuItem("Audio Enable");
MenuItem menuItem1SubItem2 =  MenuItem("Audio Disable");
MenuItem menuItem1SubItem3 =  MenuItem("Piezo Enable");
MenuItem menuItem1SubItem4 =  MenuItem("Piezo Disable");
MenuItem menuItem1SubItem5 =  MenuItem("Small LED Enable");
MenuItem menuItem1SubItem6 =  MenuItem("Small LED Disable");
MenuItem menuItem1SubItem7 =  MenuItem("Big LED Enable");
MenuItem menuItem1SubItem8 =  MenuItem("Big LED Disable");
MenuItem menu1Item2 =         MenuItem("Select Alarm Pattern");
MenuItem menuItem2SubItem1 =  MenuItem("LEDs Red");
MenuItem menuItem2SubItem2 =  MenuItem("LEDs Green");
MenuItem menuItem2SubItem3 =  MenuItem("LEDs Blue");
MenuItem menuItem2SubItem4 =  MenuItem("LEDs Random Color");
MenuItem menuItem2SubItem5 =  MenuItem("Piezo Short");
MenuItem menuItem2SubItem6 =  MenuItem("Piezo Long");
MenuItem menuItem2SubItem7 =  MenuItem("Piezo Random");
MenuItem menuItem2SubItem8 =  MenuItem("Audio Track 1");
MenuItem menuItem2SubItem9 =  MenuItem("Audio Track 2");
MenuItem menuItem2SubItem10 = MenuItem("Audio Track Random");
MenuItem menu1Item3 =         MenuItem("System Changes");
MenuItem menuItem3SubItem1 =  MenuItem("Inc Volume (+)");
MenuItem menuItem3SubItem2 =  MenuItem("Dec Volume (-)");
MenuItem menu1Item4 =         MenuItem("Done");
MenuItem menu1Item4SubItem1 = MenuItem("Finished with Menu");

const boolean BLINKM_ARDUINO_POWERED = true;  // For now this is true. This will change when moving for bench
// testing to field testing

byte blinkm_addr_a = 0x09;          // I2C Address of one of the LED's. LED A
byte blinkm_addr_b = 0x0C;          // I2C Address of one of the LED's. LED B
byte blinkm_addr_c = 0x0D;          // I2C Address of one of the LED's. LED C

byte LedArrayAddress[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07};    // Global array that houses LED addresses


// Global Variables for Menu selection items
// these are flags and booleans that determine armed state functionality

boolean audio_enabled;
boolean piezo_enabled;
boolean big_LED_enabled;
boolean small_LED_enabled;

int pattern_type;         // RED =    1
// Green =  2
// Blue =   3
// Random = 4




int piezo_time_length;    // Short = 2seconds
// Long = 4 seconds
// Random(2,6)

int play_track;           // 1,2 or 3 for random

int volume;               // 0-63. (+) means up 3. (-) means down 3. plus check boundary conditions.



volatile int state = 0;
boolean gain;     // Gain setting, 0 = X1, 1 = X16;
unsigned int ms;  // Integration ("shutter") time in milliseconds

//Redundant for LCD menu code, fix later, shouldn't be globals..
// ++
const int buttonPinLeft =   30;      // pin for the Up button
const int buttonPinRight =  31;     // pin for the Down button
const int buttonPinEsc =    32;       // pin for the Esc button
const int buttonPinEnter =  33;     // pin for the Enter button


int lastButtonPushed = 0;

int lastButtonEnterState = LOW;   // the previous reading from the Enter input pin
int lastButtonEscState = LOW;   // the previous reading from the Esc input pin
int lastButtonLeftState = LOW;   // the previous reading from the Left input pin
int lastButtonRightState = LOW;   // the previous reading from the Right input pin


long lastEnterDebounceTime = 0;  // the last time the output pin was toggled
long lastEscDebounceTime = 0;  // the last time the output pin was toggled
long lastLeftDebounceTime = 0;  // the last time the output pin was toggled
long lastRightDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 200;    // the debounce time

// ++

void setup() {
	
	
	Serial.begin(19200);
	initialize_lcd_backpack_and_screen();
	initialize_real_time_clock();
	initialize_and_test_leds();
	initialize_pin_modes();
	//initialize_and_calibrate_PIR_sensor_array();
	initialize_lux_sensor();
	initialize_datalogging_sd_card();
	initialize_vs1053_music_player();
	initialize_LCD_menu_system();
	initialize_stereo();
	
	attachInterrupt(4,pin_19_ISR,CHANGE);
	//attachInterrupt(0,DayNightISR,FALLING);
	state = STATE_DAYTIME_IDLE;
	
	
}

void loop() {
	
	
	switch(state) {
		
		case STATE_DAYTIME_IDLE:
		{
			
			
			DateTime now = RTC.now();
			lcd.setBacklight(LOW);
			Serial.print("Just chilling out at: ");
			Serial.print(now.year(), DEC);
			Serial.print('/');
			Serial.print(now.month(), DEC);
			Serial.print('/');
			Serial.print(now.day(), DEC);
			Serial.print(' ');
			Serial.print(now.hour(), DEC);
			Serial.print(':');
			Serial.print(now.minute(), DEC);
			Serial.print(':');
			Serial.print(now.second(), DEC);
			
			lcd.setCursor(0, 1);
			lcd.print(now.year(), DEC);
			lcd.print('/');
			lcd.print(now.month(), DEC);
			lcd.print('/');
			lcd.print(now.day(), DEC);
			lcd.print(' ');
			lcd.print(now.hour(), DEC);
			lcd.print(':');
			lcd.print(now.minute(), DEC);
			lcd.print(':');
			lcd.print(now.second(), DEC);
			
			// Retrieve the data from the device:

			unsigned int data0, data1;
			
			if (light.getData(data0,data1))
			{
				// getData() returned true, communication was successful
				
				Serial.print(" data0: ");
				Serial.println(data0);
				//Serial.print(" data1: ");
				// Serial.print(data1);
				
				// To calculate lux, pass all your settings and readings
				// to the getLux() function.
				
				// The getLux() function will return 1 if the calculation
				// was successful, or 0 if one or both of the sensors was
				// saturated (too much light). If this happens, you can
				// reduce the integration time and/or gain.
				// For more information see the hookup guide at: https://learn.sparkfun.com/tutorials/getting-started-with-the-tsl2561-luminosity-sensor
				
				double lux;    // Resulting lux value
				boolean good;  // True if neither sensor is saturated
				
				// Perform lux calculation:
				
				good = light.getLux(gain,ms,data0,data1,lux);
				
				
				
				
				
			}
			
			// Temporary WORKAROUND for non-triggering ISR INTERRUPT pin on TSL2561
			//
			if(data0 < LOW_LIGHT_THRESHOLD) {
				// bail out, make a mock ISR()
				state = STATE_DAY_NIGHT_ISR;
				break;
			}
			//
			
			//state = STATE_DAYTIME_IDLE;
			break;
		}
		
		case STATE_MENU_ISR:
		{
			
			
			
			Serial.println("Detaching Interrupts, then sending to menu state");
			light.clearInterrupt();
			//detachInterrupt(0);
			detachInterrupt(4);
			wipe_LCD_screen();
			lcd.setBacklight(HIGH);
			lcd.setCursor(0,0);
			lcd.print("   .:Menu System:.  ");
			state = STATE_MENU;
			
			break;
		}
		
		
		case STATE_MENU:
		{
			
			
			// Decode buttons
			
			readButtons();  //I split button reading and navigation in two procedures because
			navigateMenus();  //in some situations I want to use the button for other purpose (eg. to change some settings)
			
			break;
		}
		
		case STATE_PREPARE_FOR_DAYTIME_IDLE:
		{
			
			Serial.println("Preparing for Daytime Idle mode...");
			
			
			// Access the SD card, log current setting and timestamp to the SD card, close the SD Card
			DateTime now = RTC.now();
			SD.end();
			SD.begin(MICRO_SD_CHIP_SELECT);
			
			dataFile = SD.open("datalog.txt", FILE_WRITE);
			dataFile.print(now.year(), DEC);
			dataFile.print('/');
			dataFile.print(now.month(), DEC);
			dataFile.print('/');
			dataFile.print(now.day(), DEC);
			dataFile.print(' ');
			dataFile.print(now.hour(), DEC);
			dataFile.print(':');
			dataFile.print(now.minute(), DEC);
			dataFile.print(':');
			dataFile.print(now.second(), DEC);
			dataFile.print(" Current system settings: ");
			dataFile.print(" ");
			dataFile.print(pattern_type, DEC);
			dataFile.print(" ");
			dataFile.println(piezo_time_length, DEC);
			
			dataFile.flush();
			dataFile.close();
			SD.end();
			
			// attachInterrupt(0,DayNightISR,FALLING);  // Attach the interrupt to pin 2.
			attachInterrupt(4,pin_19_ISR,CHANGE);
			state = STATE_DAYTIME_IDLE;
			break;
			
			
			
		}
		
		case STATE_DAY_NIGHT_ISR:
		{
			Serial.println("State Day/Night ISR, transitioning to armed state. clearning and turing off interrupt 0");
			
			//light.clearInterrupt();
			//light.setInterruptControl(0, 0);
			//detachInterrupt(0);                // Turn off interrupt for now, it's night time.
			// Turn back on when day is upon us in the African Wild.
			// Tibetan Plains
			
			// Access the SD card, log current setting and timestamp to the SD card, close the SD Card
			DateTime now = RTC.now();
			SD.end();
			SD.begin(MICRO_SD_CHIP_SELECT);
			
			dataFile = SD.open("datalog.txt", FILE_WRITE);
			dataFile.print(now.year(), DEC);
			dataFile.print('/');
			dataFile.print(now.month(), DEC);
			dataFile.print('/');
			dataFile.print(now.day(), DEC);
			dataFile.print(' ');
			dataFile.print(now.hour(), DEC);
			dataFile.print(':');
			dataFile.print(now.minute(), DEC);
			dataFile.print(':');
			dataFile.print(now.second(), DEC);
			dataFile.print(" Going into Night Mode (Armed State.");
			dataFile.print(" Current system settings: ");
			dataFile.print(" ");
			dataFile.print(pattern_type, DEC);
			dataFile.print(" ");
			dataFile.println(piezo_time_length, DEC);
			
			dataFile.flush();
			dataFile.close();
			SD.end();
			
			state = STATE_ARMED_STATE;
			break;
		}
		
		case STATE_ARMED_STATE:
		{
			
			// Use timer so that the button ISR()'s come through the pipe
			// need to figure this one out. or why are interrupts working??
			attachInterrupt(4,pin_19_ISR,CHANGE);
			
			Serial.println("Armed, can accept button ISR's");
			
			unsigned int data0, data1;
			
			long randomLEDProgram = random(0,19);
			
			if(pattern_type == BLUE_LED_PATTERN_TYPE)
			{
				randomLEDProgram = 5;  // 5 and 13 are Blue only scripts
			}
			if(pattern_type == RED_LED_PATTERN_TYPE)
			{
				randomLEDProgram = 13;  // 5 and 13 are Blue only scripts
			}
			if(pattern_type == GREEN_LED_PATTERN_TYPE)
			{
				randomLEDProgram = 5;  // 5 and 13 are Blue only scripts
			}
			if(pattern_type == RANDOM_LED_PATTERN_TYPE)
			{
				randomLEDProgram = random(0,19);  // 5 and 13 are Blue only scripts
			}
			int ledSelect = random(0,11);  // Scale this when we add more LED's
			
			unsigned long delayBlinkTime = random(500,2000);
			
			int SoundSwitchState = digitalRead(SOUND_SWITCH_PIN);
			if(SoundSwitchState == HIGH)
			{
				
				int probabilityofSound = random(0,50);
				if(probabilityofSound < 10) {
					
					int randomPiezotime = random(1,1500);   // Piezo element goes High for 1ms to 1500ms
					digitalWrite(PIEZO_SOUNDER_PIN,HIGH);
					delay(randomPiezotime);
					digitalWrite(PIEZO_SOUNDER_PIN,LOW);
				}
			}
			
			
			
			BlinkM_playScript( LedArrayAddress[ledSelect], randomLEDProgram, 0x00,0x00);
			delay(delayBlinkTime);
			BlinkM_stopScript(LedArrayAddress[ledSelect]);
			BlinkM_fadeToRGB(LedArrayAddress[ledSelect], 0,0,0);
			
			
			
			if (light.getData(data0,data1))
			{
				if(data0 > 300) {
					
					state = STATE_PREPARE_FOR_DAYTIME_IDLE;
					break;
				}
			}
			
			//state = STATE_ARMED_STATE;
			break;
		}
	}
	
}

void lookForBlinkM() {
	Serial.print("Looking for a BlinkM: ");
	int a = BlinkM_findFirstI2CDevice();
	if( a == -1 ) {
		Serial.println("No I2C devices found");
		} else {
		Serial.print("Device found at addr ");
		Serial.println( a, DEC);
		blinkm_addr_a = a;
	}
}

void initialize_and_test_leds() {
	
	Serial.println("Initializing and Testing LED Array...");
	wipe_LCD_screen();
	lcd.setCursor(0,1);
	lcd.print("Init LED ARRAY..");
	
	if( BLINKM_ARDUINO_POWERED )
	BlinkM_beginWithPower();
	else
	BlinkM_begin();
	
	
	// Iterate through each LED cluster to test connections
	
	for(int index = 0; index < sizeof(LedArrayAddress);index++) {
		
		Serial.print("LED ");
		Serial.println(index+1);
		lcd.setCursor(0,2);
		lcd.print("LED ");
		lcd.print(index+1);
		BlinkM_playScript( LedArrayAddress[index], 18, 0x00,0x00);
		delay(2000);
		BlinkM_stopScript(LedArrayAddress[index]);
		//delay(100);
		BlinkM_fadeToRGB(LedArrayAddress[index], 0,0,0);
		
	}
	delay(200);
	lcd.print(" Success!");
	Serial.println("LED Testing Complete!");
	
	
}

void initialize_real_time_clock() {
	
	Serial.print("Initializing Real Time Clock...");
	wipe_LCD_screen();
	lcd.setCursor(0,1);
	lcd.print("Init Real Time Clock");
	


	RTC.begin();
	DateTime now = RTC.now();
	DateTime compiled = DateTime(__DATE__, __TIME__);
	if (now.unixtime() < compiled.unixtime()) {
		//Serial.println("RTC is older than compile time!  Updating");
		RTC.adjust(DateTime(__DATE__, __TIME__));
	}
	delay(800);
	Serial.println("Success!");
	lcd.setCursor(0,3);
	lcd.print("Success!");

	
	

}

void initialize_pin_modes() {
	
	Serial.print("Setting Pin Modes...");
	wipe_LCD_screen();
	lcd.setCursor(0,1);
	lcd.print("Setting Pin Modes");
	pinMode(MICRO_SD_CHIP_SELECT, OUTPUT);
	pinMode(PIEZO_SOUNDER_PIN,OUTPUT);
	pinMode(BUTTON_PIN_1, INPUT);
	pinMode(BUTTON_PIN_2, INPUT);
	pinMode(BUTTON_PIN_3, INPUT);
	pinMode(BUTTON_PIN_4, INPUT);
	pinMode(BUTTON_PIN_5, INPUT);
	pinMode(DAY_NIGHT_ISR_PIN, INPUT);
	pinMode(PIR_B_SIGNAL_PIN, INPUT);
	pinMode(PIR_A_LED_PIN, OUTPUT);
	pinMode(PIR_B_LED_PIN, OUTPUT);
	pinMode(SOUND_SWITCH_PIN,INPUT);
	pinMode(MUTE_AUDIO_PIN,OUTPUT);
	pinMode(SS, OUTPUT);
	delay(800);
	Serial.println("Success!");
	lcd.setCursor(0,3);
	lcd.print("Success!");
}

void initialize_and_calibrate_PIR_sensor_array() {
	
	//give the sensor some time to calibrate
	Serial.print("calibrating sensor ");
	for(int i = 0; i < PIR_CALIBRATION_TIME; i++){
		Serial.print(".");
		delay(1000);
	}
	Serial.println(" done");
	Serial.println("SENSOR ACTIVE");
	delay(50);
}

void initialize_lux_sensor() {
	
	// Initialize lux sensor
	// You can pass nothing to light.begin() for the default I2C address (0x39),
	// or use one of the following presets if you have changed
	// the ADDR jumper on the board:
	
	// TSL2561_ADDR_0 address with '0' shorted on board (0x29)
	// TSL2561_ADDR   default address (0x39)
	// TSL2561_ADDR_1 address with '1' shorted on board (0x49)

	// For more information see the hookup guide at: https://learn.sparkfun.com/tutorials/getting-started-with-the-tsl2561-luminosity-sensor

	Serial.print("Initializing Light Sensor...");
	wipe_LCD_screen();
	lcd.setCursor(0,1);
	lcd.print("Init Light Sensor");
	
	light.begin();

	// Get factory ID from sensor:
	// (Just for fun, you don't need to do this to operate the sensor)

	unsigned char ID;
	
	if (light.getID(ID))
	{
		Serial.print("Got factory ID: 0X");
		Serial.print(ID,HEX);
		Serial.println(", should be 0X5X");
	}
	// Most library commands will return true if communications was successful,
	// and false if there was a problem. You can ignore this returned value,
	// or check whether a command worked correctly and retrieve an error code:
	else
	{
		
	}

	// The light sensor has a default integration time of 402ms,
	// and a default gain of low (1X).
	
	// If you would like to change either of these, you can
	// do so using the setTiming() command.
	
	// If gain = false (0), device is set to low gain (1X)
	// If gain = high (1), device is set to high gain (16X)

	gain = 0;

	// If time = 0, integration will be 13.7ms
	// If time = 1, integration will be 101ms
	// If time = 2, integration will be 402ms
	// If time = 3, use manual start / stop to perform your own integration

	unsigned char time = 2;

	// setTiming() will set the third parameter (ms) to the
	// requested integration time in ms (this will be useful later):
	
	Serial.println("Set timing...");
	light.setTiming(gain,time,ms);

	// To start taking measurements, power up the sensor:
	
	Serial.println("Powerup...");
	light.setPowerUp();
	delay(100);
	// The sensor will now gather light during the integration time.
	// After the specified time, you can retrieve the result from the sensor.
	// Once a measurement occurs, another integration period will start.
	
	if(light.setInterruptControl(1, 5))
	{
		
		Serial.println("Successfully Set Light Dark Interrupt");
	}
	// Enable Interrupt Pin Output
	// Sets up interrupt operations
	// If control = 0, interrupt output disabled
	// If control = 1, use level interrupt, see setInterruptThreshold()
	// If persist = 0, every integration cycle generates an interrupt
	// If persist = 1, any value outside of threshold generates an interrupt
	// If persist = 2 to 15, value must be outside of threshold for 2 to 15 integration cycles
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)

	if(light.setInterruptThreshold(300, 2000))
	{
		Serial.println("Successfully reset Dawn Dusk Threshold value");
	}

	// set LOW and HIGH channel 0 threshholds for Interrupt trigger
	// Set interrupt thresholds (channel 0 only)
	// low, high: 16-bit threshold values
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)

	//attachInterrupt(0,DayNightISR,CHANGE);  // Attach the interrupt to pin 2.
	
	delay(800);
	Serial.println("Success!");
	lcd.setCursor(0,3);
	lcd.print("Success!");
}

void initialize_lcd_backpack_and_screen() {
	
	Serial.print("Initializing LCD Screen...");
	// set up the LCD's number of rows and columns:
	lcd.begin(20, 4);
	
	wipe_LCD_screen(); // function to clear the screen of contents
	// Print a message to the LCD.
	lcd.print("Initializing...");
	Serial.println("Success!");
	
}

void wipe_LCD_screen() {
	
	lcd.setCursor(0,0);
	lcd.print("                    ");
	lcd.print("                    ");
	lcd.print("                    ");
	lcd.print("                    ");
	lcd.setCursor(0,0);
}

void initialize_datalogging_sd_card() {

	wipe_LCD_screen();
	lcd.setCursor(0,1);
	lcd.print("Init SD Log");
	Serial.print("Initializing SD card...");
	// make sure that the default chip select pin is set to
	// output, even if you don't use it:
	
	
	// see if the card is present and can be initialized:
	if (!SD.begin(MICRO_SD_CHIP_SELECT)) {
		Serial.println("Card failed, or not present");
		// don't do anything more:
		while (1) ;
	}
	Serial.println("card initialized.");
	
	// Open up the file we're going to log to!
	dataFile = SD.open("datalog.txt", FILE_WRITE);
	if (! dataFile) {
		Serial.println("error opening datalog.txt");
		// Wait forever since we cant write data
		while (1) ;
	}

	delay(800);
	lcd.setCursor(0,3);
	lcd.print("Success!");
	
}

void initialize_vs1053_music_player() {
	
	
	wipe_LCD_screen();
	lcd.setCursor(0,1);
	lcd.print("Init VS1053 Audio");
	
	if (! musicPlayer.begin()) { // initialise the music player
		Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
		while (1);
	}
	Serial.println(F("VS1053 found"));
	
	// Set volume for left, right channels. lower numbers == louder volume!
	musicPlayer.setVolume(2,2);

	// Timer interrupts are not suggested, better to use DREQ interrupt!
	//musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

	// If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
	// audio playing
	musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
	musicPlayer.startPlayingFile("track001.mp3");
	
	delay(800);
	lcd.setCursor(0,3);
	lcd.print("Success!");
	
}

void initialize_LCD_menu_system() {
	
	
	wipe_LCD_screen();
	lcd.setCursor(0,1);
	lcd.print("Init Menu System");
	
	//configure menu
	menu.getRoot().add(menu1Item1);
	menu1Item1.addRight(menu1Item2).addRight(menu1Item3).addRight(menu1Item4);
	menu1Item1.add(menuItem1SubItem1).addRight(menuItem1SubItem2).addRight(menuItem1SubItem3).addRight(menuItem1SubItem4).addRight(menuItem1SubItem5).addRight(menuItem1SubItem6).addRight(menuItem1SubItem7).addRight(menuItem1SubItem8);
	menu1Item2.add(menuItem2SubItem1).addRight(menuItem2SubItem2).addRight(menuItem2SubItem3).addRight(menuItem2SubItem4).addRight(menuItem2SubItem5).addRight(menuItem2SubItem6).addRight(menuItem2SubItem7).addRight(menuItem2SubItem8).addRight(menuItem2SubItem9).addRight(menuItem2SubItem10);
	menu1Item3.add(menuItem3SubItem1).addRight(menuItem3SubItem2);
	menu1Item4.add(menu1Item4SubItem1);
	menu.toRoot();
	delay(800);
	lcd.setCursor(0,3);
	lcd.print("Success!");
	delay(500);
	wipe_LCD_screen();
}

void initialize_stereo() {
	
	if (! setvolume(thevol)) {
		Serial.println("Failed to set volume, MAX9744 not found!");
		
	}
	
	digitalWrite(PIEZO_SOUNDER_PIN,HIGH);
	delay(2000);
	digitalWrite(PIEZO_SOUNDER_PIN,LOW);
	
	digitalWrite(MUTE_AUDIO_PIN,LOW);
}



void menuChanged(MenuChangeEvent changed){
	
	MenuItem newMenuItem=changed.to; //get the destination menu
	
	lcd.setCursor(0,2); //set the start position for lcd printing to the second row
	
	if(newMenuItem.getName()==menu.getRoot()){
		wipe_LCD_screen();
		lcd.print("   .:Menu System:.  ");
		}else if(newMenuItem.getName()=="Select Alarm Modes"){
		lcd.print("Select Alarm Modes  ");
		}else if(newMenuItem.getName()=="Audio Enable"){
		lcd.print("Audio Enable        ");
		}else if(newMenuItem.getName()=="Audio Disable"){
		lcd.print("Audio Disable       ");
		}else if(newMenuItem.getName()=="Piezo Enable"){
		lcd.print("Piezo Enable        ");
		}else if(newMenuItem.getName()=="Piezo Disable"){
		lcd.print("Piezo Disable       ");
		}else if(newMenuItem.getName()=="Small LED Enable"){
		lcd.print("Small LED Enable    ");
		}else if(newMenuItem.getName()=="Small LED Disable"){
		lcd.print("Small LED Disable   ");
		}else if(newMenuItem.getName()=="Big LED Enable"){
		lcd.print("Big LED Enable      ");
		}else if(newMenuItem.getName()=="Big LED Disable"){
		lcd.print("Big LED Disable     ");
		}else if(newMenuItem.getName()=="Select Alarm Pattern"){
		lcd.print("Select Alarm Pattern");
		}else if(newMenuItem.getName()=="LEDs Red"){
		lcd.print("LEDs Red            ");
		}else if(newMenuItem.getName()=="LEDs Green"){
		lcd.print("LEDs Green          ");
		}else if(newMenuItem.getName()=="LEDs Blue"){
		lcd.print("LEDs Blue           ");
		}else if(newMenuItem.getName()=="LEDs Random Color"){
		lcd.print("LEDs Random Color   ");
		}else if(newMenuItem.getName()=="Piezo Short"){
		lcd.print("Piezo Short         ");
		}else if(newMenuItem.getName()=="Piezo Long"){
		lcd.print("Piezo Long          ");
		}else if(newMenuItem.getName()=="Piezo Random"){
		lcd.print("Piezo Random        ");
		}else if(newMenuItem.getName()=="Audio Track 1"){
		lcd.print("Audio Track 1       ");
		}else if(newMenuItem.getName()=="Audio Track 2"){
		lcd.print("Audio Track 2       ");
		}else if(newMenuItem.getName()=="Audio Track Random"){
		lcd.print("Audio Track Random  ");
		}else if(newMenuItem.getName()=="System Changes"){
		lcd.print("System Changes      ");
		}else if(newMenuItem.getName()=="Inc Volume (+)"){
		lcd.print("Inc Volume (+)      ");
		}else if(newMenuItem.getName()=="Dec Volume (-)"){
		lcd.print("Dec Volume (-)      ");
		}else if(newMenuItem.getName()=="Done"){
		lcd.print("       Done         ");
		}else if(newMenuItem.getName()=="Finished with Menu"){
		lcd.print(" Finished with Menu ");
	}
}

void menuUsed(MenuUseEvent used){
	wipe_LCD_screen();
	lcd.setCursor(0,0);
	lcd.print("You Chose        ");
	lcd.setCursor(0,1);
	lcd.print(used.item.getName());
	delay(3000);  //delay to allow message reading
	//lcd.setCursor(0,0);
	//lcd.print("APD");
	
	menu.toRoot();  //back to Main
	if(used.item.getName() == "Finished with Menu")
	{
		Serial.println("Going back to polling daytime mode now");
		state =STATE_PREPARE_FOR_DAYTIME_IDLE;
		wipe_LCD_screen();
	}
	
	if(used.item.getName() == "Audio Track 1")
	{
		digitalWrite(MUTE_AUDIO_PIN,HIGH);
		SD.end();
		SD.begin(CARDCS);    // initialise the SD card
		Serial.println("Playing Audio Track 1");
		
		while(!musicPlayer.playFullFile("track001.mp3")) {
			// do nothing
			Serial.println("Inside the while loop");
		}
		
		
		// delay(3000);
		digitalWrite(MUTE_AUDIO_PIN,LOW);
		state =STATE_PREPARE_FOR_DAYTIME_IDLE;
		wipe_LCD_screen();
	}
	
	if(used.item.getName() == "Audio Track 2")
	{
		digitalWrite(MUTE_AUDIO_PIN,HIGH);
		SD.end();
		SD.begin(CARDCS);    // initialise the SD card
		Serial.println("Playing Audio Track 2");
		musicPlayer.startPlayingFile("track002.mp3");
		delay(3000);
		digitalWrite(MUTE_AUDIO_PIN,LOW);
		state =STATE_PREPARE_FOR_DAYTIME_IDLE;
		wipe_LCD_screen();
	}
	
	if(used.item.getName() == "Audio Track Random")
	{
		digitalWrite(MUTE_AUDIO_PIN,HIGH);
		SD.end();
		SD.begin(CARDCS);    // initialise the SD card
		Serial.println("Playing Audio Track Random");
		musicPlayer.startPlayingFile("track003.mp3");
		
		delay(3000);
		digitalWrite(MUTE_AUDIO_PIN,LOW);
		state =STATE_PREPARE_FOR_DAYTIME_IDLE;
		wipe_LCD_screen();
	}
	
	if(used.item.getName() == "LEDs Blue")
	{
		pattern_type = BLUE_LED_PATTERN_TYPE;
		Serial.println("Changed LED's to blue only");
		delay(1000);
		state =STATE_PREPARE_FOR_DAYTIME_IDLE;
		wipe_LCD_screen();
	}
	
	if(used.item.getName() == "LEDs Random Color")
	{
		pattern_type = RANDOM_LED_PATTERN_TYPE;
		Serial.println("Changed LED's to Random Script");
		delay(1000);
		state =STATE_PREPARE_FOR_DAYTIME_IDLE;
		wipe_LCD_screen();
	}
	
}

void  readButtons(){  //read buttons status
	int reading;
	int buttonEnterState=LOW;             // the current reading from the Enter input pin
	int buttonEscState=LOW;             // the current reading from the input pin
	int buttonLeftState=LOW;             // the current reading from the input pin
	int buttonRightState=LOW;             // the current reading from the input pin

	//Enter button
	// read the state of the switch into a local variable:
	reading = digitalRead(buttonPinEnter);

	// check to see if you just pressed the enter button
	// (i.e. the input went from LOW to HIGH),  and you've waited
	// long enough since the last press to ignore any noise:
	
	// If the switch changed, due to noise or pressing:
	if (reading != lastButtonEnterState) {
		// reset the debouncing timer
		lastEnterDebounceTime = millis();
	}
	
	if ((millis() - lastEnterDebounceTime) > debounceDelay) {
		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:
		buttonEnterState=reading;
		lastEnterDebounceTime=millis();
	}
	
	// save the reading.  Next time through the loop,
	// it'll be the lastButtonState:
	lastButtonEnterState = reading;
	

	//Esc button
	// read the state of the switch into a local variable:
	reading = digitalRead(buttonPinEsc);

	// check to see if you just pressed the Down button
	// (i.e. the input went from LOW to HIGH),  and you've waited
	// long enough since the last press to ignore any noise:
	
	// If the switch changed, due to noise or pressing:
	if (reading != lastButtonEscState) {
		// reset the debouncing timer
		lastEscDebounceTime = millis();
	}
	
	if ((millis() - lastEscDebounceTime) > debounceDelay) {
		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:
		buttonEscState = reading;
		lastEscDebounceTime=millis();
	}
	
	// save the reading.  Next time through the loop,
	// it'll be the lastButtonState:
	lastButtonEscState = reading;
	
	
	//Down button
	// read the state of the switch into a local variable:
	reading = digitalRead(buttonPinRight);

	// check to see if you just pressed the Down button
	// (i.e. the input went from LOW to HIGH),  and you've waited
	// long enough since the last press to ignore any noise:
	
	// If the switch changed, due to noise or pressing:
	if (reading != lastButtonRightState) {
		// reset the debouncing timer
		lastRightDebounceTime = millis();
	}
	
	if ((millis() - lastRightDebounceTime) > debounceDelay) {
		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:
		buttonRightState = reading;
		lastRightDebounceTime =millis();
	}
	
	// save the reading.  Next time through the loop,
	// it'll be the lastButtonState:
	lastButtonRightState = reading;
	
	
	//Up button
	// read the state of the switch into a local variable:
	reading = digitalRead(buttonPinLeft);

	// check to see if you just pressed the Down button
	// (i.e. the input went from LOW to HIGH),  and you've waited
	// long enough since the last press to ignore any noise:
	
	// If the switch changed, due to noise or pressing:
	if (reading != lastButtonLeftState) {
		// reset the debouncing timer
		lastLeftDebounceTime = millis();
	}
	
	if ((millis() - lastLeftDebounceTime) > debounceDelay) {
		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:
		buttonLeftState = reading;
		lastLeftDebounceTime=millis();;
	}
	
	// save the reading.  Next time through the loop,
	// it'll be the lastButtonState:
	lastButtonLeftState = reading;

	//records which button has been pressed
	if (buttonEnterState==HIGH){
		lastButtonPushed=buttonPinEnter;

		}else if(buttonEscState==HIGH){
		lastButtonPushed=buttonPinEsc;

		}else if(buttonRightState==HIGH){
		lastButtonPushed=buttonPinRight;

		}else if(buttonLeftState==HIGH){
		lastButtonPushed=buttonPinLeft;

		}else{
		lastButtonPushed=0;
	}
}

void navigateMenus() {
	MenuItem currentMenu=menu.getCurrent();
	
	switch (lastButtonPushed){
		case buttonPinEnter:
		if(!(currentMenu.moveDown())){  //if the current menu has a child and has been pressed enter then menu navigate to item below
			menu.use();
			}else{  //otherwise, if menu has no child and has been pressed enter the current menu is used
			menu.moveDown();
		}
		break;
		case buttonPinEsc:
		menu.toRoot();  //back to main
		break;
		case buttonPinRight:
		menu.moveRight();
		break;
		case buttonPinLeft:
		menu.moveLeft();
		break;
	}
	
	lastButtonPushed=0; //reset the lastButtonPushed variable
}


// Setting the volume is very simple! Just write the 6-bit
// volume to the i2c bus. That's it!
boolean setvolume(int8_t v) {
	// cant be higher than 63 or lower than 0
	if (v > 63) v = 63;
	if (v < 0) v = 0;
	
	Serial.print("Setting volume to ");
	Serial.println(v);
	Wire.beginTransmission(MAX9744_I2CADDR);
	Wire.write(v);
	if (Wire.endTransmission() == 0)
	return true;
	else
	return false;
}



void pin_19_ISR() {
	
	state = STATE_MENU_ISR;
}

void DayNightISR() {
	
	state = STATE_DAY_NIGHT_ISR;
	
}