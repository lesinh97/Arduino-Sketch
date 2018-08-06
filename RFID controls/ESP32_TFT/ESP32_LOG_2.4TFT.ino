#include <SPI.h>          // SPI library for TFT & Eeprom
#include <TFT_eSPI.h>     // Graphics and font library for ST7735 driver chip
#include <WiFi.h>         // WiFi library for network
#include <TimeLib.h>      // Time library for time
#include <NtpClientLib.h> // NTP Client library fot network time

// including 3 320x240 pixel logo
#include "logo.h"         // Konica Minolta logo
#include "denied.h"       // Denied logo
#include "granted.h"      // Granted logo
#include "swipe.h"        // Granted logo
// these logo are customizable, feel free to add

//define pin in/out
#define EE_CS 2           // goes to eeporm CS
#define _cs   4           // goes to TFT CS
#define _dc   0           // goes to TFT & EEPROM DC
#define _mosi 23          // goes to TFT MOSI & EERPOM MISO
#define _sclk 18          // goes to TFT & EEPROM SCK/CLK
#define _rst  5           // goes to TFT RESET
#define _miso 19          // goes to EEPROM mosi
//       3.3V             // Goes to TFT & Eeprom Vcc & TFT LED
//       Gnd              // Goes to TFT & Eeprom Vss
#define eepromreset 12    // detect press key for delete card index from Eeprom
#define doorPin     14    // connect to door opener relay

#define BUFF_SIZE 64

const char* ssid     = " "; // Your network
const char* password = " "; // Your pass


String NTPServer = "192.168.12.1";          // NTP server address
const char* socketserver = "192.168.12.25";// logig server

String cardstr = "0";                     // define cardstr
int8_t timeZone = 1;                      // timezone in central europe (hour)
int8_t minutesTimeZone = 0;               // timezone in central europe (miniute)

byte olvas, cim = 0;
char val;
boolean programMode = false;
boolean match = false;
byte fnb = 0;                             // upper 8 bit
byte anb = 1;                             // lower 8 bit
unsigned int address;
byte storedCard[6];                       // stored byte array
byte readCard[6];                         // readed byte array
byte checksum = 0;                        //  Checksum

HardwareSerial Serial2(2);                // 2nd Serial port (IO16-> RXD, IO17-> TXD)
TFT_eSPI tft = TFT_eSPI();                // TFT driver (don't forget change settings in User_Setup.h (pin out and driver chip)

void setup() {

  Serial.begin(115200);                   // initialize serial port 0 for debug
  Serial2.begin(9600);                    // initialize serial port 2 for communicate to ID-12 card reader connet to IO16(RXD)

  WiFi.begin(ssid, password);             // initialize WiFi

  tft.init();                             // initialize TFT
  tft.setRotation(3);                     // rotate 0 degree
  tft.fillScreen(TFT_BLACK);              // clear the screen with black pixel
  tft.setTextColor(TFT_BLACK, TFT_BLACK); // text color

  tft.fillScreen(TFT_WHITE);

  drawIcon(logo,  0,0,  logoWidth,  logoHeight); // draw 128x128pix Konica Minolta logo;


  tft.setCursor(0,4
  );
  tft.print("                     ");      // show Wifi connection status
  tft.drawString("Connecting to Wifi:", 2, 0, 2);
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        tft.print(".");
        }
                                            // stop socket connection

  SPI.begin();                      // initialize SPI
  pinMode(doorPin, OUTPUT);         // set relay pin to output
  pinMode(eepromreset, INPUT);      // set eeprom reset pin to input

  digitalWrite(eepromreset, LOW);  // set eeprom reset pin to high
  digitalWrite(doorPin, HIGH);      // set door pin to high

  pinMode(EE_CS,OUTPUT);            // eeprom CS pin to output
  digitalWrite(EE_CS, HIGH);        // set CS pin to high

  NTP.begin (NTPServer, timeZone, true, minutesTimeZone); // initialize NTP
  NTP.setInterval (1200);           // sync interval

  tft.fillScreen(TFT_WHITE);

  WiFiClient client;                                          // initialize socket connection
  client.connect(socketserver,10000);                         // open socket port
  client.print(String(NTP.getTimeDateString ()));             // send time and date to loging sever
  client.print(String(";"));                                  // send semicolon (excel recognize next row)
  client.print(String("0000000000"));
  client.print(String(";"));                                // send semicolon
  client.println(String("CLIENT RESTARTED!!!!"));                         // send card status to loging server
  client.stop();

  drawIcon(logo,  0,0,  logoWidth,  logoHeight); // redraw 128x128pix Konica Minolta logo
}

