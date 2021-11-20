//************************************************************************
//
// This is ESP8266 sketch for MK14keyCP PCB
//
// It receives bytes to send to the MK14 
// and then works out which opto-relays to activate to "press" that key.
//  Taken from MK14keyCP sketch
//
//  David Allday November 2021
//   Thanks to SiriusHardware, Slothie and others on 
//    https://www.vintage-radio.net/forum
//     for a lot of the details used.
//
//************************************************************************

// These values are critical to getting the data into the MK14
// if it is not storing the data correctly then increase the values.

// Minimum length of one keypress (seconds)
#define KeyPressLength 10       // 10  == 0.01 second 
// Time for post-release debounce delay (seconds).
#define KeyReleaseLength 5   // 5  == 0.005 second 

// Minimum length of one keypress (seconds)
//#define KeyPressLength 500       // 10  == 0.01 second 
// Time for post-release debounce delay (seconds).
//#define KeyReleaseLength 100   // 5  == 0.005 second 


// The values below determine how long the uploader asserts reset for, and how long it waits after releasing
// reset before it starts uploading to the MK14.
// set to int so we can change them after sketch started
// Length of time to hold RESET low for (milliseconds)
int ResetHoldTime = 100; 
// delay after releasing reset 
// Length of time to allow for MK14 to come back up out of reset (milliseconds)
int PostResetDelay = 1000; // 1000  == 1 second 

// The values below ultimately determine the loading speed of the uploader. Too small and the uploader
// will generate keypresses faster than the MK14 can accept them, too large and the uploader will upload
// unnecessarily slowly. The default values here are the lowest / fastest which work consistently for me
// on my 4.00Mhz MK14 replica. If you are running a 4.43MHz machine you may be able to reduce these
// values a little bit.

#define KeyPressLength 5     // Length of time to keep a key pressed for (milliseconds)
#define KeyReleaseLength 5   // Length of time to wait after key release (milliseconds)

// ModeChangeSettleTime is the time allowed for the MK14 to get from one entry mode to another, ie,
// from Address Entry mode to Data Entry Mode or Data entry to Address entry mode.

//#define ModeChangeSettleTime 6 // Length of time allowed for entry mode change (milliseconds).
#define ModeChangeSettleTime 100 // Length of time allowed for entry mode change (milliseconds).


/*
 *  Serial characters that are used ( you can use upper or lower case characters
 *  
 *  Update - it uses codes 0x00 - 0x0F for keys 
 *  uses  0x10     'Go' key
 *  uses  0x11:    'Mem'
 *  uses  0x12:    'Abort' key
 *  uses  0x13:    'Term' Key
 *  uses  0x14:    'Reset'
 *  
 *  
 */

/* Charliplexing LEDs
 *  
 *  See the notes on Charlieplexing LEDs 
 *  you can wire it slighly differently so it works but the order of the leds is different
 *  check the schematic to see how I've wired the outputs to contol the various Optorelay leds
 *  
 *  see page
 *  https://www.instructables.com/Charlieplexing-the-Arduino/
 *  
 *  David Allday - October 2021
 *  
 *  
 *  
*/

/*
 * 
 * for ESP8266 LOLIN v3
 * 
 * One important thing to notice about ESP8266 is that the GPIO number doesn’t match the label on the board silkscreen. 
 * For example, D0 corresponds to GPIO16 and D1 corresponds to GPIO5. 
 * 
 * used board NodeMCU 1.0 (ESP-12E Module)
 * when using standard ESP8266 you need to use the numbers for the pins
 * 
 * D3 not good idea as programming line 
 * tested
 * D0 GPIO16  16
 * D1 GPIO5    5
 * D2 GPIO4    4 
 * D3 GPIO0    0 - Do not use for CharliePlexing  - used for wifi status indicator LED 
 * D4 GPIO2    2
 * D5 GPIO14  14
 * D6 GPIO12  12
 * D7 GPIO13  13
 * D8 GPIO15  15 - works but prevents loader and flickers LEDS on boot.
 * 
 * A0 - is the only analog pin
 * 
 * not sure of the references
 *  SD2 (S2) GPIO9   9 - do not use - used for internal access to onboard memory
 *  SD3 (S3) GPIO10 10 - do not use - used for internal access to onboard memory
 *  RX GPIO3 - do not use - this is the standard Serial port
 *  TX GPIO1 - do not use - this is the standard Serial port
 *  
 */

