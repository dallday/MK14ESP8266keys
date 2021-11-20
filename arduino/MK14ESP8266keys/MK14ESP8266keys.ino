// 
// MK14ESP8266keys - A Web server to program the MK14 computer
// 
// It lets you store Intel HEX files containing MK14 code on the webserver
// and then these can be sent tothe MK14 via the MK14keyCP PCB using a ESP2866 
//    for ESP8266 LOLIN v3 used Arduino IDE board "NodeMCU 1.0 (ESP-12E Module)"
//
// It can work in 2 different modes - both handle the web pages
//
//  as a WAP ( Wireless Access Point ) using address 192.168.8.1
//  It switches to the mode if
//      The SSID stored in the EEPROM does not have ending 0x00 byte 
//             so probably not set.
//      It is unable to connect to the SSID stored in the EEPROM
//      Or if you press the reset button twice.
//
//  connected to a WIFI network using the SSID stored in EEPROM
//  it's IP address is set using the DHCP so will depend on how your DHCP is handled.
//  
//  Note:-
//     If you have two LED connected to GPIO0 as detailed below (see STATUSLED ) then
//       during setup the lights will switch on and off with both lit as some points
//       The Red LED will light if it is in WAP mode.
//       The Green LED will light if it is connected to a WIFI network.
//
//
// *************** setup ****************************************
//  The SSID and password as stored in the "EPROM" space
//
//  To setup your SSID and password you need to put the device into WAP mode
//
//  To do this press the reset button twice within 5 seconds ( but not too quickly)
//  connect the serial port up at 9600 to see the debug info. 
//
//  Once in WAP mode connect to the network ESP8266MK14keys - the password is mynetwork
//
//  Open a browser and go to 192.168.8.1/setup 
// 
//   This should display a form where you can enter your SSID and password 
//     press send and it should return a screen saying saved.
//
//   Now reboot the device and it should connect
//  
//   The Red LED will light if it has failed and reverted to WAP mode.
//   The serial port will show details if required.
//
//
//
// see https://diyprojects.io/esp8266-web-server-tutorial-create-html-interface-connected-object/
// for icon stuff
// 
// ESP8266webServer
// https://www.esp8266.com/viewtopic.php?t=2153
//
//  Upload files to webserver - example
// https://tttapa.github.io/ESP8266/Chap12%20-%20Uploading%20to%20Server.html
//
// details of spiffs and LittleFS
// https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html#spiffs-file-system-limitations
// https://circuits4you.com/2018/01/31/example-of-esp8266-flash-file-system-spiffs/
// 


bool debugMode = false;

#define STATUSLED 0   // put it on the GPIO0 - the one that is used to active flash mode during boot.
                     // leave it as input as 2 leds light 
                     // put it high or low to light just one on them.
                     //   5v --- R330 --- GreenLED --- GPIO0 --- RedLED --- R330 --- 0v
                     //
                     // careful as if it is showing RED and the programming button is a direct connection to Ground you might get a short :(
                     // on the ESP8266 LOLIN v3 there seems to be a 200R resistor between GPIO0 and the programming button
                     //

// uncomment this line to get the webserver debug info on the serial port
// #define DEBUG_ESP_HTTP_SERVER

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <StreamLib.h>

#include <ESP8266HTTPClient.h> 

// this one allows you to access the system by name !!!
#include <ESP8266mDNS.h>
// include this as using eeprom for SSID and password ( for now ! )
#include <EEPROM.h>

#include <FS.h>   // Include the SPIFFS library 
                  // This allows us to use the SPIFFS file system

#define versionNumber 3
// update - version 2 - first release onto github
// update - version 3 
//  combines all normal <head> entries to use routine
//  

byte WAPMode=0; // set to 1 if in WAP mode

// Double Reset Detect - library by Jens-Christian Skibakk
// stores a unsigned long in the Real Time Clock Memory - which survives a reset but not power off
#include <DoubleResetDetect.h>
// maximum number of seconds between resets that
// counts as a double reset 
#define DRD_TIMEOUT 2.0
// address to the block in the RTC user memory
// change it if it collides with another usage 
// of the address block
#define DRD_ADDRESS 0x00
// call the Double Reset code to check 
DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

// end of Double Reset Detect setup 

#define doGetInterval 60000   // do something about every  60 seconds
// global variables 

unsigned long previousMillis; // previous number of milli seconds for doGetInterval


#include "wifidata.h"  // include the wifi data 

/*
 * globals used in the form setup
 * 
 */
String ssid;  // to hold ssid during during wap mode
String password; // to hold password during wap mode
String ssid_Arg;// Error message for ssid input
String password_Arg;// Error message for password input
String webpage;  // to build webpage into
String errorMessage; // show error message when processing file


// set to 1 when sending file to the MK14
char processingProgram=0;
// holds the filename being sent to the MK14 
String processingFilename;


// web server on port 80
ESP8266WebServer webServer(80);


