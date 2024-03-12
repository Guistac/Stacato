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
	bool readMotorNameplate();
	
	

	
	void resetEncoderPosition(bool* b_busy, bool* b_success);
	ThreadSafe<std::string> resetEncoderPositionProgress;
	
	void uploadParameters();
	ThreadSafe<std::string> uploadParameterStatus;
	
	void onDisconnection();
	void readInputs();
	void writeOutputs();
	
	bool save(tinyxml2::XMLElement* parent);
	bool load(tinyxml2::XMLElement* parent);

	void gui();
	
	std::shared_ptr<NodePin> actuatorPin;
	std::shared_ptr<NodePin> digitalIn1Pin;
	std::shared_ptr<NodePin> digitalIn2Pin;
	
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
		int32_t followingErrorActualValue = 0;	//6010:6
		uint16_t fbStatus = 0;					//6000:E
		uint8_t modeOfOperationDisplay = 0;		//6010:3
	}txPdo;
	
	
private:
	
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
	
	struct ProcessData{
		bool b_motorConnected = false;
		bool b_waitingForEnable = false;
		uint64_t enableRequestTime_nanos;
		bool b_hadFault = false;
		bool b_enableTarget = false;
		float manualVelocityTarget_rps = 0.0;
		double manualVelocityProfile_rps = 0.0;
	}processData;
	
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
	
	bool b_newErrorID = false;
	uint16_t lastErrorTextID = 0x0;
	std::string lastErrorString = "";

};


