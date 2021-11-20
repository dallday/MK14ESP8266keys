/*
 * hold some constansts used to create webpages etc.,
 * 
 * 
 */


// prevents the constants being defined twice 
#ifndef _WIFI_STRINGS_H_
#define _WIFI_STRINGS_H_

/*The WAP() function configures and calls all functions and
 * variables needed for creating a WiFi Access Poit
 *
 *seems password can be blank but not too short :) think it must be 8 characters 
 *
 */
 
const char *ssid2 = "ESP8266MK14keys";
const char *password2 = "mynetwork";

IPAddress wap_ip(192,168,8,1);
IPAddress wap_gateway(192,168,8,1);
IPAddress subnet(255,255,255,0);

#endif   // end of main ifndef 

// end of code