void setup() {
  // 
  // setup routine
  //  opens the Serial port for debugging
  //  activates the EEPROM area 
  //  builds the wireless connection
  //  starts the SPIFFS file system
  //  sets up the webpage handlers and starts the webserver.
  // 
  EEPROM.begin(512);  // needed for reading eprom sets "emulated" size
  
  Serial.begin(115200);
  delay(100); // delay 100ms to sensure serial working 
  
  pinMode(STATUSLED, INPUT); // input mode should means both LEDS lit

  Serial.println();
  Serial.print("MK14ESP8266keys version ");
  Serial.println(versionNumber);

  // set the pins for the MK14 programming side
  mk14Setup();

// Double Reset Detect 
// checks if reset switch was pressed again withing 2 seconds
// if so switch to WAP mode 

  if (drd.detect())
  {
      Serial.println("** Double reset boot **");
      // connected to saved WIFI address
      // activate WAP so we can set other address
      // see Wifidata.h for details 
      WAP();
  }
  else
  {
      Serial.println("** Normal boot **");
      IOT();
  }

  SPIFFS.begin();  // Start the SPI Flash Files System
  
  // define pages to be handled by the webserver
  // handle the main page 
  webServer.on("/", sendMainPage);
  // the page that handles the request to process a file
  // could be delete or program 
  webServer.on("/program",HTTP_GET,programMK14);
  webServer.on("/processing",HTTP_GET,sentprocessingMK14send);

  // handle a request to upload by sending the form to fill in
  webServer.on("/upload", HTTP_GET, sendUploadform);
  // handles the upload form returning 
  // handleFileUpload is called first to actually store the file
  // handleFileUploadComment is called second to handle to rest of the form stuff.
  webServer.on("/uploadfile", HTTP_POST,                       // if the client posts to the upload page
    handleFileUploadComment ,                          // Handles the web page post stuff after file has loaded
    handleFileUpload                                    // Receive and save the file
  );

  // handle a request to list all the files on the SPIFFS file system
  webServer.on("/dir",HTTP_GET,handleFileList);

  // handle a request to format the SPIFFS file system
  webServer.on("/format",HTTP_GET,sendAreYouSureFormat);
  webServer.on("/formatyes",HTTP_GET,clearSPIFFS);

  // handle rquest to recover files
  webServer.on("/recoverfiles",HTTP_GET,sendListofDeletedFiles);
  // handle the recover page response
  webServer.on("/restore",HTTP_GET,restoreFile);
  
  // handle request for the keystrokes form - dispalys keypad
  webServer.on("/keystrokes",HTTP_GET, sendKeyStrokes);
  // handle characters sent from the keystrokes form
  webServer.on("/sendchars",HTTP_GET,sendSendChars);

  // returns 0 or 1 depending upon the jumper setting on the PCB
  // part of the AJAX processing
  webServer.on("/getosver",HTTP_GET,sendosver);

  // send the favicon - see zimage.ino file currently a beer glass
  webServer.on("/favicon.ico",HTTP_GET,sendFavIcon);

  // handle a request to set the SSID
  webServer.on("/setssid", HTTP_GET,sendSSIDformstart);
  // handle the SSID form and store the SSID and password.
  webServer.on("/submitssid", HTTP_POST, processSSIDform);

  
  // If the client requests any URI not covered by the ones above 
  // check if the file exists and send it or a 404 response
  // Note = the handleFileRead can limit the typw of files to the ones we want to load :)
  webServer.onNotFound([]() {                              
    if (!handleFileRead(webServer.uri()))                     // send it if it exists
      webServer.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  // example of using inload proc
  //  webServer.on("/inline", []() {
  //   webServer.send(200, "text/plain", "this works as well");
  // });
 
  // start the webserver on port 80
  webServer.begin();
  Serial.println("HTTP Server started");
  
  // Print the current server IP address - so we know it ?
  // will be printed every x seconds from the main loop
  printIPaddress();
 

}


void loop() {
  //
  // this is the loop called to handle the processing
  // If connectted to a wireless access point it checks if we are still connected
  //    and if not tries to reconnect.
  // handles any web service requests
  // Every x seconds it outputs the current IP address - for debug purposes.
  int i;
  int wifistatus;

   if (WAPMode==0){
    // check if wifi still connected
    wifistatus = WiFi.status();
    if (wifistatus != WL_CONNECTED){
      Serial.print("Wifi lost status :");
      Serial.println(wifistatus);
      displayWIFIStatus(wifistatus);
      // try and reconnect
      IOT();
    }
   }
  
  // check to see if there are any requests to process
  webServer.handleClient();

  // check to see if it's time to do something; that is, 
  // if the difference between the current time and 
  // last time we did something.
  
  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis <= doGetInterval) {
    return;
  }
  // save the last time you did something
  previousMillis = currentMillis;   
  //
  // do something
  //
  printIPaddress();
 

}

void printIPaddress(){
  //
  // Print the IP address
  // 
  //Serial.print("Wap mode in hex ");
  //Serial.println(WAPMode,HEX);
  if (WAPMode==1){
    Serial.print ("WAP SSID ");
    Serial.print (ssid2);
    Serial.print (", password ");
    Serial.print (password2);
    Serial.print(", IP address ");
    Serial.println(WiFi.softAPIP());
  }
  else {
    Serial.print("IP address ");
    Serial.println(WiFi.localIP());
  }

}


void printHex(Print &DestinationPrint, byte data1){
  // 
  // prints a hex value with leading 0s
    DestinationPrint.print(data1>>4, HEX);
    DestinationPrint.print(data1&0x0F, HEX);
}


// end of code
