//FINAL
//**************************************LIBRARIES**************************************************/
//LCD LIBRARIES
#include <LiquidCrystal_I2C.h>  /*include LCD I2C Library*/
//ARDUINO LIBRARIES
#include <EEPROM.h>
//FINGER-PRINT R307 LIBRARIES
#include <Adafruit_Fingerprint.h>




//**************************************DEFINITIONS-&-MACROS**************************************/
//FINGER-PRINT R307
//#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(2, 3);
//#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
//#define mySerial Serial1
//#endif



//**************************************VARIABlES*************************************************/
//EEPROM VARIABLES
/////////////////////////////
//FINGER-PRINT R307
/////////////////////////////
//TEACHER MODE SWT
int teacherModeSwt = 13;

//LED
int giveFingerIndicatorLED = 7;



//**************************************OBJECTS******************************************/
//LCD LIBRARIES
LiquidCrystal_I2C lcd(0x27, 16, 2); /*I2C scanned address defined + I2C screen size*/
//FINGER-PRINT R307 LIBRARIES
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial); //finger is the object

//**************************************FUNCTIONS DEFINITIONS****************************/




//**************************************FUNCTIONS****************************************/
//////////////////////////////////////INIT FUCNTIONS//////////////////////////////////////

//FINGER-PRINT R307 SENSOR MODULE INIT FUCNTION
void fingerprintModuleInit() {
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(50);
  Serial.println("\n\nAdafruit Fingerprint sensor initialization!!");

  // set the data rate for the sensor serial port with aruino
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }

  //PRINT FINGER PRINT SENSOR DETAILS - can comment out following lines
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters(); // calls lib func to store fingerprint details to object finger
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
}



/////////////////////////////////GENERAL FUCNTIONS FUCNTIONS//////////////////////////////
//READ NUMBER FROM SERIAL MONITOR
uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available()); //waits in loop till true/data sent to arduino via usb
    num = Serial.parseInt(); // reads number from serial monitor
  }
  return num;
}


///////////////////////////////////FEATURE FUCNTIONS//////////////////////////////////////
//LCD FUNCTIONS
void scrollMessage(uint8_t row, String message, int delayTime, uint8_t totalColumns) {

  message = " " + message + " ";  //adds space to msg , eliminates garbage char in lcd
  //first garbage char in notice comes due to eeprom reading

  // reads total msg
  for (uint8_t i = 0; i < totalColumns; i++) {
    message = " " + message;
  }
  message = message + " ";

  //prints msg to lcd and scrolls col by col
  for (uint8_t position = 0; position < message.length(); position++) {
    lcd.setCursor(0, row); // cursor sets to first col(0) an row revcd
    // extract a protion of string (start,end)
    // first: (0,0+16) -> prints , second : (1, 1+16) -> prints an so on..
    lcd.print(message.substring(position, position + totalColumns));
    //Serial.print(message);
    delay(delayTime);  // delay of scroll each char
  }
}

//LCD DISPLAY TEXT
void lcdDisplayText(uint8_t row , String text) {
  lcd.clear();     /*Clear LCD Display*/
  lcd.backlight();      /*Turn ON LCD Backlight*/
  // lcd.setCursor(0, 0);  /*Set cursor to Row 1(col,row)*/
  // lcd.print(text); /*print text on LCD*/
  // lcd.setCursor(0, 1);  /*set cursor on row 2*/
  scrollMessage(row, text, 260, 16);
  delay(50);
}



//FINGER-PRINT MODULE R307 ENROLL FUCNTION
///////////////////////////////////FOLLOWING FUNCTION NOT USED AS USING CAUSES ARUDINO MEMORY EXCEEDED
uint8_t getFingerprintEnroll(uint8_t fingerID) {

  int p = -1; // init to -1 garabage val returns 2if no val
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(fingerID);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(fingerID);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(fingerID);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(fingerID);
  p = finger.storeModel(fingerID);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

//FINGER-PRINT ENROLL CALLING FUNCTION
///////////////////////////////////FOLLOWING FUNCTION NOT USED AS USING CAUSES ARUDINO MEMORY EXCEEDED
void enrollStudent() {
  uint8_t enrollingStudentFingerID = NULL;
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 126) you want to save this finger as...");
  Serial.println("Please type in 127 to return to menu");
  enrollingStudentFingerID = readnumber();
  if (enrollingStudentFingerID == 0) {// ID #0 not allowed, try again!
    return;
  } if (enrollingStudentFingerID == 127) {
    //Serial.setTimeout(1000);
    return;
  }

  Serial.print("Enrolling ID #");
  Serial.println(enrollingStudentFingerID);

  while (!getFingerprintEnroll(enrollingStudentFingerID));

}

