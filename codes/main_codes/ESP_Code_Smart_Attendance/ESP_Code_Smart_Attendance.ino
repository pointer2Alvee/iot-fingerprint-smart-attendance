//FINAL
//**************************************LIBRARIES**************************************************/
//FIREBASE LIBRARIES
#include <WiFi.h>
#include <FirebaseESP32.h>

/*Provide the token generation process info.*/
//#include <addons/TokenHelper.h>
/*Provide the RTDB payload printing info and other helper functions.*/
//#include <addons/RTDBHelper.h>

#include <Arduino.h>
#include <Adafruit_Fingerprint.h>

//**************************************DEFINITIONS-&-MACROS**************************************/
//ESP32 WIFI CREDENTIALS
#define WIFI_SSID "Hotspotz" // Mobile hotspot name
#define WIFI_PASSWORD "abcd_123"  // Mobile hotspot password

//FIREBASE CREDENTIALS
/*Define the API Key */
#define API_KEY "u6ZzdfvoF6BJsS5KO8qpF3MoalECKwXlV48q6LBo"
/*Define the RTDB URL */
#define DATABASE_URL "https://smartattendencesystem-b0b7a-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app


#define Receiver_Txd_pin 17
#define Receiver_Rxd_pin 16

//**************************************VARIABlES*************************************************/

//FINGER-PRINT R307 ENROLL VARIABLES
uint8_t fingerprintID = NULL;
uint8_t indexFingerID = NULL;

//LED
uint8_t imageTakenLED = 26;
uint8_t attendenceSuccessLED = 25;
uint8_t attendenceFailureLED = 13;


//**************************************OBJECTS******************************************/
//FIREBASE DATA OBJECTS
/*Define Firebase Data object*/
FirebaseData fbdo; // huge memory consume 35%
FirebaseConfig config;

//USART COM PORTS

//HardwareSerial Receiver(2); // Define a Serial port instance called 'Receiver' using serial port 2

//FINGER-PRINT R307 ENROLL OBJECTS
HardwareSerial serialPort(2); // use UART2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&serialPort);
//**************************************FUNCTIONS DEFINITIONS****************************/


//**************************************FUNCTIONS****************************************/
//////////////////////////////////////INIT FUCNTIONS//////////////////////////////////////
//FINGER PRINT MODULE INIT FUNCTIONS
void FingerprintInit() {

  Serial.println("\nFRINGER PRINT SENSOR INITIALIZATION");
  // set the data rate for the sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("FINGERPRINT SENSOR FOUND!");
  }
  else {
    Serial.println("FINGERPRINT SENSOR NOT FOUND!");
    while (1) {
      delay(1);
    }
  }
  /*
    //FINGERPRINT PARAMETERS
    Serial.println(F("Reading sensor parameters"));
    finger.getParameters();
    Serial.print(F("Status: 0x"));
    Serial.println(finger.status_reg, HEX);
    Serial.print(F("Sys ID: 0x"));
    Serial.println(finger.system_id, HEX);
    Serial.print(F("Capacity: "));
    Serial.println(finger.capacity);
    Serial.print(F("Security level: "));
    Serial.println(finger.security_level);
    Serial.print(F("Device address: "));
    Serial.println(finger.device_addr, HEX);
    Serial.print(F("Packet len: "));
    Serial.println(finger.packet_len);
    Serial.print(F("Baud rate: "));
    Serial.println(finger.baud_rate); */
}

//WIFI INIT FUCNTION
void wifiInit() {
  WiFi.mode(WIFI_STA); //Optional
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("CONNECTING TO WI-FI");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.print("CONNECTED WITH IP: ");
  Serial.print(WiFi.localIP());
  Serial.println();
}
//FIREBASE INIT FUCNTION
void firebaseInit() {
  Serial.printf("FIREBASE CLIENT: v%s\n\n", FIREBASE_CLIENT_VERSION);
  /* Assign the api key (required) */
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  //////////////////////////////////////////////////////////////////////////////////////////////
  //Please make sure the device free Heap is not lower than 80 k for ESP32 and 10 k for ESP8266,
  //otherwise the SSL connection will fail.
  //////////////////////////////////////////////////////////////////////////////////////////////
  Firebase.begin(DATABASE_URL, API_KEY);
  //Comment or pass false value when WiFi reconnection will control by your code or third party library
  //Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
}




