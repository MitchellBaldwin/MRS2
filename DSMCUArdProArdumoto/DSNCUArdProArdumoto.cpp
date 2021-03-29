#include "DSNCUArdProArdumoto.h"

void DSNCUArdProArdumoto::Init(PacketSerial::PacketHandlerFunction OnSerialClientMessage)
{
	ArduinoControllerBase::Init(SerialClient::SerialOverUSB, OnSerialClientMessage);

}

void DSNCUArdProArdumoto::Update()
{
	ArduinoControllerBase::Update();

}
