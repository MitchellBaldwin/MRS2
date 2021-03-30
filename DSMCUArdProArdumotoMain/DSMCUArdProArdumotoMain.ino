/*
 Name:		DSMCUArdProArdumotoMain.ino
 Created:	3/29/2021 11:22:22 AM
 Author:	Mitchell Baldwin

 Arduino Pro (5V, 16MHz) MCU with Ardumoto (v1.1) motor driver shield and SF QWIIC uOLED
 "Drive System MCU" (DSMCU) module mounted on a Rover 5 chassis:
 - acts as the Master Communications Controller (MCC) as a PacketSerial server until a 
 dedicated MCC is added
 - change to function as SPI slave once a dedicated MCC is added

 
 
 
 */

#include <Tasks.h>
#include "src\DSMCUArdProArdumoto.h"

DSMCUArdProArdumoto R5DSMCU;

bool TimeToUpdateR5DSMCU = false;

// Local function needed to wrap the class instance method that will be called by the task scheduler:
void UpdateR5DSMCU()
{
	TimeToUpdateR5DSMCU = true;;
}

void setup() 
{
	R5DSMCU.Init(&OnSerialClientMessage);
	Tasks_Init();
	Tasks_Add((Task)UpdateR5DSMCU, 10, 0);
	Tasks_Start();
}

void loop() 
{
	if (TimeToUpdateR5DSMCU)
	{
		R5DSMCU.Update();
		TimeToUpdateR5DSMCU = false;
	}
	//delay(10);
}

void OnSerialClientMessage(const uint8_t* buffer, size_t size)
{
	R5DSMCU.ProcessMessages(buffer, size);
}
