/*
 Name:		_4Duino_NTP_Clock.ino
 Created:	4/14/2021 11:40:24 AM
 Author:	Mitchell Baldwin

*/
//
// NB! This is a file generated from the .4Dino file, changes will be lost
//     the next time the .4Dino file is built
//

// *******************************************************************************************
// * Sketch to create a simple analogue clock in 4DGL and synchronise the time from a UDP    *
// * time server. There are more complex versions of this code that use the Arduino uSD      *
// * environmnet and fetch fancy clock faces off the uSD card, ;-)                           *
// * You will need to set SSID and PASSWORD to enable the ESP8266 to logon to your router    *
// *******************************************************************************************

#define SSID        "WeatherDog-2G"
#define PASS        "6Yhn-mjU7"

#define xc  120
#define yc  200
#define r   120
#define FACECOLOUR  DARKGREEN
#define SECONDSCOLOUR RED
#define MINUTESCOLOUR LIME
#define HOURSCOLOUR   YELLOW
unsigned long epoch, hour_NTP, min_NTP, sec_NTP;
unsigned long NTPSyncTime;
unsigned long NTPCalcTime;
uint16_t seconds, minutes, hours;

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
String ATresponse;     // response from last AT command, does not include "OK" or "ERROR" and will be partial if command timed out

// Define LOG_MESSAGES to a serial port to send SPE errors messages to. Do not use the same Serial port as SPE
//#define LOG_MESSAGES Serial

#define RESETLINE     30

#define DisplaySerial Serial1


#include "Picaso_Serial_4DLib.h"
#include "Picaso_Const4D.h"

Picaso_Serial_4DLib Display(&DisplaySerial);

// Uncomment to use ESP8266
#define ESPRESET 17
#include <SoftwareSerial.h>
#define ESPserial SerialS
SoftwareSerial SerialS(8, 9);

// routine to handle Serial errors
void mycallback(int ErrCode, unsigned char Errorbyte)
{
#ifdef LOG_MESSAGES
  const char* Error4DText[] = { "OK\0", "Timeout\0", "NAK\0", "Length\0", "Invalid\0" };
  LOG_MESSAGES.print(F("Serial 4D Library reports error "));
  LOG_MESSAGES.print(Error4DText[ErrCode]);
  if (ErrCode == Err4D_NAK)
  {
    LOG_MESSAGES.print(F(" returned data= "));
    LOG_MESSAGES.println(Errorbyte);
  }
  else
    LOG_MESSAGES.println(F(""));
  while (1); // you can return here, or you can loop
#else
  // Pin 13 has an LED connected on most Arduino boards. Just give it a name
#define led 13
  while (1)
  {
    digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(200);                // wait for a second
    digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
    delay(200);                // wait for a second
  }
#endif
}
// end of routine to handle Serial errors

bool ATcmdResp(String /*char * */ cmd, int tlimit)
{
  String str1;
  ATresponse = "";
#ifdef ESP_DEBUG
  Serial.print(F("CMD>"));
  Serial.println(cmd);
#endif
  ESPserial.print(cmd);
  //  ESPserial.setTimeout(tlimit) ;              // this gives us no real advantage
  unsigned long sttm = millis();
  while (sttm + tlimit > millis())
  {
    str1 = ESPserial.readStringUntil(0x0a);    // note 0x0a will not be included!
    if (str1 == "")                             // ignore null response
    {
    }
    else if (str1 == "OK\r")                    // exit with ok response
    {
#ifdef ESP_DEBUG
      Serial.println(F("OK>"));
#endif
      return 1;
      break;
    }
    else if (str1 == "ERROR\r")                 // exit with error response
    {
#ifdef ESP_DEBUG
      Serial.println(F("ERROR>"));
#endif
      return 0;
      break;
    }
    else if (str1.endsWith("\r\r"))             // ignore original command echo
    {
#ifdef ESP_DEBUG
      Serial.print(F("cmd>"));
      Serial.println(str1);
#endif
    }
    else                                        // otherwise concat into response string
    {
#ifdef ESP_DEBUG
      Serial.print(F("IN>"));
      Serial.println(str1);
#endif
      ATresponse += str1;
      ATresponse += "\n";                    // because it was lost earlier
    }
  }
#ifdef ESP_DEBUG
  Serial.print(F("TIMEOUT>"));
#endif
  return 0;     // timeout
}
// End of routine to send command to the ESP8266 and wait for a response

