#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/DS402Axis.h"

class ATV340 : public EtherCatDevice {
public:

	DEFINE_ETHERCAT_DEVICE(ATV340, "ATV340", "ATV340", "Schneider Electric", "Servo Drives", 0x800005A, 0x12C)
	
	//Process Data
	std::shared_ptr<DS402Axis> axis;
	uint16_t motorPower;
	uint16_t logicInputs;
	uint16_t stoState;
	uint16_t lastFaultCode;
	uint16_t logicOutputs;
	int16_t analogInput1;
	
	bool digitalOut1 = false;
	bool digitalOut2 = false;
	bool relayOut1 = false;
	bool relayOut2 = false;
	
	bool configureMotor();
	
	const char* getErrorCodeString(){
		switch(lastFaultCode){
			case 0: return "No error detected";
			case 2: return "Control memory error";
			case 3: return "Incorrect configuration";
			case 4: return "Invalid configuration";
			case 5: return "Modbus communication interruption";
			case 6: return "Internal communication interruption with option module";
			case 7: return "Fieldbus communication interruption";
			case 8: return "External detected error";
			case 9: return "Overcurrent";
			case 10: return "Precharge capacitor";
			case 11: return "Encoder feedback loss";
			case 12: return "Load slipping";
			case 15: return "Input Overheating";
			case 16: return "Device overheating";
			case 17: return "Motor overload";
			case 18: return "DC bus overvoltage";
			case 19: return "Supply mains overvoltage";
			case 20: return "Single output phase loss";
			case 21: return "Input phase loss";
			case 22: return "Supply mains undervoltage";
			case 23: return "Motor short circuit";
			case 24: return "Motor overspeed";
			case 25: return "Autotuning detected error";
			case 26: return "Internal error 1 (Rating)";
			case 27: return "Internal error 2 (Soft)";
			case 28: return "Internal error 3 (Intern Comm)";
			case 29: return "Internal error 4 (Manufacturing)";
			case 30: return "Power memory error";
			case 32: return "Ground short circuit";
			case 33: return "Output phase loss";
			case 34: return "CANopen communication interruption";
			case 35: return "Brake control";
			case 37: return "Internal error 7 (Init)";
			case 38: return "External error detected by Fieldbus";
			case 40: return "Internal error 8 (Switching Supply)";
			case 41: return "Brake feedback";
			case 42: return "PC communication interruption";
			case 43: return "Encoder coupling";
			case 44: return "Torque limitation error";
			case 45: return "HMI communication interruption";
			case 51: return "Internal error 9 (Measure)";
			case 52: return "Internal error 10 (Mains)";
			case 53: return "Internal error 11 (Temperature)";
			case 54: return "IGBT overheating";
			case 55: return "IGBT short circuit";
			case 56: return "Motor short circuit";
			case 57: return "Torque timeout";
			case 58: return "Output contactor closed error";
			case 59: return "Output contactor opened error";
			case 60: return "Internal error 12 (Internal current supply)";
			case 62: return "Encoder";
			case 64: return "input contactor";
			case 65: return "DB unit sh. circuit";
			case 68: return "Internal error 6 (Option)";
			case 69: return "Internal error 14 (CPU)";
			case 70: return "Braking resistor overload";
			case 71: return "AI3 4-20mA loss";
			case 72: return "AI4 4-20mA loss";
			case 73: return "Boards compatibility";
			case 76: return "Dynamic load detected error";
			case 77: return "Configuration transfer error";
			case 79: return "AI5 4-20 mA loss";
			case 82: return "ON lock error";
			case 99: return "Channel switching detected error";
			case 100: return "Process underload";
			case 101: return "Process overload";
			case 105: return "Angle error";
			case 106: return "AI1 4-20 mA loss";
			case 107: return "Safety function detected error";
			case 112: return "AI3 thermal level error";
			case 113: return "Thermal sensor error on AI3";
			case 120: return "AI4 thermal level error";
			case 121: return "Thermal sensor error on AI4";
			case 122: return "AI5 thermal level error";
			case 123: return "Thermal sensor error on AI5";
			case 128: return "Program loading detected error";
			case 129: return "Program running detected error";
			case 142: return "Internal error 16 (IO module - relay)";
			case 143: return "Internal error 17 (IO module - Standard)";
			case 144: return "Internal error 0 (IPC)";
			case 146: return "Internal error 13 (Diff current)";
			case 148: return "Motor stall detected error";
			case 149: return "Internal error 21 (RTC)";
			case 150: return "Embedded Ethernet communication interruption";
			case 151: return "Internal error 15 (Flash)";
			case 152: return "Firmware Update error";
			case 153: return "Internal error 22 (Embedded Ethernet)";
			case 154: return "Internal error 25 (Incompatibility CB & SW)";
			case 155: return "Internal error 20 (option interface PCBA)";
			case 156: return "Internal error 19 (Encoder module)";
			case 157: return "Internal error 27 (Diagnostics CPLD)";
			case 158: return "Internal error 23 (Module link)";
			case 159: return "AFE ShortCircuit error";
			case 160: return "AFE Bus unbalancing";
			case 161: return "Internal error 28 (AFE)";
			case 162: return "Monitoring circuit A error";
			case 163: return "Monitoring circuit B error";
			case 164: return "Monitoring circuit C error";
			case 165: return "Monitoring circuit D error";
			case 173: return "Cabinet overheat  error";
			case 175: return "Internal error 31 (Missing brick)";
			case 176: return "Internal error 29 (Inverter)";
			case 177: return "Internal error 30 (Rectifier)";
			case 179: return "AFE IGBT over-heat error";
			case 180: return "AFE contactor feedback error";
			case 181: return "Pre-settings transfer error";
			case 182: return "Circuit breaker error";
			case 184: return "DB unit open circuit";
			case 185: return "Drive overload";
			case 186: return "MultiDrive Link error";
			case 187: return "AI1 thermal level error";
			case 188: return "Thermal sensor error on AI1";
			case 189: return "Backlash error";
			case 190: return "M/S device error";
			case 194: return "Encoder module thermal level error";
			case 195: return "Thermal sensor error on encoder module";
			case 196: return "Empty configuration";
			case 200: return "FDR Eth embedded error";
			case 201: return "FDR Eth module error";
			case 206: return "DC Bus ripple error";
			case 208: return "Idle mode exit error";
			case 209: return "Load movement error";
			case 211: return "Security files corrupt";
			case 220: return "Advanced function locked error";
			default: return "unknown error";
		}
	}
	
};
