#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Project/Editor/Parameter.h"

class EL7221_9014 : public EtherCatDevice{
public:
	DEFINE_ETHERCAT_DEVICE(EL7221_9014, "EL7221-9014", "EL7221-9014", "Beckhoff", "Servo Drives", 0x2, 0x1c353052)

	class EL7211ServoMotor : public ActuatorInterface{
	public:
		EL7211ServoMotor(std::shared_ptr<EL7221_9014> parentDevice) : etherCatDevice(parentDevice){}
		std::shared_ptr<EL7221_9014> etherCatDevice;
		virtual std::string getName() override { return std::string(etherCatDevice->getName()) + " Servo Motor"; }
		virtual std::string getStatusString() override;
	};
	
	class EL7211Gpio : public GpioInterface{
	public:
		EL7211Gpio(std::shared_ptr<EL7221_9014> parentDevice) : etherCatDevice(parentDevice){}
		std::shared_ptr<EL7221_9014> etherCatDevice;
		virtual std::string getName() override { return std::string(etherCatDevice->getName()) + " GPIO"; }
		virtual std::string getStatusString() override { return "No Status String Implemented..."; }
	};
	
	std::shared_ptr<EL7211ServoMotor> actuator;
	std::shared_ptr<EL7211Gpio> gpio;
	std::shared_ptr<bool> digitalInput1_Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput2_Value = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> actuatorPin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Actuator");
	std::shared_ptr<NodePin> gpioPin = std::make_shared<NodePin>(NodePin::DataType::GPIO_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "GPIO");
	std::shared_ptr<NodePin> digitalInput1_pin = std::make_shared<NodePin>(digitalInput1_Value, NodePin::Direction::NODE_OUTPUT, "DI1");
	std::shared_ptr<NodePin> digitalInput2_pin = std::make_shared<NodePin>(digitalInput2_Value, NodePin::Direction::NODE_OUTPUT, "DI2");
	
	struct RxPDO{
		uint16_t controlWord = 0;				//7010:1
		int32_t targetVelocity = 0;				//7010:6
		uint32_t targetPosition = 0;			//7010:5
		int16_t targetTorque = 0;				//7010:9
		uint8_t modeOfOperationSelection = 0;	//7010:3
	}rxPdo;
	
	struct TxPDO{
		int32_t fbPosition = 0;					//6000:11
		uint16_t statusWord = 0;				//6010:1
		int32_t velocityActualValue = 0;		//6010:7
		int16_t torqueActualValue = 0;			//6010:8
		uint16_t infoData1_errors = 0;			//6010:12
		uint16_t infoData2_digitalInputs = 0;	//6010:13
		int32_t followingErrorActualValue = 0;	//6010:6
		uint16_t fbStatus = 0;					//6000:E/F
		uint8_t modeOfOperationDisplay = 0;		//6010:3
	}txPdo;
		
	struct DriverErrors{
		bool adc = false;
		bool overcurrent = false;
		bool undervoltage = false;
		bool overvoltage = false;
		bool overtemperature = false;
		bool i2tAmplifier = false;
		bool i2tMotor = false;
		bool encoder = false;
		bool watchdog = false;
	}driverErrors;
	
	struct DriverWarnings{
		bool undervoltage = false;
		bool overvoltage = false;
		bool overtemperature = false;
		bool i2tAmplifier = false;
		bool i2tMotor = false;
		bool encoder = false;
	}driverWarnings;
	
	struct StatusWord{
		bool readyToSwitchOn = false;
		bool switchedOn = false;
		bool operationEnabled = false;
		bool fault = false;
		bool quickstop = false;
		bool switchOnDisabled = false;
		bool warning = false;
		bool TxPdoToggle = false;
		bool internalLimitActive = false;
		bool commandValueFollowed = false;
	}statusWord;
	
	struct ControlWord{
		bool switchOn = false;
		bool enableVoltage = false;
		bool quickstop = false;
		bool enableOperation = false;
		bool faultReset = false;
	}controlWord;
	
	enum class PowerState{
		OPERATION_ENABLED,
		SWITCHED_ON,
		READY_TO_SWITCH_ON,
		SWITCH_ON_DISABLED,
		NOT_READY_TO_SWITCH_ON,
		FAULT
	};
	
	struct ProcessData{
		bool b_motorConnected = false;
		bool b_waitingForEnable = false;
		uint64_t enableRequestTime_nanos;
		bool b_hadFault = false;
		PowerState powerStateActual = PowerState::NOT_READY_TO_SWITCH_ON;
		PowerState powerStateTarget = PowerState::READY_TO_SWITCH_ON;
	}processData;
	
	float velocitySliderValue = 0.0;
	
	struct MotorNameplate{
		bool b_motorIdentified = false;
		double ratedCurrent_amps = 0.0;
		double maxCurrent_amps = 0.0;
		double maxVelocity_rps = 0.0;
		double workingRange_rev = 0.0;
		int velocityResolution_rps = 0;
		int positionResolution_rev = 0;
		double torqueConstant_mNmpA = 0.0;
		bool b_hasBrake = false;
		std::string motorType = "Unknown";
		std::string serialNumber = "Unknown";
		bool save(tinyxml2::XMLElement* parent);
		bool load(tinyxml2::XMLElement* parent);
	}motorNameplate;
	
	struct DriveSettings{
		NumberParam<double> velocityLimit = NumberParameter<double>::make(0.0, "Velocity Limit", "VelocityLimit", "%.1f", Units::AngularDistance::Revolution, false, 0, 0, "", "/s");
		NumberParam<double> accelerationLimit = NumberParameter<double>::make(0.0, "Acceleration Limit", "AccelerationLimit", "%.1f", Units::AngularDistance::Revolution, false, 0, 0, "", "/s\xc2\xb2");
		NumberParam<double> currentLimit = NumberParameter<double>::make(0.0, "Current Limit", "CurrentLimit", "%.1f", Units::Current::Ampere, false);
		BoolParam invertDirection = BooleanParameter::make(false, "Invert Direction", "InvertDirection");
		NumberParam<double> positionFollowingErrorWindow = NumberParameter<double>::make(0.0, "Position Following Error Window", "PositionFollowingErrorWindow", "%.1f", Units::AngularDistance::Revolution, false);
		NumberParam<int> positionFollowingErrorTimeout = NumberParameter<int>::make(0.0, "Position Following Error Timeout", "PositionFollowingErrorTimeout", "%i", Units::Time::Millisecond, false);
		OptionParameter::Option option_faultReaction_Disable = OptionParameter::Option(0, "Disable Drive", "Disable");
		OptionParameter::Option option_faultReaction_HaltRamp = OptionParameter::Option(1, "Halt Ramp", "HaltRamp");
		std::vector<OptionParameter::Option*> options_faultReaction = {
			&option_faultReaction_Disable,
			&option_faultReaction_HaltRamp
		};
		OptionParam faultReaction = OptionParameter::make2(option_faultReaction_Disable, options_faultReaction, "Fault Reaction", "FaultReaction");
		NumberParam<double> haltRampDeceleration = NumberParameter<double>::make(0.0, "Halt Ramp Deceleration", "HaltRampDeceleration", "%.1f", Units::AngularDistance::Revolution, false, 0, 0,"", "/s\xc2\xb2");
		bool save(tinyxml2::XMLElement* parent);
		bool load(tinyxml2::XMLElement* parent);
	}driveSettings;
	
	void controlTab();
	void settingsTab();
	
	void firstSetup();
	void resetEncoderPosition();
	void downloadDiagnostics();
	void uploadParameters();
	
	void updateActuatorInterface();
};