void loop ()
{

  if (digitalRead(eepromreset) == HIGH) {eeprom_erase(); }    // erase eeprom when button is pressed
  byte val = 0;                                               //

    normalModeOn();                                           //


  if ( programMode)                                           // program mode wait for new card data
  {
    programModeOn();                                          // go to program mode

    if(Serial2.available() > 0)                               // wait for data from serial port
    {
      if((val = Serial2.read()) == 2)                         // first 2 byte is STX byte
      {
        getID();                                              // read data from card
        if ( !isMaster(readCard) )                            // check MASTER card
        {
          writeID(readCard);                                  // if yes, the store the new card
          programMode = false;                                // turn off program mode
          checksum = 0;                                       // checksum celar
        }
      }
    }


  }

  // normal mode
  // ------------------------------------------------------------------
  else
  {
    if(Serial2.available() > 0)                                // if serial port is active
    {
      if((val = Serial2.read()) == 2)                          // first 2 byte is STX
      {
        getID();                                               // read out the card ID
        byte bytesread = 0;

        for ( int i = 0; i < 5; i++ )                          // read out 5 byte
        {
          if  ( readCard[i] < 16 )
            delay(0);

        }

        if ( readCard[5] == checksum )                        // check the chechsum between readed with calculated
        {
          checksum = 0;
          if ( isMaster( readCard ) )                         // if MASTER card
          {
            programMode = true;                               // then set program mode on

          }
          else
          {
            if ( findID(readCard) )                           // search card in stored database
            {
              openDoor(5);                                    // if find then open door and show granted logo

            }
            else
            {
              failed();                                       // if not find card data from database and show denied logo
            }
          }
        }
        else                                                  // if checksum is wrong
        {


        }
      }
    }
  }

}

