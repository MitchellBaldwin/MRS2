#pragma once
#include <ArduinoControllerBase.h>

#include <SFE_MicroOLED.h>

// The SFE_MicroOLED library assumes a reset pin is necessary. 
// The Qwiic OLED has RST hard-wired, so pick an arbitrarty IO pin that is NOT BEING USED
#define PIN_RESET 9  

// The DC_JUMPER is the I2C Address Select jumper. 
// Set to 1 if the jumper is open (Default), or set to 0 if it's closed.
#define DC_JUMPER 1

class DSNCUArdProArdumoto :
    public ArduinoControllerBase
{
private:

protected:
	MicroOLED oled = MicroOLED(PIN_RESET, DC_JUMPER);
	//bool TestMotors();
	//bool TestDisplay();
	//virtual void ExecuteCommand(uint8_t);
	//virtual void ExecuteCommand(uint8_t, uint8_t*);

public:
	void Init(PacketSerial::PacketHandlerFunction);
	void Update();

};

