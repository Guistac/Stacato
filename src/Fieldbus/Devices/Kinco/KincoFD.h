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
		virtual std::string getName() override { return drive->getName() + std::string(" Servo Motor"); }
		virtual std::string getStatusString() override;
		std::shared_ptr<KincoFD> drive = nullptr;
	};
	
	std::shared_ptr<DS402Axis> axis;
	std::shared_ptr<KincoServoMotor> actuator;
	
	std::shared_ptr<NodePin> actuatorPin;
	
	const float incrementsPerRevolutionPerSecond = 1073741.824;
	const float incrementsPerRevolutionPerSecondSquared = 1073.741824;
	const float incrementsPerRevolution = 65536.0;
	const float incrementsPerAmpere = 136.5;
	
	uint64_t enableRequestTime_nanos = 0;
	bool b_waitingForEnable = false;
	int32_t positionOffset = 0;
	uint16_t previousErrorCode = 0;
	
	std::string getErrorCodeString();
	
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
		NO_INPUT_FUNCTION		= 0x0000,
		DRIVER_FAULT_RESET		= 0x0002,
		POSITION_POSITIVE_LIMIT = 0x0010,
		POSITION_NEGATIVE_LIMIT = 0x0020,
		QUICK_STOP 				= 0x1000
	};
	Option inputFunction_none = 					Option(InputFunction::NO_INPUT_FUNCTION, 		"No Function", 				"None");
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
	
	enum OutputFunction{
		NO_OUTPUT_FUNCTION		= 0x0000,
		DRIVER_READY			= 0x0001,
		DRIVER_ERROR			= 0x0002,
		MOTOR_BRAKE				= 0x0010,
		DRIVER_ENABLED			= 0x0100
	};
	Option outputFunction_none = 			Option(OutputFunction::NO_OUTPUT_FUNCTION,	"No Function", 		"None");
	Option outputFunction_driverReady = 	Option(OutputFunction::DRIVER_READY, 		"Drive Ready", 		"DriveReady");
	Option outputFunction_driverError = 	Option(OutputFunction::DRIVER_ERROR, 		"Drive Error", 		"DriveError");
	Option outputFunction_motorBrake = 		Option(OutputFunction::MOTOR_BRAKE, 		"Motor Brake", 		"MotorBrake");
	Option outputFunction_DriverEnabled = 	Option(OutputFunction::DRIVER_ENABLED, 		"Drive Enabled", 	"DriveEnabled");
	std::vector<OptionParameter::Option*> outputFunctionOptions = {
		&outputFunction_none,
		&outputFunction_driverReady,
		&outputFunction_driverError,
		&outputFunction_motorBrake,
		&outputFunction_DriverEnabled
	};
	
	//----- Parameters
	
	float driveCurrentLimit = 0.0;							//0x60F6:F CMQ_q_Limit current limit (in ampere increments)
	NumberParam<double> maxCurrent_parameter;				//0x6073:0 CMD_q_Max max current (in ampere increments)
	NumberParam<double> maxVelocity_parameter;				//no drive parameter?
	NumberParam<double> maxAcceleration_parameter;			//0x6083:0 & 0x6084:0 profile acceleration and deceleration (in acceleration increments)
	NumberParam<double> velocityFeedforward_parameter;		//0x60FB:02 K_Velocity_FF (in %, 256 == 100%)
	NumberParam<double> maxFollowingError_parameter;		//0x6065:00 Max_Following_Error (in position increments)
	NumberParam<int> followingErrorTimeout_parameter;		//0x6066:00 Position_Window_Time (in ms)
	BoolParam invertDirectionOfMotion_parameter;			//0x607E:00 Invert_Dir (0: CCW is positive, 1: CW is positive)
	NumberParam<int> brakingResistorResistance_parameter;	//0x60F7:1 Chop_Resistor (in Ohm)
	NumberParam<int> brakingResistorPower_parameter;		//0x60F7:2 Chop_Resistor_Rated (in Watt)
	
	OptionParam DIN1Function_parameter;	//2010:3 (u16)
	OptionParam DIN2Function_parameter;	//2010:4 (u16)
	OptionParam DIN3Function_parameter;	//2010:5 (u16)
	OptionParam DIN4Function_parameter;	//2010:6 (u16)
	OptionParam DIN5Function_parameter;	//2010:7 (u16)
	OptionParam DIN6Function_parameter;	//2010:8 (u16)
	OptionParam DIN7Function_parameter;	//2010:9 (u16)
	
	BoolParam DIN1Polarity_parameter; //2010:1:b0 (u16)
	BoolParam DIN2Polarity_parameter; //2010:1:b1 (u16)
	BoolParam DIN3Polarity_parameter; //2010:1:b2 (u16)
	BoolParam DIN4Polarity_parameter; //2010:1:b3 (u16)
	BoolParam DIN5Polarity_parameter; //2010:1:b4 (u16)
	BoolParam DIN6Polarity_parameter; //2010:1:b5 (u16)
	BoolParam DIN7Polarity_parameter; //2010:1:b6 (u16)
	
	OptionParam DOUT1Function_parameter; //2010:F (u16)
	OptionParam DOUT2Function_parameter; //2010:10 (u16)
	OptionParam DOUT3Function_parameter; //2010:11 (u16)
	OptionParam DOUT4Function_parameter; //2010:12 (u16)
	OptionParam DOUT5Function_parameter; //2010:13 (u16)
	
	BoolParam DOUT1Polarity_parameter; //2010:D:b0 (u16)
	BoolParam DOUT2Polarity_parameter; //2010:D:b1 (u16)
	BoolParam DOUT3Polarity_parameter; //2010:D:b2 (u16)
	BoolParam DOUT4Polarity_parameter; //2010:D:b3 (u16)
	BoolParam DOUT5Polarity_parameter; //2010:D:b4 (u16)
	
	void uploadConfiguration();
	
	void updateActuatorInterface();
	
	bool startAutoTuning();
	bool b_isAutotuning = false;
	
};
