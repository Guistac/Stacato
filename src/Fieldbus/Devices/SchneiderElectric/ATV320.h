#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/DS402Axis.h"

#include "Motion/Interfaces.h"

#include "Project/Editor/Parameter.h"
#include "Utilities/AsynchronousTask.h"



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
	std::shared_ptr<bool> stoStatusValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput1Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput2Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput3Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput4Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput5Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput6Signal = std::make_shared<bool>(false);
	std::shared_ptr<double> actualVelocity = std::make_shared<double>(0.0);
	std::shared_ptr<double> actualLoad = std::make_shared<double>(0.0);
	
	std::shared_ptr<NodePin> stoStatusPin = std::make_shared<NodePin>(stoStatusValue, NodePin::Direction::NODE_OUTPUT, "STO", NodePin::Flags::DisableDataField);
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
	long long enableRequestTime_nanoseconds;
	bool b_waitingForEnable = false;
	bool b_reverseDirection = false;
	
	bool b_disableLimitSwitches = false; //[cls] clear limit switches if they cause problems
	bool b_canDisableLimitSwitches = false; //only available if [cls] can actually be assigned
	
	bool b_referenceReached = false;
	bool b_referenceOutsideLimits = false;
	bool b_motorVoltagePresent = false;
	bool b_remoteControlEnabled = false;
	bool b_stopKeyPressed = false;
	bool b_directionOfRotationAtOutput = false;
	bool b_stoActive = false;
	bool b_hasFault = false;
	bool b_isResettingFault = false;
	
	//————— PDO Data —————
	std::shared_ptr<DS402Axis> axis;
	uint16_t logicInputs = 0;
	uint16_t stoState = 0;
	int16_t motorPower = 0;
	uint16_t lastFaultCode = 0x0;
	int16_t velocityActual_rpm = 0;
	
	
	//—————————— DRIVE PARAMETER ENUMERATORS ————————————
	
	enum StandartMotorFrequency{
		HZ_50 = 0,
		HZ_60 = 1
	};
	OptionParameter::Option option_frequency50Hz = OptionParameter::Option(StandartMotorFrequency::HZ_50, "50 Hz", "50Hz");
	OptionParameter::Option option_frequency60Hz = OptionParameter::Option(StandartMotorFrequency::HZ_60, "60 Hz", "60Hz");
	std::vector<OptionParameter::Option*> options_standartMotorFrequency = {
		&option_frequency50Hz,
		&option_frequency60Hz
	};

	
	enum MotorControlType{
		SENSORLESS_FLUX_VECTOR = 0,
		STANDARD_MOTOR_LAW = 3,
		FIVE_POINT_VOLTAGE_FREQUENCY = 4,
		SYNCHRONOUS_MOTOR = 5,
		V_F_QUADRATIC = 6,
		ENERGY_SAVING = 7
	};
	OptionParameter::Option option_motorControlType_sensorlessFluxVector =
		OptionParameter::Option(MotorControlType::SENSORLESS_FLUX_VECTOR, "Sensorless flux vector V", "SensorlessFluxVectorV");
	OptionParameter::Option option_motorControlType_standardMotorLaw =
		OptionParameter::Option(MotorControlType::STANDARD_MOTOR_LAW, "Standard Motor Law", "StandardMotorLaw");
	OptionParameter::Option option_motorControlType_5pointVoltageFrequency =
		OptionParameter::Option(MotorControlType::FIVE_POINT_VOLTAGE_FREQUENCY, "5 point voltage/frequency", "5pointVoltageFrequency");
	OptionParameter::Option option_motorControlType_synchronousMotor =
		OptionParameter::Option(MotorControlType::SYNCHRONOUS_MOTOR, "Synchronous Motor", "SynchronousMotor");
	OptionParameter::Option option_motorControlType_VFQuadratic =
		OptionParameter::Option(MotorControlType::V_F_QUADRATIC, "V/F Quadratic", "VFQuadratic");
	OptionParameter::Option option_motorControlType_energySaving =
		OptionParameter::Option(MotorControlType::ENERGY_SAVING, "Energy Saving", "EnergySaving");
	std::vector<OptionParameter::Option*> options_motorControlType = {
		&option_motorControlType_sensorlessFluxVector,
		&option_motorControlType_standardMotorLaw,
		&option_motorControlType_5pointVoltageFrequency,
		&option_motorControlType_synchronousMotor,
		&option_motorControlType_VFQuadratic,
		&option_motorControlType_energySaving
	};
	
	
	enum LogicInput{
		NONE = 0,
		LI1 = 129,
		LI2 = 130,
		LI3 = 131,
		LI4 = 132,
		LI5 = 133,
		LI6 = 134
	};
	OptionParameter::Option option_logicInput_none =	OptionParameter::Option(LogicInput::NONE, "None", "None");
	OptionParameter::Option option_logicInput_LI1 =		OptionParameter::Option(LogicInput::LI1, "LI1", "LI1");
	OptionParameter::Option option_logicInput_LI2 =		OptionParameter::Option(LogicInput::LI2, "LI2", "LI2");
	OptionParameter::Option option_logicInput_LI3 =		OptionParameter::Option(LogicInput::LI3, "LI3", "LI3");
	OptionParameter::Option option_logicInput_LI4 =		OptionParameter::Option(LogicInput::LI4, "LI4", "LI4");
	OptionParameter::Option option_logicInput_LI5 =		OptionParameter::Option(LogicInput::LI5, "LI5", "LI5");
	OptionParameter::Option option_logicInput_LI6 =		OptionParameter::Option(LogicInput::LI6, "LI6", "LI6");
	std::vector<OptionParameter::Option*> options_logicInput = {
		&option_logicInput_none,
		&option_logicInput_LI1,
		&option_logicInput_LI2,
		&option_logicInput_LI3,
		&option_logicInput_LI4,
		&option_logicInput_LI5,
		&option_logicInput_LI6
	};
	
	enum ActiveLowHigh{
		ACTIVE_LOW = 0,
		ACTIVE_HIGH = 1
	};
	OptionParameter::Option option_activeLow = OptionParameter::Option(ActiveLowHigh::ACTIVE_LOW, "Active Low", "ActiveLow");
	OptionParameter::Option option_activeHigh = OptionParameter::Option(ActiveLowHigh::ACTIVE_HIGH, "Active High", "ActiveHigh");
	std::vector<OptionParameter::Option*> options_activeLowHigh = {
		&option_activeLow,
		&option_activeHigh
	};
	
	
	OptionParameter::Option option_frequencyReference_None = OptionParameter::Option(0, "Not Assigned", "NotAssigned");
	OptionParameter::Option option_frequencyReference_AI1 = OptionParameter::Option(1, "AI1", "AI2");
	OptionParameter::Option option_frequencyReference_AI2 = OptionParameter::Option(2, "AI2", "AI2");
	OptionParameter::Option option_frequencyReference_AI3 = OptionParameter::Option(3, "AI3", "AI3");
	OptionParameter::Option option_frequencyReference_CommunicationCard = OptionParameter::Option(169, "Communication Card", "CommunicationCard");
	std::vector<OptionParameter::Option*> options_frequencyReferences = {
		&option_frequencyReference_None,
		&option_frequencyReference_AI1,
		&option_frequencyReference_AI2,
		&option_frequencyReference_AI3,
		&option_frequencyReference_CommunicationCard
	};
	
	OptionParameter::Option option_twoWireControl = OptionParameter::Option(0, "Two Wire Control", "TwoWireControl");
	OptionParameter::Option option_threeWireControl = OptionParameter::Option(1, "Three Wire Control", "Three Wire Control");
	std::vector<OptionParameter::Option*> option_twoOrThreeWireControlOptions = {
		&option_twoWireControl,
		&option_threeWireControl
	};
	
	OptionParameter::Option option_frequencyReference1 = OptionParameter::Option(96, "Fixed to Frequency Reference 1", "FrequencyReference1");
	OptionParameter::Option option_frequencyReference2 = OptionParameter::Option(97, "Fixed to Frequency Reference 2", "FrequencyReference2");
	std::vector<OptionParameter::Option*> options_frequencyReferenceSwitch = {
		&option_frequencyReference1,
		&option_frequencyReference2,
		&option_logicInput_LI1,
		&option_logicInput_LI2,
		&option_logicInput_LI3,
		&option_logicInput_LI4,
		&option_logicInput_LI5,
		&option_logicInput_LI6
	};
	
	//———————————— DRIVE PARAMETERS —————————————
	
	//————— Motor Parameters —————
	OptionParam standartMotorFrequencyParameter;		//[bfr]
	OptionParam motorControlTypeParameter;				//[ctt]
	NumberParam<double> ratedMotorPowerParameter;		//[npr]
	NumberParam<double> nominalMotorVoltageParameter;	//[uns]
	NumberParam<double> nominalMotorCurrentParameter;	//[ncr]
	NumberParam<double> nominalMotorSpeedParameter;		//[nsp]
	
	//————— Motion Control Settings —————
	std::shared_ptr<NumberParameter<double>> accelerationRampTime;
	std::shared_ptr<NumberParameter<double>> decelerationRampTime;
	std::shared_ptr<BooleanParameter> invertDirection;
	std::shared_ptr<NumberParameter<double>> lowControlFrequencyParameter;
	std::shared_ptr<NumberParameter<double>> highControlFrequencyParameter; //[hsp]
	std::shared_ptr<NumberParameter<double>> maxFrequencyParameter; //[tfr]
	NumberParam<double> switchingFrequencyParameter;	//[sfr]
	
	//—————— Frequency Reference ————————
	OptionParam frequencyReference1_Parameter = OptionParameter::make2(option_frequencyReference_CommunicationCard, options_frequencyReferences, "Frequency Reference 1 [fr1]", "FrequencyReference1");
	OptionParam frequencyReference2_Parameter = OptionParameter::make2(option_frequencyReference_None, options_frequencyReferences, "Frequency Reference 2 [fr2]", "FrequencyReference2");
	OptionParam referenceSwitchingPin_Parameter = OptionParameter::make2(option_frequencyReference1, options_frequencyReferenceSwitch, "Reference Switching Method [rfc]", "ReferenceSwitchingMethod");

	//————— IO configuration —————
	OptionParam twoOrThreeWireControl_Parameter = OptionParameter::make2(option_twoWireControl, option_twoOrThreeWireControlOptions, "Two or Three Wire Control [tcc]", "TwoOrThreeWireControl");
	OptionParam forwardStopLimitAssignementParameter;
	OptionParam reverseStopLimitAssignementParameter;
	OptionParam stopLimitConfigurationParameter;
	OptionParam faultResetPin_Parameter = OptionParameter::make2(option_logicInput_none, options_logicInput, "Fault Reset Pin [rsf]", "FaultResetPin");
	OptionParam externalFaultPin_Parameter = OptionParameter::make2(option_logicInput_none, options_logicInput, "External Fault Pin [etf]", "ExternalFaultPin");
	OptionParam presetSlowdownSpeedPin_Parameter = OptionParameter::make2(option_logicInput_none, options_logicInput, "Preset Slowdown Speed Pin [ps2]", "PresetSlowdownSpeedPin");
	NumberParam<double> presetSlowdownSpeed_Parameter = NumberParameter<double>::make(10.0, "Preset Slowdown Speed [sp2]", "Preset Slowdown Speed", "%.1f", Units::Frequency::Hertz, false);
	
	//————— Logic input configuration —————
	NumberParam<int> logicInput1OnDelayParameter;
	NumberParam<int> logicInput2OnDelayParameter;
	NumberParam<int> logicInput3OnDelayParameter;
	NumberParam<int> logicInput4OnDelayParameter;
	NumberParam<int> logicInput5OnDelayParameter;
	NumberParam<int> logicInput6OnDelayParameter;
	BoolParam invertLogicInput1Parameter;
	BoolParam invertLogicInput2Parameter;
	BoolParam invertLogicInput3Parameter;
	BoolParam invertLogicInput4Parameter;
	BoolParam invertLogicInput5Parameter;
	BoolParam invertLogicInput6Parameter;
	
	//————————————— TASKS ———————————
	
	class ConfigurationUploadTask : public AsynchronousTask{
	public:
		void setAtv320(std::shared_ptr<ATV320> atv320_){ atv320 = atv320_; }
		virtual void onExecution() override;
		virtual bool canStart() override;
	private:
		std::shared_ptr<ATV320> atv320 = nullptr;
	};
	
	class StandstillTuningTask : public AsynchronousTask{
	public:
		void setAtv320(std::shared_ptr<ATV320> atv320_){ atv320 = atv320_; }
		virtual void onExecution() override;
		virtual bool canStart() override;
	private:
		std::shared_ptr<ATV320> atv320 = nullptr;
	};
	
	ConfigurationUploadTask configurationUploadTask;
	StandstillTuningTask standstillTuningTask;
	
	
	//—————— other —————
	
	
	std::string getStatusString();
	std::string getShortStatusString();
	glm::vec4 getStatusColor();
	std::string getFaultString();
	
	void controlsGui();
	void settingsGui();
	void statusGui();
	
	void updateActuatorInterface();
	
	
	//temp:
	float maxVelocitySliderValue = 50.0;
};

