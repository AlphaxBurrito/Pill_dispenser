#include "DFRobot_UI.h"
#include "DFRobot_GDL.h"
#include "DFRobot_Touch.h"
#include "DFRobot_Type.h"
#include "Adafruit_AHTX0.h"
#include <SD.h>
#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>

const char* ssid = "Erics93";
const char* password = "heejae9936";

const char* PARAM_Container = "Container";
const char* PARAM_Name = "Name";
const char* PARAM_Quantity = "Quantity";
const char* PARAM_TimeToTake = "TimeToTake";
const char* PARAM_PerDispense = "PerDispense";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;
AsyncWebServer server(80);

#define minUs 500
#define maxUs 2500
#define rising_thresh 600
#define falling_thresh 500

#define TFT_MISO 13 // (leave TFT SDO disconnected if other SPI devices share MISO)
#define TFT_MOSI 11
#define TFT_SCLK 12

#define TFT_CS    10  // Chip select control pin
#define TFT_DC    15  // Data Command control pin
#define TFT_RST   6  // Reset pin (could connect to RST pin)
#define TFT_BL    7
#define TFT_SCL   9
#define TFT_SDA   8
#define TFT_INT   16



static int servoPin [] = {15,16,14,32};
static int photoresistorpin = 48;

int dispensed = 0;
int button_int = 0;

Servo servo1;
ESP32PWM pwm;
TaskHandle_t readtask;

const int buzzer;

float temp1;
float humid1;
int mode;
String IP;
int currContainer;

class pillContainer {       // The class
  public:             // Access specifier
    String amountTaken;            // Amount taken per each dispense cycle
    String timesTaken;             // Amount of times a dispense cycle should
    String numberPills;            // Amount of pills stored in the container
    String alarmTime;           // Attribute (string variable)
    String medNickName;         // Name user would like to give pill
    boolean ready = false;      // Variable used for dispensing pills that are past alarm time
};

pillContainer cont1;
pillContainer cont2;
pillContainer cont3;
pillContainer cont4;

//Adafruit_AHTX0 aht;

/**
   @brief Constructor  When the touch uses the gt series chip, you can call this constructor
*/
DFRobot_Touch_GT911 touch;

/**
   @brief Constructor When the screen uses hardware SPI communication, the driver IC is st7789, and the screen resolution is 240x320, this constructor can be called
   @param dc Command/data line pin for SPI communication
   @param cs Chip select pin for SPI communication
   @param rst Reset pin of the screen
*/
DFRobot_ILI9488_320x480_HW_SPI screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);
/* M0 mainboard DMA transfer */
//DFRobot_ILI9488_320x480_DMA_SPI screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);


/**
   @brief Constructor
   @param gdl Screen object
   @param touch Touch object
*/
DFRobot_UI ui(&screen, &touch);

/*
  FUNCTION PROTOTYPE DECLARATIONS
*/
int servoDispense(int ServoNum, int DispenseAmount);
void newMedication();
void containerSelection();
void homeScreen();
void clearScreen();
void refresh();
void btnCallback(DFRobot_UI::sButton_t &btn,DFRobot_UI::sTextBox_t &obj);
/*
  FUNCTION DECLARATIONS
*/

