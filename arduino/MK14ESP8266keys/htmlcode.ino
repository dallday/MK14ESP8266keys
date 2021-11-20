/* 
 *  ********************************************
 *  holds the html generating code
 *  for the main processes
 *  
 *  see wifihtmlcode for the routines to do code for some pages
 *  
 *  David Allday November 2021
 *  
 *  ********************************************
 *  
 *  see https://www.w3schools.com/charsets/ref_emoji.asp for emojis  
 *     <p>I will display &#x1F981;</p> show a lion :)
 *  
 *  It generates all the pages using a global string webpage
 *  For the bigger pages it sends the response in chunks to avoid large strings
 *  TODO - would be a good idea to so much String manipulation :)
 *  
 *  
 *  
 */

//*************************************************************
// send main page
//      the one that is displayed when HTTP_GET / requested
//*************************************************************

void sendMainPage(){
  // 
  // sends the main page to the client
  // if the url includes ?messsage=xxxxx
  // then the message is displayed on the page
  //
  String message="";
  String errormessage="";
  if( ! webServer.hasArg("message") || webServer.arg("message") == NULL ) { // If the get request doesn't have program name
      Serial.println("No message agument");
  }
  else {
    message=webServer.arg("message");
    Serial.print("message agument ");
    Serial.println(message);
  }
  if( ! webServer.hasArg("emessage") || webServer.arg("emessage") == NULL ) { // If the get request doesn't have program name
      Serial.println("No error message agument");
  }
  else {
    errormessage=webServer.arg("emessage");
    Serial.print("emessage agument ");
    Serial.println(errormessage);
  }
  

  // output standard header and don't do a refresh/redirect 
  setupHTMLheader(0,"");

  // when the page has loaded run the code to set the OS 
  webpage+=(F("<body onload=getos();>"));
  // setup a number of scripts that may or maynot be used by this page
  setupScripts();
  webpage+=(F("<p>&nbsp;</p>"));

  webpage+=(F("<form action=\"/program\">"));

  //webpage+=(F("<table class='center'> "));
  webpage+=(F("<table> "));
  webpage+=(F("<tr><td style='text-align: center;' colspan='2'>MK14 <label id='os';></label></td></tr>"));
  webpage+=(F("<tr><td style='text-align: center;' colspan='2'>Programs available</td></tr>"));

  // switch to chuncking 
  // use HTTP/1.1 Chunked response to avoid building a huge temporary string
  if (!webServer.chunkedResponseModeStart(200, "text/html")) {
    webServer.send(505, F("text/html"), F("HTTP1.1 required"));
    return;
  }
  // send first bit
  webServer.sendContent(webpage);
  webpage="";
  
  Serial.println("Looking for files for main page");
  Dir dir = SPIFFS.openDir("/");

  while(dir.next()){
    //File entry = dir.openFile("r");
    //Serial.println(entry.name());
    Serial.println(dir.fileName());
    // String name1=String(entry.name()).substring(1);
    String name1=String(dir.fileName()).substring(1);
    name1.toLowerCase();
    if (name1.endsWith(".hex")) {
      // fetch it again to get case correct
      name1=String(dir.fileName()).substring(1);
      webpage+=(F("<tr><td>"));
      webpage+="<input type=\"radio\" id=\"" + name1 + "\" name=\"program\" value=\"" + name1 + "\" onclick=\"loadDoc('" + name1 + "')\" >";
      webpage+="<label for=\"" + name1 + "\">" + name1 + "</label> <br>\n";
      webpage+=(F("</td><td>"));
      File commentfile = SPIFFS.open("/" + name1 + ".txt","r");
      if (commentfile){
         webpage+=commentfile.readString();
         commentfile.close();
      }
      else
      {
        webpage+=(F("&nbsp;"));
      }
      
      webpage+=(F("</td></tr>"));
      // send that bit
      webServer.sendContent(webpage);
      webpage="";
    }
    // entry.close();
  }
  webpage+=(F("<tr><td colspan='2'>"));
  webpage+=(F("<input type='submit' name='action' value='Program MK14'>"));
  webpage+=(F("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"));
  webpage+=(F("<input type='submit' name='action' value='Delete File'></form> "));
  webpage+=(F("</td></tr>"));
  webpage+=(F("</table>"));

  // this will add the used and total size messages
  //setupUsedInfo();
  
  //webpage+=(F("<br/><table> "));
  //webpage+="<tr><td>" + message + "</td></tr>";
  //webpage+=(F("</table>"));

  webpage+=(F("<p> "));
  webpage+= message ;
  webpage+=(F("</p>"));
  if (errormessage.length() > 0 ){
  webpage+=(F("<p style='color: red;'> "));
  webpage+= errormessage ;
  webpage+=(F("</p>"));
    
  }


  webpage+=("<p>");
  webpage+=(F("</p>"));

  webpage+=(F("<table> "));
  webpage+=(F("<tr><td>File contents</td></tr>"));
  webpage+=(F("<tr><td>"));
  webpage+=(F("<div id='filecontents'>file contents will appear here</div>"));
  webpage+=(F("</td></tr>"));
  webpage+=(F("</table>"));

  webpage+=(F("<br><br><a href=\"/keystrokes\">Send keystrokes to MK14</a><br><br><br>"));
  
  int deletedfiles=checkForDeletedFiles();
  if (deletedfiles>0){
    webpage+="<a href=\"/recoverfiles\">recover " + String(deletedfiles) + " files </a><br>";
  }
  webpage+=(F("<a href=\"/upload\">upload a new file</a><br>"));
  webpage+=(F("<a href=\"/format\">format the file store</a><br>"));
  webpage+=(F("<a href=\"/setssid\">Set SSID and password</a><br>"));

  webpage+=(F("</body></html>"));

  // send that bit
  webServer.sendContent(webpage);
  webpage="";
  // close send
  webServer.chunkedResponseFinalize();

}


