/* 
 *   ***********************************************************************
 *   
 *  contains the code needed to setup wifi as a server or as an Access Point
 *  
 *   David Allday November 2021
 *   
 */

/*The WAP() function configures and calls all functions and
 * variables needed for creating a WiFi Access Poit
 *
 */

void WAP(){
  // 
  // creates a WAP  (Wireless Access Point)
  // You can connect to this to set the SSID or just use it.
  //
  delay(1000);
  Serial.println("WAP mode set to 1");
  WAPMode=1;    // tell the rest of the code we are using WAP
  if (!WiFi.softAPConfig(wap_ip, wap_gateway, subnet)){
    Serial.println("failure in softAPConfig");
  }
  Serial.print("Setting soft-AP ... ");
  if (!WiFi.softAP(ssid2, password2)){
    Serial.println("failure in softAP");
  }
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

        int wifistatus = WiFi.status();
        Serial.print("Wifi Status:");
        Serial.println(wifistatus);
        displayWIFIStatus(wifistatus);

  
  // turn on one of the LEDs
  digitalWrite(STATUSLED, HIGH);
  pinMode(STATUSLED, OUTPUT); 
  
}

/*The IOT() function configures and calls all functions and variables 
 * needed for configuring an IOT device
 * TODO - add time delay if not connected sometimes 
 */

void IOT(){
    // 
    // Connects to a previously setup SSID
    //   
    //reading the ssid and password out of memory
    // stored in "EPROM"
  
    String string_Ssid="";
    String string_Password="";
    int wifistatus=0;
    string_Ssid= read_string(30,0); 
    string_Password= read_string(30,100); 

    Serial.println("WAP mode set to 0");
    WAPMode=0; // tell the rest of the code we are using an existing SSID

    if (string_Ssid.length() == 0 ){
      // no data stored for SSID
      Serial.println("No stores SSID - switching to WAP");
      // setup a wap 
      WAP();
      return;
    }
    
    // turn on both LEDs
    pinMode(STATUSLED, INPUT); // input mode means both LEDS lit

    int connectattempts=2; // number of times to try connecting
    unsigned long prevMillis = millis();
    unsigned long currMillis = millis();
    unsigned long checkInterval= 2000;   // do something about every 2 second

    do { 
      
      Serial.println("ssid: '"+string_Ssid+"'");
      Serial.println("Password:'"+string_Password+"'");
      //configuring conection parameters, and connecting to the WiFi router
      if (WiFi.begin(string_Ssid, string_Password)){
        Serial.println("Failure in WIFI.Begin - switching to WAP");
        WAP();
        return;
      }
      int onoff=0; // used to pulse the LED
      int waitxchecks=20;      // number of times to check status before giving up

      wifistatus=7; // start as disconnected
      while (wifistatus != WL_CONNECTED) {
        delay (200);
        // flash the led
        if(onoff==0){
          // turn on one of the LEDs
          digitalWrite(STATUSLED, LOW);
          pinMode(STATUSLED, OUTPUT); // output mode means both LEDS lit
          onoff=1;
        }else{
          // turn on one of the LEDs
          digitalWrite(STATUSLED, HIGH);
          pinMode(STATUSLED, OUTPUT); // output mode means both LEDS lit
          //pinMode(STATUSLED, INPUT); // input mode means both LEDS lit
         onoff=0;
        }
        currMillis = millis();
        if(currMillis - prevMillis > checkInterval) {
          // save the last time you did the check
          prevMillis = currMillis;   
          pinMode(STATUSLED, INPUT); // input mode means both LEDS lit
          // now check wifi status
          wifistatus = WiFi.status();
          Serial.print("Wifi Status:");
          Serial.println(wifistatus);
          displayWIFIStatus(wifistatus);
          if (wifistatus == WL_CONNECTED){
            Serial.println("Connected- break out");
            break;
          }
          if (wifistatus == WL_IDLE_STATUS || wifistatus == WL_DISCONNECTED ){
            // waiting for a connection
            Serial.print(".");
          }
          else{
            Serial.println("unknow status - break out");
            break; // restart connection attempt
          }
          if ((waitxchecks--)<1){
            Serial.println("waited long enough - break out");
            break; // restart the connection attempt
          }
        }        
      }

      Serial.println("back in top loop");
      Serial.print("returned Wifi Status:");
      Serial.println(wifistatus);
      displayWIFIStatus(wifistatus);

      if (wifistatus == WL_CONNECTED){
        break;
      }
      
      // failed to connect that time 
      // what to do next
      if ((connectattempts--)<1){
        // no connection - switch to WAP mode
        // turn on both LEDs
        pinMode(STATUSLED, INPUT); // input mode means both LEDS lit
        Serial.println("connection failed doing WAP");
        WAP();
        return;
      }

      Serial.println("Trying again");

    } while (1); // will exit on break or return 
  
    Serial.println("Connected");
    // Print the IP address
    Serial.println(WiFi.localIP());
    // turn on one of the LEDs
    digitalWrite(STATUSLED, LOW);
    pinMode(STATUSLED, OUTPUT); // input mode means both LEDS lit
  
}