void btnCallback(DFRobot_UI::sButton_t &btn,DFRobot_UI::sTextBox_t &obj) {
   String text((char *)btn.text);
   if(text == "IP"){
    obj.setText(IP);
   }
   else if(text == "1"){
    cont1.ready = true;

    String textBox = "Alarm:" + cont1.alarmTime + "        Per dispense:" + cont1.amountTaken + "    Pills Remaining" + cont1.numberPills + " Pill:" + cont1.medNickName;

    obj.setText(textBox);
   }
  else if(text == "2"){
    cont2.ready = true;

    String textBox = "Alarm:" + cont2.alarmTime + "        Per dispense:" + cont2.amountTaken + "    Pills Remaining" + cont2.numberPills + "Pill:" + cont2.medNickName;

    obj.setText(textBox);
   }
   else if(text == "3"){
    cont3.ready = true;

    String textBox = "Alarm:" + cont3.alarmTime + "        Per dispense:" + cont3.amountTaken + "    Pills Remaining" + cont3.numberPills + "Pill:" + cont3.medNickName;


    obj.setText(textBox);
   }
   else if(text == "4"){
    cont4.ready = true;

    String textBox = "Alarm:" + cont4.alarmTime + "        Per dispense:" + cont4.amountTaken + "    Pills Remaining" + cont4.numberPills + "Pill:" + cont4.medNickName;

    obj.setText(textBox);
   }
   else if(text == "GO"){
    int temp = 0;
    if (cont1.ready) {
      cont1.ready = false;
      temp = cont1.numberPills.toInt() - servoDispense(0, cont1.amountTaken.toInt());
      cont1.numberPills = (char*)temp;
    }
    if (cont2.ready) {
      cont2.ready = false;
      temp = cont2.numberPills.toInt() - servoDispense(1, cont2.amountTaken.toInt());
      cont2.numberPills = (char*)temp;
    }
    if (cont3.ready) {
      cont3.ready = false;
      temp = cont3.numberPills.toInt() - servoDispense(2, cont3.amountTaken.toInt());
      cont3.numberPills = (char*)temp;
    }
    if (cont4.ready) {
      cont4.ready = false;
      temp = cont4.numberPills.toInt() - servoDispense(3, cont4.amountTaken.toInt());
      cont4.numberPills = (char*)temp;
    }

  if (cont1.numberPills.toInt() <= 5) {
    tb.setText("Container low on     pills");
  }
  else if (cont2.numberPills.toInt() <= 5) {
    tb.setText("Container low on     pills");
  }
  else if (cont3.numberPills.toInt() <= 5) {
    tb.setText("Container low on     pills");
  }
  else if (cont4.numberPills.toInt() <= 5) {
    tb.setText("Container low on     pills");
  }
  else {
    tb.setText("");
  }
   }
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

  digitalWrite(47,LOW);
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

void homeScreen() {
  //Create a text box control
  DFRobot_UI::sTextBox_t & tb = ui.creatText();
  tb.bgColor = 0xe6B6;
  ui.draw(&tb);
  //Create a button control on the screen
  DFRobot_UI::sButton_t & btn1 = ui.creatButton();
  //Set the name of the button
  btn1.setText("ON");
  btn1.bgColor = COLOR_RGB565_RED;
  btn1.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  btn1.setOutput(&tb);
  ui.draw(&btn1,/**x=*/screen.width()/10,/**y=*/screen.height()/10*4,/*width*/screen.width()/3*2,/*height*/screen.width()/10*2);
  
  DFRobot_UI::sButton_t & btn2 = ui.creatButton();
  btn2.setText("OFF");
  btn2.bgColor = COLOR_RGB565_GREEN;
  btn2.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  btn2.setOutput(&tb);
  ui.draw(&btn2,/**x=*/(screen.width()/10),/**y=*/screen.height()/10*6,/*width*/screen.width()/3*2,/*height*/screen.width()/10*2);
 
  DFRobot_UI::sButton_t & btn3 = ui.creatButton();
  btn3.bgColor = COLOR_RGB565_BLUE;
  btn3.setText("clr");

  //Set the callback function of the button
  btn3.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  
  btn3.setOutput(&tb);
  ui.draw(&btn3,/**x=*/(screen.width()/10),/**y=*/screen.height()/10*8,/*width*/screen.width()/3*2,/*height*/screen.width()/10*2);
}

void backupScreen(DFRobot_UI::sTextBox_t &tb) {

  //Create a button control on the screen
  DFRobot_UI::sButton_t & btn1 = ui.creatButton();
  //Set the name of the button
  btn1.setText("1");
  btn1.bgColor = COLOR_RGB565_BLACK;
  btn1.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  btn1.setOutput(&tb);
  ui.draw(&btn1,/**x=*/screen.width()/10,/**y=*/screen.height()/2,/*width*/screen.width()/10*2,/*height*/screen.width()/10*2);
  
  DFRobot_UI::sButton_t & btn2 = ui.creatButton();
  btn2.setText("2");
  btn2.bgColor = COLOR_RGB565_BLACK;
  btn2.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  btn2.setOutput(&tb);
  ui.draw(&btn2,/**x=*/(screen.width()/10)*4,/**y=*/screen.height()/2,/*width*/screen.width()/10*2,/*height*/screen.width()/10*2);
 
  DFRobot_UI::sButton_t & btn3 = ui.creatButton();
  btn3.bgColor = COLOR_RGB565_BLUE;
  btn3.setText("IP");

  //Set the callback function of the button
  btn3.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  
  btn3.setOutput(&tb);
  ui.draw(&btn3,/**x=*/(screen.width()/10)*7,/**y=*/screen.height()/2,/*width*/screen.width()/10*2,/*height*/screen.width()/10*2);

  DFRobot_UI::sButton_t & btn4 = ui.creatButton();
  btn4.bgColor = COLOR_RGB565_BLACK;
  btn4.setText("3");

  //Set the callback function of the button
  btn4.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  
  btn4.setOutput(&tb);
  ui.draw(&btn4,/**x=*/(screen.width()/10),/**y=*/(screen.height()/5)*4,/*width*/screen.width()/10*2,/*height*/screen.width()/10*2);

  DFRobot_UI::sButton_t & btn5 = ui.creatButton();
  btn5.bgColor = COLOR_RGB565_BLACK;
  btn5.setText("4");

  //Set the callback function of the button
  btn5.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  
  btn5.setOutput(&tb);
  ui.draw(&btn5,/**x=*/(screen.width()/10)*4,/**y=*/(screen.height()/5)*4,/*width*/screen.width()/10*2,/*height*/screen.width()/10*2);

  DFRobot_UI::sButton_t & btn6 = ui.creatButton();
  btn6.bgColor = COLOR_RGB565_GREEN;
  btn6.setText("GO");

  //Set the callback function of the button
  btn6.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  
  btn6.setOutput(&tb);
  ui.draw(&btn6,/**x=*/(screen.width()/10)*7,/**y=*/(screen.height()/5)*4,/*width*/screen.width()/10*2,/*height*/screen.width()/10*2);

  // DFRobot_UI::sButton_t & btn7 = ui.creatButton();
  // btn7.bgColor = COLOR_RGB565_BLACK;
  // btn7.setText("GO");

  // //Set the callback function of the button
  // btn7.setCallback(btnCallback);
  // //Each button has a text box, its parameter needs to be set by yourself.
  
  // btn7.setOutput(&tb);
  // ui.draw(&btn7,/**x=*/(screen.width()/10)*7,/**y=*/(screen.height()/8)*7,/*width*/screen.width()/10*2,/*height*/screen.width()/10*2);
  
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    Container: <input type="number" name="Container">
    <br>
  <form action="/get">
    Name: <input type="text" name="Name">
    <br>
  <form action="/get">
    Quantity: <input type="number" name="Quantity">
    <br>
  <form action="/get">
    Time To Take: <input type="text" name="TimeToTake">
    <br>
  <form action="/get">    
    Per Dispense: <input type="number" name="PerDispense">
    <br>
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
 
  Serial.println();
}        
                                                                                                                                                                                                                                             
void setup() { 
  Serial.begin(115200);
  ui.begin();
  WiFi.enableSTA(true);

  // Set the UI theme, there are two themes to choose from: CLASSIC and MODERN.
  ui.setTheme(DFRobot_UI::MODERN);
  mode=0;
  pinMode(48,OUTPUT);
  // if (! aht.begin()) {
  //   Serial.println("Could not find AHT? Check wiring");
  //   while (1) delay(10);
  // }
  // Serial.println("AHT10 or AHT20 found");

  //Create a text box control
  DFRobot_UI::sTextBox_t & tb = ui.creatText();
  tb.bgColor = 0xe6B6;
  ui.draw(&tb);
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA); //Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");
  int timeout_counter = 0;

    // while (WiFi.status() != WL_CONNECTED) {
    //     Serial.print(".");
    //     delay(200);
    //     timeout_counter++;
    //     if(timeout_counter >= WL_CONNECTION_LOST*10){
    //       ESP.restart();
    //     }
    // }

    // if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //   Serial.println("WiFi Failed!");
    // return;
    // }

  Serial.println();
  Serial.print("IP Address: ");
  IP = WiFi.localIP().toString();
  tb.setText(IP);
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_Container) && request->hasParam(PARAM_Name)&& request->hasParam(PARAM_Quantity) && request->hasParam(PARAM_TimeToTake) && request->hasParam(PARAM_PerDispense)){ 
      inputMessage = "Information Saved";
    }
    else {
      inputMessage = "No message sent";
    }
  Serial.println(inputMessage);
  
  String temp = (request->getParam(PARAM_Container)->value());
  currContainer = temp.toInt();
  if (currContainer == 1) {
    cont1.medNickName = (request->getParam(PARAM_Name)->value());
    temp = (request->getParam(PARAM_Quantity)->value());
    cont1.numberPills = temp;
    temp = (request->getParam(PARAM_TimeToTake)->value());
    cont1.alarmTime = temp;
    temp = (request->getParam(PARAM_PerDispense)->value());
    cont1.amountTaken = temp;
  }
  else if (currContainer == 2) {
    cont2.medNickName = (request->getParam(PARAM_Name)->value());
    temp = (request->getParam(PARAM_Quantity)->value());
    cont2.numberPills = temp;
    temp = (request->getParam(PARAM_TimeToTake)->value());
    cont2.alarmTime = temp;
    temp = (request->getParam(PARAM_PerDispense)->value());
    cont2.amountTaken = temp;
  }
  else if (currContainer == 3) {
    cont3.medNickName = (request->getParam(PARAM_Name)->value());
    temp = (request->getParam(PARAM_Quantity)->value());
    cont3.numberPills = temp;
    temp = (request->getParam(PARAM_TimeToTake)->value());
    cont3.alarmTime = temp;
    temp = (request->getParam(PARAM_PerDispense)->value());
    cont3.amountTaken = temp;
  }
  else if (currContainer == 4) {
    cont4.medNickName = (request->getParam(PARAM_Name)->value());
    temp = (request->getParam(PARAM_Quantity)->value());
    cont4.numberPills = temp;
    temp = (request->getParam(PARAM_TimeToTake)->value());
    cont4.alarmTime = temp;
    temp = (request->getParam(PARAM_PerDispense)->value());
    cont4.amountTaken = temp;
  }

  request->send(200, "text/html", inputMessage + "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
  
  backupScreen(tb);

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  if (cont1.numberPills.toInt() <= 5) {
    tb.setText("Container low on     pills");
  }
  else if (cont2.numberPills.toInt() <= 5) {
    tb.setText("Container low on     pills");
  }
  else if (cont3.numberPills.toInt() <= 5) {
    tb.setText("Container low on     pills");
  }
  else if (cont4.numberPills.toInt() <= 5) {
    tb.setText("Container low on     pills");
  }
  else {
    tb.setText("");
  }
}

