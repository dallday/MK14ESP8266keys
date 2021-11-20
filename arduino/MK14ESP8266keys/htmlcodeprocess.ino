/* 
 *  holds the code to process the html 
 *  
 *      for the main processes
 *  
 *  David Allday November 2021
 *  
 *  
 */

//************************************************************88
// some functions for bits of web pages
//************************************************************88

void setupButton(String keyName, String keyVal){
  //
  // setup webpage button - used in the keystrokes page
  //
  String keyname1=keyName;
  // width and font is handled by the CSS style for class .button
  webpage+="<button onclick=\"sendkeys('" + keyVal + "')\" class='button' > " + keyname1 + " </button>\n";
 
}

//**************************************************************
// Setup the standard header details for web pages
//**************************************************************
void setupHTMLheader(int redirect,String urlpath){
  //
  // create the HTML header in the webpage string
  // it clears the webpage string at start
  // redirect will redirect it to a new url if one specified
  // 

      webpage="";
      webpage+=(F("<!DOCTYPE HTML>"));
      webpage+=(F("<html>"));
      webpage+=(F("<head><title>MK14 Keys</title>\n"));
      if (urlpath!=""){
        // move to a new page
        webpage+="<meta http-equiv='refresh' content='" + String(redirect) + "; URL=" + urlpath + "' />\n";
      }
      webpage+=(F("<meta name='viewport' content='width=device-width, initial-scale=1' />\n"));
      setupHTMLstyle();
      webpage+=(F("</head>\n"));
}

//**************************************************************
// Setup the style details for web pages
//**************************************************************
void setupHTMLstyle(){
  // 
  // adds the HTML style details to the webpage string
  //
  webpage+=(F("<style>"));
  webpage+=(F("table, th, td { border: 1px solid black;} table.center {margin-left: auto; margin-right: auto;}"));
  webpage+=(F("td {padding: 5px;text-align: left;}"));
  // not this  "tr:nth-child(even) {background-color: #f2f2f2;}"
  webpage+=(F("p {text-align: left;}"));
  webpage+=(F(".button  {  width: 70px; font-family:'Courier New'; background-color: white;   border: 3;   border-radius: 10px;   color: black;   padding: 15px 10px;   text-align: center;   text-decoration: none;   display: inline-block;   font-size: 16px;   margin: 4px 2px;   cursor: pointer;   width: 100; }\n"));
  webpage+=(F(".button:hover {  background-color: #ccc;   color: blue;}\n"));
  webpage+=(F("</style>\n"));

}


//**************************************************************
// creates the  got to home page link
//**************************************************************
void setupGotoHomePage(){
  // 
  // set up the go home link
  //
      webpage+=(F("<a href=\"/\">return to home</a><br>"));

}


//**************************************************************
// This provides a success page 
// Normally called from other functions
// which redirect to home page after 10 seconds
//**************************************************************
void sendsuccessmessage(String message){
  
     setupHTMLheader(10,"/");
     
     webpage+=(F("<body>"));
     webpage+=(F("<h2> "));
     webpage+=message;
     webpage+=(F("</h2>"));
     webpage+=(F("<br/><br/><br/>"));
     setupGotoHomePage();
     webpage+=(F("</body>"));
     webpage+=(F("</html>"));

    webServer.send(200,"text/html",webpage);
    webpage="";

}

//**************************************************************
// This provides a failure page 
// Normally called from other functions
// which NO redirect to home page
//**************************************************************
void sendfailuremessage(String message){
  
     setupHTMLheader(0,""); // setupo header but no redirect 
     
     webpage+=(F("<body>"));
     webpage+=(F("<h2> "));
     webpage+=message;
     webpage+=(F("</h2>"));
     webpage+=(F("<br/><br/><br/>"));
     setupGotoHomePage();
     webpage+=(F("</body>"));
     webpage+=(F("</html>"));

    webServer.send(200,"text/html",webpage);
    webpage="";

}

//**************************************************************
// This provides a load save page 
// Normally called from other functions
// which redirect to home page after 10 seconds
//**************************************************************
void senduploadsaved(String pfilename){
  
     setupHTMLheader(10,"/");
     
     webpage+=(F("<body>"));
     webpage+=(F("<h2>The file "));
     webpage+=pfilename;
     webpage+=(F(" been saved</h2>"));
     webpage+=(F("<br/>"));
     setupGotoHomePage();
     webpage+=(F("</body>"));
     webpage+=(F("</html>"));

    webServer.send(200,"text/html",webpage);
    webpage="";

}