void eeprom_erase()
  {
  drawIcon(logo,  0,0,  logoWidth,  logoHeight);  // redraw 128x128pix Konica Minolta logo
  tft.setTextSize(4);                                         // set font size 2
  tft.setTextColor(TFT_RED);                                  // set font color red

  tft.drawCentreString("EEPROM", 160, 15, 2);                  // draw red "EEPROM ERASE STARTED" to TFT
  tft.drawCentreString("ERASE", 160, 60, 2);
  tft.drawCentreString("STARTED", 160, 105, 2);
  delay(1000);                                                 // wait 500ms

  drawIcon(logo,  0,0,  logoWidth,  logoHeight);  // redraw 128x128pix Konica Minolta logo

      pinMode(EE_CS, OUTPUT);                                 // eeprom CS out
      digitalWrite(EE_CS, HIGH);                              // eeprom CS high (initialization)
      delay (1);                                              // wait 1ms
      digitalWrite(EE_CS, LOW);                               // set eeprom chip select pin to high (chip is active)
      SPI.transfer(1);                                        // initialize eeprom
      SPI.transfer(0);                                        // initialize eeprom
      digitalWrite(EE_CS, HIGH);                              // set eeprom chip selec pint to low

    for (unsigned int j = 0; j < 2; )
    {
      digitalWrite(EE_CS, LOW);                               // set eeprom chip select pin to high (chip is active)
      SPI.transfer(6);                                        // write enabled 6
      digitalWrite(EE_CS, HIGH);                              // set eeprom chip select pin to high (chip is inactive)

      digitalWrite(EE_CS, LOW);                               // set eeprom chip select pin to high (chip is active)
      SPI.transfer(2);                                        // write 2,0
      SPI.transfer(j >> 8);                                   // eepro address upper 8 bit
      SPI.transfer(j & 0x00FF);                               // eeprom address lower 8 bit
      SPI.transfer(0);                                        // data write to eeprom
      digitalWrite(EE_CS, HIGH);                              // set eeprom chip select pin to high (chip is inactive)
      delay(4);                                               // delay 4ms to wite is gone

      j++;

      digitalWrite(EE_CS, LOW);                               // set eeprom chip select pin to high (chip is active)
      SPI.transfer(4);                                        // write disabled 4
      digitalWrite(EE_CS, HIGH);                              // set eeprom chip select pin to high (chip is inactive)

    }

    drawIcon(logo,  0,0,  logoWidth,  logoHeight);  // redraw 128x128pix Konica Minolta logo

    // write date and time under logo
    tft.setTextSize(4);                                       // set font size 2
    tft.setTextColor(TFT_GREEN);                              // set font color green
    tft.drawCentreString("EEPROM", 160, 15, 2);                // draw red "EEPROM ERASE SUCCES" to TFT
    tft.drawCentreString("ERASING", 160, 60, 2);
    tft.drawCentreString("SUCCES", 160, 105, 2);
    delay(1000);                                               // wait 500ms

    WiFiClient client;                                          // initialize socket connection
    client.connect(socketserver,10000);                         // open socket port
    client.print(String(NTP.getTimeDateString ()));             // send time and date to loging sever
    client.print(String(";"));                                  // send semicolon (excel recognize next row)
    client.print(String("0000000000"));
    client.print(String(";"));                                // send semicolon
    client.println(String("EEPROM ERASED!!!!!"));                         // send card status to loging server
    client.stop();                                            // stop socket connection


    drawIcon(logo,  0,0,  logoWidth,  logoHeight);  // redraw 128x128pix Konica Minolta logo
    }

void normalModeOn()
{

if ( programMode)                                           // program mode wait for new card data
  {
    normalModeOn2();
  }

  else {
  drawIcon(swipe,  0, 0,  swipeWidth,  swipeHeight);  // redraw 128x128pix Konica Minolta logo


  tft.setCursor(50, 225);                                    // set text position
  tft.setTextColor(TFT_BLACK, TFT_WHITE);                   // set font color to black with white background
  tft.setTextSize(2);                                       // set font size 1
  tft.print(NTP.getTimeDateString ());                      // print to tft current time and date hh:mm:ss dd/mm/yyyy
  tft.setTextSize(3);                                       // set font size 2
  tft.setTextColor(TFT_BLACK);                              // set font color black

  }
}
void normalModeOn2()
{
  delay(0);
}