boolean connectWiFi()
{
  String cmd = F("AT+CWJAP=\"");  // use Flash constants to avoid using up all RAM
  cmd += SSID;
  cmd += F("\",\"");              // use Flash constants to avoid using up all RAM
  cmd += PASS;
  cmd += F("\"\r\n");             // use Flash constants to avoid using up all RAM
  Display.print(F("Attempting Connection to WiFi\n"));  // use Flash constants to avoid using up all RAM

  for (int z = 0; z < 5; z++)
  {
    // allow sufficient time for responses, otherwise we can end up 'busy'
    // this command will return as soon as result is available, it will not always 'wait' max time
    // this commaand also leaves the result in ATresponse, so errors can be printed
    // it also allows diagnostics to be printed on the serial port
    if (ATcmdResp(cmd, 20000))
    {
      Display.print("Connected to WiFi: ");
      Display.print(SSID);
      Display.print("\n");
      return true;
    }
    else
    {
      Display.print(ATresponse);
      Display.print(".");
    }
  }
  return false;
}

void setup()
{
  // Ucomment to use the Serial link to the PC for debugging
  //  Serial.begin(115200) ;        // serial to USB port
  // Note! The next statement will stop the sketch from running until the serial monitor is started
  //       If it is not present the monitor will be missing the initial writes
  //    while (!Serial) ;             // wait for serial to be established

  pinMode(RESETLINE, OUTPUT);       // Display reset pin
  digitalWrite(RESETLINE, 1);       // Reset Display, using shield
  delay(100);                       // wait for it to be recognised
  digitalWrite(RESETLINE, 0);       // Release Display Reset, using shield
  // Uncomment when using ESP8266
  pinMode(ESPRESET, OUTPUT);        // ESP reset pin
  digitalWrite(ESPRESET, 1);        // Reset ESP
  delay(100);                       // wait for it t
  digitalWrite(ESPRESET, 0);        // Release ESP reset
  delay(3000);                     // give display time to startup

  // now start display as Serial lines should have 'stabilised'
  DisplaySerial.begin(200000);     // Hardware serial to Display, same as SPE on display is set to
  Display.TimeLimit4D = 5000;      // 5 second timeout on all commands
  Display.Callback4D = mycallback;

  // uncomment if using ESP8266
  ESPserial.begin(115200);         // assume esp set to 115200 baud, it's default setting
                                    // what we need to do is attempt to flip it to 19200
                                    // the maximum baud rate at which software serial actually works
                                    // if we run a program without resetting the ESP it will already be 19200
                                    // and hence the next command will not be understood or executed
  ESPserial.println("AT+UART_CUR=19200,8,1,0,0\r\n");
  ESPserial.end();
  delay(10);                         // Necessary to allow for baud rate changes
  ESPserial.begin(19200);            // start again at a resonable baud rate
  Display.gfx_ScreenMode(PORTRAIT); // change manually if orientation change
  // put your setup code here, to run once:
  Display.txt_FGcolour(BLUE);
  Display.txt_FontID(FONT1);         // largest internal font
  Display.putstr("Hello 4Duino'er!\n");

  String temp1 = "Connecting to Wifi AP: ";
  temp1 += SSID;
  temp1 += "\r\n";
  Display.print(temp1);
  //Display.print("Connecting to Wifi AP: " + SSID + "\n") ;

  ATcmdResp(F("AT+CWMODE_CUR=1\r\n"), 50);

  //Only required if ESP doesnt have a MAC set, and AP uses Mac Filtering
  //ATcmdResp(F("AT+CIPSTAMAC_CUR=\"18:aa:35:97:d4:7b\"\r\n"), 50);

  if (connectWiFi()) // Attempt connection 5 times
  {
    Display.print("Wifi Connected sucessfully\n");
  }
  else
  {
    Display.print("Wifi connection failed\n");
    Display.print("Check WiFi settings, and restart 4Duino\n");
    while (1); // crash here
  }

  ATcmdResp(F("AT+CIFSR\r\n"), 100);
  delay(250);
  ATcmdResp(F("AT+CIPMUX=0\r\n"), 100); //if (!cipmux0()) hang("cipmux0 failed");
  delay(250);
  ATcmdResp(F("AT+CIPMODE=0\r\n"), 100); //if (!cipmode0()) hang("cipmode0 failed");
  delay(250);

  Display.gfx_CircleFilled(xc, yc, r - 16, FACECOLOUR);

  // draw the clocks outer dress ring
  int n = -8;
  while (n++ < 8)
  {
    word colr = abs(n) + 12 ^ 31;
    Display.gfx_Circle(xc, yc, r + n - 8, colr);
  }

  // set up the centre point
  Display.gfx_MoveTo(xc, yc);

  // mark the hours round the clockface
  Display.gfx_MoveTo(xc, yc);
  n = -90;   // 12 o'clock position
  word targetX, targetY;
  while (n < 270)
  {
    Display.gfx_Orbit(n, r - 6, &targetX, &targetY);
    int k = 3;
    if (!(n % 90)) k = 5;
    Display.gfx_Circle(targetX, targetY, k, BLUE);
    n = n + 30;   // each 30 degreees
  }
} // end Setup **do not alter, remove or duplicate this line**

