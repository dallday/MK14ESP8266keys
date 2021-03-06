**The MK14ESP8266keys sketch**

This folder contains the arduino IDE sketch to make use of the MK14keysCP PCB board and provide a webserver to load files onto an attached MK14.

The webserver can connect to a wifi network if a SSID and password has been previously given to it.\
Else it will create it's own WAP using ip address 192.168.8.1\
The WAP has an SSID of **ESP8266MK14keys** and the password is **mynetwork**.\
You can connect to that WAP and use it as is or you can supply it with a new SSID and password to connect to.\
You need to reboot to get it to use the new SSID.

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

**ISSUES**

* It is supposed to respond to mk14keys.local by using the MDNS from ESP8266mDNS, but I've not managed to get that to work as yet.

* It comes up very small on a mobile device and I've yet to work out the best way to make it look better on a mobile device.

It sends a lot of debug info out to the serial port at 115200 some of which may be useful if it does not work.

Hope this is of some use. \
Enjoy and stay safe 

David
