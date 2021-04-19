/*
 Name:		MFCD4Duino.ino
 Created:	4/9/2021 2:46:22 PM
 Author:	Mitchell Baldwin
*/

// Define LOG_MESSAGES to a serial port to send SPE errors messages to. Do not use the same Serial port as SPE
//#define LOG_MESSAGES Serial

//#define LIBCALL_ENABLEINTERRUPT
#define EI_NOTEXTERNAL
// #define EI_NOTPINCHANGE allows successful compile
// #define EI_NOTPORTB allows successful compile
// #define EI_NOTINT0 does not compile: multiple definition of __vector_9
// #define EI_NOTINT1 does not compile: multiple definition of __vector_9
// #define EI_NOTINT2 does not compile: multiple definition of __vector_9
// #define EI_NOTINT3 does not compile: multiple definition of __vector_9
// #define EI_NOTINT6 does not compile: multiple definition of __vector_9
#define EI_NOTPINCHANGE
#include <EnableInterrupt.h>
#include <ky-040.h>
constexpr auto HDGENCCLK = 7;
constexpr auto HDGENCDT = 6;
constexpr auto HDGENCPB = 5;

ky040 HeadingEncoder(HDGENCCLK, HDGENCDT, HDGENCPB, 1);

constexpr auto CRSENCCLK = 10;
constexpr auto CRSENCDT = 11;
constexpr auto CRSENCPB = 12;

ky040 CourseEncoder(CRSENCCLK, CRSENCDT, CRSENCPB, 1);

#include <Tasks.h>
bool TimeToCheckInputs = false;

void CheckInputs()
{
	TimeToCheckInputs = true;
}

#include "Picaso_Serial_4DLib.h"
#include "Picaso_Const4D.h"

#define RESETLINE     30
#define DisplaySerial Serial1
//#define CommSerial SoftSerial1

const word screenWidth = 240;
const word screenHeight = 320;

Picaso_Serial_4DLib Display(&DisplaySerial);

// Uncomment to use ESP8266:
#define ESPRESET 17
#include <SoftwareSerial.h>
#define ESPserial SerialS
SoftwareSerial SerialS(8, 9) ;

// Uncomment next 2 lines to use ESP8266 with ESP8266 library from https://github.com/itead/ITEADLIB_Arduino_WeeESP8266
#include "ESP8266.h"
//#include <ESPAsync_WiFiManager.h>
ESP8266 wifi(SerialS,19200);

#define SSID        "WeatherDog-2G"
#define PASSWORD    "6Yhn-mjU7"

// Routine to handle Serial errors
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
}	// end of routine to handle Serial errors

bool InDebugMode = true;
constexpr auto SerialDebugTimeout = 1000;	// ms

