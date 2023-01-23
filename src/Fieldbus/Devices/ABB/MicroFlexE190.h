#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Utilities/ScrollingBuffer.h"

#include "Fieldbus/Utilities/DS402Axis.h"
#include "Project/Editor/Parameter.h"

class MicroFlex_e190 : public EtherCatDevice {
public:

    DEFINE_ETHERCAT_DEVICE(MicroFlex_e190, "MicroFlex e190", "MicroFlex_e190", "ABB", "Servo Drives", 0xB7, 0x2C1)
	
	//Process data
	std::shared_ptr<DS402Axis> axis;
	uint32_t digitalInputs;
	uint32_t digitalOutputs;
	int16_t AI0;
	int16_t AO0;
	
	class MicroFlexServoMotor : public ServoActuatorDevice{
	public:
		MicroFlexServoMotor(std::shared_ptr<MicroFlex_e190> microflex) :
		MotionDevice(Units::AngularDistance::Revolution),
		ServoActuatorDevice(Units::AngularDistance::Revolution, PositionFeedbackType::INCREMENTAL),
		drive(microflex){
			setParentDevice(microflex);
		}
		
		virtual std::string getName() override { return std::string(drive->getName()) + " Servo Motor"; };
		
		virtual std::string getStatusString() override { return drive->getStatusString(); }
		std::shared_ptr<MicroFlex_e190> drive;
		
		virtual bool canHardReset() override { return true; }
		virtual void executeHardReset() override { drive->b_resetEncoder = true; }
		virtual bool isExecutingHardReset() override { return drive->b_encoderResetBusy; }
	};
	
	class MicroFlexGpio : public GpioDevice{
	public:
		MicroFlexGpio(std::shared_ptr<MicroFlex_e190> microflex) :
		GpioDevice(),
		drive(microflex){
			setParentDevice(microflex);
		}
		
		virtual std::string getName() override { return std::string(drive->getName()) + " GPIO"; }
		
		virtual std::string getStatusString() override { return drive->getStatusString(); }
		std::shared_ptr<MicroFlex_e190> drive;
	};
	
