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
 *  
 */

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
  // This is a script to get the name of the entry selected and display the file contents in the relevant box
  webpage+="<script>function loadDoc(name){document.getElementById('filecontents').innerHTML='Loading file contents';\nconst xhttp=new XMLHttpRequest();xhttp.onload=function(){\ndocument.getElementById('filecontents').innerHTML ='<pre>'+this.responseText+'</pre>';}\nxhttp.open('GET', name ); xhttp.send();}\n</script>";
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
      Serial.println("Comment file " + name2 + ".txt");
      File commentfile = SPIFFS.open("/" + name2 + ".txt","r");
      if (commentfile){
         Serial.print("found");
         webpage+=commentfile.readString();
         commentfile.close();
      }
      else
      {
        Serial.print("Not found");
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

  webpage+=("<body>");
  // This is a script to get the name of the entry selected and diaply the file contents in the relevant box
  webpage+="<script>function loadDoc(name){document.getElementById('filecontents').innerHTML='Loading file contents';\nconst xhttp=new XMLHttpRequest();xhttp.onload=function(){\ndocument.getElementById('filecontents').innerHTML ='<pre>'+this.responseText+'</pre>';}\nxhttp.open('GET', name ); xhttp.send();}\n</script>";
  webpage+=(F("<p>&nbsp;</p>"));

  webpage+=(F("<form action=\"/program\">"));

  //webpage+=(F("<table class='center'> "));
  webpage+=(F("<table> "));
  webpage+=(F("<tr><td style='text-align: center;' colspan='2'>Programs</td></tr>"));

  // switch to chuncking 
  // use HTTP/1.1 Chunked response to avoid building a huge temporary string
  if (!webServer.chunkedResponseModeStart(200, "text/html")) {
    webServer.send(505, F("text/html"), F("HTTP1.1 required"));
    return;
  }
  // send first bit
  webServer.sendContent(webpage);
  webpage="";
  
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

  webpage+=(F("<br/>"));
  int deletedfiles=checkForDeletedFiles();
  if (deletedfiles>0){
    webpage+="<a href=\"/recoverfiles\">recover " + String(deletedfiles) + " files </a><br>";
  }
  webpage+=(F("<a href=\"/upload\">upload a new file</a><br>"));
  webpage+=(F("<a href=\"/format\">format the file store</a><br>"));
  webpage+=(F("<a href=\"/setssid\">Set SSID and password</a><br>"));

  webpage+=(F("</body></html>"));

//  webServer.send(200,"text/html",webpage);
//  webpage="";
  // send that bit
  webServer.sendContent(webpage);
  webpage="";
  // close send
  webServer.chunkedResponseFinalize();


}

void setupHTMLheader(int redirect,String urlpath){
  //
  // create the HTML header in the webpage string
  // it clears the webpage string at start
  // redirect will redirect it to a new url if one specified
  // 

      webpage="";
      webpage+=(F("<!DOCTYPE HTML>"));
      webpage+=(F("<html>"));
      webpage+=(F("<head><title>MK14 Keys</title>"));
      if (urlpath!=""){
        // move to a new page
        webpage+="<meta http-equiv='refresh' content='" + String(redirect) + "; URL=" + urlpath + "' />";
      }
      setupHTMLstyle();
      webpage+=(F("</head>\n"));
}


void setupHTMLstyle(){
  // 
  // adds the HTML style details to the webpage string
  //

  webpage+=(F("<style>"));
  webpage+=(F("table, th, td { border: 1px solid black;} table.center {margin-left: auto; margin-right: auto;}"));
  webpage+=(F("td {padding: 5px;text-align: left;}"));
  // not this  "tr:nth-child(even) {background-color: #f2f2f2;}"
  webpage+=(F("p {text-align: left;}"));
  webpage+=(F("</style>\n"));

}


void sendHTMLredirect(String urlpath){
  //
  // sends a HTML page to redirect the client to a new page
  // did it since this will update the url on the client web page
  //  just outputting the details would not fix the url on the client
  //
  Serial.print("sendHTMLredirect called for ");
  Serial.println(urlpath);
  
  webpage="";
  webpage+=(F("<!DOCTYPE HTML>"));
  webpage+=(F("<html>"));
  webpage+=(F("<head><title>MK14 Keys</title>"));
  if (urlpath!=""){
    // move to a new page
    webpage+="<meta http-equiv='refresh' content='1; URL=" + urlpath + "' />";
  }
  setupHTMLstyle();
  webpage+=(F("</head>\n"));
  webpage+=F(("<body>"));
  webpage+="<a href='"+ urlpath + "'>You should be taken to " + urlpath + "</a>";
  webpage+=(F("</body>"));
  webpage+=(F("</html>"));

  webServer.send(200,"text/html",webpage);
  webpage="";

}



/*
 * sends the upload file request form to client
 */
void sendUploadform(){
    //
    // Sends the upload form to the client 
    //

    Serial.println("Sending upload form");

    setupHTMLheader(0,"");

    webpage+=("<body>");

    webpage+=(F("<h1>Upload a new .hex file</h1>"));

    webpage+=("<p>select the hex file to upload and enter a comment to be displayed with it</p>");

    webpage+=(F("<form action='/upload' method='post'  enctype='multipart/form-data'>"));
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

void sendAreYouSureFormat(){
  //
  // are you sure request for fomat 
  //
  sendAreYouSure("Format File System", "Formatting the file system cannot be undone","/formatyes");
  
}

void sendAreYouSure(String heading, String message,String url){
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
     webpage+="</p><a onclick='loadmess()' href='" + url + "'>&#x1F44D; YES</a>&nbsp;&nbsp;&nbsp;";
     webpage+="<a href='/?message=operation cancelled'>&#x1F44E; NO</a><br/><br/>";
     
     webpage+=(F("<a href=\"/\">return to home</a><br>"));
     webpage+=(F("</div>"));
     webpage+=(F("</body>"));
     webpage+=(F("</html>"));

    webServer.send(200,"text/html",webpage);
    webpage="";

}

void setupGotoHomePage(){
  // 
  // set up the go home link
  //
      webpage+=(F("<a href=\"/\">return to home</a><br>"));

}

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

void sendfailuremessage(String message){
  
     setupHTMLheader(0,"/");
     
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

void sentprocessingMK14send(){
  //
  // processing web page 
  //  says processing until processingProgram identifies that the sent has finished
  //  the refreshes to the main page

  Serial.println("sentprocessingMK14send called");
  String progstate;

      if (processingProgram==0){
        progstate="MK14 programmed with " + processingFilename;
      }
      else {
        progstate="processing " + processingFilename;
      }

  
      webpage="";
      webpage+=(F("<!DOCTYPE HTML>"));
      webpage+=(F("<html>"));
      webpage+=(F("<head><title>MK14 Keys sending"));
      webpage+=(F("</title>"));
      if (processingProgram==0){
        webpage+="<meta http-equiv='refresh' content='5; URL=/?message=" + progstate + "' >";
      }
      else {
        webpage+="<meta http-equiv='refresh' content='5; URL=/processing' >";
      }
      setupHTMLstyle();
      webpage+=(F("</head>\n"));

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


// end of code
