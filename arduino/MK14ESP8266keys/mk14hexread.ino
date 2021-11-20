/*
 * 
 * It reads a HEX file from the file system
 *     validates each line and 
 *     sends the "keystroke" to the MK14 via the optical switches 
 * 
 *  Note: the input is not case sensitive the hex values can use A to F or a to f
 *    
 *  It checks A0 - if high then uses new OS key strokes else uses old OS key strokes.
 *  
 *  Special case - send R to do a rest of the MK14
 *  
 *  It should leave the MK14 in address mode after entering all the data
 *  
 *  A line with address 0xFFFE will be treated as a execute at suppled address
 *  e.g.
 *  :02FFFE000F20D2
 *  says execute the program at 0x0F20
 *    
 *  returns >0 if error with message stored in global String  errorMessage
 *  TODO - check for a better way to handle this String 
 *  
 *  MK14 Monitor  Operation  Summary 
 *  New OS 
 *     TERM  —Change to 'data entry'  mode  
 *     MEM   —Increment memory  address  
 *     ABORT —Change to  'address entry'  mode  
 *     GO    —Execute program at address displayed
 *  
 *  Original OS - is more complex
 *  
 *    Enter address 
 *      MEM - to go to address entry mode
 *    Data entry 
 *      TERM to switch to data entry mode 
 *      enter  data value
 *      then TERM to save it
 *  
 *   MEM always increments memory address
 *  
 *  
 *  copyright David Allday November 2021
 *  but you can use it as you wish -just mention that you got it from me 
 *  cheers :)
 *  
 */



int processHexFile(String hexFileName){
  //
  // process a hex file
  //
  int errorFlag=0;
  int lineNumber=0;
  
  String MK14filename=hexFileName;
  if (! MK14filename.startsWith("/") ){
    // make sure we have the dummy directory marker / at the start of the file
    MK14filename = "/" + MK14filename;
  }
  if ( ! SPIFFS.exists(MK14filename)) { // If file does not exist
    errorFlag=9;
    errorMessage="File " + MK14filename + "not found";
    return 1;
  }
  File hexFile = SPIFFS.open(MK14filename, "r");  // Open the file

  // reset mk14
  mk14Reset();
  

  while(hexFile.available()){
    lineNumber++;
    errorFlag=processHexLine(hexFile);
    if (errorFlag>0){
      errorMessage = "Line " + String(lineNumber) + ":" + errorMessage;
      break;
    }
  }
    
  hexFile.close();  // Close the file 

}



