**ESP8266MK14keys - A Web server to program the MK14 computer**

A MK14 keys interface using Charlieplexing and a web interface.

**History**\
There has been a number of designs to use a Raspberry Pi or Arduino to “press” the keys on the MK14, making it quick and easy to load programs into it’s limited memory.

see https://www.vintage-radio.net/forum/showthread.php?t=175912

I wanted to use an ESP8266 to operate the keys interface for the MK14, as this has an internal “file”  area to hold the hex files and a web server to allow control from a mobile etc.

**The PCB design**\
The interface as defined needed 13 lines to control the optical relays. Optical relays were used so there was no direct connection made between the controlling device and the MK14. The problem was that the ESP8266 really only has 7 useable lines.

Having looked at Charlieplexing for LEDs it seems it is possible to control the 13 optical relays with 7 lines. The ESP8266 just about has 7 pins that can be used as output. In fact the new design actually uses 16 optical relays. There are 14 that allow it to simulate all of the column and row cross overs.  The other 2 optical relays are used to handle the reset switch.\
Chip U5 is used when you connect flying leads to the reset capacitor on the original MK14 boards.\
Chip U6 is used if using the newer replica Issue VI boards where the reset is present on the edge connector.\
You can populate either U5 or U6, or both if you want to switch between board types. This was simpler than trying to sort out jumpers to switch between the different reset options and my experiments showed it was capable of driving both optical relays.

The PCB is designed to allow either an Arduino nano or a NodeMCU-ESP8266 to control the optical relays.\
The Kicad files are provided so you can make your own version.

Note:- Using Charlieplexing works best if all the LEDs are of the same type so use the same type of optical relay in all sockets.

There are a number of  programs to use with the system.

**MK14ESP8266keys**
* The ESP8266 based web server which can “program” the MK14 using “hex” files loaded into the ESP8266 file store, and it’s all done on the web. See separate document on this program.

A demo of the ESP8266 web pages are at http://www.saturn5.force9.co.uk/mk14program \
These are just to show what the web pages look like - there is no processing behind them.

There is a short video demostrating the web server https://www.youtube.com/watch?v=Oq6K6SKH3e0

**MK14keysCP1**
* A version of MK14keys using the ESP8266 or Arduino on  mk14keyscp PCB.\
The MK14keysCP is like the standard MK14keys software, but using the ESP8266 or Arduino on  mk14keyscp PCB. A Python3 program reads a hex file and sends the “key values” down the serial line. The arduino or ESP8266 software takes “key  values” from the serial port and activates the correct key. 

* **MK14keysCP1.ino** – load this code into the arduino or ESP2866.\
&nbsp;&nbsp;use #define USE_ESP8266_PINS to switch from the Arduino to the ESP2866 pins.\
&nbsp;&nbsp;You can use the Arduino IDE Serial Monitor to send characters to test the setup.\

* **Send14.py** ( in python folder )\
Python3 program to send a Hex file to the serial port\
You will need to change the line in the code to select your serial port\
&nbsp;&nbsp;&nbsp; #SerialPort = 'com4'		# windows style\
&nbsp;&nbsp;&nbsp; #SerialPort = '/dev/ttyUSB0'  # linux style Arduino Nano\
&nbsp;&nbsp;&nbsp; #SerialPort = '/dev/ttyACM0'  #  linux style Arduino UNO\
You will need to change \
&nbsp;&nbsp;&nbsp; MK14_OS = 1 to MK14_OS = 0 \
if using the original MK14 operating system.


**MK14keysCPtest**

*	This sketch tests all the relays but it does need a special test circuit\
 &nbsp;&nbsp;&nbsp; Documentation to follow.