//**************************************************************
// This streams a file to the webserver 
// Will return true if successful
// will return false if the file not found
// is used by 
// an inline function on the webserver.on command file not found
// also used by the AJAX code to supply file contents
//**************************************************************
bool handleFileRead(String path) { // send the right file to the client (if it exists)
  //
  // sends a file to the client if it exists 
  // else returns false 
  //  used for the 404 processing
  // TODO - check what type of files we will send ???
  //
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  // this was to handle compressed versions of a file 
  // normally a html file
  // taken from the original basesupport library 
  // 
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = webServer.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

//**************************************************************
// setsup the file store used and total space entries
//
//**************************************************************
void setupUsedInfo(){

  FSInfo fs_info;
  SPIFFS.info(fs_info);
  // or LittleFS.info(fs_info);

  Serial.print (fs_info.usedBytes);
  Serial.print ( " of ");
  Serial.print (fs_info.totalBytes);
  webpage+=(F("<p>"));
  webpage+=(F( "Total used "));
  webpage+= String(fs_info.usedBytes) + " of "+ String(fs_info.totalBytes) + " bytes";
  webpage+=(F("</p>"));

}

//**************************************************************
//
// the next code handle the file upload 
// File upload generated a number of calls from the webserver
// to set the file name 
// to upload the file data
// to close the file
// then to handle the standard HTTP_POST part of the page
// see webServer.on("/upload", HTTP_POST, etc.
//
//**************************************************************

// put filename here so it can be accessed by various parts of the code
// TODO should we be using STATIC variables
String uploadfilename="";
// used during file uploads 
File fsUploadFile;              // a File object to temporarily store the received file

//**************************************************************
// handles the file upload HTTP_POST page of the file upload
//  provides the comment field
// TODO - it still gets called if file upload fails
//        a comment file will be created if incorrect file uploaded
//**************************************************************
void handleFileUploadComment(){
      // get any comment with file 
    String comment="";
    if( ! webServer.hasArg("comment") || webServer.arg("comment") == NULL ) { // If the get request doesn't have program name
        Serial.println("No comment");
    }
    else {
      comment=webServer.arg("comment");
      Serial.print("comment '");
      Serial.print(comment);
      Serial.println("'");
    }
    if (comment.length()!=0){
      String commentfilename=uploadfilename  + ".txt";
      File fsFilecomment = SPIFFS.open(commentfilename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
      if(fsFilecomment){
        fsFilecomment.print(comment); // Write the received bytes to the file
        fsFilecomment.close();
      }
    }
    if (SPIFFS.exists(uploadfilename+".old")){
      // delete any .old files left over from the delete process
      SPIFFS.remove(uploadfilename+".old");
    }
    sendHTMLredirect( "/?message=File " + uploadfilename.substring(1) + " loaded");
}

//**************************************************************
// handles the actual file upload 
//**************************************************************
void handleFileUpload(){ 
  // upload a new file to the SPIFFS
  // is called by the webserver to do the actual upload
  // the process then calls the process to handle the web page call
  //   handleFileUploadComment
  //
  HTTPUpload& upload = webServer.upload();
  Serial.print("handleFileUpload status ");
  Serial.println(upload.status);
  if(upload.status == UPLOAD_FILE_START){
    uploadfilename = upload.filename;
    // ensure filename starts with /
    if(!uploadfilename.startsWith("/")) uploadfilename = "/"+uploadfilename;
    Serial.print("handleFileUpload Name: ");
    Serial.println(uploadfilename);
    String lcasefilename=uploadfilename;
    lcasefilename.toLowerCase();
    if (lcasefilename.endsWith(".hex")){   // only allow upload of the .hex files 
      // this is declared global ?
      fsUploadFile = SPIFFS.open(uploadfilename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
      // what does this do ??
      //filename = String();
    }
    else{
      String eMess=uploadfilename.substring(1) + " is not a valid file type";
      Serial.println(eMess);
      sendHTMLredirect("/?emessage=" + eMess); 
    }
    
  } else if(upload.status == UPLOAD_FILE_WRITE){
    Serial.print("handleFileUpload UPLOAD_FILE_WRITE: ");
    if(fsUploadFile){                                    // If the file was successfully created
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
      Serial.print(upload.currentSize);
      Serial.println(" bytes");
    }
    else{
      Serial.println("file not created");
    }
  } else if(upload.status == UPLOAD_FILE_END){
    Serial.print("handleFileUpload UPLOAD_FILE_END: ");
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print(" Size: ");
      Serial.println(upload.totalSize);
      //webServer.sendHeader("Location","/success.html");      // Redirect the client to the success page
      //webServer.send(303);
      // remove the leading / from filename
      // senduploadsaved(String(uploadfilename).substring(1));
      // think we need to do the second part to get the comment before completing
    } else {
      Serial.println("Could not create it.");
      webServer.send(500, "text/plain", "500: couldn't create file");
    }
  }
}


//**************************************************************
// sends a "not found" error 404 to the server
//**************************************************************
void handleNotFound() {

  Serial.println("not found");

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMethod: ";
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";

  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }

  Serial.println(message);
  
  webServer.send(404, "text/plain", message);
  
}

//**************************************************************
// get the typr of document being sent 
//**************************************************************
String getContentType(String filename) { // convert the file extension to the MIME type
  // returns the contents type for the HTML response
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}




//**************************************************************
// Formats the SPIFFS file system
// good idea to do this the very first time
//**************************************************************
void clearSPIFFS(){
  //
  // remove all files from the SPIFFS file system
  //
  if (SPIFFS.format()){
    //success 
    sendsuccessmessage("File system formatted");
  }
  else {
    sendfailuremessage("Error in SPIFFS.format");
  }
}

//**************************************************************
// Send a hex file to the MK14
// It returns the redirect webpage to say processing
// before calling the code to process the .hex file
// on completion the processingProgram flag is set to 0
//   This allows the processing refresh to return to main page
// TODO :-
//   prevent the main page etc., from processsing requests 
//    until the program has been loaded
//  There is currently no way to stop the programming process once started
//
//**************************************************************
void programMK14(){
  //
  // webpage has asked us to prorgam the MK14()
  //
  Serial.println("programMK14 called");
  if( ! webServer.hasArg("program") || webServer.arg("program") == NULL ) { // If the get request doesn't have program name
    // redisplay main page with error message
    sendHTMLredirect("/?emessage=No file specified");        
    return;
  }
  String action=webServer.arg("action");
  Serial.print("Action");
  Serial.println (action);

  // update global variable
  processingFilename=webServer.arg("program");
  processingProgram=1;
  
  if (action=="Program MK14") {
    // redirect the client to the processing web page
    sendHTMLredirect("/processing");
    // process the hex file 
    // all delays do a check to see if the webserver needs servicing
    processHexFile(processingFilename);
    processingProgram=0;
    
  }
  else if (action=="Delete File") {
    deleteFile(processingFilename);
    // sendHTMLredirect("/delete");   
    //webServer.sendHeader("Location","/delete");      // Redirect the client to the success page
    //webServer.send(303);
  }
  else {
        sendHTMLredirect("/?emessage=Invalid Action requested");   
  }
  return;  

}


//**************************************************************
// delete a .hex file by doing a rename
//**************************************************************
void deleteFile(String filename){
  //
  // delete a file by doing a rename
  //
  String oldname=filename;
  // make sure we have the / at the start of the filename
  if (!oldname.startsWith("/") ) {
    oldname="/" + oldname; 
  }
  String newname=oldname + ".old";
  Serial.print("renaming ");
  Serial.print(oldname);
  Serial.print(" to ");
  Serial.println(newname);
  
  if ( SPIFFS.exists(newname) ){
    Serial.print("removing ");
    Serial.print(newname);
    if (! SPIFFS.remove(newname)){
      sendHTMLredirect("/?emessage=Error in delete of file " + filename );  
    }
  }

  if ( SPIFFS.rename(oldname,newname)){
    sendHTMLredirect("/?message=File " + filename + " deleted");  
  }
  else {
    sendHTMLredirect("/?emessage=Error in deleting file " + filename );  
  }
  
}


//**************************************************************
// do a delay but also  check if webserver needs servicing
//**************************************************************
void delayWebEnable(unsigned long mstodelay){
  //
  // delay x milliseconds but also allow the web server to work
  //

  unsigned long  previousMillis=millis(); // set originally to current time

  do{
    // check to see if there are any requests to process
    // Serial.println("Processingweb server");
    webServer.handleClient(); 
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis > mstodelay) {
      return;
    }
  } while (true);
 
}