void programModeOn()
{

  drawIcon(logo,  0, 0,  logoWidth,  logoHeight);  // redraw 128x128pix Konica Minolta logo
  tft.setTextColor(TFT_BLACK,TFT_WHITE);                     // set text position
  tft.setCursor(50, 225);                                     // set font color to black with white background
  tft.setTextSize(2);                                        // set font size 1
  tft.print(NTP.getTimeDateString ());                       // print to tft current time and date hh:mm:ss dd/mm/yyyy
  tft.setTextSize(4);                                        // set font size 2
  tft.setTextColor(TFT_RED);                                 // set font color red
  tft.drawCentreString("SWIPE", 160, 15, 2);                  // draw red "SWIPE NEW CARD" to TFT
  tft.drawCentreString("NEW", 160, 60, 2);
  tft.drawCentreString("CARD", 160, 105, 2);
  delay(20);                                                // wait 200ms

  tft.setTextColor(TFT_GREEN);                               // set font color red
  tft.drawCentreString("SWIPE", 160, 15, 2);                  // draw red "SWIPE NEW CARD" to TFT
  tft.drawCentreString("NEW", 160, 60, 2);
  tft.drawCentreString("CARD", 160, 105, 2);
  delay(20);                                                //wait 200ms

  tft.setTextColor(TFT_YELLOW);                               // set font color red
  tft.drawCentreString("SWIPE", 160, 15, 2);                  // draw red "SWIPE NEW CARD" to TFT
  tft.drawCentreString("NEW", 160, 60, 2);
  tft.drawCentreString("CARD", 160, 105, 2);
  delay(20);

  tft.setTextColor(TFT_BLUE);                               // set font color red
  tft.drawCentreString("SWIPE", 160, 15, 2);                  // draw red "SWIPE NEW CARD" to TFT
  tft.drawCentreString("NEW", 160, 60, 2);
  tft.drawCentreString("CARD", 160, 105, 2);
  delay(20);

  tft.setTextColor(TFT_PINK);                               // set font color red
  tft.drawCentreString("SWIPE", 160, 15, 2);                  // draw red "SWIPE NEW CARD" to TFT
  tft.drawCentreString("NEW", 160, 60, 2);
  tft.drawCentreString("CARD", 160, 105, 2);
  delay(20);

  tft.setTextColor(TFT_ORANGE);                               // set font color red
  tft.drawCentreString("SWIPE", 160, 15, 2);                  // draw red "SWIPE NEW CARD" to TFT
  tft.drawCentreString("NEW", 160, 60, 2);
  tft.drawCentreString("CARD", 160, 105, 2);
  delay(20);

  tft.setTextColor(TFT_MAGENTA);                               // set font color red
  tft.drawCentreString("SWIPE", 160, 15, 2);                  // draw red "SWIPE NEW CARD" to TFT
  tft.drawCentreString("NEW", 160, 60, 2);
  tft.drawCentreString("CARD", 160, 105, 2);
  delay(20);

  tft.setTextColor(TFT_BLACK);                               // set font color red
  tft.drawCentreString("SWIPE", 160, 15, 2);                  // draw red "SWIPE NEW CARD" to TFT
  tft.drawCentreString("NEW", 160, 60, 2);
  tft.drawCentreString("CARD", 160, 105, 2);
  delay(20);
 }

 void getID()
{
  byte bytesread = 0;
  byte i = 0;
  byte val = 0;
  byte tempbyte = 0;

                                                              // a 5 hexa byte az 10 ASCII byte
  while ( bytesread < 12 )                                    // read out 12 byte, last 2 byte is checksum
  {
    if( Serial2.available() > 0)                              // wait serial port is active
    {
      val = Serial2.read();                                   // read data from serial port

      if((val == 0x0D)||(val == 0x0A)||(val == 0x03)||(val == 0x02)) // when readed byte is 0d or 0a or 03 or 02 break
      {
        break;
      }

      if ( (val >= '0' ) && ( val <= '9' ) )                  // convert ASCII to HEX
      {
        val = val - '0';
      }
      else if ( ( val >= 'A' ) && ( val <= 'F' ) )
      {
        val = 10 + val - 'A';
      }

      if ( bytesread & 1 == 1 )                               // every ASCII byte pair convert to one BYTE
      {

        readCard[bytesread >> 1] = (val | (tempbyte << 4));
        if ( bytesread >> 1 != 5 )
        {
          checksum ^= readCard[bytesread >> 1];               // calculate Checksum
        };
      }
      else
      {
        tempbyte = val;
      };
      bytesread++;
    }
  }
  bytesread = 0;
}

