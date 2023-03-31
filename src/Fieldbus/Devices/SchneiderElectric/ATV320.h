#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/DS402.h"

#include "Motion/Interfaces.h"

#include "Project/Editor/Parameter.h"

class ATV320 : public EtherCatDevice{
public:
	
	DEFINE_ETHERCAT_DEVICE(ATV320, "ATV320", "ATV320", "Schneider Electric", "Motor Drives", 0x800005A, 0x389)
	
	class ATV_Motor : public ActuatorInterface{
	public:
		ATV_Motor(std::shared_ptr<ATV320> drive_) : drive(drive_){}
		virtual std::string getName() override { return std::string(drive->getName()) + " Motor"; };
		virtual std::string getStatusString() override { return drive->getStatusString(); }
		std::shared_ptr<ATV320> drive;
		
	};
	
	class ATV_GPIO : public GpioInterface{
	public:
		
		ATV_GPIO(std::shared_ptr<ATV320> drive_) : drive(drive_){}
		virtual std::string getName() override { return std::string(drive->getName()) + " GPIO"; };
		virtual std::string getStatusString() override { return drive->getStatusString(); }
		std::shared_ptr<ATV320> drive;
	};
	
	std::shared_ptr<ATV_Motor> actuator;
	std::shared_ptr<ATV_GPIO> gpio;
	std::shared_ptr<bool> digitalInput1Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput2Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput3Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput4Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput5Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput6Signal = std::make_shared<bool>(false);
	std::shared_ptr<double> actualVelocity = std::make_shared<double>(0.0);
	std::shared_ptr<double> actualLoad = std::make_shared<double>(0.0);
	
