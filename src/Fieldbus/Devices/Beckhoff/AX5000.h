#pragma once

#include "Motion/Interfaces.h"
#include "Project/Editor/Parameter.h"
#include "Fieldbus/EtherCatDevice.h"

class AX5000{
public:
	void initialize(std::shared_ptr<EtherCatDevice> ecatDevice);
	std::shared_ptr<EtherCatDevice> etherCatDevice = nullptr;
	
	bool startupConfiguration(){}
	void readInputs(){}
	void writeOutputs(){}
	bool save(tinyxml2::XMLElement* xml){}
	bool load(tinyxml2::XMLElement* xml){}
	void onDisconnection(){}
	void onConnection(){}
	
	struct StatusWord{
		bool followsCommand;
		bool realtimeBit1;
		bool realtimeBit2;
		uint8_t operatingMode;
		bool infoChange;
		bool warningChange;
		bool shutdownError;
		uint8_t status;
		void decode(uint16_t driveStatusWord){
			followsCommand = (driveStatusWord >> 3) & 0x1;
			realtimeBit1 = (driveStatusWord >> 6) & 0x1;
			realtimeBit2 = (driveStatusWord >> 7) & 0x1;
			operatingMode = (driveStatusWord >> 8) & 0x7;
			infoChange = (driveStatusWord >> 11) & 0x1;
			warningChange = (driveStatusWord >> 12) & 0x1;
			shutdownError = (driveStatusWord >> 13) & 0x1;
			status = (driveStatusWord >> 14) & 0x3;
		}
		bool isEnabled(){ return status == 3 && followsCommand; }
		bool isReady(){ return status == 1 || status == 2 || status == 3; }
	};
	
	struct ControlWord{
		bool b_realtimeBit1 = false;
		bool b_realtimeBit2 = false;
		uint8_t operatingMode = 0;
		bool b_syncBit = false;
		bool b_haltRestart = false;
		bool b_enable = false;
		bool b_driveOnOff = false;
		void encode(uint16_t& masterControlWord){
			masterControlWord = 0x0;
			masterControlWord |= b_realtimeBit1 << 6;
			masterControlWord |= b_realtimeBit2 << 7;
			masterControlWord |= (operatingMode & 0x3) << 8;
			masterControlWord |= b_syncBit << 10;
			masterControlWord |= (operatingMode & 0x4) << 11;
			masterControlWord |= b_haltRestart << 13;
			masterControlWord |= b_enable << 14;
			masterControlWord |= b_driveOnOff << 15;
		}
		void enable(){ b_haltRestart = true; b_enable = true; b_driveOnOff = true; }
		void disable(){ b_haltRestart = false; b_enable = false; b_driveOnOff = false; }
		bool isRequestingEnable(){ return b_haltRestart && b_enable && b_driveOnOff; }
		void toggleSyncBit(){ b_syncBit = !b_syncBit; }
	};
	
	class Actuator : public ActuatorInterface{
	public:
		Actuator(std::shared_ptr<AX5000> d, std::string n, uint8_t c) : parentDrive(d), name(n), channel(c){
			//motorType = OptionParameter::make2(drive->option_motor_none, drive->motorTypeOptions, "Motor Type", "MotorType");
		}
		//Gui Stuff
		virtual std::string getName() override { return name; }
		virtual std::string getStatusString() override;
		void settingsGui();
		float guiVelocitySliderValue = 0.0;
		//Process Data
		StatusWord statusWord;
		ControlWord controlWord;
		uint16_t class1Errors;
		void updateInputs(uint16_t status, int32_t pos, int32_t vel, int16_t tor, uint16_t err, bool sto);
		void updateOutputs(uint16_t& controlWord, int32_t& vel, uint32_t& pos);
		//Parameters
		//OptionParam motorType;
		//MotorType getMotorType(){ return MotorType(motorType->value); }
		NumberParam<double> velocityLimit_revps = NumberParameter<double>::make(0.0, "Velocity Limit", "VelocityLimit", "%.1f", Units::AngularDistance::Revolution, false, 0, 0, "", "/s");
		NumberParam<double> accelerationLimit_revps2 = NumberParameter<double>::make(0.0, "Acceleration Limit", "AccelerationLimit", "%.1f", Units::AngularDistance::Revolution, false, 0, 0, "", "/s\xc2\xb2");
		NumberParam<double> positionFollowingErrorLimit_rev = NumberParameter<double>::make(0.0, "Position Following Error Limit", "PositionFollowingErrorLimit", "%.1f", Units::AngularDistance::Revolution, false);
		NumberParam<double> currentLimit_amps = NumberParameter<double>::make(0.0, "Current Limit", "CurrentLimit", "%.3f", Units::Current::Ampere, false);
		BoolParam invertDirection_param = BooleanParameter::make(false, "Invert Direction", "InvertDirection");
		bool save(tinyxml2::XMLElement* xml);
		bool load(tinyxml2::XMLElement* xml);

		bool b_warning = false;
		bool b_info = false;

