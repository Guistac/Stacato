#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Utilities/ScrollingBuffer.h"

#include "Fieldbus/Utilities/DS402Axis.h"

class MicroFlex_e190 : public EtherCatDevice {
public:

    DEFINE_ETHERCAT_DEVICE(MicroFlex_e190, "MicroFlex e190", "MicroFlex_e190", "ABB", "Servo Drives", 0xB7, 0x2C1)
	
	//Process data
	std::shared_ptr<DS402Axis> axis;
	int16_t AI0;
	
	//TODO: stuff
	//figure out position velocity acceleration units
	//figure out why we get "Motion Aborted" on first startup
	//figure out how to auto clear STO state
	
	
	//0x5062 : input pin function assignement (int16)
	//values: -1 (function disabled) or 0-3 (input pin number)
	//.1 error input
	//.2 home input
	//.3 limit forward input
	//.4 limit reverse input
	//.5 motor temperature input
	//.6 phase search input
	//.7 reset input
	//.8 stop input
	//.9 suspend input
	//.A power ready input
	
	//0x5061 : outpt pin function assignement (int16)
	//values: -1 (function disabled) or 1-2 (output pin number)
	//.1 drive enabled output
	//.2 motor brake output
	//.3 phase search output
	//.4 power ready output
	
	//0x4035.1 int32 Encoder Multiturn Resolution
	//0x4036.1 int32 Encoder Singleturn Resolution ???
	
	const char* getErrorCodeString(){
		switch(axis->getErrorCode()){
			case 0x0000: return "No Error";
			case 0x2310: return "Over current";
			case 0x2350: return "Drive overload";
			case 0x3110: return "Bus overvoltage";
			case 0x3120: return "Bus undervoltage";
			case 0x3130: return "Supply phase loss";
			case 0x4210: return "Motor temperature input";
			case 0x4310: return "Drive overtemperature";
			case 0x4320: return "Drive undertemperature";
			case 0x5110: return "Internal power supply loss";
			case 0x5114: return "Encoder supply lost";
			case 0x5400: return "Power base not ready";
			case 0x5410: return "Power module fault";
			case 0x5441: return "Drive enabled input inactive";
			case 0x5442: return "Forward hardware limit";
			case 0x5443: return "Reverse hardware limit";
			case 0x5444: return "Safe torque off is active";
			case 0x5445: return "Error input active";
			case 0x7303: return "Resolver signals lost or incorrect";
			case 0x7305: return "Encoder signals lost";
			case 0x7310: return "Over speed";
			case 0x7500: return "PDO data Lost";
			case 0x8400: return "Fatal velocity exceeded";
			case 0x8611: return "Following error";
			case 0x8612: return "Forward or reverse soft limit hit";
			case 0xFF00: return "Phase search failed";
			case 0xFF01: return "Heatsink too hot to phase search";
			case 0xFF02: return "Encoder not ready";
			case 0xFF03: return "Motor overload";
			case 0xFF04: return "Production data not valid";
			case 0xFF05: return "Motion aborted";
			case 0xFF06: return "Safe torque off hardware is faulty";
			case 0xFF07: return "Safe torque off inputs not same level";
			case 0xFF08: return "Internal API error";
			case 0xFF09: return "Encoder reading wrong";
			case 0xFF0A: return "Axis has reached following error warning";
			case 0xFF0B: return "Encoder battery dead";
			case 0xFF0C: return "Encoder battery low";
			case 0xFF0D: return "The DSL encoder is reporting an error";
			case 0xFF0E: return "Drive output frequency limit exceeded";
			case 0xFF0F: return "Phase loss detected";
			case 0xFF10: return "Motor temperature has not been read";
			default: return "Unknown Error";
		}
	}
	
	
	
	
	
	
	
	void controlTab();
	
	double axisUnitsPerPos = 100.0;
	double axisUnitsPerVel = 100.0;
	double acceleration = 100.0;
	double velocity = 0.0;
	double position = 0.0;
	double maxVelocity = 190.0;
	float manualVelocity = 0.0;
	
	
	
	
	
};