//STORED FINGERPRINTS COUNT
///////////////////////////////////FOLLOWING FUNCTION NOT USED AS USING CAUSES ARUDINO MEMORY EXCEEDED
void storedFingerprintCount() {
  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }

}





//READING AND MATCHING FINGER-PRINTS FUCNTION
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage(); // calls lib func and gets img
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p = NULL;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p = NULL;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p = NULL;
    default:
      Serial.println("Unknown error");
      return p = NULL;
  }

  // OK success!

  p = finger.image2Tz(); // converts to suitable format
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p = 1111;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p = 1111;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p = 1111;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p = 1111;
    default:
      Serial.println("Unknown error");
      return p = 1111;
  }

  // OK converted!
  p = finger.fingerSearch(); // searches finger print module storage
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p = 1111; // custom error code
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p = 1111;// custom error code

  } else {
    Serial.println("Unknown error");
    return p = 1111; // custom error code
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID ; // returns finger ID of the matched finger
}

//NOTICE FUCNTION
void notice() {
  String notice = "";
  notice = readNoticeFromEEPROM();// reads from eeprom
  lcdDisplayText(0, notice); // displays notice to LCD
}

//CT MARKS FUCNTION
void showCTmarksAndNotice() {
  digitalWrite(giveFingerIndicatorLED, HIGH); // YELLOW INDICATOR
  delay(500);
  uint8_t reqFingerID = getFingerprintID(); // READS FINGER PRINT
  uint8_t reqStudentMarks = readCTmarksFromEEPROM(reqFingerID); // GETS CT MARKS FOR FINGERPRINT
  Serial.print(reqFingerID);
  delay(500);
  digitalWrite(giveFingerIndicatorLED, LOW);
  if (reqFingerID == 0)
    notice();
  else if (reqFingerID == 87) { //error code 87 from finger print module
    lcdDisplayText(1, "ERROR OR DID NOT FIND A FINGERPRINT");
  }
  else if (reqFingerID != 0) {
    lcdDisplayText(1, "YOUR CT MARKS: " + String(reqStudentMarks));
  }

}


//EEPROM FUCNTIONS
// MAX 1024 Bytes
//STORE TO EEPROM
void storeCTmarksToEEPROM(uint8_t fingerID, uint8_t ctMarks) {
  //STORE CT MARKS
  EEPROM.write(fingerID, ctMarks); //WRITING TO EEPROM (addr,val) eeprom cell :addr:  fingerID
}
void storeNoticeToEEPROM(const String &notice) {
  uint8_t startingAddr = 11; // NOTICE MUST START FROM ADDR 11
  byte len = notice.length();
  EEPROM.write(startingAddr, len);
  //WRITES FULL length msg to eeprom
  for (int i = 0; i < len; i++)
  {
    //EEPROM.write(startingAddr + 1 + i, notice[i]);
    EEPROM.write(startingAddr + 1 + i, notice[i]);
  }
}


//READ FROM EEPROM
uint8_t readCTmarksFromEEPROM(uint8_t fingerID) {
  uint8_t ctMarks = NULL;
  ctMarks =  EEPROM.read(fingerID);// reads CT marks againt fingerID
  return ctMarks;
}
String readNoticeFromEEPROM() {
  String notice = "";
  uint8_t startingAddr = 11; // reads notice from addr : 11

  int newStrLen = EEPROM.read(startingAddr);
  char data[newStrLen + 1];

  //STARTS READING TILL MSG END
  for (int i = 0; i < newStrLen - 1; i++)
  {
    data[i] = EEPROM.read(startingAddr + 1 + i); //STORES IN DATA ARRAY
  }
  data[newStrLen] = '\0'; // !!! NOTE !!! Remove the space between the slash "/" and "0" (I've added a space because otherwise there is a display bug)
  return String(data); //RETURNS NOTICE
}

