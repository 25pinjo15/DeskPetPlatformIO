/*********\
 * DeskPet
 *
 * NOTE: im currentrly setting hand state in the ultrasound function
 *
 *********/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h>
#include <NewPing.h>

unsigned long curMillis = 0; // Current time at start of cycle

/* NOTE:
	Cat state:
	0:
	1: Happy
	2: Wandering aroung
	3: :| face .



*/

// 88888888888888888888 DEBUGER ON / OFF
bool DEBUGER = true;

// ==== Display variable ====
unsigned long frameMillis = 0; // Used for timer thing
bool menuShowing = 0;

// ==== Cat related stuff ====
int catXPos = 0;		// Current cat X position
#define catYPOS 19 		// Cat y position . but now it still since the sprite take the entire screen
bool catWay = true; 	// True mean right , false mean left when moving 
int catState = 1;		// Current state of the cat
byte catMoving = 0;		 /* Is the cat is moving 0=no 1=yes(left to right slowly) */
int catHunger = 0;		// How much the cat is hungry . 1000 mean full , 0 starving AF
int catHappiness = 0;	// How much the cat is happy , 1000 happy AF 0 Depressed AF

unsigned long lastActionSince = 0; // Time since a last action was done on cat
unsigned long lastAction = 0;
#define lastActionLenght 4000 // Time it take before it triger wandering

bool newFrame = false; // If yes or no a new frame is needed

// ===== Ultrasound sensor ====

#define ECHO_PIN 11		// attach pin D2 Arduino to pin Echo of HC-SR04
#define TRIGGER_PIN 12	// attach pin D3 Arduino to pin Trig of HC-SR04
#define MAX_DISTANCE 50 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

unsigned int pingSpeed = 50; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long pingTimer;	 // Holds the next ping time.

// ==== Sound related ====
#define SPEAKER 5

// ==== Hand related variable =====
byte handDistance = 0; // Hold hand distance in cm
byte handState = 0;	   // Will hold hand state like example 1 is petting 2 is near 3 is away
unsigned long handLastAction = 0;
unsigned long handActionDuration = 0;
bool handPresent = false;
unsigned long handMillis = 0;

// ==== Button related stuff ====

long lastDebounceTime = 0;
#define DEBOUNCE_DELAY 50

#define buttonPinPet 10			// the number of the pushbutton pin
byte buttonPetState;			// Button event pressed
byte buttonPetPressed;			// To read the physical button
byte lastButtonPetPressed;		// For button detection and debounce

#define buttonPinFeed 9			// the number of the pushbutton pin
byte buttonFeedState;			// Button event pressed
byte buttonFeedPressed;			// To read the physical button
byte lastButtonFeedPressed;		// For button detection and debounce

#define buttonPinTreat 8		// the number of the pushbutton pin
byte buttonTreatState;			// Button event pressed
byte buttonTreatPressed;		// To read the physical button
byte lastButtonTreatPressed;	// For button detection and debounce

#define buttonPinPlay 7			// the number of the pushbutton pin
byte buttonPlayState;			// Button event pressed
byte buttonPlayPressed;			// To read the physical button
byte lastButtonPlayPressed;		// For button detection and debounce




	
	
	

// ==== LCD Related stuff ====

#define XPOS 0 // Indexes into the 'icons' array in function below
#define YPOS 1
#define DELTAY 2

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==== Cat sprite , whith dimention ====

#define SPRITE_HEIGHT 40
#define SPRITE_WIDTH 40

