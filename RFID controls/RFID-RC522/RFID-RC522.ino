#include <Wire.h>

#include "RTClib.h"
#include <MFRC522.h> // for the RFID
#include <SPI.h> // for the RFID and SD card module
#include <SD.h> // for the SD card


// define pins for RFID
#define CS_RFID 10
#define RST_RFID 9
// define select pin for SD card module
#define CS_SD 4 

// Create a file to store the data
File myFile;

// Instance of the class for RFID
MFRC522 rfid(CS_RFID, RST_RFID); 

// Variable to hold the tag's UID
String uidString;

// Instance of the class for RTC
  RTC_DS3231 rtc;
  char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


// Define check in time
const int checkInHour = 11;
const int checkInMinute = 5;

//Variable to hold user check in
int userCheckInHour;
int userCheckInMinute;

// Pins for LEDs and buzzer
const int redLED = 6;
const int greenLED = 7;
const int buzzer = 5;

void setup() {

  // Set LEDs and buzzer as outputs
  pinMode(redLED, OUTPUT);  
  pinMode(greenLED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  // Init Serial port
  Serial.begin(9600);

  // Init SPI bus
  SPI.begin(); 
  // Init MFRC522 
  rfid.PCD_Init(); 
  digitalWrite(10, HIGH);
  // Setup for the SD card
  Serial.print("Initializing SD card...");
  if(!SD.begin(CS_SD)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  digitalWrite(10, LOW);
  // Setup for the RTC  
  if(!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while(1);
  }
  else {
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {
  //look for new cards
  if(rfid.PICC_IsNewCardPresent()) {
    readRFID();
    logCard();
    verifyCheckIn();
  }
  delay(10);
}

void readRFID() {
  rfid.PICC_ReadCardSerial();
  Serial.print("Tag UID: ");
  uidString = String(rfid.uid.uidByte[0]) + " " + String(rfid.uid.uidByte[1]) + " " + 
    String(rfid.uid.uidByte[2]) + " " + String(rfid.uid.uidByte[3]);
  Serial.println(uidString);
 
  // Sound the buzzer when a card is read
  tone(buzzer, 2000); 
  delay(100);        
  noTone(buzzer);
  
  delay(100);

}

void logCard() {
  float temp = rtc.getTemperature();
  // Enables SD card chip select pin
  digitalWrite(CS_SD,LOW);
  
  // Open file
  myFile=SD.open("DATA.txt", FILE_WRITE);

  // If the file opened ok, write to it
  if (myFile) {
    SD.remove("DATA.txt");
    myFile=SD.open("DATA.txt", FILE_WRITE);
    Serial.println("File opened ok");
    myFile.print("UID: ");
    myFile.print(uidString);
    myFile.print(", ");   
    
    // Save time on SD card
    DateTime now = rtc.now();
    userCheckInHour = now.hour();
    userCheckInMinute = now.minute();
    myFile.print(now.year(), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.day(), DEC);
    myFile.print(" , ");
    myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.print(now.minute(), DEC);
    myFile.print(" , ");
    myFile.print(daysOfTheWeek[now.dayOfTheWeek()]);
    myFile.print(" , ");
    if((userCheckInHour < checkInHour)||((userCheckInHour==checkInHour) && (userCheckInMinute <= checkInMinute))) {
      myFile.println("INTIME");
    }
    else {
      myFile.println("LATE");
    }
    myFile.print("Temp is: ");
    myFile.println(temp);
    myFile.println("Next user");
    // Print time on Serial monitor
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.println(now.minute(), DEC);
    Serial.print("Temp is: ");
    Serial.println(temp);
    Serial.println("sucessfully written on SD card");
    myFile.close();

    // Save check in time;
    Serial.print("Your checked in hour: ");
    Serial.println(userCheckInHour);
    Serial.print("Your checked in minute: ");
    Serial.println(userCheckInMinute);
    Serial.print("Today is: ");
    Serial.println(daysOfTheWeek[now.dayOfTheWeek()]);
  }
  else {
    Serial.println("error opening data.txt");  
  }
  // Disables SD card chip select pin  
  digitalWrite(CS_SD,HIGH);

}

void verifyCheckIn(){
  if((userCheckInHour < checkInHour)||((userCheckInHour==checkInHour) && (userCheckInMinute <= checkInMinute))) {
    digitalWrite(greenLED, HIGH);
    delay(2000);
    digitalWrite(greenLED,LOW);
    Serial.println("You're welcome!");
  }
  else{
    digitalWrite(redLED, HIGH);
    delay(2000);
    digitalWrite(redLED,LOW);
    Serial.println("You are late...");
  }
}

