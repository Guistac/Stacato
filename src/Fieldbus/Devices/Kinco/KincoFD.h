#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/DS402Axis.h"

#include "Project/Editor/Parameter.h"

class KincoFD : public EtherCatDevice {
public:
	
	DEFINE_ETHERCAT_DEVICE(KincoFD, "Kinco FD", "KincoFD", "Kinco", "Servo Drives", 0x681168, 0x464445)

	virtual void onDetection() override;
	
	class KincoServoMotor : public ActuatorInterface{
	public:
		virtual std::string getName() override { return "Kinco Servo Motor"; }
		virtual std::string getStatusString() override { return "No Status String available"; }
	};
	
	std::shared_ptr<DS402Axis> axis;
	std::shared_ptr<KincoServoMotor> actuator;
	
	std::shared_ptr<NodePin> actuatorPin;
	
	const float incrementsPerRevolutionPerSecond = 1073741.824;
	const float incrementsPerRevolutionPerSecondSquared = 1073.741824;
	const float incrementsPerRevolution = 65536.0;
	const float incrementsPerAmpere = 136.5;
	
	//0x2010:A (U16) Din Real
	//0x60FD:0 (U32) digital inputs
	//0x2601:0 (U16) error status
	
	//TODO:
	//read digital Inputs
	//write digital Outputs
	//auto fault clearing on enable
	//fault reporting
	
	
	//----- Option and Enumerators
	
	enum InputFunction{
		NO_FUNCTION				= 0x0000,
		DRIVER_FAULT_RESET		= 0x0002,
		POSITION_POSITIVE_LIMIT = 0x0010,
		POSITION_NEGATIVE_LIMIT = 0x0020,
		QUICK_STOP 				= 0x1000
	};
	Option inputFunction_none = 					Option(InputFunction::NO_FUNCTION, 				"No Function", 				"None");
	Option inputFunction_DriverFaultReset = 		Option(InputFunction::DRIVER_FAULT_RESET, 		"Driver Fault Reset",		"DriverFaultReset");
	Option inputFunction_PositionPositiveLimit = 	Option(InputFunction::POSITION_POSITIVE_LIMIT, 	"Position Positive Limit",	"PositionPositiveLimit");
	Option inputFunction_PositionNegativeLimit = 	Option(InputFunction::POSITION_NEGATIVE_LIMIT, 	"Position Negative Limit",	"PositionNegativeLimit");
	Option inputFunction_Quickstop = 				Option(InputFunction::QUICK_STOP, 				"Quickstop",				"Quickstop");
	std::vector<OptionParameter::Option*> inputFunctionOptions = {
		&inputFunction_none,
		&inputFunction_DriverFaultReset,
		&inputFunction_PositionPositiveLimit,
		&inputFunction_PositionNegativeLimit,
		&inputFunction_Quickstop
	};
	
	//----- Parameters
	
	float driveCurrentLimit = 0.0;							//0x60F6:F CMQ_q_Limit current limit (in ampere increments)
	NumberParam<double> maxCurrent_parameter;				//0x6073:0 CMD_q_Max max current (in ampere increments)
	NumberParam<double> maxVelocity_parameter;				//no drive parameter?
	NumberParam<double> maxAcceleration_parameter;			//0x6083:0 & 0x6084:0 profile acceleration and deceleration (in acceleration increments)
	NumberParam<double> velocityFeedforward_parameter;		//0x60FB:02 K_Velocity_FF (in %, 256 == 100%)
	NumberParam<double> maxFollowingError_parameter;		//0x6065:00 Max_Following_Error (in position increments)
	NumberParam<int> followingErrorTimeout_parameter;		//0x6066:00 Position_Window_Time (in ms)
	NumberParam<int> brakingResistorResistance_parameter;	//0x60F7:1 Chop_Resistor (in Ohm)
	NumberParam<int> brakingResistorPower_parameter;		//0x60F7:2 Chop_Resistor_Rated (in Watt)
	
	OptionParam DIN1Function_parameter;	//2010:3 (u16)
	OptionParam DIN2Function_parameter;	//2010:4 (u16)
	OptionParam DIN3Function_parameter;	//2010:5 (u16)
	OptionParam DIN4Function_parameter;	//2010:6 (u16)
	OptionParam DIN5Function_parameter;	//2010:7 (u16)
	OptionParam DIN6Function_parameter;	//2010:8 (u16)
	OptionParam DIN7Function_parameter;	//2010:9 (u16)
	
	void uploadConfiguration();
	
	void updateActuatorInterface();
	
	bool startAutoTuning();
	bool b_isAutotuning = false;
	
};
