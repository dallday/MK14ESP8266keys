/* 
 *  holds the code to process the html 
 *  
 *      for the main processes
 *  
 *  David Allday November 2021
 *  
 *  
 */


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
  
//  webServer.send(200,"text/html",webpage);
//  webpage="";
}

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

// put filename here so it can be accessed by various parts of the code
// TODO should we be using STATIC variables
String uploadfilename="";
// used during file uploads 
File fsUploadFile;              // a File object to temporarily store the received file


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

void handleFileUpload(){ 
  // upload a new file to the SPIFFS
  // is called by the webserver to do the actual upload
  // the process then calls the process to handle the web page call
  //   handleFileUploadComment
  //
  Serial.println("handleFileUpload ");
  HTTPUpload& upload = webServer.upload();
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
      sendHTMLredirect("/?emessage=" + uploadfilename.substring(1) + " is not a valid file type"); 
      //webServer.send(500, "text/plain", "500: invalid file type");
    }
    
  } else if(upload.status == UPLOAD_FILE_WRITE){
    Serial.print("handleFileUpload UPLOAD_FILE_WRITE: ");
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    Serial.print("handleFileUpload UPLOAD_FILE_END: ");
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: ");
      Serial.println(upload.totalSize);
      //webServer.sendHeader("Location","/success.html");      // Redirect the client to the success page
      //webServer.send(303);
      // remove the leading / from filename
      // senduploadsaved(String(uploadfilename).substring(1));
      // think we need to do the second part to get the comment before completing
    } else {
      webServer.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

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


String getContentType(String filename) { // convert the file extension to the MIME type
  // returns the contents type for the HTML response
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}





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
    sendprogramMK14(processingFilename);
    
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
    sendHTMLredirect("/recoverfiles?emessage=File " + Filename + " failed to recovered");        
  }
  
  
  return;  

}



void deleteFile(String filename){
  //
  // delete a file by doing a rename
  //   TODO - enable undo for delete
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


void sendprogramMK14(String program){
  // 
  // now actually handle the send of the codes to the MK14
  //
//  for (int x=0;x<100;x++){
//    delay (100);
//    // check to see if there are any requests to process
//    //handleWebServer();
//    webServer.handleClient();
//  }
  processHexFile(program);
  processingProgram=0;
  
}

void delayWebEnable(unsigned long mstodelay){
  //
  // delay x milliseconds but also allow the web server to work
  //

  unsigned long  previousMillis=millis(); // set originally to current time

  do{
    // check to see if there are any requests to process
    Serial.println("Processingweb server");
    webServer.handleClient(); 
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis > mstodelay) {
      return;
    }
  } while (true);
 
}

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


// end of code