boolean isMaster( byte test[] )
{
  byte bytesread = 0;
  byte i = 0;                                                 // Example card, replace with one of yours you want to be the master
  byte val[10] = {'0','F','0','0','5','3','F','0','3','0' };  // master card data byte
  byte master[6];
  byte checksum = 0;
  byte tempbyte = 0;
  bytesread = 0;

  for ( i = 0; i < 10; i++ )                                   // First we need to convert the array above into a 5 HEX BYTE array
  {
    if ( (val[i] >= '0' ) && ( val[i] <= '9' ) )               // Convert one char to HEX
    {
      val[i] = val[i] - '0';
    }
    else if ( (val[i] >= 'A' ) && ( val[i] <= 'F' ) )
    {
      val[i] = 10 + val[i] - 'A';
    }

    if (bytesread & 1 == 1)                                    // Every two hex-digits, add byte to code:
    {
                                                               // make some space for this hex-digit by
                                                               // shifting the previous hex-digit with 4 bits to the left:
      master[bytesread >> 1] = (val[i] | (tempbyte << 4));

      if (bytesread >> 1 != 5)                                 // If we're at the checksum byte,
      {
        checksum ^= master[bytesread >> 1];                    // Calculate the checksum... (XOR)
      };
    }
    else
    {
      tempbyte = val[i];                                       // Store the first hex digit first...
    };
    bytesread++;
  }

  if ( checkTwo( test, master ) )                              // Check to see if the master = the test ID
    return true;
  else
    return false;
}

void writeID( byte a[] )
{
  if ( !findID( a ) )                                          // Before we write to the EEPROM, check to see if we have seen this card before!
  {
     digitalWrite(EE_CS, LOW);                                 // set eeprom chip select pin to high (chip is active)
     SPI.transfer(3);                                          // read 3,0
     SPI.transfer(0);                                          // eeprom address upper 8 bit
     SPI.transfer(0);                                          // eeprom address lower 8 bit
     fnb = SPI.transfer(0x00);                                 // read addess upper 8 bit
     digitalWrite(EE_CS, HIGH);                                // set eeprom chip select pin to high (chip is inactive)

     digitalWrite(EE_CS, LOW);                                 // set eeprom chip select pin to high (chip is active)
     SPI.transfer(3);                                          // read 3,0
     SPI.transfer(0);                                          // eeprom address upper 8 bit
     SPI.transfer(1);                                          // eeprom address lower 8 bit
     anb = SPI.transfer(0x00);                                 // read addess upper 8 bit
     digitalWrite(EE_CS, HIGH);                                // set eeprom chip select pin to high (chip is inactive)

     unsigned num = ((fnb * 256) + anb);                       // calculate 16 bit address from upper 8 bit and lower 8 bit

     unsigned int start = ( num * 5 ) + 2;                     // Calculate where the next slot starts

     num++;                                                    // Increment the counter

     delay(1);                                                 // wait 1ms
     digitalWrite(EE_CS, LOW);                                 // set eeprom chip select pin to high (chip is active)
     SPI.transfer(6);                                          // write enabled 6
     digitalWrite(EE_CS, HIGH);                                // set eeprom chip select pin to high (chip is inactive)
     delay(1);                                                 // wait 1ms
     digitalWrite(EE_CS, LOW);                                 // set eeprom chip select pin to high (chip is active)
     SPI.transfer(2);                                          // write 2,0
     SPI.transfer(0);                                          // eeprom address upper 8 bit
     SPI.transfer(0);                                          // eeprom address lower 8 bit
     fnb = num >>8;                                            // calculate 16 address to two byte (upper and lower 8 bit)
     SPI.transfer(fnb);                                        // write upper 8 bit to eeprom index
     digitalWrite(EE_CS, HIGH);                                // set eeprom chip select pin to high (chip is inactive)
     delay(6);                                                 // wait 6ms to write is done
     digitalWrite(EE_CS, LOW);                                 // set eeprom chip select pin to high (chip is active)
     SPI.transfer(4);                                          // write disabled 4
     digitalWrite(EE_CS, HIGH);                                // set eeprom chip select pin to high (chip is inactive)
     delay(1);                                                 // wait 1ms
     digitalWrite(EE_CS, LOW);                                 // set eeprom chip select pin to high (chip is active)
     SPI.transfer(6);                                          // write enabled 6
     digitalWrite(EE_CS, HIGH);                                // set eeprom chip select pin to high (chip is inactive)
     delay(1);                                                 // wait 1ms
     digitalWrite(EE_CS, LOW);                                 // set eeprom chip select pin to high (chip is active)
     SPI.transfer(2);                                          // write 2,0
     SPI.transfer(0);                                          // eeprom address upper 8 bit
     SPI.transfer(1);                                          // eeprom address lower 8 bit
     anb = num & 0x00FF;                                       // calculate 16 address to two byte (upper and lower 8 bit)
     SPI.transfer(anb);                                        // write lower 8 bit to eeprom index
     digitalWrite(EE_CS, HIGH);                                // set eeprom chip select pin to high (chip is inactive)
     delay(6);                                                 // wait 6ms  to write is done
     digitalWrite(EE_CS, LOW);                                 // set eeprom chip select pin to high (chip is active)
     SPI.transfer(4);                                          // write disabled 4
     digitalWrite(EE_CS, HIGH);                                // set eeprom chip select pin to high (chip is inactive)
     delay(1);                                                 // wait 1ms
    for ( int j = 0; j < 5; j++ )                              // Loop 5 times to record to eeprom new card data
    {

      digitalWrite(EE_CS, LOW);                                // set eeprom chip select pin to high (chip is active)
      SPI.transfer(6);                                         // write enabled 6
      digitalWrite(EE_CS, HIGH);                               // set eeprom chip select pin to high (chip is inactive)

      address = start + j;                                     // incement address
      digitalWrite(EE_CS, LOW);                                // set eeprom chip select pin to high (chip is active)
      SPI.transfer(2);                                         // write 2,0
      SPI.transfer(address >>8);                               // eeprom address upper 8 bit
      SPI.transfer(address & 0x00FF);                          // eeprom address lower 8 bit
      SPI.transfer(a[j]);                                      // store new card data in eeprom
      digitalWrite(EE_CS, HIGH);                               // set eeprom chip select pin to high (chip is inactive)
      delay(6);                                                // wait 6 ms to write is done

      digitalWrite(EE_CS, LOW);                                // set eeprom chip select pin to high (chip is active)
      SPI.transfer(4);                                         // write disabled 4
      digitalWrite(EE_CS, HIGH);                               // set eeprom chip select pin to high (chip is inactive)

    }
    successWrite();                                            // print to tft card data is succes in eeprom
  }
  else
  {
    failedWrite();                                             // print to tft card is already stored
  }
}