// 'cat_blink', 40x40px
const unsigned char PROGMEM cat_blink [] = {
	0x18, 0x00, 0x00, 0x00, 0x18, 0x1c, 0x00, 0x00, 0x00, 0x38, 0x3e, 0x00, 0x00, 0x00, 0x7c, 0x37, 
	0x00, 0x00, 0x00, 0xec, 0x33, 0x80, 0x00, 0x01, 0xcc, 0x31, 0xc0, 0x00, 0x03, 0x8c, 0x34, 0xe0, 
	0x00, 0x07, 0x2c, 0x36, 0x70, 0x00, 0x0e, 0x6c, 0x37, 0x38, 0x00, 0x1c, 0xec, 0x37, 0x9c, 0x00, 
	0x39, 0xec, 0x37, 0xce, 0x00, 0x73, 0xec, 0x37, 0xe7, 0xff, 0xe7, 0xec, 0x37, 0xc3, 0xff, 0xc3, 
	0xec, 0x36, 0x00, 0x3c, 0x00, 0x6c, 0x30, 0x00, 0x00, 0x00, 0x0c, 0x70, 0x00, 0x00, 0x00, 0x0e, 
	0x70, 0x00, 0x00, 0x20, 0x0e, 0x60, 0x00, 0x00, 0x70, 0x06, 0x60, 0x1f, 0x00, 0xf8, 0x06, 0x60, 
	0x00, 0x00, 0x70, 0x06, 0xe0, 0x00, 0x00, 0x20, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 
	0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x7e, 0x00, 0x03, 0xc0, 0x18, 0x3c, 
	0x18, 0x03, 0xe0, 0x06, 0x18, 0x60, 0x07, 0x60, 0x00, 0x18, 0x00, 0x06, 0x70, 0x1e, 0x18, 0x78, 
	0x0e, 0x30, 0x00, 0x18, 0x00, 0x0c, 0x18, 0x02, 0x18, 0x40, 0x18, 0x1e, 0x04, 0x7f, 0x20, 0x78, 
	0x0f, 0x88, 0x0f, 0x11, 0xf0, 0x03, 0xe0, 0x0f, 0x07, 0xc0, 0x00, 0xf8, 0x06, 0x1f, 0x00, 0x00, 
	0x3e, 0x00, 0x7c, 0x00, 0x00, 0x0f, 0x81, 0xf0, 0x00, 0x00, 0x01, 0xe7, 0x80, 0x00, 0x00, 0x00, 
	0x7e, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00
};
// 'cat_exasp', 40x40px
const unsigned char PROGMEM cat_exasp [] = {
	0x18, 0x00, 0x00, 0x00, 0x18, 0x1c, 0x00, 0x00, 0x00, 0x38, 0x3e, 0x00, 0x00, 0x00, 0x7c, 0x37, 
	0x00, 0x00, 0x00, 0xec, 0x33, 0x80, 0x00, 0x01, 0xcc, 0x31, 0xc0, 0x00, 0x03, 0x8c, 0x34, 0xe0, 
	0x00, 0x07, 0x2c, 0x36, 0x70, 0x00, 0x0e, 0x6c, 0x37, 0x38, 0x00, 0x1c, 0xec, 0x37, 0x9c, 0x00, 
	0x39, 0xec, 0x37, 0xce, 0x00, 0x73, 0xec, 0x37, 0xe7, 0xff, 0xe7, 0xec, 0x37, 0xc3, 0xff, 0xc3, 
	0xec, 0x36, 0x00, 0x3c, 0x00, 0x6c, 0x30, 0x00, 0x00, 0x00, 0x0c, 0x70, 0x00, 0x00, 0x00, 0x0e, 
	0x70, 0x00, 0x00, 0x00, 0x0e, 0x60, 0x00, 0x00, 0x00, 0x06, 0x60, 0x3f, 0x81, 0xfc, 0x06, 0x60, 
	0x18, 0x00, 0xc0, 0x06, 0xe0, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 
	0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x7e, 0x00, 0x03, 0xc0, 0x18, 0x3c, 
	0x18, 0x03, 0xe0, 0x06, 0x18, 0x60, 0x07, 0x60, 0x00, 0x18, 0x00, 0x06, 0x70, 0x1e, 0x18, 0x78, 
	0x0e, 0x30, 0x00, 0x18, 0x00, 0x0c, 0x18, 0x02, 0x00, 0x40, 0x18, 0x1e, 0x04, 0x00, 0x20, 0x78, 
	0x0f, 0x88, 0xff, 0x11, 0xf0, 0x03, 0xe0, 0x00, 0x07, 0xc0, 0x00, 0xf8, 0x00, 0x1f, 0x00, 0x00, 
	0x3e, 0x00, 0x7c, 0x00, 0x00, 0x0f, 0x81, 0xf0, 0x00, 0x00, 0x01, 0xe7, 0x80, 0x00, 0x00, 0x00, 
	0x7e, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00
};
// 'cat_happy', 40x40px
const unsigned char PROGMEM cat_happy [] = {
	0x18, 0x00, 0x00, 0x00, 0x18, 0x1c, 0x00, 0x00, 0x00, 0x38, 0x3e, 0x00, 0x00, 0x00, 0x7c, 0x37, 
	0x00, 0x00, 0x00, 0xec, 0x33, 0x80, 0x00, 0x01, 0xcc, 0x31, 0xc0, 0x00, 0x03, 0x8c, 0x34, 0xe0, 
	0x00, 0x07, 0x2c, 0x36, 0x70, 0x00, 0x0e, 0x6c, 0x37, 0x38, 0x00, 0x1c, 0xec, 0x37, 0x9c, 0x00, 
	0x39, 0xec, 0x37, 0xce, 0x00, 0x73, 0xec, 0x37, 0xe7, 0xff, 0xe7, 0xec, 0x37, 0xc3, 0xff, 0xc3, 
	0xec, 0x36, 0x00, 0x3c, 0x00, 0x6c, 0x30, 0x00, 0x00, 0x00, 0x0c, 0x70, 0x00, 0x00, 0x00, 0x0e, 
	0x70, 0x04, 0x00, 0x20, 0x0e, 0x60, 0x0a, 0x00, 0x50, 0x06, 0x60, 0x11, 0x00, 0x88, 0x06, 0x60, 
	0x11, 0x00, 0x88, 0x06, 0xe0, 0x11, 0x00, 0x88, 0x07, 0xc0, 0x11, 0x00, 0x88, 0x03, 0xc0, 0x11, 
	0x00, 0x88, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x7e, 0x00, 0x03, 0xc0, 0x18, 0x3c, 
	0x18, 0x03, 0xe0, 0x06, 0x18, 0x60, 0x07, 0x60, 0x00, 0x18, 0x00, 0x06, 0x70, 0x1e, 0x18, 0x78, 
	0x0e, 0x30, 0x00, 0x18, 0x00, 0x0c, 0x18, 0x02, 0x18, 0x40, 0x18, 0x1e, 0x04, 0x7e, 0x20, 0x78, 
	0x0f, 0x88, 0x3c, 0x11, 0xf0, 0x03, 0xe0, 0x18, 0x07, 0xc0, 0x00, 0xf8, 0x00, 0x1f, 0x00, 0x00, 
	0x3e, 0x00, 0x7c, 0x00, 0x00, 0x0f, 0x81, 0xf0, 0x00, 0x00, 0x01, 0xe7, 0x80, 0x00, 0x00, 0x00, 
	0x7e, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00
};
// 'cat_left', 40x40px
const unsigned char PROGMEM cat_left [] = {
	0x60, 0x00, 0x00, 0x00, 0x60, 0x70, 0x00, 0x00, 0x00, 0xe0, 0x78, 0x00, 0x00, 0x01, 0xf0, 0x7c, 
	0x00, 0x00, 0x03, 0xb0, 0x6e, 0x00, 0x00, 0x07, 0x30, 0x67, 0x00, 0x00, 0x0e, 0x30, 0x63, 0x80, 
	0x00, 0x1c, 0xb0, 0x69, 0xc0, 0x00, 0x39, 0xb0, 0x6c, 0xe0, 0x00, 0x73, 0xb0, 0x6e, 0x70, 0x00, 
	0xe7, 0xb0, 0x6f, 0x38, 0x01, 0xcf, 0xb0, 0x6f, 0x9f, 0xff, 0x9f, 0xb0, 0x6f, 0x0f, 0xff, 0x0f, 
	0xb0, 0x68, 0x01, 0xe0, 0x01, 0xb0, 0x60, 0x00, 0x00, 0x00, 0x30, 0x60, 0x00, 0x00, 0x00, 0x38, 
	0x60, 0x20, 0x01, 0x00, 0x1e, 0x60, 0x70, 0x03, 0x80, 0x06, 0xe0, 0xf8, 0x07, 0xc0, 0x06, 0xc0, 
	0x70, 0x03, 0x80, 0x06, 0xc0, 0x20, 0x01, 0x00, 0x06, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 
	0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x03, 0xf0, 0x00, 0x03, 0xc0, 0xc1, 0xe0, 
	0xc0, 0x03, 0xc0, 0x30, 0xc3, 0x00, 0x07, 0xe0, 0x00, 0xc0, 0x00, 0x06, 0x70, 0xf0, 0xc3, 0xc0, 
	0x0e, 0x30, 0x00, 0xc0, 0x00, 0x0c, 0x18, 0x10, 0xc2, 0x00, 0x18, 0x1c, 0x21, 0x21, 0x00, 0x78, 
	0x0f, 0x02, 0x10, 0x81, 0xf0, 0x03, 0x84, 0x08, 0x07, 0xc0, 0x01, 0xc0, 0x00, 0x1f, 0x00, 0x00, 
	0xe0, 0x00, 0x7c, 0x00, 0x00, 0x70, 0x01, 0xf0, 0x00, 0x00, 0x3c, 0x1f, 0x80, 0x00, 0x00, 0x1f, 
	0xf8, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00
};
// 'cat_neutral', 40x40px
const unsigned char PROGMEM cat_neutral [] = {
	0x18, 0x00, 0x00, 0x00, 0x18, 0x1c, 0x00, 0x00, 0x00, 0x38, 0x3e, 0x00, 0x00, 0x00, 0x7c, 0x37, 
	0x00, 0x00, 0x00, 0xec, 0x33, 0x80, 0x00, 0x01, 0xcc, 0x31, 0xc0, 0x00, 0x03, 0x8c, 0x34, 0xe0, 
	0x00, 0x07, 0x2c, 0x36, 0x70, 0x00, 0x0e, 0x6c, 0x37, 0x38, 0x00, 0x1c, 0xec, 0x37, 0x9c, 0x00, 
	0x39, 0xec, 0x37, 0xce, 0x00, 0x73, 0xec, 0x37, 0xe7, 0xff, 0xe7, 0xec, 0x37, 0xc3, 0xff, 0xc3, 
	0xec, 0x36, 0x00, 0x3c, 0x00, 0x6c, 0x30, 0x00, 0x00, 0x00, 0x0c, 0x70, 0x00, 0x00, 0x00, 0x0e, 
	0x70, 0x04, 0x00, 0x20, 0x0e, 0x60, 0x0e, 0x00, 0x70, 0x06, 0x60, 0x1f, 0x00, 0xf8, 0x06, 0x60, 
	0x0e, 0x00, 0x70, 0x06, 0xe0, 0x04, 0x00, 0x20, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 
	0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x7e, 0x00, 0x03, 0xc0, 0x18, 0x3c, 
	0x18, 0x03, 0xe0, 0x06, 0x18, 0x60, 0x07, 0x60, 0x00, 0x18, 0x00, 0x06, 0x70, 0x1e, 0x18, 0x78, 
	0x0e, 0x30, 0x00, 0x18, 0x00, 0x0c, 0x18, 0x02, 0x18, 0x40, 0x18, 0x1e, 0x04, 0x24, 0x20, 0x78, 
	0x0f, 0x88, 0x42, 0x11, 0xf0, 0x03, 0xe0, 0x81, 0x07, 0xc0, 0x00, 0xf8, 0x00, 0x1f, 0x00, 0x00, 
	0x3e, 0x00, 0x7c, 0x00, 0x00, 0x0f, 0x81, 0xf0, 0x00, 0x00, 0x01, 0xe7, 0x80, 0x00, 0x00, 0x00, 
	0x7e, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00
};
// 'cat_O', 40x40px
const unsigned char PROGMEM cat_O [] = {
	0x18, 0x00, 0x00, 0x00, 0x18, 0x1c, 0x00, 0x00, 0x00, 0x38, 0x3e, 0x00, 0x00, 0x00, 0x7c, 0x37, 
	0x00, 0x00, 0x00, 0xec, 0x33, 0x80, 0x00, 0x01, 0xcc, 0x31, 0xc0, 0x00, 0x03, 0x8c, 0x34, 0xe0, 
	0x00, 0x07, 0x2c, 0x36, 0x70, 0x00, 0x0e, 0x6c, 0x37, 0x38, 0x00, 0x1c, 0xec, 0x37, 0x9c, 0x00, 
	0x39, 0xec, 0x37, 0xce, 0x00, 0x73, 0xec, 0x37, 0xe7, 0xff, 0xe7, 0xec, 0x37, 0xc3, 0xff, 0xc3, 
	0xec, 0x36, 0x00, 0x3c, 0x00, 0x6c, 0x30, 0x00, 0x00, 0x00, 0x0c, 0x70, 0x00, 0x00, 0x00, 0x0e, 
	0x70, 0x00, 0x00, 0x00, 0x0e, 0x60, 0x0e, 0x00, 0x70, 0x06, 0x60, 0x0e, 0x00, 0x70, 0x06, 0x60, 
	0x0e, 0x00, 0x70, 0x06, 0xe0, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 
	0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x7e, 0x00, 0x03, 0xc0, 0x18, 0x3c, 
	0x18, 0x03, 0xe0, 0x06, 0x18, 0x60, 0x07, 0x60, 0x00, 0x18, 0x00, 0x06, 0x70, 0x1e, 0x18, 0x78, 
	0x0e, 0x30, 0x00, 0x00, 0x00, 0x0c, 0x18, 0x02, 0x00, 0x40, 0x18, 0x1e, 0x04, 0x06, 0x20, 0x78, 
	0x0f, 0x88, 0x0f, 0x11, 0xf0, 0x03, 0xe0, 0x0f, 0x07, 0xc0, 0x00, 0xf8, 0x00, 0x1f, 0x00, 0x00, 
	0x3e, 0x00, 0x7c, 0x00, 0x00, 0x0f, 0x81, 0xf0, 0x00, 0x00, 0x01, 0xe7, 0x80, 0x00, 0x00, 0x00, 
	0x7e, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00
};
// 'cat_right', 40x40px
const unsigned char PROGMEM cat_right [] = {
	0x06, 0x00, 0x00, 0x00, 0x06, 0x07, 0x00, 0x00, 0x00, 0x0e, 0x0f, 0x80, 0x00, 0x00, 0x1e, 0x0d, 
	0xc0, 0x00, 0x00, 0x3e, 0x0c, 0xe0, 0x00, 0x00, 0x76, 0x0c, 0x70, 0x00, 0x00, 0xe6, 0x0d, 0x38, 
	0x00, 0x01, 0xc6, 0x0d, 0x9c, 0x00, 0x03, 0x96, 0x0d, 0xce, 0x00, 0x07, 0x36, 0x0d, 0xe7, 0x00, 
	0x0e, 0x76, 0x0d, 0xf3, 0x80, 0x1c, 0xf6, 0x0d, 0xf9, 0xff, 0xf9, 0xf6, 0x0d, 0xf0, 0xff, 0xf0, 
	0xf6, 0x0d, 0x80, 0x07, 0x80, 0x16, 0x0c, 0x00, 0x00, 0x00, 0x06, 0x1c, 0x00, 0x00, 0x00, 0x06, 
	0x78, 0x00, 0x80, 0x04, 0x06, 0x60, 0x01, 0xc0, 0x0e, 0x06, 0x60, 0x03, 0xe0, 0x1f, 0x07, 0x60, 
	0x01, 0xc0, 0x0e, 0x03, 0x60, 0x00, 0x80, 0x04, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 
	0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x0f, 0xc0, 0x03, 0xc0, 0x03, 0x07, 
	0x83, 0x03, 0xe0, 0x00, 0xc3, 0x0c, 0x03, 0x60, 0x00, 0x03, 0x00, 0x07, 0x70, 0x03, 0xc3, 0x0f, 
	0x0e, 0x30, 0x00, 0x03, 0x00, 0x0c, 0x18, 0x00, 0x43, 0x08, 0x18, 0x1e, 0x00, 0x84, 0x84, 0x38, 
	0x0f, 0x81, 0x08, 0x40, 0xf0, 0x03, 0xe0, 0x10, 0x21, 0xc0, 0x00, 0xf8, 0x00, 0x03, 0x80, 0x00, 
	0x3e, 0x00, 0x07, 0x00, 0x00, 0x0f, 0x80, 0x0e, 0x00, 0x00, 0x01, 0xf8, 0x3c, 0x00, 0x00, 0x00, 
	0x1f, 0xf8, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00
};
// 'cat_sad', 40x40px
const unsigned char PROGMEM cat_sad [] = {
	0x18, 0x00, 0x00, 0x00, 0x18, 0x1c, 0x00, 0x00, 0x00, 0x38, 0x3e, 0x00, 0x00, 0x00, 0x7c, 0x37, 
	0x00, 0x00, 0x00, 0xec, 0x33, 0x80, 0x00, 0x01, 0xcc, 0x31, 0xc0, 0x00, 0x03, 0x8c, 0x34, 0xe0, 
	0x00, 0x07, 0x2c, 0x36, 0x70, 0x00, 0x0e, 0x6c, 0x37, 0x38, 0x00, 0x1c, 0xec, 0x37, 0x9c, 0x00, 
	0x39, 0xec, 0x37, 0xce, 0x00, 0x73, 0xec, 0x37, 0xe7, 0xff, 0xe7, 0xec, 0x37, 0xc3, 0xff, 0xc3, 
	0xec, 0x36, 0x00, 0x3c, 0x00, 0x6c, 0x30, 0x04, 0x00, 0x20, 0x0c, 0x70, 0x08, 0x00, 0x10, 0x0e, 
	0x70, 0x10, 0x00, 0x08, 0x0e, 0x60, 0x00, 0x00, 0x00, 0x06, 0x60, 0x04, 0x00, 0x20, 0x06, 0x60, 
	0x0e, 0x00, 0x70, 0x06, 0xe0, 0x1f, 0x00, 0xf8, 0x07, 0xc0, 0x0e, 0x00, 0x70, 0x03, 0xc0, 0x04, 
	0x00, 0x20, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x7e, 0x00, 0x03, 0xc0, 0x18, 0x3c, 
	0x18, 0x03, 0xe0, 0x06, 0x18, 0x60, 0x07, 0x60, 0x00, 0x18, 0x00, 0x06, 0x70, 0x1e, 0x18, 0x78, 
	0x0e, 0x30, 0x00, 0x18, 0x00, 0x0c, 0x18, 0x02, 0x18, 0x40, 0x18, 0x1e, 0x04, 0x24, 0x20, 0x78, 
	0x0f, 0x88, 0x42, 0x11, 0xf0, 0x03, 0xe0, 0x42, 0x07, 0xc0, 0x00, 0xf8, 0x42, 0x1f, 0x00, 0x00, 
	0x3e, 0x00, 0x7c, 0x00, 0x00, 0x0f, 0x81, 0xf0, 0x00, 0x00, 0x01, 0xe7, 0x80, 0x00, 0x00, 0x00, 
	0x7e, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00
};
// 'cat_sick', 40x40px
const unsigned char PROGMEM cat_sick [] = {
	0x18, 0x00, 0x00, 0x00, 0x18, 0x1c, 0x00, 0x00, 0x00, 0x38, 0x3e, 0x00, 0x00, 0x00, 0x7c, 0x37, 
	0x00, 0x00, 0x00, 0xec, 0x33, 0x80, 0x00, 0x01, 0xcc, 0x31, 0xc0, 0x00, 0x03, 0x8c, 0x34, 0xe0, 
	0x00, 0x07, 0x2c, 0x36, 0x70, 0x00, 0x0e, 0x6c, 0x37, 0x38, 0x00, 0x1c, 0xec, 0x37, 0x9c, 0x00, 
	0x39, 0xec, 0x37, 0xce, 0x00, 0x73, 0xec, 0x37, 0xe7, 0xff, 0xe7, 0xec, 0x37, 0xc3, 0xff, 0xc3, 
	0xec, 0x36, 0x00, 0x3c, 0x00, 0x6c, 0x30, 0x00, 0x00, 0x00, 0x0c, 0x70, 0x00, 0x00, 0x00, 0x0e, 
	0x70, 0x00, 0x00, 0x00, 0x0e, 0x60, 0x0c, 0x00, 0x30, 0x06, 0x60, 0x0c, 0x00, 0x30, 0x06, 0x60, 
	0x0c, 0x00, 0x30, 0x06, 0xe0, 0x0c, 0x00, 0x30, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 
	0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x7e, 0x00, 0x03, 0xc0, 0x18, 0x3c, 
	0x18, 0x03, 0xe0, 0x06, 0x18, 0x60, 0x07, 0x60, 0x00, 0x18, 0x00, 0x06, 0x70, 0x1e, 0x18, 0x78, 
	0x0e, 0x30, 0x00, 0x18, 0x00, 0x0c, 0x18, 0x02, 0x00, 0x40, 0x18, 0x1e, 0x04, 0x88, 0x20, 0x78, 
	0x0f, 0x89, 0x55, 0x11, 0xf0, 0x03, 0xe2, 0x22, 0x07, 0xc0, 0x00, 0xf8, 0x00, 0x1f, 0x00, 0x00, 
	0x3e, 0x00, 0x7c, 0x00, 0x00, 0x0f, 0x81, 0xf0, 0x00, 0x00, 0x01, 0xe7, 0x80, 0x00, 0x00, 0x00, 
	0x7e, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00
};

