/******************************************************************************
Google Earth KML GPS Position Logger v1.7
brandon.williams@sparkfun.com
May 6, 2019

The user will press a momentary button to log a GPS location into a KML file, 
a file that's stored on a microSD card in the microSD shield. If the user holds 
the button for 5 seconds, the program will effectively "end" with an infinite 
while loop after safely closing the file. The user can then remove the memory 
card to retrive the file and open using Google Earth.

** Significant changes and improvements can be made, please enjoy mod-ing! **

Resources:
SFE MicroOLED library: SFE_MicroOLED.h
SFE u-blox GNSS library: //http://librarymanager/All#SparkFun_u-blox_GNSS 
Arduino SD required libraries: SPI.h & SD.h

Download Google Earth: https://www.google.com/earth/versions/

Development environment specifics:
Arduino IDE 1.8.9
Board Definition Packages:
  Arduino SAMD board Boards (32-bits ARM Cortex-M0+) 1.6.21
  SFE SAMD Boards 1.6.1
******************************************************************************/
//SD Shield libraries
#include <SPI.h>
#include <SD.h>
//OLED and Ublox libraries
#include <Wire.h>
#include <SFE_MicroOLED.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS

#define PIN_RESET 9   //OLED
#define DC_JUMPER 1   //OLED

//create objects
SFE_UBLOX_GNSS myGNSS; 
MicroOLED oled(PIN_RESET, DC_JUMPER);
File dataFile;

//declare golbal variables
const int buttonPin =  2;
const int chipSelect = 8; //Specific for SFE microSD shield, differs from Arduino SD libraries
int buttonState = 0;

void setup() {

  Wire.begin();

  //Classic SFE flame
  oled.begin();
  oled.clear(ALL);
  oled.display();
  delay(500);
  oled.clear(PAGE);
  oled.display();
  oled.setFontType(0);
  oled.setCursor(0,0);
  
  pinMode(buttonPin, INPUT_PULLUP);

  pinMode(chipSelect,OUTPUT);
  
  if(!SD.begin(chipSelect)){
    //If the SD card can't be initiallized/found just freeze with a loop
    oled.setCursor(0,0);
    oled.clear(PAGE);
    oled.print("SD, no work");
    oled.display();
    while(1);
  }
  oled.setCursor(0,0);
  oled.clear(PAGE);
      //Oh yea! don't forget the GPS shield needs to get it's first fix
  oled.print("Revving up the GPS unit, please wait");
  oled.display();
  delay(750);
  while(myGNSS.getLatitude() == 0){
    oled.clear(PAGE);
    oled.setCursor(0,0);
    oled.print("Looking for first fix, please wait");
    oled.display();
    delay(1000);
  }
  oled.setCursor(0,0);
  oled.clear(PAGE);
  oled.print("Ready to start!");
  oled.display();
  oled.clear(PAGE);
  oled.display();
  
}

void loop() {
  double latitude;
  double longitude;

  // A little redundant, but simple fix to not add on to the file 
  SD.remove("sparkGPS.kml");

  dataFile = SD.open("sparkGPS.kml", FILE_WRITE);
  
  if (dataFile){
    //Write opening tags to file
    dataFile.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    dataFile.println("<kml xmlns=\"http://www.opengis.net/kml/2.2\">");
    dataFile.println("<Document>");
    int state = 0;
    myGNSS.begin();
    /* "Continous" (not quite) loop will run until user performs kill action.
     *  
     *  1. Button is pushed and released, then one coordinate point will add to the file
     *  
     *  2. If the button is pushed and held, one last coordinate is saved. Hold button till
     *    "Goodbye" appears to kill the loop. File is closed and it is safe to remove power
     *     and remove the SD card.
     */
    while(state < 4){
      buttonState = digitalRead(buttonPin);
      state = 0;
      if(buttonState == LOW){
        oled.setCursor(0,0);
        oled.clear(PAGE);
        oled.print("button pressed");
        oled.display();
        delay(1000);
        oled.clear(PAGE);
        oled.display();
        
        state = 1;
        delay(900);
        if(buttonState == LOW){

          oled.setCursor(0,0);
          oled.clear(PAGE);
          oled.print("Getting Coordinates");
          oled.display();
          delay(1000);
          oled.clear(PAGE);
          oled.display();
        
          float latitude = myGNSS.getLatitude();
          latitude = latitude / 10000000;
          
          float longitude = myGNSS.getLongitude();
          longitude = longitude / 10000000;

          
          dataFile.println("\t<Placemark>");
          dataFile.println("\t\t<name>SFE GPS Extravaganza</name>");
          dataFile.println("\t\t<description>Where am I?</description>");
          dataFile.println("\t\t<Point>");
          dataFile.print("\t\t\t<coordinates>");
          dataFile.print(longitude,6);
          dataFile.print(",");
          dataFile.print(latitude,6);
          dataFile.print(",0");
          dataFile.println("</coordinates>");
          dataFile.println("\t\t</Point>");
          dataFile.println("\t</Placemark>");
          //Visual coordinates for the user to see
          oled.clear(PAGE);
          oled.setCursor(0,0);
          oled.print("Lat:");
          oled.print(latitude,6);
          oled.print("\nLong:");
          oled.print(longitude,6);
          oled.display();
          delay(1500);
          oled.clear(PAGE);
          oled.display();
        }
        buttonState = digitalRead(buttonPin);
        if(buttonState == LOW && state == 1){
          state = 2;
          delay(4000);
          buttonState = digitalRead(buttonPin);
          if(buttonState == LOW && state == 2){
            dataFile.println("</Document>");
            dataFile.println("</kml>");
            dataFile.close();
            oled.setCursor(0,0);
            oled.print("Goodbye");
            oled.display();
            delay(1000);
            oled.clear(PAGE);
            oled.display();
            //it is safe to remove power and remove data
            while(1); //"Ends program" more or less
          }
        
        }
        
      }
    }
  }
}