boolean checkTwo ( byte a[], byte b[] )
{
  if ( a[0] != NULL )                                          // Make sure there is something in the array first
    match = true;                                              // Assume they match at first

  for ( int k = 0;  k < 5; k++ )                               // Loop 5 times
  {

    if ( a[k] != b[k] )                                        // IF a != b then set match = false, one fails, all fail
     match = false;
  }
  if ( match )                                                 // Check to see if if match is still true
  {
    return true;                                               // Return true
  }
  else {
    return false;                                              // Return false
  }
}

boolean findID( byte find[] )
{

     digitalWrite(EE_CS, LOW);                                 // set eeprom chip select pin to high (chip is active)
     SPI.transfer(3);                                          // read 3,0
     SPI.transfer(0);                                          // eeprom address upper 8 bit
     SPI.transfer(0);                                          // eeprom address lower 8 bit
     int fnb = SPI.transfer(0x00);                             // read out upper 8 bit
     digitalWrite(EE_CS, HIGH);                                // set eeprom chip select pin to high (chip is inactive)

     digitalWrite(EE_CS, LOW);                                 // set eeprom chip select pin to high (chip is active)
     SPI.transfer(3);                                          // read 3,0
     SPI.transfer(0);                                          // eeprom address upper 8 bit
     SPI.transfer(1);                                          // eeprom address lower 8 bit
     int anb = SPI.transfer(0x00);                             // read out lower 8 bit
     digitalWrite(EE_CS, HIGH);                                // set eeprom chip select pin to high (chip is active)

     unsigned count = ((fnb * 256) + anb);                     // calculate address

  for ( int i = 1; i <= count; i++ )                           // Loop once for each EEPROM entry
  {
    readID(i);                                                 // Read an ID from EEPROM, it is stored in storedCard[6]
    if( checkTwo( find, storedCard ) )                         // Check to see if the storedCard read from EEPROM
    {                                                          // is the same as the find[] ID card passed
      return true;
      break;                                                   // Stop looking we found it
    }
    else                                                       // If not, return false
    {

    }

  }
  return false;
}

