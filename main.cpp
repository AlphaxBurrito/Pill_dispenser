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

const char* ssid = "Erics93";
const char* password = "heejae9936";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;

#define TFT_MISO 13 // (leave TFT SDO disconnected if other SPI devices share MISO)
#define TFT_MOSI 11
#define TFT_SCLK 12

#define TFT_CS    10  // Chip select control pin
#define TFT_DC    15  // Data Command control pin
#define TFT_RST   6  // Reset pin (could connect to RST pin)
#define TFT_BL    7
#define TFT_SCL   18
#define TFT_SDA   17
#define TFT_INT   16

const int buzzer = 47;

float temp1;
float humid1;
int mode;

class pillContainer {       // The class
  public:             // Access specifier
    int amountTaken;            // Amount taken per each dispense cycle
    int timesTaken;             // Amount of times a dispense cycle should
    int numberPills;            // Amount of pills stored in the container
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
void storeInfo();
void newMedication();
void containerSelection();
void homeScreen();
void clearScreen();
void refresh();
void btnCallback(DFRobot_UI::sButton_t &btn,DFRobot_UI::sTextBox_t &obj);

/*
  FUNCTION DECLARATIONS
*/

void storeInfo(){
  pillContainer cont1;
  cont1.alarmTime = "12:00";
  cont1.amountTaken = 1;
  cont1.medNickName = "Tasty Drugs";
}

void newMedication() {
  clearScreen();
  containerSelection();
  return;
}

void home() {
  clearScreen();
  homeScreen();
  return;
}

void clearScreen() {
  ui.endInput();
  screen.fillScreen(0xFFFF);
  return;
}

void containerSelection() {
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
  ui.draw(&btn1,/**x=*/screen.width()/10,/**y=*/screen.height()/2,/*width*/screen.width()/10*2,/*height*/screen.width()/10*2);
  
  DFRobot_UI::sButton_t & btn2 = ui.creatButton();
  btn2.setText("NEW");
  btn2.bgColor = COLOR_RGB565_GREEN;
  btn2.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  btn2.setOutput(&tb);
  ui.draw(&btn2,/**x=*/(screen.width()/10)*4,/**y=*/screen.height()/2,/*width*/screen.width()/10*2,/*height*/screen.width()/10*2);
 
  DFRobot_UI::sButton_t & btn3 = ui.creatButton();
  btn3.bgColor = COLOR_RGB565_BLUE;
  btn3.setText("clr");

  //Set the callback function of the button
  btn3.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  
  btn3.setOutput(&tb);
  ui.draw(&btn3,/**x=*/(screen.width()/10)*7,/**y=*/screen.height()/2,/*width*/screen.width()/10*2,/*height*/screen.width()/10*2);


  // if (temp1 > 24.00)
  // {
  //   tb.setText("ERROR: TOO HOT");
  //   tone(buzzer,3000);
  // } 
  // else if (humid1 > 26.00)
  // {
  //   tb.setText("ERROR: TOO HUMID");
  //   tone(buzzer,3000);
  // }
  // else {
  //   tb.setText("");
  //   noTone(buzzer);
  //}
}

void refresh() {
  //noTone(buzzer);
  switch(mode){
    case 0: home(); break;
    case 1: newMedication();  break;
  }
  ui.refresh();
  return;
}

void btnCallback(DFRobot_UI::sButton_t &btn,DFRobot_UI::sTextBox_t &obj) {
   String text((char *)btn.text);
   if(text == "ON"){
    mode = 1;
    }
   else if(text == "OFF"){
    obj.setText("you have touch button off");
    }
   else if(text == "clr"){
    mode = 0;
   }
   else if(text == "1"){
    cont1.ready = true;
    cont1.alarmTime = "12:00";
    cont1.amountTaken = 1;
    cont1.medNickName = "Fluoxetine";
    cont1.numberPills = 30;
    cont1.timesTaken = 1;

    String textBox = "Alarm:" + cont1.alarmTime + "        Pill:" + cont1.medNickName + "\n   Pills Remaining:" + cont1.numberPills;
    obj.setText(textBox);
   }
  else if(text == "2"){
    cont2.ready = true;
    cont2.alarmTime = "10:00";
    cont2.amountTaken = 1;
    cont2.medNickName = "Escitalopram";
    cont2.numberPills = 30;
    cont2.timesTaken = 1;

    String textBox = "Alarm:" + cont2.alarmTime + "        Pill:" + cont2.medNickName + "\n Pills Remaining:" + cont2.numberPills;
    obj.setText(textBox);
   }
   else if(text == "GO"){
    cont1.ready = false;
    cont2.ready = false;
    cont3.ready = false;
    cont4.ready = false;

    obj.setText("");
   }
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

    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(200);
        timeout_counter++;
        if(timeout_counter >= WL_CONNECTION_LOST*10){
        ESP.restart();
        }
    }

  Serial.println();
  Serial.print("IP Address: ");
  String IP = WiFi.localIP().toString();
  tb.setText(IP);
  Serial.println(WiFi.localIP());


  backupScreen(tb);

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

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
