#include <ESP32Servo.h>


#define minUs 500
#define maxUs 2500
#define rising_thresh 600
#define falling_thresh 500
#define PushButton 0

int dispensed = 0;
int button_int = 0;
// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42

static int servoPin [] = {15,16,14,32};
static int photoresistorpin = 13;

Servo servo1;
ESP32PWM pwm;
TaskHandle_t readtask;

void IRAM_ATTR buttonInput()
{
  digitalWrite(LED_BUILTIN, HIGH);
  ++dispensed;
  Serial.println("Button Pressed");
}

void photoresistread(void * parameter) {
	bool analog_int = 1;
	while(1){
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
		delay(1);
	}
}

int servoDispense(int ServoNum, int DispenseAmount){
	// Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);

	servo1.setPeriodHertz(50);      // Standard 50hz servo
	servo1.attach(servoPin[ServoNum], minUs, maxUs);
	pwm.attachPin(27, 10000);//10khz
	dispensed = 0;
	Serial.println("dispensing");

	xTaskCreatePinnedToCore(photoresistread, "task1", 10000, NULL, 1, &readtask, 0);

	for (size_t i = 0; i < 2*DispenseAmount+3; i++)
	{
		int pos;
		for (pos = 120; pos >= 0; pos -= 1) { // sweep from 180 degrees to 0 degrees
			servo1.write(pos);
			digitalWrite(LED_BUILTIN, HIGH);
			//photoresistread();
			delay(5);
		}
		delay(500);
		for (pos = 0; pos <= 180; pos += 1) { // sweep from 0 degrees to 180 degrees
			// in steps of 1 degree
			servo1.write(pos);
			digitalWrite(LED_BUILTIN, LOW);
			//photoresistread();
			delay(5);             // waits 20ms for the servo to reach the position
		}
		for (pos = 180; pos >= 120; pos -= 1) { // sweep from 180 degrees to 0 degrees
			servo1.write(pos);
			digitalWrite(LED_BUILTIN, HIGH);
			//photoresistread();
			delay(5);
		}
		if(DispenseAmount <= dispensed) break;
	}
	vTaskDelete(readtask);
	servo1.detach();
	pwm.detachPin(27);
	return(dispensed);
}

void setup() {
	
	Serial.begin(115200);
	pinMode(PushButton, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(PushButton),buttonInput,FALLING);
}

void loop() {
	Serial.println(servoDispense(0,1));
	delay(10000);
}