void successWrite()
{
  tft.fillScreen(TFT_WHITE);
  drawIcon(logo,  0,0,  logoWidth,  logoHeight);              // redraw 128x128pix Konica Minolta logo

  tft.setTextColor(TFT_GREEN);                                // set font color to black
  tft.setTextSize(4);                                         // set font size 2
  tft.drawCentreString("CARD", 160, 15, 2);                   // draw "CARD STORED SUCCES" to TFT
  tft.drawCentreString("STORED", 160, 60, 2);
  tft.drawCentreString("SUCCES", 160, 105, 2);
  delay(2000);                                                // wait 2000ms

  drawIcon(logo,  0,0,  logoWidth,  logoHeight);              // redraw 128x128pix Konica Minolta logo

  WiFiClient client;                                          // initialize socket connection
  client.connect(socketserver,10000);                         // open socket port
  client.print(String(NTP.getTimeDateString ()));             // send time and date to loging sever
  client.print(String(";"));                                  // send semicolon (excel recognize next row)
   for ( int i = 0; i < 5; i++ )                              // read 5 byte
   {
     byte cardbyte = readCard[i];                             // read out byte from readCard array
      if (cardbyte < 16)                                      // if readed byte is lower 0x0F
      {
        cardstr = "0" + String(cardbyte,HEX);                 // then add 0 to byte (example converting 2 to 02)
      }
       else {
            cardstr = String(cardbyte,HEX);                   // else store to cardstr
            }
      client.print(String(cardstr));                          // send card byte to loging server

    }

  client.print(String(";"));                                  // send semicolon
  client.println(String("new"));                          // send card status to loging server
  client.stop();                                              // stop socket connection

}

void failedWrite()
{
  tft.fillScreen(TFT_WHITE);
  drawIcon(logo,  0,0,  logoWidth,  logoHeight);              // redraw 128x128pix Konica Minolta logo
  tft.setTextColor(TFT_RED);                                // set font color to black
  tft.setTextSize(4);                                         // set font size 2
  tft.drawCentreString("CARD", 160, 15, 2);                   // draw "CARD ALREADY STORED" to TFT
  tft.drawCentreString("ALREADY", 160, 60, 2);
  tft.drawCentreString("STORED", 160, 105, 2);
  delay(2000);                                                // wait 2000ms
  drawIcon(logo,  0,0,  logoWidth,  logoHeight);              // redraw 128x128pix Konica Minolta logo
}
void readID( unsigned int number )                            // calculating number from eeprom first two byte
{
   unsigned int start = (number * 5 ) - 3;                    // calculating addres from number * 5

   for ( int i = 0; i < 5; i++ )                              // read out 5 byte from calculated address
   {
     address = start + i;
     digitalWrite(EE_CS, LOW);                                // set eeprom chip select pin to high (chip is active)
     SPI.transfer(3);                                         // read 3,0
     SPI.transfer(address >>8);                               // eeprom address upper 8 bit
     SPI.transfer(address & 0xff);                            // eeprom address lower 8 bit
     storedCard[i] = SPI.transfer(0x00);                      // read data from eeprom stroreCard array
     digitalWrite(EE_CS, HIGH);                               // set eeprom chip select pin to high (chip is inactive)
   }
}

