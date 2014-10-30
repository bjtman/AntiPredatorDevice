/* 
	Editor: http://www.visualmicro.com
	        visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
	        the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
	        all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
	        note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: Arduino Mega w/ ATmega2560 (Mega 2560), Platform=avr, Package=arduino
*/

#define __AVR_ATmega2560__
#define _VMDEBUG 1
#define ARDUINO 101
#define ARDUINO_MAIN
#define F_CPU 16000000L
#define __AVR__
#define __cplusplus
extern "C" void __cxa_pure_virtual() {;}

//
//
void lookForBlinkM();
void initialize_and_test_leds();
void initialize_real_time_clock();
void initialize_pin_modes();
void initialize_and_calibrate_PIR_sensor_array();
void initialize_lux_sensor();
void initialize_lcd_backpack_and_screen();
void wipe_LCD_screen();
void initialize_datalogging_sd_card();
void initialize_vs1053_music_player();
void initialize_LCD_menu_system();
void initialize_stereo();
void menuChanged(MenuChangeEvent changed);
void menuUsed(MenuUseEvent used);
void  readButtons();
void navigateMenus();
boolean setvolume(int8_t v);
void pin_19_ISR();
void DayNightISR();

#include "C:\Program Files (x86)\Arduino\hardware\arduino\avr\variants\mega\pins_arduino.h" 
#include "C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\arduino.h"
#include "C:\Users\Brian\Documents\Atmel Studio\6.2\AntiPredatorDevice\AntiPredatorDevice\AntiPredatorDevice.ino"
#include "C:\Users\Brian\Documents\Atmel Studio\6.2\AntiPredatorDevice\AntiPredatorDevice\BlinkM_funcs.h"