// The MK14_OS_Select input is used to tell the uploader which OS the target machine has as there are
// some differences in the sequence of keypresses required to enter and run code. If left unconnected
// the default output is to the 'new' OS with the '0000 00' prompt. If tied low the uploader
// outputs keypresses as required by the original OS with the '---- --' prompt.
// for ESP8266 we only have A0 

#define MK14_OS_Select A0 // ESP8266 Analog Pin ADC0 = A0

// these are specific to the ESP8266 
// set the ColumnControls and RowControls using the GPIO numbers 
#define ColC0  2     //  ColumnControl 1
#define ColC1  14     // ColumnControl 2    
#define ColC2  12     // ColumnControl 3
#define ColC3  13     // ColumnControl 4

#define RowC0  16     // RowControl 0
#define RowC1  05     // RowControl 1
#define RowC2  04     // RowControl 2


/*
 * Edge connector starting at reset button end
 * pin  Standard use 
 *  1  0v
 *  2  0v
 *  3  col 6 (6 ,E )
 *  4  col 7 (7, F, Term )
 *  5  col 5 (5)
 *  6  not used
 *  7  col 4 (4, Abort )
 *  8  not used
 *  9  col 3 (3, D, Mem )
 * 10  col 2 (2, C, Go )
 * 11  col 1 (1, 9, B )
 * 12  col 0 (0, 8, A )
 * 13  row 3 (Go, Mem, Abort, Term )
 * 14  row 2 (A, B, C, D, E, F )
 * 15  row 1 (8, 9)
 * 16  row 0 (0, 1, 2, 3, 4, 5, 6, 7 )
 * 
 * To activate a key then the correct row and column need to be connected.
 * This is done by activating the relevant Optocal relays LEDs.
 * 
 * The reset is done by shorting out the reset capacitor and that is achieved by activating reset Opto leds  ( RowC0 high and RowC2 Low )
 * Note: this activates both U5 and U6 
 * U6 connects pins 1 and pin 2 of the edge connector which does a reset in Version VI 
 * it has no impact on the version V as both pins are 0v (Gnd)
 * 
 */


// define rows and column numbers 
// these are the values passed to the PressKey function
// It allows and easy switch to JMP version of the connector
// maybe should be variables
#define Row0 0
#define Row1 1
#define Row2 2
#define Row3 3

#define Col0 0
#define Col1 1
#define Col2 2
#define Col3 3
#define Col4 5
#define Col5 7
#define Col6 9  // columns 6 and 7 seem to be switched on the edge connector
#define Col7 8

// NOTE: The reset Opto led is not in this list and is activated by putting RowC0 high and RowC2 Low 

// createarray for each MK14 keyboard column 0 to 9 
// showing which pin should be set HIGH and which LOW
// all other pins are set as INPUT
int  ColumnHigh[]   = { ColC0,ColC1,ColC1,ColC2,ColC0,ColC2,ColC0,ColC3,ColC1,ColC3 };
int  ColumnLow[]    = { ColC1,ColC0,ColC2,ColC1,ColC2,ColC0,ColC3,ColC0,ColC3,ColC1 };

// create an array for each MK14 keyboard row 0 to 3
int  RowHigh[]      = { RowC0,RowC1,RowC1,RowC2 };
int  RowLow[]       = { RowC1,RowC0,RowC2,RowC1 };



// some defines for the special keys to make the code more readable
#define GOKEY 0x10
#define MEMKEY 0x11
#define ABORTKEY 0x12
#define TERMKEY 0x13
#define RESETKEY 0x14



void mk14Setup(){
  //
  // routine to do the setup needed for the MK14 
  //

  // set the low values before setting output
  mk14ClearLEDRows();
  
  // just for info at this point - will be checked for each send :)
  int MK14_OS = mk14GetOS(); // Determine which MK14 OS to output to.
  // display is in the routine mk14GetOS
  //Serial.print("MK14_OS is ");
  // Serial.println(MK14_OS);

}