/* reads a string from the "eprom" memory 
 *  There are 512 bytes of "eprom" memory 
 *    
 *  reads until end marker 0x0 found or reaches number of bytes
 */

String read_string(int numberOfBytes, int pos){
  String temp;
  char readvalue;
  for (int n = pos; n < numberOfBytes+pos; ++n){
    readvalue=EEPROM.read(n);

/*    Serial.print("reading at ");
    Serial.print(n);
    Serial.print("=");
    Serial.println(readvalue,HEX);
    */
     if(readvalue==0x0){
        // found end of string return the data
        return temp;
     }
     else {
        // add the value to the end of the string
        temp += String(readvalue);
     }
  }
  // if we fall off the end assume there was no valid data stored in the EEPROM
  temp = "";
  return temp;
}

/* Write 2 strings to memory
   
*/
void write_to_Memory(String ssid, String password) {
  write_EEPROM(ssid, 0);
  write_EEPROM(password, 100);
  EEPROM.commit();
}
/*
 * write a string to EPROM memory
 * 
 * This writes each character including the terminating 0 value
 *  Note you only have 512 bytes of eprom memory
 *  
*/
void write_EEPROM(String stringdata, int pos) {
  for (int n = pos; n < stringdata.length()+pos+1; n++) {
    EEPROM.write(n, stringdata[n - pos]);
/*
    Serial.print("writing at ");
    Serial.print(n);
    Serial.print("=");
    Serial.println(stringdata[n - pos],HEX);
    */
  }
}

void displayWIFIStatus(int status1){
  // 
  // displays the current wifi status in words
  //
    Serial.print("status ");
    Serial.print(status1);
    Serial.print("=");
    switch (status1){
    case WL_CONNECTED: 
          Serial.print("connected");
          break;
    case WL_NO_SHIELD: 
          Serial.print("no WiFi shield");
          break;
    case WL_IDLE_STATUS: 
          Serial.print("connecting");
          break;
    case WL_NO_SSID_AVAIL: 
          Serial.print("SSID not found");
          break;
    case WL_SCAN_COMPLETED: 
          Serial.print("scan completed");
          break;
    case WL_CONNECT_FAILED: 
          Serial.print("connection failed");
          break;
    case WL_CONNECTION_LOST: 
          Serial.print("connection lost");
          break;
    case WL_DISCONNECTED: 
          Serial.print("disconnected"); 
          break;
    default: 
          Serial.print("unknown");
          break;
    }
    Serial.println();
}


void flash(int ledPin, int duration)
{
  // turn it on - then off 
  digitalWrite(ledPin, LOW);
  delay(duration);
  digitalWrite(ledPin, HIGH);
  delay(duration);
}


int gethexvalue(char chardata){
  //
  // convert a hex character value into binary
  //
  if (chardata>='0' && chardata <='9'){
    return ((int)chardata-'0');
  }
  else if (chardata>='a' && chardata <='f'){
    return ((int)chardata-'a'+10);
  }
  else if (chardata>='A' && chardata <='F'){
    return ((int)chardata-'A'+10);
  }
  return 0; // not sure what to do if error
}