int processHexLine(File hexFile) {

  // Routine to load a hex line from a file  
  // validate it
  // and then if valid send to the MK14
  // returns 0 if all okay
  // return >0 if issue 
  //   put message in hexmessage String 
  // TODO - handle if address continues from previous call
  //     ?? how to know if we are starting a new file that just happens to start at that point
  //     
  
    int currentByte=0;
    int currentPosition=0;
    # define MAXHEXLINE 520
    char hexLine[MAXHEXLINE+1];    // a store for the hex line 
                          // the "information" part of a hex line can only be so may characters long
                          // The byte count is a single byte maximum value of 255
                          // That gives a :, 1 for byte count, 4 for the address, 1 record type, 255*2 for the data, 1 for the checksum
                          // 1+1+4+1+(255*2)+1 = 518 - allowing 520 + end marker, and anything after that is ignored.
                          
    int errorFlag=0;      // set to non zero if an error is found
    int lineLength=0;     // Length of the data part of the Hex line 
    int addressHighByte;  // High byte of the address used when calculating the data and execute address
    int addressLowByte;   // Low byte of the address used when calculating the data and execute address
    int addressFull;    // the address where data needs to be loaded
    int recordType=0;   // record type from hex line
    int executeFound=0; // set to 1 if record found with address 0xFFFE
    int executionAddress;
    
    static int previousEndAddress=0;

    int MK14_OS; // set to determine which MK14 OS to output to set after line check

    // reads characters from file
    //  until 
    //  end of file
    //  LF 0x10 is found
    //    MAXHEXLINE characters are stored in hexLine - rest is discarded 
    //    This should cover even the largest HEX file line
    //     I've put some comments after the hex lines in some files.
    //
    
    currentPosition=0;
    currentByte=0;
    hexLine[currentPosition]=0x0; // mark end of line

   
    while(hexFile.available()){
       currentByte=hexFile.read();
       if (currentByte == 012) {
        // cr - ignore
        break;
       }
       if (currentByte == 0x10 ) {
        // lf - treat as end of line
        if (currentPosition!=0){
          // only if we have soem characters in the line 
          // else ignore empty line
          return 0;
        }
       }
       if (currentPosition<MAXHEXLINE) {
        // still room in the array
        hexLine[currentPosition]=currentByte;
        currentPosition++;
        hexLine[currentPosition]=0x0; // mark end of line
       }
       // else ignore the byte
    }
    
    
    Serial.println("checking line");
    Serial.println(hexLine);

    errorFlag=checkhexline(hexLine,currentPosition);  
    
    Serial.print("result ");
    Serial.println(errorFlag);

    // Read this line's 'record type',
    // If it is not a normal record (00), don't do anything with the line
    // 
    if (errorFlag==0) {
      

      MK14_OS = mk14GetOS(); // Determine which MK14 OS to output to 
      
      recordType=CombineNibbles(Ascii_To_Nibble(hexLine[7]), Ascii_To_Nibble(hexLine[8]));
      Serial.print("Record type ");    
      Serial.println(recordType);    
      if (recordType==0){
        // only process it if it is a record type 0
        // get length of line
        lineLength = CombineNibbles(Ascii_To_Nibble(hexLine[1]), Ascii_To_Nibble(hexLine[2]));
        Serial.print("Line length ");
        Serial.println(lineLength);
        // get address
        addressHighByte = CombineNibbles(Ascii_To_Nibble(hexLine[3]), Ascii_To_Nibble(hexLine[4]));
        addressLowByte = CombineNibbles(Ascii_To_Nibble(hexLine[5]), Ascii_To_Nibble(hexLine[6]));
        addressFull = (addressHighByte << 8)|addressLowByte ; //Shift to upper byte of 16-bit raw address
        Serial.print("Address ");
        Serial.println(addressFull,HEX);
        if (addressFull == 0xFFFE) // Is this an 'Autorun' line?
        {
           //        Yes, it's an Auto-Run Line
           //        For the rest of this line, don't output data bytes to the MK14
           if (lineLength==2){
             //        Indicate that an execution address was found in the file
             executeFound = 1;
             //        Initialise the number of databytes to read the execution address from
              addressHighByte = CombineNibbles(Ascii_To_Nibble(hexLine[9]), Ascii_To_Nibble(hexLine[10]));
              addressLowByte = CombineNibbles(Ascii_To_Nibble(hexLine[11]), Ascii_To_Nibble(hexLine[12]));
              executionAddress = (addressHighByte << 8)|addressLowByte ; //Shift to upper byte of 16-bit raw address
              Serial.print("execution address is ");
              Serial.println(executionAddress,HEX);
           }
           else {
            // not the correct number of bytes in the execution line 
            errorMessage="Execution line 0xFFFE should be only 2 bytes long";
            Serial.println(errorMessage);
            errorFlag=10;
           }
        }
        else {
          if (lineLength>0){
            // only send address if there is some data on the line - not sure about this ?
            // send address first 
            // this test should work but not tested yet
            //if (previousEndAddress!=addressFull){
            if (true) {// dummy test whilst thinking about the above test
              previousEndAddress=addressFull;
              mk14EnterAddress(addressFull,MK14_OS);
            }
            mk14EnterDataMode();
            
            // now handle all the data
            for (int charpos=9;charpos<((lineLength*2)+9);charpos+=2){
              unsigned char keyToPress=Ascii_To_Nibble(hexLine[charpos]);
              OutToMK14 (keyToPress); 
              keyToPress=Ascii_To_Nibble(hexLine[charpos+1]);
              OutToMK14 (keyToPress); 
              // step onto next address and (enter)/(stay in) data entry mode
              mk14NextAddress(MK14_OS);
              // update the address we end up on - used in full sketch
              previousEndAddress++;
            }
  
            // for now enter address mode
            mk14EnterAddressMode();
          }
        } // end of autoexec else
        
        
      } // end of record tye == 0

      
    } // end of error flag == 0 
    
   if (executeFound==1){
      Serial.print("Sending execute address ");
      Serial.println(executionAddress,HEX);
      mk14GoAddress(executionAddress,MK14_OS);
   }
   return errorFlag;

} // end of processhexline