//**************************************************************
// Send the page that allows keystrokes to be entered
// 
//**************************************************************
void sendKeyStrokes(){

  // output standard header and don't do a refresh/redirect 
  setupHTMLheader(0,"");

  // when the page has loaded run the code to set the OS 
  webpage+=(F("<body onload=getos();>"));
  webpage+=(F("<h1>Send keystrokes to the MK14</h1>"));
  webpage+=(F("<p> enter the keystrokes to send to the MK14 <br/>"));
  webpage+=(F("Use 0 to 9, A to F, <br/>&nbsp;G=GO, M=MEM , T=TERM, Z=ABORT, R=Reset</p>"));
  webpage+=(F("<input type='text' id='chars' name='chars'>&nbsp;&nbsp;&nbsp;"));
  webpage+=(F("<button type='button' onclick=\"sendkeys(document.getElementById('chars').value);document.getElementById('chars').value='';\">Send keystrokes</button>"));

  webpage+=(F("<p>or click on the keys below</p>\n<p>"));

  webpage+=(F("<p id='reply'>&nbsp;</p>"));

  webpage+=(F("<table style='border:none;background-color:black;color:white;'>\n"));

  webpage+=(F("<tr><td style='width:25%;'>&nbsp;</td><td style='border:none;text-align:center;background-color:white;color:black;font-size:12px'>Science of Cambridge</td><td style='width:25%;'>&nbsp;</td></tr>\n"));
  webpage+=(F("<tr><td colspan=3></td></tr>"));
  webpage+=(F("<tr><td colspan=3 style='text-align: center;'>"));
  setupButton("go","g");
  setupButton("mem","m");
  setupButton("abort","z");
  setupButton("a","a");
  webpage+="<br>\n";
  for (int numkey=7;numkey<10;numkey++){
      String numkeys=String(numkey,HEX);
      setupButton(numkeys,numkeys);
  }
  setupButton("b","b");
  webpage+="<br>\n";
  for (int numkey=4;numkey<7;numkey++){
      String numkeys=String(numkey,HEX);
      setupButton(numkeys,numkeys);
  }
  setupButton("c","c");
  webpage+="<br>\n";
  for (int numkey=1;numkey<4;numkey++){
      String numkeys=String(numkey,HEX);
      setupButton(numkeys,numkeys);
  }
  setupButton("d","d");
  webpage+="<br>\n";
  setupButton("Term","t");
  setupButton("0","0");
  setupButton("f","f");
  setupButton("e","e");

  webpage+="<br>\n";
  webpage+="</td></tr>\n";
  webpage+="<tr><td>&nbsp;</td><td><p style='text-align:center;background-color:white;color:black;'>MK 14</p></td><td>&nbsp;</td></tr>\n";
  webpage+="<tr><td>&nbsp;</td><td>&nbsp;</td><td>";
  setupButton("reset","r");
  webpage+="</td></tr>\n";
  webpage+="</table>\n";
  webpage+="</p>\n";
  
  webpage+=(F("<p id='os'>&nbsp;</p>"));
  webpage+=(F("<p id='help'>&nbsp;</p>"));
  setupGotoHomePage();
  setupScripts();

  
  webpage+=(F("</body>"));
  webpage+=(F("</html>"));

  webServer.send(200,"text/html",webpage);
  webpage="";

}