// ********************************************************************
//  HTML pages for the SSID stuff
// ********************************************************************

void processSSIDform(){
  //
  // handle the returning SSID Form
  //
  String ssid="";
  String password="";
  String request;

  // it is submit of SSID form hopefully
//  auto httpCode = http.POST(postData); 
//  Serial.println(httpCode); //Print HTTP return code 
//  String payload = http.getString(); 
//  Serial.println(payload); //Print request response payload 
//  http.end(); //Close connection Serial.println(); 
  
  // we have the line of data
  ssid=webServer.arg("ssid");
  password=webServer.arg("Password");

  
  Serial.print("ssid '");
  Serial.print(ssid);
  Serial.print("' password '");
  Serial.print(password);
  Serial.println("'");
  Serial.print("length");
  Serial.println(ssid.length());
  
  if (ssid.length() == 0) {
    ssid_Arg=F("please provide SSID");
    sendSSIDform(ssid,password);
  }
  else
  {
    // save it
    write_to_Memory(ssid,password);
    sendSSIDsaved(ssid,password);
  }  

}


void setupHTMLSSIDheader(){
  //
  // setup the SSID html header details
  //
      webpage="";
      webpage+=(F("<!DOCTYPE HTML>"));
      webpage+=(F("<html>"));
      webpage+=(F("<head><title>Set SSID"));
      webpage+=(F("</title>"));
      webpage+=(F("<style>"));
      webpage+=(F("table, th, td { border: 1px solid black;} table.center {margin-left: auto; margin-right: auto;}"));
      webpage+=(F("td {padding: 15px;text-align: center;}"));
      webpage+=(F("</style></head"));
      webpage+=F("\n");

}


void sendSSIDformstart(){

  sendSSIDform("","");

}

/*
 * sends the ssid and password request form to the client 
 */
void sendSSIDform(String ssidr,String passwordr){
  // 
  // send a page to request the SSID and Password
  //
    webpage="";
    setupHTMLSSIDheader();
    webpage+=("<body>");
    webpage+=("<h1>Set SSID</h1>");
    webpage+=("<FORM action=\"/submitssid\" enctype=\"application/x-www-form-urlencoded\" method=\"post\">");
    webpage+=("<P>");
    webpage+=ssid_Arg;
    webpage+=("<br><label>ssid:&nbsp;</label>");
    webpage+=("<input size=\"30\" maxlength=\"30\" value=\"");
    webpage+=ssidr;
    webpage+=("\" name=\"ssid\">");
    webpage+=("<br>");
    webpage+=password_Arg;
    webpage+=("<br><label>Password:&nbsp;</label><input size=\"30\" maxlength=\"30\"  value=\"");
    webpage+=passwordr;
    webpage+=("\"name=\"Password\">");
    webpage+=("<br>");
    webpage+=("<br>");
    webpage+=("<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\">");
    webpage+=("</P>");
    webpage+=("</FORM>");
    webpage+=("</body>");
    webpage+=("</html>");

    webServer.send(200,"text/html",webpage);
    webpage="";
    
}

void sendSSIDsaved(String ssidr,String passwordr){
  //
  // send a web page saying what details were used for the SSID and password
  //
     webpage="";
     setupHTMLSSIDheader();
     
     webpage+=(F("<body>"));
     webpage+=(F("<h1>This is the information you entered </h1>"));
     webpage+=(F("<p>The ssid is "));
     webpage+=(ssidr);
     webpage+=(F("<br>"));
     webpage+=(F("And the password is "));
     webpage+=(passwordr);
     webpage+=(F("</P><BR>"));
     webpage+=(F("<P>restart your device<br>"));
     webpage+=(F("setup task completed.</P>"));
     webpage+=(F("<H2><a href=\"/\">go home</a></H2><br>"));
     webpage+=(F("</body>"));
     webpage+=(F("</html>"));

    webServer.send(200,"text/html",webpage);
    webpage="";

}

// end of code