void setup()
{
	// Ucomment to use the Serial link to the PC for debugging:
	Serial.begin(115200);							// serial to USB port
	//while (!Serial);

	pinMode(RESETLINE, OUTPUT);       // Display reset pin
	digitalWrite(RESETLINE, 1);       // Reset Display, using shield
	delay(100);                       // wait for it to be recognised
	digitalWrite(RESETLINE, 0);       // Release Display Reset, using shield

	// Uncomment when using ESP8266:

	pinMode(ESPRESET, OUTPUT);        // ESP reset pin
	digitalWrite(ESPRESET, 1);        // Reset ESP
	delay(100);                       // wait for it t
	digitalWrite(ESPRESET, 0);        // Release ESP reset

	delay(3000);											// give USB Serial, 4D display and ESP8266 all time to startup

	// Now start display as Serial lines should have 'stabilised'

	DisplaySerial.begin(200000);			// Hardware serial to Display, same as SPE on display is set to
	Display.TimeLimit4D = 5000;				// 5 second timeout on all commands
	Display.Callback4D = mycallback;
	Display.gfx_ScreenMode(PORTRAIT); // change manually if orientation change
	Display.txt_FontID(FONT1);				// Start with smallest font

	// Note! The next statement will stop the sketch from running until the serial monitor is started
	// If it is not present the monitor will be missing the initial writes
	//while (!Serial);
	if (!Serial)
	{
		InDebugMode = false;
	}
	else
	{
		InDebugMode = true;
	}


	// Uncomment if using ESP8266:

	ESPserial.begin(115200) ;         // assume esp set to 115200 baud, it's default setting
	// what we need to do is attempt to flip it to 19200
	// the maximum baud rate at which software serial actually works
	// if we run a program without resetting the ESP it will already be 19200
	// and hence the next command will not be understood or executed
	ESPserial.println("AT+UART_CUR=19200,8,1,0,0\r\n") ;
	ESPserial.end() ;
	delay(10) ;                         // Necessary to allow for baud rate changes
	ESPserial.begin(19200) ;            // start again at a resonable baud rate


	
																			
	// Set up left rotary encoder with default function of heading entry (HDG):
	HeadingEncoder.AddRotaryCounter(1, 0, 0, 359, 1, true);
	HeadingEncoder.SetRotary(1);

	enableInterrupt(CRSENCCLK, ky040::RotaryClkInterruptOn_10, CHANGE);
	//enableInterrupt(CRSENCCLK, CheckInputs, CHANGE);

	// Set up right rotary encoder with default function of course entry (CRS):
	CourseEncoder.AddRotaryCounter(1, 0, 0, 359, 1, true);
	CourseEncoder.SetRotary(1);



	char TextBuffer[24];
	char TempBuffer[10];
	
	if (InDebugMode)
	{
		sprintf(TextBuffer, "%s", "MAIN - DEBUG");
		Serial.println("");
	}
	else
	{
		sprintf(TextBuffer, "%s", "MAIN");
	}
	word titleWidth = 0;
	word titleX = 0;
	word titleY = 0;
	for (unsigned i = 0; i < sizeof(TextBuffer); ++i)
	{
		titleWidth += Display.charwidth(TextBuffer[i]) + 1;	// Adding 1 for the gap (estimate)
	}
	titleWidth += 1;
	titleX = (screenWidth - titleWidth) / 2;
	Display.gfx_MoveTo(titleX, titleY);
	Display.print(TextBuffer);

	Display.txt_FGcolour(SPRINGGREEN);
	Display.gfx_MoveTo(10, 20);
	Display.println("SYS");
	Display.sys_GetModel(TempBuffer);
	sprintf(TextBuffer, "%s v%5d", TempBuffer, Display.sys_GetVersion());
	Display.txt_FGcolour(WHEAT);
	Display.println(TextBuffer);
	Display.txt_FGcolour(WHITESMOKE);
	sprintf(TextBuffer, "PmmC v%5d", Display.sys_GetPmmC());
	Display.println(TextBuffer);
	Display.txt_FGcolour(GREENYELLOW);
	sprintf(TextBuffer, "Screen: %dx%d(%dx%d)", screenWidth, screenHeight, screenWidth / Display.charwidth('W'), screenHeight / Display.charheight('H'));
	Display.println(TextBuffer);
	Display.txt_FGcolour(ALICEBLUE);
	sprintf(TextBuffer, "Font: %d(%dx%d)", FONT1, Display.charwidth('W'), Display.charheight('H'));
	Display.println(TextBuffer);

	Display.println("");

	Display.gfx_MoveTo(0, 100);
	Display.print(">");

	Display.txt_FGcolour(SPRINGGREEN);
	Display.gfx_MoveTo(10, 100);
	Display.println("COM");
	Display.txt_FGcolour(ALICEBLUE);
	sprintf(TextBuffer, "WiFi %s", wifi.getIPStatus().c_str());
	Display.println(TextBuffer);
	Serial.println(wifi.getIPStatus());

	if (wifi.setOprToStation()) {
		Display.print("Set to station mode OK\r\n");
	}
	else {
		Display.print("Set to station mode ERR\r\n");
	}

	if (wifi.joinAP(SSID, PASSWORD)) {
		Display.print("Joined ");
		Display.println(SSID);
		Display.println(wifi.getLocalIP().c_str());
	}
	else {
		Display.print("Join AP FAIL\r\n");
	}

	//delay(2000);
	//if (wifi.leaveAP())
	//{
	//	Display.print("Left ");
	//	Display.println(SSID);
	//}
	//else
	//{
	//	Display.print("Fail leaving ");
	//	Display.println(SSID);
	//}

//	pinMode(ESPRESET, OUTPUT);        // ESP reset pin
//	digitalWrite(ESPRESET, 1);        // Reset ESP
//	delay(100);                       // wait for it t
//	digitalWrite(ESPRESET, 0);        // Release ESP reset
//
//	delay(3000);											// give USB Serial, 4D display and ESP8266 all time to startup
//
//	ESPserial.begin(115200);         // assume esp set to 115200 baud, it's default setting
//// what we need to do is attempt to flip it to 19200
//// the maximum baud rate at which software serial actually works
//// if we run a program without resetting the ESP it will already be 19200
//// and hence the next command will not be understood or executed
//	ESPserial.println("AT+UART_CUR=19200,8,1,0,0\r\n");
//	ESPserial.end();
//	delay(10);                         // Necessary to allow for baud rate changes
//	ESPserial.begin(19200);            // start again at a resonable baud rate


	//if (wifi.setSoftAPParam("MFCD4Duino", "1234"))
	//{
	//	Display.println("Set MFCD4Duino params OK");
	//}
	//else
	//{
	//	Display.println("Set SoftAP params FAIL");
	//}
	//delay(2000);
	//if (wifi.setOprToSoftAP())
	//{
	//	Display.println("Set SoftAP mode OK");
	//}
	//else
	//{
	//	Display.println("Set SoftAP FAIL");
	//}

	//Display.println(wifi.getAPList());

	//Display.print("IP: ");
	//Display.println(wifi.getLocalIP().c_str());

	//Display.txt_FGcolour(CORNFLOWERBLUE);
	//sprintf(TextBuffer, "WiFi: v %s", wifi.getVersion());
	//Display.println(TextBuffer);

	Display.txt_FGcolour(SPRINGGREEN);
	Display.gfx_MoveTo(10, 180);
	Display.print("DRV");

	Display.gfx_MoveTo(10, 260);
	Display.print("NAV");

	Display.txt_FGcolour(MEDIUMSLATEBLUE);
	Display.txt_MoveCursor(38, 10);
	Display.print("Scratchpad 1\n\rScratchpad 2");

	Display.gfx_Line(1, 9, 238, 9, YELLOWGREEN);
	Display.gfx_Line(1, 18, 238, 18, YELLOWGREEN);

	Tasks_Init();
	Tasks_Add((Task)CheckInputs, 10, 0);
	Tasks_Start();


} // end Setup