//**************************************************************
// The page that is sent when keystrokes are requested
//  It could be just a single character when a button is pressed
//  or it could be a string of characters if the text box used.
//  Part of the AJAX call
//**************************************************************
void sendSendChars(){
  //
  // webpage has asked us send keystrokes to the MK14
  //
  String chars;
  
  Serial.println("programMK14 called");
  // get the characters requested  
  chars=webServer.arg("chars");
  Serial.print("Send char called with ");
  Serial.println (chars);

  for (int charpos=0;charpos<chars.length();charpos++){
    int charvalue = isHexCharacter(chars[charpos]);
    if (charvalue<16){
      Serial.print("Sending ");
      Serial.println(charvalue);
      OutToMK14((unsigned char) charvalue); 
    }
    else {
        switch (chars[charpos])
        {
          case 'G': // 'Go' key
          case 'g':
            OutToMK14(0x10);
            break;
          case 'M': // 'Mem'
          case 'm': 
            OutToMK14(0x11);
            break;
          case 'Q': // 'Abort' key
          case 'q': 
          case 'Z': // 'Abort' key
          case 'z': 
            OutToMK14(0x12);
            break;
          case 'T': // 'Term' Key
          case 't': 
            OutToMK14(0x13);
            break;
          case 'R': // 'Reset'            
          case 'r': 
            OutToMK14(0x14);
            break;
          default:
            break;      
        }
      
    }
    
    
  }

  // return the characters requested 
  // can then be displayed on the page   
  webpage=chars;
  webServer.send(200,"text/html",webpage);
  webpage="";

}


//**************************************************************
// The page that is sent when OS type is requested
//  It checks the jumper on the board and returns either 0 or 1
//  Part of the AJAX call
//**************************************************************
void sendosver(){

  webServer.send(200,"text/html",String(mk14GetOS()));
  
}


//**************************************************************
// The page that is sent when a list of "deleted" items is requested
//  responds to HTTP_GET /recoverfiles
//**************************************************************
void sendListofDeletedFiles(){
  // 
  // sends the page of deleted files to the client
  //  and allows for them to be recovered
  //  delete just renames the file as file.hex.old
  //   and it does not delete the file.hex.txt file
  //
  // 
  String message="";
  String errormessage="";
  if( ! webServer.hasArg("message") || webServer.arg("message") == NULL ) { // If the get request doesn't have program name
      Serial.println("No message agument");
  }
  else {
    message=webServer.arg("message");
    Serial.print("message agument ");
    Serial.println(message);
  }
  if( ! webServer.hasArg("emessage") || webServer.arg("emessage") == NULL ) { // If the get request doesn't have program name
      Serial.println("No error message agument");
  }
  else {
    errormessage=webServer.arg("emessage");
    Serial.print("emessage agument ");
    Serial.println(errormessage);
  }

  // output standard header and don't do a refresh/redirect 
  setupHTMLheader(0,"");

  webpage+=("<body>");
  setupScripts();
  webpage+=(F("<p>&nbsp;</p>"));

  webpage+=(F("<form action=\"/restore\">"));

  webpage+=(F("<table> "));
  webpage+=(F("<tr><td style='text-align: center;' colspan='2'>Deleted Programs</td></tr>"));

  // switch to chuncking 
  // use HTTP/1.1 Chunked response to avoid building a huge temporary string
  if (!webServer.chunkedResponseModeStart(200, "text/html")) {
    webServer.send(505, F("text/html"), F("HTTP1.1 required"));
    return;
  }
  // send first bit
  webServer.sendContent(webpage);
  webpage="";
  Serial.println("looking for 'deleted' files");
  Dir dir = SPIFFS.openDir("/");
  String name1;
  String name2;
  while(dir.next()){
    //File entry = dir.openFile("r");
    //Serial.println(entry.name());
    Serial.println(dir.fileName());
    // String name1=String(entry.name()).substring(1);
    name1=String(dir.fileName()).substring(1);
    name1.toLowerCase();
    if (name1.endsWith(".old")) {
      // fetch it again to get case correct
      // lose the leading /
      name1=String(dir.fileName()).substring(1);
      // now lose the .old
      name2=name1.substring(0,name1.length()-4);
      webpage+=(F("<tr><td>"));
      webpage+="<input type=\"radio\" id=\"" + name1 + "\" name=\"program\" value=\"" + name2 + "\" onclick=\"loadDoc('" + name1 + "')\" >";
      webpage+="<label for=\"" + name1 + "\">" + name2 + "</label> <br>\n";
      webpage+=(F("</td><td>"));
      Serial.print("Comment file " + name2 + ".txt ");
      File commentfile = SPIFFS.open("/" + name2 + ".txt","r");
      if (commentfile){
         Serial.println("found");
         webpage+=commentfile.readString();
         commentfile.close();
      }
      else
      {
        Serial.println("not found");
        webpage+=(F("&nbsp;"));
      }
      
      webpage+=(F("</td></tr>"));
      // send that bit
      webServer.sendContent(webpage);
      webpage="";

    }
    // entry.close();
  }
  webpage+=(F("<tr><td colspan='2'>"));
  webpage+=(F("<input type='submit' name='action' value='Restore'>"));
  webpage+=(F("</td></tr>"));
  webpage+=(F("</table>"));

  // this will add the used and total size messages
  //setupUsedInfo();
  
  //webpage+=(F("<br/><table> "));
  //webpage+="<tr><td>" + message + "</td></tr>";
  //webpage+=(F("</table>"));

  webpage+=(F("<p> "));
  webpage+= message ;
  webpage+=(F("</p>"));
  if (errormessage.length() > 0 ){
  webpage+=(F("<p style='color: red;'> "));
  webpage+= errormessage ;
  webpage+=(F("</p>"));
    
  }


  webpage+=("<p>");
  webpage+=(F("</p>"));

  webpage+=(F("<table> "));
  webpage+=(F("<tr><td>File contents</td></tr>"));
  webpage+=(F("<tr><td>"));
  webpage+=(F("<div id='filecontents'>file contents will appear here</div>"));
  webpage+=(F("</td></tr>"));
  webpage+=(F("</table>"));

  webpage+=(F("<br/>"));

  setupGotoHomePage();
  
  webpage+=(F("</body></html>"));
  // send that bit
  webServer.sendContent(webpage);
  webpage="";
  // close send
  webServer.chunkedResponseFinalize();
  
//  webServer.send(200,"text/html",webpage);
//  webpage="";

}