//**************************************************************
// Do a count of the number of deleted (renamed tp .old )  files
// Used to add link to front page
//**************************************************************
int checkForDeletedFiles(){

  int countDeleted=0;
  Dir dir = SPIFFS.openDir("/");
  while(dir.next()){
    if (dir.fileName().endsWith(".old")){
      countDeleted++;
    }
  }

  return countDeleted;
  
}

//**************************************************************
// setup some scripts used by the various pages
// Not all scripts are used on all pages 
// 
//**************************************************************
void  setupScripts(){
  //
  // setup the larger scripts used to handle AJAX data requests
  //

  // loadDoc - used to load a file contents into 'filecontents' element on the page
  // This is a script to get the name of the entry selected and display the file contents in the relevant box
  webpage+="<script>function loadDoc(name){document.getElementById('filecontents').innerHTML='Loading file contents';\nconst xhttp=new XMLHttpRequest();xhttp.onload=function(){\ndocument.getElementById('filecontents').innerHTML ='<pre>'+this.responseText+'</pre>';}\nxhttp.open('GET', name ); xhttp.send();}\n";
  // sendkeys - sends the value in element 'chars' to the server and updates the element reply 
  webpage+=(F("function sendkeys(keys){"));
  webpage+=(F("var xhttpsk = new XMLHttpRequest();"));
  webpage+=(F("xhttpsk.onreadystatechange = function() {"));
  webpage+=(F("if (this.readyState == 4 && this.status == 200) {"));
  webpage+=(F("document.getElementById('reply').innerHTML = 'Last keys sent:' + this.responseText;"));
  webpage+=(F("}};"));
  webpage+=(F("xhttpsk.open('GET', 'sendchars?chars=' + keys , true);"));
  webpage+=(F("xhttpsk.send();"));
  webpage+=(F("}\n"));
  // get the OS system from the ESP8266 webserver and set the id=os element and id=help elements 
  // does not fail if the element does not exist
  webpage+=(F("function getos(){"));
  webpage+=(F("var xhttpsk = new XMLHttpRequest();"));
  webpage+=(F("xhttpsk.onreadystatechange = function() {"));
  webpage+=(F("if (this.readyState == 4 && this.status == 200) {"));
  //webpage+=(F("document.getElementById('os').innerHTML = 'Operating system ' + (this.responseText + 1);"));
  webpage+=(F("if (this.responseText=='0'){ "));
  webpage+=(F("document.getElementById('os').innerHTML = 'version 1 of monitor ---- --';"));
  webpage+=(F("document.getElementById('help').innerHTML = '<pre>usage:<br/>Z M 0 F 2 0<br/> T C 4 T M T 0 7 T M T 0 7 T M T 3 F T <br/>Z G 0 F 2 0 T<br/></pre>';"));
  webpage+=(F("}else{"));
  webpage+=(F("document.getElementById('os').innerHTML = 'version 2 of monitor 0000 00';"));
  webpage+=(F("document.getElementById('help').innerHTML = '<pre>usage:<br/>Z 0 F 2 0  M C 4 M 0 7 M 0 7 M 3 F Z 0 F 2 0 G</pre><br/>';"));
  webpage+=(F("}"));
  webpage+=(F("}};"));
  webpage+=(F("xhttpsk.open('GET', 'getosver' , true);"));
  webpage+=(F("xhttpsk.send();"));
  webpage+=(F("}"));
  
  webpage+=(F("</script>\n"));
  // add script to made the enter key sent the text box 
  // TODO - fix - this does not work ?????
  webpage+=(F("<script>\n"));
  webpage+=(F("input.addEventListener('keyup', function(event) {if (event.keyCode == 13) { event.preventDefault(); document.getElementById('chars').click(); }});"));

  webpage+=(F("</script>\n"));
  
}

// end of code