struct tm timeinfo;

void loop()
{

  ui.refresh();
  if(cont1.ready) {
    screen.fillRect(/*x=*/(screen.width()/10), /*y=*/(screen.height()/10)*5 , /*w=*/20, /*h=*/20, /*color=*/COLOR_RGB565_GREEN);
  }
  else {
    screen.fillRect(/*x=*/(screen.width()/10), /*y=*/(screen.height()/10)*5 , /*w=*/20, /*h=*/20, /*color=*/COLOR_RGB565_RED);
  }

  if(cont2.ready) {
    screen.fillRect(/*x=*/(screen.width()/10)*4, /*y=*/(screen.height()/10)*5 , /*w=*/20, /*h=*/20, /*color=*/COLOR_RGB565_GREEN);
  }
  else {
    screen.fillRect(/*x=*/(screen.width()/10)*4, /*y=*/(screen.height()/10)*5 , /*w=*/20, /*h=*/20, /*color=*/COLOR_RGB565_RED);
  }

  if(cont3.ready) {
    screen.fillRect(/*x=*/(screen.width()/10), /*y=*/(screen.height()/10)*8 , /*w=*/20, /*h=*/20, /*color=*/COLOR_RGB565_GREEN);
  }
  else {
    screen.fillRect(/*x=*/(screen.width()/10), /*y=*/(screen.height()/10)*8 , /*w=*/20, /*h=*/20, /*color=*/COLOR_RGB565_RED);
  }

  if(cont4.ready) {
    screen.fillRect(/*x=*/(screen.width()/10)*4, /*y=*/(screen.height()/10)*8 , /*w=*/20, /*h=*/20, /*color=*/COLOR_RGB565_GREEN);
  }
  else {
    screen.fillRect(/*x=*/(screen.width()/10)*4, /*y=*/(screen.height()/10)*8 , /*w=*/20, /*h=*/20, /*color=*/COLOR_RGB565_RED);
  }

  String time = (String)(&timeinfo, "%H:%M");
  //ui.drawString(/*x=*/(screen.width()/10), /*y=*/(screen.height()/2), time, COLOR_RGB565_BLACK,COLOR_RGB565_WHITE,2,0);

  if (time == cont1.alarmTime) {
    cont1.ready = true;
  }
  if (time == cont2.alarmTime) {
    cont2.ready = true;
  }
  if (time == cont3.alarmTime) {
    cont3.ready = true;
  }
  if (time == cont4.alarmTime) {
    cont4.ready = true;
  }



  // switch(ui.getGestures()) {
  //   case DFRobot_Gesture::SCLICK : refresh(); break;
  //   default : break;
  // }
  
  // sensors_event_t humidity, temp;
  // aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  // Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  // Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
  // temp1=temp.temperature;
  // humid1=humidity.relative_humidity; 

  // if (humidity.relative_humidity > 35.00)
  // {
  //   //tb.setText("ERROR: TOO HUMID");
  //   tone(buzzer,3000);
  // } 
  // else if (temp.temperature > 25.00){
  //   tone(buzzer, 3000);
  // }
  // else {
  //   noTone(buzzer);
  // }
}