void mk14EnterAddress(int addressval,int mk14OS){
  //
  // set the address value on the MK14
  //
  if (mk14OS == 1)
  {
    OutToMK14 (ABORTKEY); // (New OS) 'Abort' keycode (Go to address entry mode)
  }
  else
  {
    OutToMK14 (MEMKEY); // (Old OS) 'Mem' keycode (Go to address entry mode)
  } 
  delayWebEnable(ModeChangeSettleTime);
  OutToMK14 ((addressval>>12) & 0xF);   //Address digit 1
  OutToMK14 ((addressval>>8) & 0xF);   //Address digit 2
  OutToMK14 ((addressval>>4) & 0xF);   //Address digit 3
  OutToMK14 ((addressval) & 0xF);   //Address digit 4

  OutToMK14 (TERMKEY); //0x13 = 'Term' keycode (Go back to data entry mode)
  delayWebEnable(ModeChangeSettleTime);

}

void mk14EnterDataMode(){
  //
  // switch back to data entry mode 
  // Really only holds in data entry mode for the new OS 
  //
  OutToMK14 (TERMKEY); //0x13 = 'Term' keycode (Go back to data entry mode)
  delayWebEnable(ModeChangeSettleTime);
}

void mk14NextAddress(int mk14OS){
  //
  // save current data if needed and then step memory on 1
  // if old operating system then re-enter data mode.
  // New OS only leaves data entry mode if Abort is pressed
  //
    if (mk14OS == 0)
    {
      OutToMK14(TERMKEY); //        'Term' (only when OS = old)
    }
    OutToMK14 (MEMKEY); //         'Mem' to increment address
    if (mk14OS == 0)
    {
      OutToMK14(TERMKEY); //        'Term' (Only when OS = old)
    }

}

void mk14EnterAddressMode(){
  //
  // put MK14 into address entry mode
  
  OutToMK14 (ABORTKEY); // 'Abort' to exit data entry mode.
  
}

void mk14GoAddress(int addressval,int mk14OS){
  // 
  // send the commands to the MK14 to execute program at address
  // assume MK14 is in address mode ??
  //
  
  if (mk14OS == 0){
     OutToMK14(GOKEY); // (Old OS only) - 'Go'
  }
  delayWebEnable(ModeChangeSettleTime);
  OutToMK14 ((addressval>>12) & 0xF);   //Address digit 1
  OutToMK14 ((addressval>>8) & 0xF);   //Address digit 2
  OutToMK14 ((addressval>>4) & 0xF);   //Address digit 3
  OutToMK14 ((addressval) & 0xF);   //Address digit 4

  if (mk14OS == 0)
  {
    OutToMK14 (TERMKEY); // (Old OS only) - 'Term'
  }
  else
  {
    OutToMK14 (GOKEY); // (New OS only) - 'Go'
  }

}


int mk14GetOS(){
  //
  // read the relevant analog input pin
  // set to 1 if result > 100
  // 
  
  pinMode(MK14_OS_Select,INPUT);
  // do an analog read and then check if greater than 100  to set mk14_OS to 1
  int sensorValue = analogRead(MK14_OS_Select);
  Serial.print("sensorValue is ");
  Serial.print(sensorValue);
  int mk14_OS = ( sensorValue > 100 ); // Determine which MK14 OS to output to.
  Serial.print(" MK14_OS set to ");
  Serial.println(mk14_OS);
  return (mk14_OS);

}


void mk14ClearLEDRows(){
  // set all row outputs to input - sets high impedence
  // which should turn off all the opto relays
  pinMode(ColC0, INPUT);     //Col 0
  pinMode(ColC1, INPUT);     //Col 1
  pinMode(ColC2, INPUT);      //Col 2
  pinMode(ColC3, INPUT);      //Col 3
  pinMode(RowC0, INPUT);      //row 0
  pinMode(RowC1, INPUT);      //row 1
  pinMode(RowC2, INPUT);      //row 2
  // then set them all low
  digitalWrite(ColC0, LOW);
  digitalWrite(ColC1, LOW);
  digitalWrite(ColC2, LOW);
  digitalWrite(ColC3, LOW);
  digitalWrite(RowC0, LOW);
  digitalWrite(RowC1, LOW);
  digitalWrite(RowC2, LOW);
  
}