String NTPTime(int GMT)
{
  String temp;

  hour_NTP = (((epoch % 86400L) / 3600) + GMT) % 24;   // must %24 to ensure countries ahead don't end up more than 24 hours
  min_NTP = (epoch % 3600) / 60;
  sec_NTP = epoch % 60;

  if (hour_NTP < 10)
    temp = String(0) + String(hour_NTP);
  else
    temp = String(hour_NTP);
  temp += ":";
  if (min_NTP < 10)
    temp += String(0) + String(min_NTP);
  else
    temp += String(min_NTP);
  temp += ":";
  if (sec_NTP < 10)
    temp += String(0) + String(sec_NTP);
  else
    temp += String(sec_NTP);
  temp += "    ";

  return temp;
}

unsigned long GetTime()
{
  //130.102.128.23  Australia / New Zeeland
  //132.163.96.5    Boulder, CO USA
  //String cmd = "AT+CIPSTART=\"UDP\",\"130.102.128.23\",123\r\n"; // NTP server Australia / New Zeeland
  String cmd = "AT+CIPSTART=\"UDP\",\"132.163.96.5\",123\r\n"; // NTP server Boulder, CO USA
  ESPserial.println(cmd);
  delay(50);

  int counta = 0;
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  ESPserial.print("AT+CIPSEND=");
  ESPserial.println(NTP_PACKET_SIZE);
  if (ESPserial.find(">"))
  {
    for (byte i = 0; i < NTP_PACKET_SIZE; i++)
    {
      ESPserial.write(packetBuffer[i]);
      delay(5);
    }
  }
  else
  {
    ESPserial.println("AT+CIPCLOSE");
    return 0;
  }
  delay(50);
  ESPserial.find("+IPD,48:");

  memset(packetBuffer, 0, NTP_PACKET_SIZE);

  Serial.println("Server answer : ");

  int i = 0;
  while (ESPserial.available() > 0)
  {
    byte ch = ESPserial.read();
    if (i <= NTP_PACKET_SIZE)
    {
      packetBuffer[i] = ch;
    }
    if (ch < 0x10) Serial.print('0');
    Serial.print(ch, HEX);
    Serial.print(' ');
    if ((((i + 1) % 15) == 0))
    {
      Serial.println();
    }
    delay(5);
    i++;
    if ((i < NTP_PACKET_SIZE) && (ESPserial.available() == 0))
    {
      while (ESPserial.available() == 0)  // you may have to wait for some bytes
      {
        counta += 1;
        Serial.print("!");
        delay(100);
        if (counta == 15)
        {
          return 0;
        }
      }
    }
  }

  Serial.println();
  Serial.println();
  Serial.print(i + 1);
  Serial.println(" bytes received"); // will be more than 48

  Serial.print(packetBuffer[40], HEX);
  Serial.print(" ");
  Serial.print(packetBuffer[41], HEX);
  Serial.print(" ");
  Serial.print(packetBuffer[42], HEX);
  Serial.print(" ");
  Serial.print(packetBuffer[43], HEX);
  Serial.print(" = ");

  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;

  Serial.print(secsSince1900, DEC);

  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  unsigned long epochCalc = secsSince1900 - seventyYears;

  unsigned long DST = 60 * 60 * 2; // adjust to your GMT+DST

  unsigned long timestamp = epoch + DST;

  Serial.println();
  Serial.print("Epoch: ");
  Serial.println(epochCalc, DEC);


  // print the hour, minute and second:
  Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
  Serial.print((epochCalc % 86400L) / 3600); // print the hour (86400 equals secs per day)
  Serial.print(':');
  if (((epochCalc % 3600) / 60) < 10)
  {
    // In the first 10 minutes of each hour, we'll want a leading '0'
    Serial.print('0');
  }
  Serial.print((epochCalc % 3600) / 60); // print the minute (3600 equals secs per minute)
  Serial.print(':');
  if ((epochCalc % 60) < 10)
  {
    // In the first 10 seconds of each minute, we'll want a leading '0'
    Serial.print('0');
  }
  Serial.println(epochCalc % 60); // print the second
  return epochCalc;
}