// ==== Icon Sprite ====

#define ICON_HEIGHT 12		// Size of the icon
#define ICON_WIDTH 12

#define HungerXPos 2		// Top left corner
#define HungerYPos 2
#define HapinessXpos 48		// Just after hunger in the top bar
#define HapinessYpos 2

// 'icon_hapiness', 12x12px
const unsigned char PROGMEM icon_happiness [] = {
	0x00, 0x00, 0x00, 0x00, 0x30, 0xc0, 0x30, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x40, 0x20, 0x20, 0x40, 0x1f, 0x80, 0x00, 0x00
};
// 'icon_hunger', 12x12px
const unsigned char PROGMEM icon_hunger [] = {
	0x01, 0x00, 0x02, 0x00, 0x7b, 0x80, 0x86, 0x80, 0x80, 0xc0, 0x80, 0x70, 0x80, 0x10, 0x80, 0x10, 
	0x80, 0x10, 0x40, 0x20, 0x26, 0x40, 0x19, 0x80
};

//######## All Function start

static void playTone(uint16_t tone1, uint16_t duration)
{
	if (tone1 < 50 || tone1 > 15000)
		return; // these do not play on a piezo
	for (long i = 0; i < duration * 1000L; i += tone1 * 2)
	{
		digitalWrite(SPEAKER, HIGH);
		delayMicroseconds(tone1);
		digitalWrite(SPEAKER, LOW);
		delayMicroseconds(tone1);
	}
}
void meow2()
{ // cat meow (emphasis on "ow")
	uint16_t i;
	playTone(5100, 55);			   // "m" (short)
	playTone(394, 170);			   // "eee" (long)
	delay(30);					   // wait a tiny bit
	for (i = 330; i < 360; i += 2)
	{
	 // vary "ooo" down
		playTone(i, 10);
	}
	playTone(5100, 40); // "w" (short)
}