//-------------------------------------------------------------------
// OutToMK14: Output a character or command keypress to the MK14
// 
//  This is a changed one from the keys14 sketch 
//    as it uses binary values to define the key to "press"
void OutToMK14 (unsigned char Key)
{
  Serial.print("Pressing key ");
  if (Key < 0x10){
    Serial.println(Key,HEX);
  }
  switch (Key)
  {
    case 0x00: // '0' key
      mk14PressKey(Row0, Col0);
      break;
    case 0x01: // '1' key
      mk14PressKey(Row0, Col1);
      break;
    case 0x02: // '2' key
      mk14PressKey(Row0, Col2);
      break;
    case 0x03: // '3' key
      mk14PressKey(Row0, Col3);
      break;
    case 0x04: // '4' key
      mk14PressKey(Row0, Col4);
      break;
    case 0x05: // '5' key
      mk14PressKey(Row0, Col5);
      break;
    case 0x06: // '6' key
      mk14PressKey(Row0, Col6);
      break;
    case 0x07: // '7' key
      mk14PressKey(Row0, Col7);
      break;
    case 0x08: // '8' key
      mk14PressKey(Row1, Col0);
      break;
    case 0x09: // '9' key
      mk14PressKey(Row1, Col1);
      break;
    case 0x0a: // 'A' key
      mk14PressKey(Row2, Col0);
      break;
    case 0x0b: // 'B' key
      mk14PressKey(Row2, Col1);
      break;
    case 0x0c: // 'C' Key
      mk14PressKey(Row2, Col2);
      break;
    case 0x0d: // 'D' key
      mk14PressKey(Row2, Col3);
      break;
    case 0x0e: // 'E' key
      mk14PressKey(Row2, Col6);
      break;
    case 0x0f: // 'F' key
      mk14PressKey(Row2, Col7);
      break;
    case 0x10: // 'Go' key
      Serial.println("GO");
      mk14PressKey(Row3, Col2);
      break;
    case 0x11: // 'Mem'
      Serial.println("MEM");
      mk14PressKey(Row3, Col3);
      break;
    case 0x12: // 'Abort' key
      Serial.println("ABORT");
      mk14PressKey(Row3, Col4);
      break;
    case 0x13: // 'Term' Key
      Serial.println("TERM");
      mk14PressKey(Row3, Col7);
      break;
    case 0x14: // 'Reset'
      Serial.println("RESET");
      mk14Reset();
      break;
    default:
      break;
  }

}


//-------------------------------------------------------------------
// Reset: Reset the MK14. Used to force the machine back to the
// monitor if it is busy running a program.
void mk14Reset (void)
{
  Serial.println ("Resetting...");

     // need to set new bits 
    // reset is RowC0 high and RowC2 Low 
    digitalWrite(RowC0, HIGH);
    digitalWrite(RowC2, LOW); 
    // set them as output  
    pinMode(RowC0, OUTPUT); 
    pinMode(RowC2, OUTPUT); 
    // delay to allow reset to occur
    delayWebEnable(ResetHoldTime);    // delay .1 second = 100 milliseconds 
    mk14ClearLEDRows();  // clear all outputs
    delayWebEnable(PostResetDelay); // give it time to actually reset

}



// Momentarily connect the specified key row / key column
// together by activating their associated GPIO port pins

void mk14PressKey(int RowNumber, int ColumnNumber){

    if (debugMode){
      Serial.print(" ");
      Serial.print("Row ");
      Serial.print(RowNumber);
      Serial.print(" Column ");
      Serial.print(ColumnNumber);
      Serial.print(" ");
    }
    // this is the major change for charlieplexing
    // See the arrays defined above

    int RowHighPin = RowHigh[RowNumber];
    int RowLowPin  = RowLow[RowNumber];

    int ColumnHighPin = ColumnHigh[ColumnNumber];
    int ColumnLowPin  = ColumnLow[ColumnNumber];
    
    
    if (debugMode){
      Serial.print(" ");
      Serial.print("Row High pin ");
      Serial.print(RowHighPin);
      Serial.print(" low pin ");
      Serial.print(RowLowPin);
      Serial.print(" Column High pin");
      Serial.print(ColumnHighPin);
      Serial.print(" low pin");
      Serial.print(ColumnLowPin);
      Serial.print(" ");
    }
    
    digitalWrite(RowHighPin, HIGH);
    digitalWrite(RowLowPin, LOW);   
    digitalWrite(ColumnHighPin, HIGH);
    digitalWrite(ColumnLowPin, LOW);   
    pinMode(RowHighPin, OUTPUT);     //row 1
    pinMode(RowLowPin, OUTPUT);     //row 2
    pinMode(ColumnHighPin, OUTPUT);     //row 1
    pinMode(ColumnLowPin, OUTPUT);     //row 2
    delayWebEnable(KeyPressLength);
    mk14ClearLEDRows();
    delayWebEnable(KeyReleaseLength);
}

// end of code