//**************************************************************
// The page that is sent when a redirect is required 
// if basically sends a page that has a autorefresh to the new page
//  the refresh time of 1 should be 0 to get a "instant" refresh
//**************************************************************
void sendHTMLredirect(String urlpath){
  //
  // sends a HTML page to redirect the client to a new page
  // did it since this will update the url on the client web page
  //  just outputting the details would not fix the url on the client
  //
  Serial.print("sendHTMLredirect called for ");
  Serial.println(urlpath);

  // change this to longer refresh for testing putposes 
  setupHTMLheader(0,urlpath);
  webpage+=F(("<body>"));
  webpage+="<a href='"+ urlpath + "'>Click here if you are not taken to next page</a>";
  webpage+=(F("</body>"));
  webpage+=(F("</html>"));

  webServer.send(200,"text/html",webpage);
  webpage="";

}


//**************************************************************
// This page is sent to allow the upload of a file
//**************************************************************
void sendUploadform(){
    //
    // Sends the upload form to the client 
    //

    Serial.println("Sending upload form");

    setupHTMLheader(0,"");

    webpage+=("<body>");

    webpage+=(F("<h1>Upload a new .hex file</h1>"));

    webpage+=("<p>select the hex file to upload and enter a comment to be displayed with it</p>");

    webpage+=(F("<form action='/uploadfile' method='post'  enctype='multipart/form-data'>"));
    // try to limit the type of files.
    webpage+=(F("<input type='file' name='data' accept='.hex'><br/><br/>"));
    webpage+=(F("<input type='text' name='comment' value=''>"));
    webpage+=(F("<button>Upload</button>"));
    webpage+=(F("</form>"));

    
    webpage+=(F("</body>"));
    webpage+=(F("</html>"));

    webServer.send(200,"text/html",webpage);
    webpage="";

}

//**************************************************************
// This provides an "Are You Sure" page  
// This one is for the request to format the file system
//**************************************************************
void sendAreYouSureFormat(){
  //
  // are you sure request for fomat 
  //
  sendAreYouSure("Format File System", "Formatting the file system cannot be undone","/formatyes","/?message=format cancelled");
  
}