void mew()
{ // cat mew
	playTone(5100, 55); // "m"   (short)
	playTone(394, 130); // "eee" (long)
	playTone(384, 35);	// "eee" (up a tiny bit on end)
	playTone(5100, 40); // "w"   (short)
}
static void meow() // cat meow (emphasis ow "me")
{
	uint16_t i;
	playTone(5100, 50);				// "m" (short)
	playTone(394, 180);				// "eee" (long)
	for (i = 990; i < 1022; i += 2) // vary "ooo" down
	{
		playTone(i, 8);
	}
	playTone(5100, 40); // "w" (short)
}

static void RefreshDisplay(void)
{

	// This will calculate when a frame is needed.
	if (curMillis - frameMillis >= 200)
	{
		newFrame = true;
		frameMillis = millis(); // Reset the counter

	} // else newFrame = false;

	/* Cat position changer 2000
	 The cat can only change 1 posistion before each frame if the condition is true
	 */
	if (newFrame == true)
	{
		// ----- Moving thing around if needed
		// Will make the cat move if true
		if (catMoving == 1) // Moving 1 is left to right slowly
		{
			if (catWay == true)
			{
				catXPos++;
				if (catXPos >= 86)
				{
					catWay = false;
				}
			}
			else if (catWay == false)
			{
				catXPos--;
				if (catXPos <= 2)
				{
					catWay = true;
				}
			}
		}
		// ---- Lets display sone stuff if a frame is needed

		// Clear display here
		display.clearDisplay();

		// ---- Update the gui
		/*The hunger gauge is from 15 to 47 (including contour) to have a 30px gauge
		The happiness gauge is from 61 to 93 (including contour to have a 30px gauge*/
		
		display.drawRect(0,0,128,16,1);									// Draw the top rectangle
		display.drawRect(0,16,128,48,1);								// Draw the botom rectangle
		
		// If menu value is false will display the HUD
		if (menuShowing == false)
		{
			display.drawRect(15, 2, 32, 12, 1);								   // Draw the hunger gauge contour
			display.fillRect(16, 3, map(catHappiness, 0, 1000, 0, 30), 10, 1); // Fill the gauge with the value
			display.drawRect(61, 2, 32, 12, 1);								   // Draw the Happiness gauge contour
			display.fillRect(62, 3, map(catHunger, 0, 1000, 0, 30), 10, 1);	   // Fill the gauge with the value

			display.drawBitmap(HungerXPos, HungerYPos, icon_hunger, ICON_HEIGHT, ICON_WIDTH, 1);		// Put the hunger icon
			display.drawBitmap(HapinessXpos, HapinessYpos, icon_happiness, ICON_HEIGHT, ICON_WIDTH, 1); // Put the happiness icon
		} // Future else for menu 

		// ---- Update our cat

		switch (catState)
		{
		case 0:
			newFrame = false;
			break;

		case 1: 
			
			display.drawBitmap(catXPos, catYPOS, cat_happy, SPRITE_HEIGHT, SPRITE_WIDTH, 1);
			
			newFrame = false;
			break;

		case 2:		//Wandering around normal face

			// Will make the cat go right
			
			

			// Display the cat going right
			if (catWay == true)
			{
				display.drawBitmap(catXPos, catYPOS, cat_right, SPRITE_HEIGHT, SPRITE_WIDTH, 1);
			}
			else // Display the cat going right
			{
				display.drawBitmap(catXPos, catYPOS, cat_left, SPRITE_HEIGHT, SPRITE_WIDTH, 1);
			}

			newFrame = false;
			break;

		case 3: // Cat showing a emotion 
				// Cat is happy

			
			display.drawBitmap(catXPos, catYPOS, cat_happy, SPRITE_HEIGHT, SPRITE_WIDTH, 1);

			break;

		case 4: // Cat showing exclamation
			
			display.drawBitmap(catXPos, catYPOS, cat_O, SPRITE_HEIGHT, SPRITE_WIDTH, 1);
			newFrame = false;
			break;

		case 5:

			newFrame = false;
			break;

		default:
			break;
		}
		// ---- Final draw ! 
		display.display();		// Finally we draw everything

	}
}