		std::string name;
		std::shared_ptr<AX5000> parentDrive;
		uint8_t channel;
		const double unitsPerRPM = 17895.697;
		const double unitsPerRev = 1048576.0;
		const uint64_t enableTimeout_nanos = 500'000'000; //300ms
	};
	
	std::vector<std::shared_ptr<Actuator>> actuators = {};
	
	struct __attribute__((__packed__)) MasterDataTelegram{
		uint16_t ax0_masterControlWord;
		uint32_t ax0_positionCommandValue;
		int32_t ax0_velocityCommandValue;
		uint16_t digitalOutput;
		//optional 2nd axis
		uint16_t ax1_masterControlWord;
		uint32_t ax1_positionCommandValue;
		int32_t ax1_velocityCommandValue;
	}masterDataTelegram;
	
	struct __attribute__((__packed__)) AcknowledgeTelegram{
		uint16_t ax0_driveStatusWord;
		uint32_t ax0_positionFeedbackValue1;
		int32_t ax0_velocityFeedbackValue;
		int16_t ax0_torqueFeedbackValue;
		uint16_t ax0_class1Diagnostics;
		uint16_t digitalInputsState;
		uint16_t safetyOptionState;
		//optional 2nd axis
		uint16_t ax1_driveStatusWord;
		uint32_t ax1_positionFeedbackValue1;
		int32_t ax1_velocityFeedbackValue;
		int16_t ax1_torqueFeedbackValue;
		uint16_t ax1_class1Diagnostics;
	}acknowledgeTelegram;
	
	
	std::shared_ptr<bool> STOValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalin0Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalin1Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalin2Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalin3Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalin4Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalin5Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalin6Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalout7Value = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> gpioPin = std::make_shared<NodePin>(NodePin::DataType::GPIO_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "GPIO");
	std::shared_ptr<NodePin> STO_pin = std::make_shared<NodePin>(STOValue, NodePin::Direction::NODE_OUTPUT, "STO");
	std::shared_ptr<NodePin> digitalInput0_pin = std::make_shared<NodePin>(digitalin0Value, NodePin::Direction::NODE_OUTPUT, "DI0");
	std::shared_ptr<NodePin> digitalInput1_pin = std::make_shared<NodePin>(digitalin1Value, NodePin::Direction::NODE_OUTPUT, "DI1");
	std::shared_ptr<NodePin> digitalInput2_pin = std::make_shared<NodePin>(digitalin2Value, NodePin::Direction::NODE_OUTPUT, "DI2");
	std::shared_ptr<NodePin> digitalInput3_pin = std::make_shared<NodePin>(digitalin3Value, NodePin::Direction::NODE_OUTPUT, "DI3");
	std::shared_ptr<NodePin> digitalInput4_pin = std::make_shared<NodePin>(digitalin4Value, NodePin::Direction::NODE_OUTPUT, "DI4");
	std::shared_ptr<NodePin> digitalInput5_pin = std::make_shared<NodePin>(digitalin5Value, NodePin::Direction::NODE_OUTPUT, "DI5");
	std::shared_ptr<NodePin> digitalInput6_pin = std::make_shared<NodePin>(digitalin6Value, NodePin::Direction::NODE_OUTPUT, "DI6");
	std::shared_ptr<NodePin> digitalOutput7_pin = std::make_shared<NodePin>(digitalout7Value, NodePin::Direction::NODE_INPUT, "DO7");
	
	BoolParam invertSTO_param = BooleanParameter::make(false, "Invert STO Pin", "InvertSTOPin");
	BoolParam invertDigitalIn0_param = BooleanParameter::make(false, "Invert Digital Input 0", "InvertDigitalIn0");
	BoolParam invertDigitalIn1_param = BooleanParameter::make(false, "Invert Digital Input 1", "InvertDigitalIn1");
	BoolParam invertDigitalIn2_param = BooleanParameter::make(false, "Invert Digital Input 2", "InvertDigitalIn2");
	BoolParam invertDigitalIn3_param = BooleanParameter::make(false, "Invert Digital Input 3", "InvertDigitalIn3");
	BoolParam invertDigitalIn4_param = BooleanParameter::make(false, "Invert Digital Input 4", "InvertDigitalIn4");
	BoolParam invertDigitalIn5_param = BooleanParameter::make(false, "Invert Digital Input 5", "InvertDigitalIn5");
	BoolParam invertDigitalIn6_param = BooleanParameter::make(false, "Invert Digital Input 6", "InvertDigitalIn6");
	BoolParam invertDigitalOut7_param = BooleanParameter::make(false, "Invert Digital Output 7", "InvertDigitalOut7");
	
};






class AX5103 : public AX5000, public EtherCatDevice{
	
	DEFINE_ETHERCAT_DEVICE(AX5103, "AX5103", "AX5103", "Beckhoff", "Servo Drives", 0x2, 0x14566012)
	AX5000 ax5000;
	
};


class AX5203 : public AX5000, public EtherCatDevice{
	
	DEFINE_ETHERCAT_DEVICE(AX5203, "AX5203", "AX5203", "Beckhoff", "Servo Drives", 0x2, 0x14566012)
	AX5000 ax5000;
	
};