//**************************************************************
// This provides "Are You Sure" page 
// Normally called from functions like above format one
// and given the message to display 
// and where to go to if yes
// 
//**************************************************************
void sendAreYouSure(String heading, String message,String yesurl,String nourl){
    //
    // creates an are you sure page 
    //  yes carries on
    //  no returns to main screen
    //
  
     setupHTMLheader(0,"");
     
     webpage+=(F("<body>"));
     webpage+="<script>function loadmess(){document.getElementById('message').innerHTML='Processing please wait';}</script>";
     webpage+="<h1>" + heading + "</h1>";
     webpage+=(F("<div id=message>"));
     webpage+=(F("<h2> Are You Sure ?</h2><p>"));
     webpage+=message;
     // used /" in href in case the urls contain 's
     webpage+="</p><a onclick='loadmess()' href=\"" + yesurl + "\">&#x1F44D; YES</a>&nbsp;&nbsp;&nbsp;";
     webpage+="<a href=\"" + nourl + "\">&#x1F44E; NO</a><br/><br/>";
     
     webpage+=(F("<a href=\"/\">return to home</a><br>"));
     webpage+=(F("</div>"));
     webpage+=(F("</body>"));
     webpage+=(F("</html>"));

    webServer.send(200,"text/html",webpage);
    webpage="";

}




//**************************************************************
// Sends either processing or programmed web page
// Called when programming initiated
// The waits for the processingProgram variable to become 0
//  It is sent with as 5 second refresh 
//     to itself if MK14 programming still going on
//      or to the main page if the MK14 programming compled
//**************************************************************
void sentprocessingMK14send(){
  //
  // processing web page 
  //  says processing until processingProgram identifies that the sent has finished
  //  the refreshes to the main page

  Serial.println("sentprocessingMK14send called");
  String progstate;
  String urlredirect;
  
      if (processingProgram==0){
        progstate="MK14 programmed with " + processingFilename;
        urlredirect="/?message=" + progstate;
      }
      else {
        progstate="processing " + processingFilename;
        urlredirect="/processing";
      }

     setupHTMLheader(5,urlredirect);
     webpage+=(F("<body>"));
     webpage+=(F("<h2>"));
     webpage+=progstate;
     webpage+=(F("</h2>"));
     webpage+=(F("</P><BR>"));
     webpage+=(F("</body>"));
     webpage+=(F("</html>"));

    webServer.send(200,"text/html",webpage);
    webpage="";
  
}

//**************************************************************
// This sends the list of all files stored in the SPIFF file system
// respond normally to the HTTP_GET /dir web request
//**************************************************************
void handleFileList() {
  // list all the files and overall size
  String path = "/";
  Serial.printf_P(PSTR("handleFileList: %s\r\n"),path.c_str());

  setupHTMLheader(0,""); // header no redirect

  webpage+=("<body>");

  webpage+=(F("<table> "));
  webpage+=(F("<tr><td style='text-align: center;' colspan='2'>Files</td></tr>"));
  webpage+=(F("<tr><td style='text-align: center;'>Name</td><td style='text-align: center;'>Size</td></tr>"));

  // switch to chuncking 
  // use HTTP/1.1 Chunked response to avoid building a huge temporary string
  if (!webServer.chunkedResponseModeStart(200, "text/html")) {
    webServer.send(505, F("text/html"), F("HTTP1.1 required"));
    return;
  }
  // send first bit
  webServer.sendContent(webpage);
  webpage="";

  Serial.println("Listing all files");

  Dir dir = SPIFFS.openDir(path);
  // path = String();


  while(dir.next()){
    webpage+=(F("<tr><td>"));
    // don't need to open file 
    //File entry = dir.openFile("r");
    Serial.print("FILE: ");
    Serial.println(dir.fileName());
    webpage += String(dir.fileName()).substring(1);
    webpage+=(F("</td><td>"));
    webpage += String(dir.fileSize());
    webpage+=(F("</td></tr>"));

    // send next bit
    webServer.sendContent(webpage);
    webpage="";
  }
  webpage+=(F("</table>"));

  setupUsedInfo();
  setupGotoHomePage();

  webpage+=(F("</body></html>"));

  // send last bit
  webServer.sendContent(webpage);
  webpage="";

  // close send
  webServer.chunkedResponseFinalize();
 
}


//**************************************************************
//  restore a file as asked by webpage
//**************************************************************
void restoreFile(){
  //
  // webpage has asked us to recover a file
  //
  Serial.println("programMK14 called");
  if( ! webServer.hasArg("program") || webServer.arg("program") == NULL ) { // If the get request doesn't have program name
    // redisplay main page with error message
    sendHTMLredirect("/recoverfiles?emessage=No file specified");        
    return;
  }
  String action=webServer.arg("action");
  Serial.print("Action");
  Serial.println (action);

  // update global variable
  String Filename=webServer.arg("program");
  if (!Filename.startsWith("/") ) {
    Filename="/" + Filename; 
  }
  
  if (SPIFFS.rename(Filename + ".old",Filename)){
    sendHTMLredirect("/?message=File " + Filename + " recovered");        
  }
  else {
    sendHTMLredirect("/recoverfiles?emessage=File " + Filename + " failed to recover");        
  }
  
  
  return;  

}


// end of code