void DrawHand(int length, int angle, int colour, int size)
{
  word targetX, targetY;
  Display.gfx_MoveTo(xc, yc);  // MUST RE_ESTABLISH THE CENTRE POINT!!!
  Display.gfx_Set(OBJECT_COLOUR, colour);
  Display.gfx_Orbit(angle - 90, length, &targetX, &targetY);
  Display.gfx_LineTo(targetX, targetY);     // but it should be gfx_LineTo, this is now correct
  Display.gfx_CircleFilled(targetX, targetY, size, colour);
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (millis() > NTPSyncTime)
  {
    NTPSyncTime = millis() + 1000; // 1 second from now, this should occur again

    epoch = GetTime();
  }

  if (millis() > NTPCalcTime)
  {
    NTPCalcTime = millis() + 1000; // 1 second from now, this should occur again

    Display.txt_FontID(FONT1);         // smallest internal font
    Display.txt_MoveCursor(7, 0);
    // Print out NZ Time
    Display.txt_FGcolour(ORANGE);
    Display.print("New Zealand Time: ");
    String temp1 = NTPTime(13);
    Display.txt_FGcolour(LIME);
    Display.println(temp1);
    // Print out Ausi Time
    Display.txt_FGcolour(ORANGE);
    Display.print("Sydney Australia Time: ");
    String temp2 = NTPTime(11);
    Display.txt_FGcolour(LIME);
    Display.println(temp2);

    DrawHand(r - 20, seconds, FACECOLOUR, 3);            // undraw the second hand
    DrawHand(r - 35, minutes, FACECOLOUR, 5);            // undraw the minute hand
    DrawHand(r - 50, hours, FACECOLOUR, 7);            // undraw the hour hand

    seconds = sec_NTP * 6;                                       // save current for later 'undraw'
    minutes = min_NTP * 6 + sec_NTP / 10;
    hours = hour_NTP * 30 + (min_NTP >> 1);
    DrawHand(r - 20, seconds, SECONDSCOLOUR, 3);         // redraw the second hand
    DrawHand(r - 35, minutes, MINUTESCOLOUR, 5);         // redraw the minute hand
    DrawHand(r - 50, hours, HOURSCOLOUR, 7);         // redraw the hour hand
    Display.gfx_CircleFilled(xc, yc, 5, ORANGE);
  }
}


