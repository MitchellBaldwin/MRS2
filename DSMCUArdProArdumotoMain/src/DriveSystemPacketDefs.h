#pragma once

#include "ArduinoControllerDefs.h"

union DSCommandPacket
{
	struct
	{
		int16_t Speed;
		int16_t TurnRate;
		uint16_t STThetaXY;
		uint16_t STThetaZX;
	} Kinematics;
	byte packet[PACKET_PAYLOAD_SIZE];	// DONE: Change this to use globally defined length of command packet
};

union DSStatusPacket
{
	struct
	{
		uint16_t VSupply;
		int16_t LMotorCurrent;
		int16_t RMotorCurrent;
		int16_t LActuatorSpeed;
		int16_t RActuatorSpeed;
	} Measurements;
	byte packet[PACKET_PAYLOAD_SIZE];
};