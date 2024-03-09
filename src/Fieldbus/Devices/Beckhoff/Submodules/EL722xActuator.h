#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Project/Editor/Parameter.h"

template<typename T>
class ThreadSafe{
public:
	void write(T newData){
		mutex.lock();
		data = newData;
		mutex.unlock();
	}
	T read(){
		std::lock_guard<std::mutex> lock(mutex);
		return data;
	}
private:
	T data;
	std::mutex mutex;
};

class EL722x_Actuator : public ActuatorInterface{
public:
	
	EL722x_Actuator(std::shared_ptr<EtherCatDevice> parentDevice, int ch = 1){
		etherCatDevice = parentDevice;
		switch(ch){
			case 1:
			case 2: channel = ch; break;
			default: channel = 1; break;
		}
		std::string pinName = getChannelName();
		actuatorPin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, pinName.c_str());
	}
	void initialize();
	
	void gui();
	
	void firstSetup();
	ThreadSafe<std::string> firstSetupProgress;
	
	void resetEncoderPosition();
	ThreadSafe<std::string> resetEncoderPositionProgress;
	
	void uploadParameters();
	ThreadSafe<std::string> uploadParameterStatus;
	
	void onDisconnection();
	void readInputs();
	void writeOutputs();
	
	bool save(tinyxml2::XMLElement* parent);
	bool load(tinyxml2::XMLElement* parent);
	
	std::shared_ptr<NodePin> actuatorPin;
	
	struct Nameplate{
		bool b_motorIdentified = false;
		double ratedCurrent_amps = 0.0;
		double maxCurrent_amps = 0.0;
		double maxVelocity_rps = 0.0;
		double workingRange_rev = 0.0;
		int velocityResolution_rps = 0;
		int positionResolution_rev = 0;
		double torqueConstant_mNmpA = 0.0;
		std::string motorType = "Unknown";
		std::string serialNumber = "Unknown";
		std::string brakeType = "Unknown";
	}nameplate;
	
	struct RxPDO{
		uint16_t controlWord = 0;				//7010:1
		int32_t targetVelocity = 0;				//7010:6
		uint32_t targetPosition = 0;			//7010:5
		int16_t targetTorque = 0;				//7010:9
		uint8_t modeOfOperationSelection = 8;	//7010:3
	}rxPdo;
	
	struct TxPDO{
		int32_t fbPosition = 0;					//6000:11
		uint16_t statusWord = 0;				//6010:1
		int32_t velocityActualValue = 0;		//6010:7
		int16_t torqueActualValue = 0;			//6010:8
		uint16_t infoData1_errors = 0;			//6010:12
		uint16_t infoData2_digitalInputs = 0;	//6010:13
		uint16_t infoData3 = 0;
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
	
	
	
	
private:
	std::shared_ptr<EtherCatDevice> etherCatDevice;
	int channel; //add an offset of 0x100 on CanOpen indeces for 2nd channel on devices that support it
	virtual std::string getName() override {
		return std::string(etherCatDevice->getName()) + " " + getChannelName();
	}
	std::string getChannelName(){
		switch(channel){
			case 1 : return "Actuator 1";
			case 2 : return "Actuator 2";
			default: return "[Error]";
		}
	}
	uint16_t getCanOpenIndexOffset(){
		switch(channel){
			case 2 : return 0x100;
			default: return 0x0;
		}
	}
	virtual std::string getStatusString() override;
	void updateProprieties();

};


