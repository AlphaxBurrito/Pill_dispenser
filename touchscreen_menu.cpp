/*!
 * @file gettureFont.ino
 * @brief Control the words in the center of the screen by zooming in, zooming out, sliding up, and sliding down in a two-finger gesture.
 * @n The demo supports Arduino Uno, Mega2560, FireBeetle-ESP32, FireBeetle-ESP8266, FireBeetle-M0
 *
 * @copyright Copyright (c) 2010 DFRobot Co. Ltd (http://www.dfrobot.com)
 * @licence The MIT License (MIT)
 * @author [fengli] (li.feng@dfrobot.com)
 * @version V1.0
 * @date 2019-12-6
 * @get from https://www.dfrobot.com
 * @url https://github.com/DFRobot/DFRobot_GDL/src/DFRpbot_UI
*/
#include "wire.h"
#include "DFRobot_UI.h"
#include "Arduino.h"
#include "DFRobot_GDL.h"
#include "DFRobot_Touch.h"

#include <SD.h>


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
void newMedication();
void containerSelection();
void homeScreen();
void btnCallback(DFRobot_UI::sButton_t &btn,DFRobot_UI::sTextBox_t &obj);

/*
  FUNCTION DECLARATIONS
*/
void newMedication() {
  containerSelection();
}

// New Medication information input screen 
void containerSelection(){
/*
    First button, btn1, for container 1
*/ 
  DFRobot_UI::sTextBox_t & tb = ui.creatText();
  tb.bgColor = 0xe6B6;
  ui.draw(&tb);
  tb.setText("Which container are you using?");

/*
    First button, btn1, for container 1
*/
  DFRobot_UI::sButton_t & btn1 = ui.creatButton();
  //Set the name of the button
  btn1.setText("CONTAINER 1");
  btn1.bgColor = COLOR_RGB565_RED;
  btn1.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  btn1.setOutput(&tb);
  ui.draw(&btn1,/**x=*/screen.width(),/**y=*/screen.height()/10,/*width*/screen.width()/3*2,/*height*/screen.width()/10*2);

 /*
    Second button, btn2, for container 2
 */ 
  DFRobot_UI::sButton_t & btn2 = ui.creatButton();
  btn2.setText("CONTAINER 2");
  btn2.bgColor = COLOR_RGB565_GREEN;
  btn2.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  btn2.setOutput(&tb);
  ui.draw(&btn2,/**x=*/screen.width(),/**y=*/screen.height()/10*2,/*width*/screen.width()/3*2,/*height*/screen.width()/10*2);


/*
    Third button, btn3, for container 3
 */ 
  DFRobot_UI::sButton_t & btn3 = ui.creatButton();
  btn3.setText("CONTAINER 3");
  btn3.bgColor = COLOR_RGB565_BLUE;
  btn3.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  btn3.setOutput(&tb);
  ui.draw(&btn3,/**x=*/screen.width(),/**y=*/screen.height()/10*3,/*width*/screen.width()/3*2,/*height*/screen.width()/10*2);

  /*
    Fourth button, btn4, for container 4
  */ 
  DFRobot_UI::sButton_t & btn4 = ui.creatButton();
  btn4.setText("CONTAINER 4");
  btn4.bgColor = COLOR_RGB565_CYAN;
  btn4.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  btn4.setOutput(&tb);
  ui.draw(&btn4,/**x=*/screen.width(),/**y=*/screen.height()/10*4,/*width*/screen.width()/3*2,/*height*/screen.width()/10*2);
}

//Callback function for three buttons
void btnCallback(DFRobot_UI::sButton_t &btn,DFRobot_UI::sTextBox_t &obj) {
   String text((char *)btn.text);
   if(text == "NEW MEDICATION"){
      newMedication();
    }
   else if(text == "CHANGE ALARM TIME"){
    obj.setText("you have touch button off");
    }
   else if(text == "CHANGE AMOUNT TO DISPENSE"){
    obj.deleteChar();
    }
   else{
    homeScreen();
   }
    
}

void homeScreen(){
  /*
    First button, btn1, for user to input a medication not currently in the device
 */ 
  DFRobot_UI::sTextBox_t & tb = ui.creatText();
  tb.bgColor = 0xe6B6;
  ui.draw(&tb);
  //Create a button control on the screen
  DFRobot_UI::sButton_t & btn1 = ui.creatButton();
  //Set the name of the button
  btn1.setText("NEW MEDICATION");
  btn1.bgColor = COLOR_RGB565_RED;
  btn1.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  btn1.setOutput(&tb);
  ui.draw(&btn1,/**x=*/screen.width()/10,/**y=*/screen.height()/5*2,/*width*/screen.width()/3*2,/*height*/screen.width()/10*2);

 /*
    Second button, btn2, for user to input a medication not currently in the device
 */ 
  DFRobot_UI::sButton_t & btn2 = ui.creatButton();
  btn2.setText("CHANGE ALARM TIME");
  btn2.bgColor = COLOR_RGB565_GREEN;
  btn2.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  btn2.setOutput(&tb);
  ui.draw(&btn2,/**x=*/screen.width()/10,/**y=*/screen.height()/5*3,/*width*/screen.width()/3*2,/*height*/screen.width()/10*2);


/*
    Third button, btn3, for user to input a medication not currently in the device
 */ 
  DFRobot_UI::sButton_t & btn3 = ui.creatButton();
  btn3.setText("CHANGE AMOUNT TO DISPENSE");
  btn3.bgColor = COLOR_RGB565_BLUE;
  btn3.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  btn3.setOutput(&tb);
  ui.draw(&btn3,/**x=*/screen.width()/10,/**y=*/screen.height()/5*4,/*width*/screen.width()/3*2,/*height*/screen.width()/10*2);
}

void setup()
{ 
  Serial.begin(115200);
  ui.begin();

  // Set the UI theme, there are two themes to choose from: CLASSIC and MODERN.
  ui.setTheme(DFRobot_UI::MODERN);

  homeScreen();
}

void loop()
{

}