static void UltraSound(void)
{

	if (sonar.check_timer())
	{														// This is how you check to see if the ping was received.
															// Here's where you can add code.
		handDistance = sonar.ping_result / US_ROUNDTRIP_CM; // Put the distance in cm in the hand distance
		handMillis = curMillis;								// Used to remember the last time a hand was present
	}
	// This will reset the distance if the hand is there for to long or not present at all
	if (curMillis - handMillis > 500)
	{
		handDistance = 50;
	}
}

static void HandState(void)
{

	if (handDistance <= 15)
	{
		if (handPresent == false)
		{
			handLastAction = curMillis;
		}
		handPresent = true;
		handActionDuration = curMillis - handLastAction;

		if ((handDistance < 6) && (handActionDuration > 400))
		{
			handState = 1;
			handLastAction = curMillis;
			handActionDuration = 0;
		}
		else if (((handDistance >= 6) && (handDistance < 13)) && (handActionDuration > 400))
		{
			handState = 2;
			handLastAction = curMillis;
			handActionDuration = 0;
		}
	}
	else
	{
		handState = 3;
		handActionDuration = 0;
		handPresent = false;
	}

	


}

// Will set the variable for the mood of the cat
static void Mood()
{
	if (handState == 1)
	{
		catState = 3;
	}
	else if (handState == 2)
	{
		catState = 4;
	}

	switch (catState)
	{
	case 1:
		catMoving = 0;
		break;
	case 2:
		catMoving = 1;
		break;
	case 3:
		catMoving = 0;
		break;
	case 4:
		catMoving = 0;
		break;
	case 5:
		catMoving = 0;
		break;

	default:
		break;
	}
}

