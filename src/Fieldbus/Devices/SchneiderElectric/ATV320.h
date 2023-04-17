#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/DS402Axis.h"

#include "Motion/Interfaces.h"

#include "Legato/Editor/Parameters.h"
#include "Utilities/AsynchronousTask.h"

class ATV320 : public EtherCatDevice{
public:
	
	DEFINE_ETHERCAT_DEVICE(ATV320, "Schneider Electric", "Motor Drives", 0x800005A, 0x389)
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		EtherCatDevice::onCopyFrom(source);
	};
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	
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
	
	std::shared_ptr<NodePin> digitalInput1Pin;
	std::shared_ptr<NodePin> digitalInput2Pin;
	std::shared_ptr<NodePin> digitalInput3Pin;
	std::shared_ptr<NodePin> digitalInput4Pin;
	std::shared_ptr<NodePin> digitalInput5Pin;
	std::shared_ptr<NodePin> digitalInput6Pin;
	std::shared_ptr<NodePin> actuatorPin;
	std::shared_ptr<NodePin> gpioPin;
	std::shared_ptr<NodePin> actualVelocityPin;
	std::shared_ptr<NodePin> actualLoadPin;
	
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

	Legato::Option option_frequency50Hz = Legato::Option(StandartMotorFrequency::HZ_50, "50 Hz", "50Hz");
	Legato::Option option_frequency60Hz = Legato::Option(StandartMotorFrequency::HZ_60, "60 Hz", "60Hz");
	std::vector<Legato::Option*> options_standartMotorFrequency = {
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
	Legato::OptionParam option_motorControlType_sensorlessFluxVector = Legato::Option(MotorControlType::SENSORLESS_FLUX_VECTOR,
																					  "Sensorless flux vector V", "SensorlessFluxVectorV");
	Legato::Option option_motorControlType_standardMotorLaw = Legato::Option(MotorControlType::STANDARD_MOTOR_LAW,
																			 "Standard Motor Law", "StandardMotorLaw");
	Legato::Option option_motorControlType_5pointVoltageFrequency = Legato::Option(MotorControlType::FIVE_POINT_VOLTAGE_FREQUENCY,
																				   "5 point voltage/frequency", "5pointVoltageFrequency");
	Legato::Option option_motorControlType_synchronousMotor = Legato::Option(MotorControlType::SYNCHRONOUS_MOTOR,
																			 "Synchronous Motor", "SynchronousMotor");
	Legato::Option option_motorControlType_VFQuadratic = Legato::Option(MotorControlType::V_F_QUADRATIC,
																		"V/F Quadratic", "VFQuadratic");
	Legato::Option option_motorControlType_energySaving = Legato::Option(MotorControlType::ENERGY_SAVING,
																		 "Energy Saving", "EnergySaving");
	std::vector<Legato::Option*> options_motorControlType = {
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
	Legato::Option option_logicInput_none =	Legato::Option(LogicInput::NONE, "None", "None");
	Legato::Option option_logicInput_LI1 =	Legato::Option(LogicInput::LI1, "LI1", "LI1");
	Legato::Option option_logicInput_LI2 =	Legato::Option(LogicInput::LI2, "LI2", "LI2");
	Legato::Option option_logicInput_LI3 =	Legato::Option(LogicInput::LI3, "LI3", "LI3");
	Legato::Option option_logicInput_LI4 =	Legato::Option(LogicInput::LI4, "LI4", "LI4");
	Legato::Option option_logicInput_LI5 =	Legato::Option(LogicInput::LI5, "LI5", "LI5");
	Legato::Option option_logicInput_LI6 =	Legato::Option(LogicInput::LI6, "LI6", "LI6");
	std::vector<Legato::Option*> options_logicInput = {
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
	Legato::Option option_activeLow =	Legato::Option(ActiveLowHigh::ACTIVE_LOW, "Active Low", "ActiveLow");
	Legato::Option option_activeHigh =	Legato::Option(ActiveLowHigh::ACTIVE_HIGH, "Active High", "ActiveHigh");
	std::vector<Legato::Option*> options_activeLowHigh = {
		&option_activeLow,
		&option_activeHigh
	};
	
	//———————————— DRIVE PARAMETERS —————————————
	
	//————— Motor Parameters —————
	
	Legato::OptionParam standartMotorFrequencyParameter;		//[bfr]
	Legato::OptionParam motorControlTypeParameter;				//[ctt]
	Legato::NumberParam<double> ratedMotorPowerParameter;		//[npr]
	Legato::NumberParam<double> nominalMotorVoltageParameter;	//[uns]
	Legato::NumberParam<double> nominalMotorCurrentParameter;	//[ncr]
	Legato::NumberParam<double> nominalMotorSpeedParameter;		//[nsp]
	
	//————— Limit signal configuration —————

	Legato::OptionParam forwardStopLimitAssignementParameter;
	Legato::OptionParam reverseStopLimitAssignementParameter;
	Legato::OptionParam stopLimitConfigurationParameter;
	
	//————— Logic input configuration —————
	
	Legato::NumberParam<int> logicInput1OnDelayParameter;
	Legato::NumberParam<int> logicInput2OnDelayParameter;
	Legato::NumberParam<int> logicInput3OnDelayParameter;
	Legato::NumberParam<int> logicInput4OnDelayParameter;
	Legato::NumberParam<int> logicInput5OnDelayParameter;
	Legato::NumberParam<int> logicInput6OnDelayParameter;
	
	
	
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
};

