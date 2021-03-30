#include "DSMCUArdProArdumoto.h"
#include "DriveSystemPacketDefs.h"

#include <Wire.h>

void DSMCUArdProArdumoto::SetupArdumotoBoard()
{
	pinMode(LMDirPin, OUTPUT);
	pinMode(LMPWMPin, OUTPUT);
	pinMode(RMDirPin, OUTPUT);
	pinMode(RMPWMPin, OUTPUT);

	digitalWrite(LMDirPin, LOW);
	digitalWrite(LMPWMPin, LOW);
	digitalWrite(RMDirPin, LOW);
	digitalWrite(RMPWMPin, LOW);
}

void DSMCUArdProArdumoto::SetMotor(byte motor, byte dir, byte speed)
{
	switch (motor)
	{
	case LeftMotor:
	{
		digitalWrite(LMDirPin, dir);
		analogWrite(LMPWMPin, speed);
		break;
	}

	case RightMotor:
	{
		digitalWrite(RMDirPin, dir);
		analogWrite(RMPWMPin, speed);
		break;
	}

	default:
		break;

	}
}

bool DSMCUArdProArdumoto::TestMotors()
{
	SetMotor(LeftMotor, FORWARD, 127);
	delay(2000);
	SetMotor(LeftMotor, 0, 0);
	delay(500);
	SetMotor(LeftMotor, HIGH, 127);
	delay(2000);
	SetMotor(LeftMotor, 0, 0);
	delay(500);
	SetMotor(RightMotor, FORWARD, 127);
	delay(2000);
	SetMotor(RightMotor, 0, 0);
	delay(500);
	SetMotor(RightMotor, REVERSE, 127);
	delay(2000);
	SetMotor(RightMotor, 0, 0);
	return false;
}

bool DSMCUArdProArdumoto::TestDisplay()
{
	Wire.beginTransmission(I2C_ADDRESS_SA0_1);
	if (Wire.endTransmission(0) == 0)
	{
		uint8_t screenWidth = oled.getLCDWidth();
		uint8_t screenHeight = oled.getLCDHeight();
		oled.setCursor(0, 10);
		oled.print("WxH: " + String(screenWidth) + "x" + String(screenHeight));
		oled.display();
		DisplayPresent = true;
	}
	else
	{
		DisplayPresent = false;
	}
	return DisplayPresent;
}

void DSMCUArdProArdumoto::ExecuteCommand(uint8_t command, uint8_t* commandPayload)
{
	bool commandHandled = false;
	char CmdReportText[10];
	sprintf(CmdReportText, "CMD: %2X", command);
	oled.setCursor(0, 40);
	oled.print(CmdReportText);
	oled.display();

	switch (command)
	{
	case ArduinoControllerPacketTypes::MRSTextMessage:
	{
		// Display the incoming text message locally (and/or log it):
		String msg = String((char*)commandPayload);
		oled.setCursor(0, 30);
		oled.print(msg);
		oled.display();
		SendTextMessage("This is a test...");

		commandHandled = true;
	}
	break;

	case ArduinoControllerPacketTypes::DSMCUSetMotors:
	{
		DSCommandPacket* DSCP = (DSCommandPacket*)commandPayload;
		int16_t speed = DSCP->Kinematics.Speed;
		// This works:
		//DSCommandPacket DSCP;
		//for (int i = 0; i < sizeof(commandPayload); ++i)
		//{
		//	DSCP.packet[i] = commandPayload[i];
		//}
		//int16_t speed = DSCP.Kinematics.Speed;
		// So does this:
		//int16_t speed = (((int16_t)commandPayload[0x01]) << 8) + (int16_t)(commandPayload[0x00]);
		// This works, too:
		//byte raw[2] = { commandPayload[0x00], commandPayload[0x01] };
		//int16_t speed = *((int16_t*)raw);

		// Build and send Drive System Status Packet:
		DSStatusPacket DSSP;
		DSSP.Measurements.VSupply = VBat5Raw;
		DSSP.Measurements.LMotorCurrent = 0;
		DSSP.Measurements.RMotorCurrent = 0;
		DSSP.Measurements.LActuatorSpeed = speed;
		DSSP.Measurements.RActuatorSpeed = speed;
		ClientConnection.outPacket[0] = ArduinoControllerPacketTypes::DSMCUStatusPacket;
		for (int i = 0; i < sizeof(DSSP); ++i)
		{
			ClientConnection.outPacketPayload[i] = *((byte*)(&DSSP) + i);
		}
		ClientConnection.SendPacket();

		oled.setCursor(0, 20);
		oled.print("Speed: " + String(speed));
		oled.display();

		byte direction = FORWARD;
		if (speed < 0)
		{
			direction = REVERSE;
			speed = abs(speed);
		}
		SetMotor(LeftMotor, direction, speed);
		SetMotor(RightMotor, direction, speed);

		commandHandled = true;
	}
	break;

	case ArduinoControllerPacketTypes::RunSystemDiagnostics:
	{
		// Run demo / test of local display hardware:
		if (TestDisplay())
		{
			ArduinoControllerBase::SendTextMessage("Local display OK");
		}
		else
		{
			ArduinoControllerBase::SendTextMessage("Local display FAIL");
		}

		// Run demo / test of local display hardware:
		if (TestMotors())
		{
			ArduinoControllerBase::SendTextMessage("Motor test complete");
		}
		else
		{
			ArduinoControllerBase::SendTextMessage("Motor test FAIL");
		}

		commandHandled = true;
	}
	break;

	case ArduinoControllerPacketTypes::TestLocalDisplay:
	{
		// Run demo / test of local display hardware:
		if (TestDisplay())
		{
			ArduinoControllerBase::SendTextMessage("Local display OK");
		}
		else
		{
			ArduinoControllerBase::SendTextMessage("Local display FAIL");
		}
		commandHandled = true;
	}
	break;

	case ArduinoControllerPacketTypes::TestMotors:
	{
		TestMotors();
		ArduinoControllerBase::SendTextMessage("Motor test complete");
		commandHandled = true;
	}
	break;

	default:
		break;
	}

	if (!commandHandled)
	{
		ArduinoControllerBase::ExecuteCommand(command, commandPayload);
	}
}

void DSMCUArdProArdumoto::GetStatusReport()
{
	ArduinoControllerBase::GetStatusReport();
}

void DSMCUArdProArdumoto::Init(PacketSerial::PacketHandlerFunction OnSerialClientMessage)
{
	ArduinoControllerBase::Init(SerialClient::SerialOverUSB, OnSerialClientMessage);
	SetupArdumotoBoard();

	Wire.begin();
	delay(100);
	if (TestDisplay())
	{
		oled.begin();			// Initialize the OLED
		oled.clear(PAGE); // Clear the display's internal memory
		oled.display();	  // Display what's in the buffer (splashscreen)
		DisplayPresent = true;
	}
}

void DSMCUArdProArdumoto::Update()
{
	ArduinoControllerBase::Update();
	// Read and display battery supply voltage
	VBat5Raw = analogRead(VMotorPin);
	oled.setCursor(0, 0);
	oled.print("VBat5: " + String(VBat5Raw));
	oled.display();
}