int checkhexline(char hexLine[],int numberOfChars){
  //
  // checks a line from the hex file to ensure that 
  //   it starts with :
  //   it is a least minimum hex file line length ( 11 characters )
  //   The 2 length characters are valid hex values
  //   The line is at least as long as the length field suggests
  //   That all the characters making up the valid part of the hex line are hex values.
  //   Adding up all the bytes should give a checksum of 00 if it is okay
  //      if wrong - subtract the value from 256 and add the checksum provided will give the required checksum 
  //      need to and it with 0xFF as only using the low byte.
  //
  // passed and array and the number of characters in that array
  // returns 0 if all okay 
  //  else .....

  int retval=0;
  int checksum=0;
  int linelengthh;
  int linelengthl;
  int linelength;
  
    if (numberOfChars<11){
          // line is too short to hold a valid hex line
          retval=3;
          errorMessage="Line too short for Hex File";
          Serial.println(errorMessage);
          return retval;
    }
    // we have at least some characters in the line
    // look for start marker
    if (hexLine [0]!=':'){
        retval=1;
        Serial.println ("Invalid Hex File");
        return retval;
    }
    // need to check if the line is a valid hex file line   
    linelengthh = isHexCharacter(hexLine[1]);
    linelengthl = isHexCharacter(hexLine[2]);
    if (linelengthh>15 or linelengthl > 15){
        retval=2;
        errorMessage=F("Invalid Hex data in size field");
        Serial.println(errorMessage);
        return retval;
    }
    linelength = (linelengthh * 16 ) + linelengthl;
    Serial.print("Characters received ");
    Serial.println(numberOfChars);
    Serial.print("line length from record ");
    Serial.println(linelength);
    Serial.print("Number chars ");
    Serial.println((1+2+4+2+(linelength*2)+2));
    // characters in the line should be 
    // 1 :, 2 length characters, 4 address characters, 2 for Rectype and 2 check sum characters
    if (numberOfChars<(1+2+4+2+(linelength*2)+2)){
        retval=3;
        errorMessage=F("Hex line too short for data");
        Serial.println(errorMessage);
        return retval;
    }
    checksum+=linelength;

    int hNibble;
    int lNibble;
    // check the data bytes ( 2 chars each ) plus 
    //  4 address characters, 2 for Rectype and 2 check sum characters
    // plus 3 as starting at position 3
    for (int checkpos=3;checkpos<((linelength*2)+8)+3 ;checkpos+=2){
      //Serial.print(checkpos);
      //Serial.print("=");
      //Serial.print(hexLine[checkpos]);
      //Serial.println();
      if ((hNibble=isHexCharacter(hexLine[checkpos])) > 15 ){
        retval=4;
        // reporting character positions from 1
        errorMessage=F("Invalid Hex data in line at character ") + String(checkpos+1);
        Serial.println(errorMessage);
        return retval;
      }
      if ((lNibble=isHexCharacter(hexLine[checkpos+1])) > 15 ){
        retval=4;
        // reporting character positions from 1
        errorMessage=F("Invalid Hex data in line at character ") + String(checkpos+2);
        Serial.println(errorMessage);
        return retval;
      }
      // we need just the last byte
      // should be zero if all adds up
      checksum=(checksum+(hNibble<<4)+lNibble)&0xFF;
      // Serial.print("checksum ");
      // Serial.println(checksum,HEX);
    }
    Serial.print("Final checksum ");
    Serial.println(checksum,HEX);
    if (checksum != 0 ){
        retval=5;
        errorMessage=F("check sum is invalid ")+String(checksum,HEX) + F("check sum should be ")+
                (((hNibble<<4)+lNibble+(256-checksum))&0xFF,HEX);
        // (hNibble<<4)+lNibble contains the last byte processed - the checksum byte
        Serial.println(errorMessage);
        return retval;
    }
    
    Serial.println("all done");

    return retval;
  
}



//-------------------------------------------------------------------
// HEX / MK14 FUNCTIONS
//-------------------------------------------------------------------

// check if the unsigned byte is a valid hex character 0 to 9, A to F
// returns it's binary value if true  
// else returns > 15 - actually 255 

unsigned char isHexCharacter(unsigned char hexChar ){
  
  unsigned char retval = 0;
  
  if (hexChar >= '0' && hexChar <=  '9') {
      retval = ( hexChar - '0');
  }
  else if (hexChar >= 'a' && hexChar <=  'f') {
      retval = ( hexChar - 'a') + 10;
  }
  else if (hexChar >= 'A' && hexChar <=  'F') {
  
      retval = ( hexChar - 'A') + 10;
  }
  else {
     retval = 255;
  }
  return (retval);  
}

// Ascii_To_Nibble
// Enter with 8-bit ascii code of one hex digit
// Exit with 4-bit raw digit equivalent and
// upper nibble cleared.
// works with uppercase and lower case A to F
// due to the ascii character set layout
unsigned char Ascii_To_Nibble (unsigned char a)
{
  a = (a - 0x30);

  if (a > 9) //Is it an alphabetic character?
  {
    a = (a - 0x07);
  }
  a = (a & 0x0f);
  return (a);
}
//-------------------------------------------------------------------
// CombineNibbles: Enter with raw high nibble in hinib, raw low nibble in lonib
// Returns the two nibbles combined in one raw byte
unsigned char CombineNibbles (unsigned char hinib, unsigned char lonib)
{
  unsigned char b = 0;
  //  Put the raw high nibble in the upper nibble of b
  b = hinib;
  b = (b << 4);
  //  Merge the raw low nibble
  b = (b | lonib);
  return (b);
}



// end of code