//UPDATE CT MARKS
void updateCTmarks() {
  uint8_t updatedmarks = NULL;
  uint8_t reqStudentFingerID = NULL;

  Serial.println("ENTER THE STUDENT ID TO UPDATE MARKS:");
  reqStudentFingerID = readnumber();

  Serial.println("ENTER THE UPDATED MARKS:");
  updatedmarks = readnumber();

  storeCTmarksToEEPROM( reqStudentFingerID, updatedmarks); // STORE MARKS TO EEPROM

  Serial.println("MARKS UPDATED SUCCESSFULL");

}

//USER MENU TEACHERS MENU SERIAL MONITOR
void userMenu(uint8_t selectedOption) {
  //FOLLOWING OPTIONS TRIED BUT ARUINO MEMORY EXCEEDED
  /*
    if (selectedOption == 1) {
      //ENROLL NEW STUDENTS TO FINGER-PRINT MODULE
       //enrollStudent();
      return;
    } else if (selectedOption == 2) {
      //FULL CLEAN FINGER-PRINT MODULE DATABASE

    } else if (selectedOption == 3) {
      //SHOW ALL THE STORED FINGERPRINT TEMPLATES
       //storedFingerprintCount();*/


  if (selectedOption == 1) {
    //UPDATE NOTICE
    String  msg = "";
    Serial.print(" ENTER THE UPDATED NOTICE (WITHIN 10 SECONDS):  ");
    Serial.setTimeout(10000); // MUST GIVE TIMEOUT FROM STRING READ
    msg = Serial.readString();
    storeNoticeToEEPROM(String(msg));
    Serial.print(msg);
  } else if (selectedOption == 2) {
    //UPDATE STUDENT CT MARKS
    Serial.println(selectedOption);
    updateCTmarks();
  }
}



///////////////////////////////SETUP-LOOP DRIVER CODES////////////////////////////////////
void setup() {
  Serial.begin(9600);
  lcd.init();  /*LCD display initialized*/
  fingerprintModuleInit(); // fingerprint moule initialization
  pinMode(teacherModeSwt, INPUT);
  pinMode(giveFingerIndicatorLED, OUTPUT);
  // storeNoticeToEEPROM("CLASS POSPONED");
}
void loop() {

  uint8_t optionNumber = NULL;
  if (digitalRead(teacherModeSwt) == HIGH) {

    Serial.println("");
    Serial.println("WELCOME TO FINGER-PRINT BASED SMART ATTENDENCE SYSTEM");
    Serial.println("PLEASE PRESS THE REQUIRED OPTION NUMBER:");
    //Serial.println("1: ENROLL STUDENT");
    //Serial.println("2: FULL CLEAN FINGER-PRINT MODULE DATABASE");
    //Serial.println("3: SHOW TOTAL NUMBER OF FINGER PRINTS STORED");
    Serial.println("1: UPDATE NOTICE");
    Serial.println("2: UPDATE CT MARKS");
    Serial.println("3: EXIT");
    optionNumber = readnumber();
    if (optionNumber == 3) {
      Serial.println("EXIT SUCCESSFUL! CLOSE AND RESTART ARDUINO!");
      Serial.end();

    }
    userMenu(optionNumber);
  } else
    showCTmarksAndNotice();



  //String msg = Serial.readStringUntil('\n');
  // Serial.println(msg);

  //Serial.end();
  //  break;



  // showCTmarksAndNotice();
  //}
  /*  enrollStudent();
    storeCTmarksToEEPROM(1, 20) ;
    storeNoticeToEEPROM(msg);
    String notice = "";
    uint8_t Ctmarks =  readCTmarksFromEEPROM(1);
    notice = readNoticeFromEEPROM();
    Serial.println(Ctmarks);
    Serial.println(notice);
    //Serial.print(EEPROM.length());
    //lcdDisplayText(notice);
    lcdDisplayText(String(Ctmarks)); */


  // }
}