	std::shared_ptr<NodePin> digitalInput1Pin = std::make_shared<NodePin>(digitalInput1Signal, NodePin::Direction::NODE_OUTPUT, "DI1", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalInput2Pin = std::make_shared<NodePin>(digitalInput2Signal, NodePin::Direction::NODE_OUTPUT, "DI2", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalInput3Pin = std::make_shared<NodePin>(digitalInput3Signal, NodePin::Direction::NODE_OUTPUT, "DI3", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalInput4Pin = std::make_shared<NodePin>(digitalInput4Signal, NodePin::Direction::NODE_OUTPUT, "DI4", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalInput5Pin = std::make_shared<NodePin>(digitalInput5Signal, NodePin::Direction::NODE_OUTPUT, "DI5", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalInput6Pin = std::make_shared<NodePin>(digitalInput6Signal, NodePin::Direction::NODE_OUTPUT, "DI6", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> actuatorPin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Actuator");
	std::shared_ptr<NodePin> gpioPin = std::make_shared<NodePin>(NodePin::DataType::GPIO_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Gpio");
	std::shared_ptr<NodePin> actualVelocityPin = std::make_shared<NodePin>(actualVelocity, NodePin::Direction::NODE_OUTPUT, "Velocity");
	std::shared_ptr<NodePin> actualLoadPin = std::make_shared<NodePin>(actualLoad, NodePin::Direction::NODE_OUTPUT, "Load");
	
	//————— Drive State —————
	DS402::PowerState requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
	DS402::PowerState actualPowerState = DS402::PowerState::UNKNOWN;
	long long enableRequestTime_nanoseconds;
	
	bool b_reverseDirection = false;
	bool b_velocityTargetReached = false;
	bool b_motorVoltagePresent = false;
	
	bool b_remoteControlEnabled = false;
	bool b_stoActive = false;
	bool b_hasFault = false;
	bool b_isResettingFault = false;
	
	//————— RX PDO —————
	DS402::Control ds402Control;
	int16_t velocityTarget_rpm = 0;
	
	//————— TX PDO —————
	DS402::Status ds402Status;
	int16_t velocityActual_rpm = 0;
	uint16_t logicInputs = 0;
	uint16_t stoState = 0;
	int16_t motorPower = 0;
	uint16_t lastFaultCode = 0x0;
	
	
	//————— General Settings —————
	long long enableRequestTimeout_nanoseconds = 250'000'000; //250ms enable timeout
	
	std::shared_ptr<NumberParameter<double>> accelerationRampTime = NumberParameter<double>::make(3.0, "Acceleration Ramp", "AccelerationRamp",
																								  "%.1f", Units::Time::Second, false);
	std::shared_ptr<NumberParameter<double>> decelerationRampTime = NumberParameter<double>::make(1.0, "Deceleration Ramp", "DecelerationRamp",
																								  "%.1f", Units::Time::Second, false);
	std::shared_ptr<NumberParameter<int>> maxVelocityRPM = NumberParameter<int>::make(1400, "Max Velocity", "MaxVelocity", "%i ",
																					  Units::AngularDistance::Revolution, false, 0, 0, "", "/min");
	std::shared_ptr<NumberParameter<double>> slowdownVelocityHertz = NumberParameter<double>::make(5.0, "Slowdown Velocity", "SlowdownVelocity",
																								"%.1f", Units::Frequency::Hertz, false);
	std::shared_ptr<BooleanParameter> invertDirection = BooleanParameter::make(false, "Invert Motion Direction", "InvertMotionDirection");
	
    std::shared_ptr<NumberParameter<double>> lowSpeedHertz = NumberParameter<double>::make(0.0, "Low Speed", "LowSpeed", "%.1f",
                                                                                           Units::Frequency::Hertz, false);
    
	enum LogicInput{
		NONE = 0,
		LI1 = 129,
		LI2 = 130,
		LI3 = 131,
		LI4 = 132,
		LI5 = 133,
		LI6 = 134
	};
	
	NumberParam<double> ratedMotorCurrentParameter;
	NumberParam<double> ratedMotorPowerParameter;
	
	OptionParameter::Option option_logicInput_none =	OptionParameter::Option(LogicInput::NONE, "None", "None");
	OptionParameter::Option option_logicInput_LI1 =		OptionParameter::Option(LogicInput::LI1, "LI1", "LI1");
	OptionParameter::Option option_logicInput_LI2 =		OptionParameter::Option(LogicInput::LI2, "LI2", "LI2");
	OptionParameter::Option option_logicInput_LI3 =		OptionParameter::Option(LogicInput::LI3, "LI3", "LI3");
	OptionParameter::Option option_logicInput_LI4 =		OptionParameter::Option(LogicInput::LI4, "LI4", "LI4");
	OptionParameter::Option option_logicInput_LI5 =		OptionParameter::Option(LogicInput::LI5, "LI5", "LI5");
	OptionParameter::Option option_logicInput_LI6 =		OptionParameter::Option(LogicInput::LI6, "LI6", "LI6");
	OptionParam forwardStopLimitAssignementParameter;
	OptionParam reverseStopLimitAssignementParameter;
	
	enum ActiveLowHigh{
		ACTIVE_LOW = 0,
		ACTIVE_HIGH = 1
	};
	OptionParameter::Option option_activeLow = OptionParameter::Option(ActiveLowHigh::ACTIVE_LOW, "Active Low", "ActiveLow");
	OptionParameter::Option option_activeHigh = OptionParameter::Option(ActiveLowHigh::ACTIVE_HIGH, "Active High", "ActiveHigh");
	OptionParam stopLimitConfigurationParameter;
	
	NumberParam<int> logicInput1OnDelayParameter;
	NumberParam<int> logicInput2OnDelayParameter;
	NumberParam<int> logicInput3OnDelayParameter;
	NumberParam<int> logicInput4OnDelayParameter;
	NumberParam<int> logicInput5OnDelayParameter;
	NumberParam<int> logicInput6OnDelayParameter;
	
	std::string getStatusString();
	std::string getShortStatusString();
	glm::vec4 getStatusColor();
	std::string getFaultString();
	
	void controlsGui();
	void settingsGui();
	void statusGui();
	
	void updateActuatorInterface();
	
};