	std::shared_ptr<MicroFlexServoMotor> servo;
	std::shared_ptr<NodePin> servoPin = std::make_shared<NodePin>(NodePin::DataType::SERVO_ACTUATOR, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Servo Motor");
	
	std::shared_ptr<MicroFlexGpio> gpio;
	std::shared_ptr<NodePin> gpioPin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Gpio");
	
	std::shared_ptr<double> position_Value = std::make_shared<double>(0.0);
	std::shared_ptr<double> velocity_Value = std::make_shared<double>(0.0);
	std::shared_ptr<double> load_Value = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> position_Pin = std::make_shared<NodePin>(position_Value, NodePin::Direction::NODE_OUTPUT, "Position", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> velocity_Pin = std::make_shared<NodePin>(velocity_Value, NodePin::Direction::NODE_OUTPUT, "Velocity", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> load_Pin = std::make_shared<NodePin>(load_Value, NodePin::Direction::NODE_OUTPUT, "Load", NodePin::Flags::DisableDataField);
	
	std::shared_ptr<bool> digitalIn0_Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn1_Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn2_Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn3_Value = std::make_shared<bool>(false);
	std::shared_ptr<NodePin> digitalIn0_Pin = std::make_shared<NodePin>(digitalIn0_Value, NodePin::Direction::NODE_OUTPUT, "Digital Input 0", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalIn1_Pin = std::make_shared<NodePin>(digitalIn1_Value, NodePin::Direction::NODE_OUTPUT, "Digital Input 1", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalIn2_Pin = std::make_shared<NodePin>(digitalIn2_Value, NodePin::Direction::NODE_OUTPUT, "Digital Input 2", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalIn3_Pin = std::make_shared<NodePin>(digitalIn3_Value, NodePin::Direction::NODE_OUTPUT, "Digital Input 3", NodePin::Flags::DisableDataField);
	
	std::shared_ptr<bool> digitalOut0_Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalOut1_Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalOut2_Value = std::make_shared<bool>(false);
	std::shared_ptr<NodePin> digitalOut0_Pin = std::make_shared<NodePin>(digitalOut0_Value, NodePin::Direction::NODE_INPUT, "Digital Output 0");
	std::shared_ptr<NodePin> digitalOut1_Pin = std::make_shared<NodePin>(digitalOut1_Value, NodePin::Direction::NODE_INPUT, "Digital Output 1");
	std::shared_ptr<NodePin> digitalOut2_Pin = std::make_shared<NodePin>(digitalOut2_Value, NodePin::Direction::NODE_INPUT, "Digital Output 2");
	
	std::shared_ptr<double> analogIn0_Value = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> analogIn0_Pin = std::make_shared<NodePin>(analogIn0_Value, NodePin::Direction::NODE_OUTPUT, "Analog Input 0", NodePin::Flags::DisableDataField);
	
	std::shared_ptr<double> analogOut0_Value = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> analogOut0_Pin = std::make_shared<NodePin>(analogOut0_Value, NodePin::Direction::NODE_INPUT, "Analog Output 0");
	
	//Parameters
	NumberParam<double> velocityLimit_parameter = NumberParameter<double>::make(10.0, "Velocity Limit", "VelocityLimit", "%.1f",
																				Units::AngularDistance::Revolution, false, 0, 0, "", "/s");
	NumberParam<double> accelerationLimit_parameter = NumberParameter<double>::make(10.0, "Acceleration Limit", "AccelerationLimit", "%.1f",
																					Units::AngularDistance::Revolution, false, 0, 0, "", "/s\xc2\xb2");
	BoolParam  invertMotor_parameter = BooleanParameter::make(false, "Invert Direction", "InvertDirection");
	NumberParam<double> currentLimit_parameter = NumberParameter<double>::make(100.0, "Max Current", "MaxCurrent", "%.1f",
																			   Units::Fraction::Percent, false);
	NumberParam<double> maxFollowingError_parameter = NumberParameter<double>::make(1.0, "Max Following Error", "MaxFollowingError", "%.1f",
																					Units::AngularDistance::Revolution, false);
	
	ParameterGroup axisParameters = ParameterGroup("Axis",{
		velocityLimit_parameter,
		accelerationLimit_parameter,
		invertMotor_parameter,
		currentLimit_parameter,
		maxFollowingError_parameter
	});
	
	//for error logging
	uint16_t previousErrorCode = 0x0;
	
	//enable request
	bool b_waitingForEnable = false;
	long long enableRequestTime_nanoseconds;
	
	//encoder reset (DS402 Homing mode)
	bool b_resetEncoder = false;
	bool b_encoderResetBusy = false;
	
	double actualPositionFollowingError = 0.0;
	

	//———— Drive Unit Conversion
	
	//Position Unit: Revolution
	double incrementsPerPositionUnit = 10000;
	//Velocity Unit: Revolution per second
	double incrementsPerVelocityUnit = 10000;
	//Acceleration Unit: Revolution per second squared
	double incrementsPerAccelerationUnit = 10000;
	//Torque Unit: Newton Meter
	double incrementsPerTorqueUnit = 10000;
		
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
	
	std::string getStatusString(){
		std::string status;
		if(!isConnected()) {
			status = "Device is Offline.\n";
			return status;
		}
		if(servo->isEmergencyStopped()) status += "STO is Active.\n";
		if(axis->hasFault()) status += "Fault : " + std::string(getErrorCodeString()) + " (Fault will be cleared when enabling)\n";
		return status;
	}
	
	
	
	
	
	
	
	void controlTab();
	void settingsTab();
	
	double profiler_velocity = 0.0;
	double profiler_position = 0.0;
	float manualVelocityTarget = 0.0;
	
	void updateServoConfiguration();
	
	
	
	
	
};

//0x5062 : input pin function assignement (int16)
//values: -1 (function disabled) or 0-3 (input pin number)
//5062.1 error input
//5062.2 home input
//5062.3 limit forward input
//5062.4 limit reverse input
//5062.5 motor temperature input
//5062.6 phase search input
//5062.7 reset input
//5062.8 stop input
//5062.9 suspend input
//5062.A power ready input

//0x5061 : outpt pin function assignement (int16)
//values: -1 (function disabled) or 1-2 (output pin number)
//5061.1 drive enabled output
//5061.2 motor brake output
//5061.3 phase search output
//5061.4 power ready output

//0x4035.1 int32 Encoder Multiturn Resolution
//0x4036.1 int32 Encoder Singleturn Resolution ???

//status word:
//b10 : Target Reached (quickstop, halt ???)
//b12 :	homing mode 1 = homing attained
//		cyclic modes 0 = target ignored
//b13 : homing mode 1 = homing error
//		cyclic position 1 = following error

//control word:
//b4 : 	Homing = Start homing
//b11 : reset warnings

//unclear:
//6410.13 motor rated speed rpm
//5027.1 int32 drive current ???


///When comissionning the drive through Mint Workbench, make sure the mint program is completely disabled or commented out
///It generated weird "incrorrect reference source" and "motion aborted" errors that were very unpredictable
