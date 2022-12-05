#include <ESP32Servo.h>

// create four servo objects 
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
// Published values for SG90 servos; adjust if needed
int minUs = 500;
int maxUs = 2500;
bool start = 0;
int dispensed = 0;
int button_int = 0;
#define PushButton 0

// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42

int servo1Pin = 15;
int servo2Pin = 16;
int servo3Pin = 14;
int servo4Pin = 32;
int servoPin [] = {15,16,14,32};

int photoresistorpin = 13;
static uint16_t rising_thresh = 600;
static uint16_t falling_thresh = 500;
bool analog_int;

int pos = 0;      // position in degrees
ESP32PWM pwm;

void IRAM_ATTR buttonInput()
{
  digitalWrite(LED_BUILTIN, HIGH);
  ++button_int;
  Serial.println("Button Pressed");
}

void photoresistread() {
	if(!analog_int){
		if(analogRead(photoresistorpin) > rising_thresh){
			analog_int = 1;
			++dispensed;
			Serial.println("Pill Dispensed");
		}
	}
	if(analog_int){
		if(analogRead(photoresistorpin) < falling_thresh){
			analog_int = 0;
		}
	}
}

void setup() {
	// Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	Serial.begin(115200);
	servo1.setPeriodHertz(50);      // Standard 50hz servo

	pinMode(PushButton, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(PushButton),buttonInput,FALLING);
}

void loop() {
	servo1.attach(servo1Pin, minUs, maxUs);
#if defined(ARDUINO_ESP32S2_DEV)
	pwm.attachPin(37, 10000);//10khz
#else
	pwm.attachPin(27, 10000);//10khz
#endif
	if (start==0){
		servo1.write(0); 
		delay(10);
	}
	if (start==1){
		servo1.write(120);
		delay(10);
	}
	if(button_int > dispensed){
		start=1;
		for (pos = 180; pos >= 0; pos -= 1) { // sweep from 180 degrees to 0 degrees
			servo1.write(pos);
			digitalWrite(LED_BUILTIN, HIGH);
			photoresistread();
			delay(5);
		}
		for (int i = 0; i <= 100; ++i){
			photoresistread();
			delay(10);
		}
		for (pos = 0; pos <= 180; pos += 1) { // sweep from 0 degrees to 180 degrees
			// in steps of 1 degree
			servo1.write(pos);
			digitalWrite(LED_BUILTIN, LOW);
			photoresistread();
			delay(5);             // waits 20ms for the servo to reach the position
		}
	}
	servo1.detach();
	pwm.detachPin(27);
}