void loop()
{
	if (HeadingEncoder.HasRotaryValueChanged(1))
	{
		//Serial.print("Pin " + arduinoInterruptedPin);
		//Serial.println(" " + HeadingEncoder.GetRotaryValue(1));
		//Serial.println(HeadingEncoder.GetRotaryValue(1));
		int HDGDisplayLeft = 10;
		int HDGDisplayWidth = 30;
		int HDGDisplayTop = 300;
		// Right justify display of HDG setting:
		char HDGTextBuffer[4];
		sprintf(HDGTextBuffer, "%03d", HeadingEncoder.GetRotaryValue(1));
		int x = HDGDisplayLeft + HDGDisplayWidth - 3 * Display.charwidth('0');
		Display.gfx_MoveTo(x, HDGDisplayTop);
		Display.txt_FGcolour(SPRINGGREEN);
		Display.print(HDGTextBuffer);

	}
	if (CourseEncoder.HasRotaryValueChanged(1))
	{
		//Serial.print("Pin " + arduinoInterruptedPin);
		//Serial.println(" " + CourseEncoder.GetRotaryValue(1));
		//Serial.println(CourseEncoder.GetRotaryValue(1));
		int CRSDisplayWidth = 30;
		int CRSDisplayLeft = screenWidth - CRSDisplayWidth;
		int CRSDisplayTop = 300;
		// Right justify display of CRS setting:
		char CRSTextBuffer[4];
		sprintf(CRSTextBuffer, "%03d", CourseEncoder.GetRotaryValue(1));
		int x = CRSDisplayLeft + CRSDisplayWidth - 3 * Display.charwidth('0');
		Display.gfx_MoveTo(x, CRSDisplayTop);
		Display.txt_FGcolour(SPRINGGREEN);
		Display.print(CRSTextBuffer);

	}
	if (TimeToCheckInputs)
	{
		int LOSBRaw = analogRead(A0);
		int LOSBRawDisplayLeft = 10;
		int LOSBRawDisplayWidth = 30;
		int LOSBRawDisplayTop = 309;
		char LOSBRawTextBuffer[5];
		sprintf(LOSBRawTextBuffer, "%04d", LOSBRaw);
		int x = LOSBRawDisplayLeft + LOSBRawDisplayWidth - 4 * Display.charwidth('0');
		Display.gfx_MoveTo(x, LOSBRawDisplayTop);
		Display.print(LOSBRawTextBuffer);

		int ROSBRaw = analogRead(A1);
		int ROSBRawDisplayLeft = 200;
		int ROSBRawDisplayWidth = 30;
		int ROSBRawDisplayTop = 309;
		char ROSBRawTextBuffer[5];
		sprintf(ROSBRawTextBuffer, "%04d", ROSBRaw);
		x = ROSBRawDisplayLeft + ROSBRawDisplayWidth - 4 * Display.charwidth('0');
		Display.gfx_MoveTo(x, ROSBRawDisplayTop);
		Display.print(ROSBRawTextBuffer);
		TimeToCheckInputs = false;
	}
}

