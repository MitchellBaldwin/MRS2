/*
 Name:		KY_040_Test.ino
 Created:	4/7/2021 11:43:03 AM
 Author:	Mitchell Baldwin

 Testing Bill Williams' KY-040 library using Arduino Micro
 Pin 7 is an interrupt pin by default on the Micro, so initially use pin 7 for the CLK input from the KY-040

 Testing GreyGnome's EnableInterrupt library to investigate its usefulness enabling other pins to function
 as interrupt pins to support additional KY-040 rotary encoders

*/

//#define EI_NOTINT0
//#define EI_NOTINT1
//#define EI_NOTINT2
//#define EI_NOTINT3
//#define EI_NOTINT7
#define LIBCALL_ENABLEINTERRUPT
#define EI_ARDUINO_INTERRUPTED_PIN
#include <EnableInterrupt.h>

#include <BuiltinLED.h>
#include <ky-040.h>

constexpr auto RXLED = LED_BUILTIN_RX;
constexpr auto TXLED = LED_BUILTIN_TX;

constexpr auto HDGENCCLK = 7;
constexpr auto HDGENCDT = 8;
constexpr auto HDGENCPB = 9;

ky040 HeadingEncoder(HDGENCCLK, HDGENCDT, HDGENCPB, 1);

void setup() {
  
  Serial.begin(115200);
  while (!Serial);

  pinMode(RXLED, OUTPUT);
  pinMode(TXLED, OUTPUT);
  digitalWrite(RXLED, HIGH);
  digitalWrite(TXLED, HIGH);

  HeadingEncoder.AddRotaryCounter(1, 0, 0, 359, 1, true);
  HeadingEncoder.SetRotary(1);

}

void loop() {
  
  if (HeadingEncoder.HasRotaryValueChanged(1))
  {
    //Serial.print("Pin " + arduinoInterruptedPin);
    //Serial.println(" " + HeadingEncoder.GetRotaryValue(1));
    Serial.println(HeadingEncoder.GetRotaryValue(1));
  }

  if (HeadingEncoder.SwitchPressed())
  {
    digitalWrite(RXLED, !digitalRead(RXLED));
  }
}