static void Input()
{
		// beware for now action is done twice, once pressed and once released
	
	buttonPetPressed = digitalRead(buttonPinPet);		// Read the hardware button
	buttonFeedPressed = digitalRead(buttonPinFeed);		// Read the hardware button
	buttonTreatPressed = digitalRead(buttonPinTreat);	// Read the hardware button
	buttonPlayPressed = digitalRead(buttonPinTreat);	// Read the hardware button

	if (buttonPetPressed != lastButtonPetPressed) lastDebounceTime = curMillis;			// Resset the timer
	if (buttonFeedPressed != lastButtonFeedPressed) lastDebounceTime = curMillis;		// Resset the timer
	if (buttonTreatPressed != lastButtonTreatPressed) lastDebounceTime = curMillis;		// Resset the timer
	if (buttonPlayPressed != lastButtonPlayPressed) lastDebounceTime = curMillis;		// Resset the timer


	if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY)				// If the timer for debouce is meet , an action will be done per button
	{
		if (buttonPetPressed != buttonPetState)
		{
			buttonPetState = buttonPetPressed;
			catHappiness = catHappiness + 25;
			
		}
		if (buttonFeedPressed != buttonFeedState)
		{
			buttonFeedState = buttonFeedPressed;
		}
		if (buttonTreatPressed != buttonTreatState)
		{
			buttonTreatState = buttonTreatPressed;
		}
		if (buttonPlayPressed != buttonPlayState)
		{
			buttonPlayState = lastButtonTreatPressed;
		}

		
		
	}
	lastButtonPetPressed = buttonPetPressed;		// Set the variable to the reading of the button
	lastButtonFeedPressed = buttonFeedPressed;
	lastButtonTreatPressed = buttonTreatPressed;
	lastButtonPlayPressed = buttonPlayPressed;
}