//READ FINGERPRINTS FROM SENSOR MODULE
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  Serial.print("p: ");
  Serial.println(p);
  switch (p)
  {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      digitalWrite(imageTakenLED, HIGH);
      delay(400);
      digitalWrite(imageTakenLED, LOW);
      p = NULL;
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p = NULL;

    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return  p = NULL;;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return  p = NULL;;
    default:
      Serial.println("Unknown error");
      return  p = NULL;

  }

  // OK success!

  p = finger.image2Tz();
  Serial.print("p2: ");
  Serial.println(p);
  switch (p)
  {
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
  p = finger.fingerSearch();
  Serial.print("p3: ");
  Serial.println(p);
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Found a print match!");
    // found a match!
    Serial.println(" ");
    Serial.println(" ");
    Serial.println(" ");
    Serial.print("FINGERPRINT MATCHED! STUDENT FOUND");
    Serial.print("Found ID #");
    Serial.print(finger.fingerID);
    Serial.print(" with confidence of ");
    Serial.println(finger.confidence);
    Serial.println(" ");
    Serial.println(" ");
    Serial.println(" ");
    return finger.fingerID;
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    //  return p;
    return 1111;
  }
  else if (p == FINGERPRINT_NOTFOUND)
  {
    Serial.println("Did not find a match");

    digitalWrite(attendenceFailureLED, HIGH);
    delay(400);
    digitalWrite(attendenceFailureLED, LOW);

    return finger.fingerID = NULL;
    //return p;
    //return 1111;
  }
  else
  {
    Serial.println("Unknown error");
    // return p;
    return 1111;
  }
  return 1111;

}

////////////////////////////////////////////////FIREBASE/////////////////////////////////////
//FIREBASE SEND DATA TO DB
//REGISTER STUDENT ATTENDENCE TO FIRABASE
void registerStudentAttendence(uint8_t fingerprintIDnum) {
  if (fingerprintIDnum != 0) {
    //if (Firebase.ready()) {
    //SEARCH AN FETCH THUMB MATCHING FINGER-ID IN DATABASE-STUDENT

    //READS FROM FIREBASE
    Firebase.getInt(fbdo, "/Student/" + String(fingerprintIDnum) + "/StudentID" );
    int recvStudentID = fbdo.intData();
    Serial.print("FINGER ID MATHCED! RECVD STUDENT ID FROM DB: ");
    Serial.println(recvStudentID);

    //ATTENDENCE TIME STAMP
    unsigned long currentTime = millis();

    //REGISTER ATTENDENCE AND TIMESTAMP CORRESPONDING TO THE RECVD STUDENT ID
    // Firebase.setInt(fbdo, "/Attendence/" + String(recvStudentID) + "/FingerprintID", fingerprintIDnum);
   // Firebase.setInt(fbdo, "/Attendence/" + String(recvStudentID) + "/TimeStamp", currentTime);

   
   //SETS DATA TO FIREBASE
    Firebase.setInt(fbdo, "/Attendence/" + String(recvStudentID) + "/fingerprintIDnum", fingerprintIDnum);

    //GREEN LIGHT
    digitalWrite(attendenceSuccessLED, HIGH);
    delay(400);
    digitalWrite(attendenceSuccessLED, LOW);
    delay(100);
    // }
    fingerprintIDnum = NULL;
    finger.fingerID = NULL;
  }

}

//READ CALLING FUCNTION
void readFingerprints() {
  uint8_t presentStudentThumbID ;
  finger.getTemplateCount();
  if (finger.templateCount == 0) {
    Serial.print("FINGER-PRINT R307 MODULE DATABASE DOES NOT CONTAIN ANY FINGER PRINTS. PLEASE ENROLL!");
  }
  else {
    Serial.print("FINGER-PRINT R307 MODULE DATABASE CONTAINS: ");
    Serial.print(finger.templateCount);
    Serial.println(" TEMPLATES");
    Serial.println("FINGER-PRINT READ MODE ENABLED!");
    delay(50);
    presentStudentThumbID =  getFingerprintID();

  }

  registerStudentAttendence(presentStudentThumbID);
  //presentStudentThumbID = NULL;

}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //FINGER-PRINT R307
  FingerprintInit();
  //WIFI INIT
  wifiInit();
  //FIREBASE INIT
  firebaseInit();
  pinMode(2, OUTPUT);
  pinMode(imageTakenLED, OUTPUT);
  pinMode(attendenceSuccessLED, OUTPUT);
  pinMode(attendenceFailureLED, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  // Wait for the Receiver to get the characters
  //digitalWrite(2, HIGH);
  Serial.println("Working Correctly.");
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(2, HIGH); //WWIFI WORKING CORRECTLY
    readFingerprints();
    //getFingerprintID();
    Serial.println("Wifi  Connected.");

    // registerStudentAttendence(1);
  }
  else {
    digitalWrite(2, LOW);

  }



}