void openDoor( int setDelay )
{

  setDelay *= 1000; // Sets delay in seconds
  drawIcon(granted,  0,0,  grantedWidth,  grantedHeight); // draw 128x128pix granted logo
  digitalWrite(doorPin, LOW);                                 // Unlock door!
  // send log information to loging server
  WiFiClient client;                                          // initialize socket connection
  client.connect(socketserver,10000);                         // open socket port
  client.print(String(NTP.getTimeDateString ()));             // send time and date to loging sever
  client.print(String(";"));                                  // send semicolon (excel recognize next row)
   for ( int i = 0; i < 5; i++ )                              // read 5 byte
   {
     byte cardbyte = readCard[i];                             // read out byte from readCard array
      if (cardbyte < 16)                                      // if readed byte is lower 0x0F
      {
        cardstr = "0" + String(cardbyte,HEX);                 // then add 0 to byte (example converting 2 to 02)
      }
       else {
            cardstr = String(cardbyte,HEX);                   // else store to cardstr
            }
      client.print(String(cardstr));                          // send card byte to loging server

    }

  client.print(String(";"));                                  // send semicolon
  client.println(String("granted"));                          // send card status to loging server
  client.stop();                                              // stop socket connection

  delay(setDelay);                                            // Hold door lock open for 5 seconds

  digitalWrite(doorPin, HIGH);                                // Relock door

  drawIcon(logo,  0,0,  logoWidth,  logoHeight);  // redraw 128x128pix Konica Minolta logo

}

void failed()
{
  drawIcon(denied,  0,0,  deniedWidth,  deniedHeight);  // draw 128x128pix denied logo
  // send log information to loging server
  WiFiClient client;                                          // initialize socket connection
  client.connect(socketserver,10000);                         // open socket port
  client.print(String(NTP.getTimeDateString ()));             // send time and date to loging sever
  client.print(String(";"));                                  // send semicolon (excel recognize next row)

  for ( int i = 0; i < 5; i++ )                               // read 5 byte
    {
      byte cardbyte = readCard[i];                            // read out byte from readCard array
      if (cardbyte < 16)                                      // if readed byte is lower 0x0F

      {
        cardstr = "0" + String(cardbyte,HEX);                 // then add 0 to byte (example converting 2 to 02)
      }

       else {
            cardstr = String(cardbyte,HEX);                   // else store to cardstr
            }

     client.print(String(cardstr));                           // send card byte to loging server

    }

    client.print(String(";"));                                // send semicolon
    client.println(String("denied"));                         // send card status to loging server
    client.stop();                                            // stop socket connection

  digitalWrite(doorPin, HIGH);                                // lock door
  delay(5000);                                                // wait 5000ms

  drawIcon(logo,  0,0,  logoWidth,  logoHeight);   // redraw 128x128pix Konica Minolta logo

}

// logo wire routine
void drawIcon(const unsigned short* icon, int16_t x, int16_t y,  uint16_t width, uint16_t height) {

  uint16_t  pix_buffer[BUFF_SIZE];   // Pixel buffer (16 bits per pixel)

  // Set up a window the right size to stream pixels into
  tft.setAddrWindow(x, y, x + width - 1, y + height - 1);

  // Work out the number whole buffers to send
  uint16_t nb = ((uint16_t)height * width) / BUFF_SIZE;

  // Fill and send "nb" buffers to TFT
  for (int i = 0; i < nb; i++) {
    for (int j = 0; j < BUFF_SIZE; j++) {
      pix_buffer[j] = pgm_read_word(&icon[i * BUFF_SIZE + j]);
    }
    tft.pushColors(pix_buffer, BUFF_SIZE);
  }

  // Work out number of pixels not yet sent
  uint16_t np = ((uint16_t)height * width) % BUFF_SIZE;

  // Send any partial buffer left over
  if (np) {
    for (int i = 0; i < np; i++) pix_buffer[i] = pgm_read_word(&icon[nb * BUFF_SIZE + i]);
    tft.pushColors(pix_buffer, np);
  }
}
