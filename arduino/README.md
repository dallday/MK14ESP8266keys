**The MK14ESP8266keys sketch**

This folder contains the arduino IDE sketch to make use of the MK14keysCP PCB board and provide a webserver to load files onto an attached MK14.

The sketch is broken down into a number of files to make it easier to understand and modify.

It uses a number of libraries that you may need to download to get it to work.

* <ESP8266WiFi.h>       provides the wifi access
* <ESP8266WebServer.h>  provides the actual webserver wrapper
* <ESP8266HTTPClient.h> provides functions to handle data from browsers
* <ESP8266mDNS.h>       this one allows you to access the system by name !!! 
* <EEPROM.h>            provides access the EEPROM to store SSID and password
* <FS.h>                provides access to the SPIFFS file system
* <DoubleResetDetect.h> provides the code to check for double resets

Don't think I've missed any but let me know if I have.

For my ESP8266 development board (ESP8266 LOLIN v3) I selected the Arduino IDE board "NodeMCU 1.0 (ESP-12E Module)" from the tools->board menu.

Hope this is of some use. \
Enjoy and stay safe 

David