// 88888888 All Function end

//######## Setup start 

void setup()
{
	// Input pin setup to all input and integrated pull up to on
	pinMode(buttonPinPet, INPUT_PULLUP);
	pinMode(buttonPinFeed, INPUT_PULLUP);
	pinMode(buttonPinTreat, INPUT_PULLUP);
	pinMode(buttonPinPlay, INPUT_PULLUP);
	
	
	
	// Value for testing 
	catHappiness = 500;
	catHunger = 750;



	
	if (DEBUGER == true)
	{
		Serial.begin(115200);
	}
	
	

	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
	{
		Serial.println(F("SSD1306 allocation failed"));
		for (;;)
			; // Don't proceed, loop forever
	}

	// Show initial display buffer contents on the screen --
	// the library initializes this with an Adafruit splash screen.
	display.display();
	delay(2000); // Pause for 2 seconds

	// ==== Variable setup =====
	catXPos = 30; // Set the initial cat position on the screen
	handDistance = 50;

	// ---- Sound Setup ----
	pinMode(SPEAKER,OUTPUT);  // important to set pin as output

	// ---- Ultrasound sensor setup ----
	pingTimer = millis(); // Start now.
}
//88888888 Setup end


//######## Loop Start
void loop()
{
	curMillis = millis();

	// Will check if a hand is over the sensor
	// Notice how there's no delays in this sketch to allow you to do other processing in-line while doing distance pings.
	if (millis() >= pingTimer) // pingSpeed milliseconds since last ping, do another ping.
	{
		pingTimer += pingSpeed;		  // Set the next ping time.
		sonar.ping_timer(UltraSound); // Send out the ping, calls "echoCheck" function every 24uS where you can check the ping status. and get the hand status
	}
	// Do other stuff here, really. Think of it as multi-tasking.

	Mood();
	RefreshDisplay();
	Input();

	// Will put the cat in wandering mode if no action is done .
	if (lastActionSince >= lastActionLenght)
	{
		catState = 2;
	}

	switch (handState)
	{
	case 1:
		catState = 3;
		lastAction = millis();
		break;

	case 2:
		catState = 3;
		lastAction = millis();
		break;

	case 3:
		/* code */
		break;

	default:
		break;
	}

	lastActionSince = millis() - lastAction;

	HandState();
	

	





	// DEBUGER:

	if (DEBUGER == true)
	{
		Serial.print("Last debounce time :");
		Serial.print(lastDebounceTime);
		Serial.print(" button pet state :");
		Serial.print(buttonPetState);
		Serial.print(" last button pet :");
		Serial.print(lastButtonPetPressed);
		Serial.print(" button pet pressed :");
		Serial.print(buttonPetPressed);
		Serial.print(" Cat State: ");
		Serial.print(catState);
		Serial.print(" frame time: ");
		Serial.print(curMillis - frameMillis);
		Serial.print(" Distance: ");
		Serial.print(handDistance);
		Serial.print("  Duration: ");
		Serial.print(handActionDuration);
		Serial.print("  Last action since: ");
		Serial.println(lastActionSince);
	}
}

//88888888 Loop End