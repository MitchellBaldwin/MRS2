#pragma once
#include <ArduinoControllerBase.h>

#include <SFE_MicroOLED.h>

// The SFE_MicroOLED library assumes a reset pin is necessary. 
// The Qwiic OLED has RST hard-wired, so pick an arbitrarty IO pin that is NOT BEING USED
#define PIN_RESET 9  

// The DC_JUMPER is the I2C Address Select jumper. 
// Set to 1 if the jumper is open (Default), or set to 0 if it's closed.
#define DC_JUMPER 1

constexpr auto FORWARD = 0;
constexpr auto REVERSE = 1;

constexpr auto LeftMotor = 0;
constexpr auto RightMotor = 1;

const byte LMDirPin = 12;
const byte LMPWMPin = 3;
const byte RMDirPin = 13;
const byte RMPWMPin = 11;

constexpr byte VMotorPin = A0;

class DSMCUArdProArdumoto : public ArduinoControllerBase
{
private:
	void SetupArdumotoBoard();
	//void StopMotor(byte);
	//void StopBothMotors();
	void SetMotor(byte, byte, byte);
	bool DisplayPresent = false;
	uint16_t VBat5Raw;

protected:
	MicroOLED oled = MicroOLED(PIN_RESET, DC_JUMPER);
	bool TestMotors();
	bool TestDisplay();
	//virtual bool TestLocalDisplay();
	//virtual void ExecuteCommand(uint8_t);
	virtual void ExecuteCommand(uint8_t, uint8_t*);
	virtual void GetStatusReport();

public:
	void Init(PacketSerial::PacketHandlerFunction);
	void Update();

};

