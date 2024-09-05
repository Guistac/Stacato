#pragma once

#include "Submodules/EL722xActuator.h"

class AX5206 : public EtherCatDevice{
public:
	DEFINE_ETHERCAT_DEVICE(AX5206, "AX5206", "AX5206", "Beckhoff", "Servo Drives", 0x2, 0x14566012)
	
	double velMin = -90;
	double velMax = 90.0;
	
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
	
	struct __attribute__((__packed__)) MasterDataTelegram{
		uint16_t ax0_masterControlWord;
		uint32_t ax0_positionCommandValue;
		int32_t ax0_velocityCommandValue;
		uint16_t digitalOutput;
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
		uint16_t ax1_driveStatusWord;
		uint32_t ax1_positionFeedbackValue1;
		int32_t ax1_velocityFeedbackValue;
		int16_t ax1_torqueFeedbackValue;
		uint16_t ax1_class1Diagnostics;
	}acknowledgeTelegram;
	
	struct ProcessData{
		bool b_stoActive = false;
		bool digitalInput0 = false;
		bool digitalInput1 = false;
		bool digitalInput2 = false;
		bool digitalInput3 = false;
		bool digitalInput4 = false;
		bool digitalInput5 = false;
		bool digitalInput6 = false;
		bool digitalOutput7 = false;
	}processData;
	
	void getInvalidIDNsForSafeOp();
	void getInvalidIDNsForOp();
	void getShutdownErrorList();
	void getErrorHistory();
	void getDiagnosticsMessage();
	void requestFaultReset(uint8_t axis);
	std::string getGpioStatus();
	std::string getErrorString(uint32_t errorCode);
	std::string getClass1Errors(uint16_t class1diagnostics);
	
	enum class MotorType{
		NO_MOTOR = 0,
		AM8051_1G20 = 1,
		AM8052_1J20 = 2
	};
	
	bool AM8051_1G20_0000_startupList(uint8_t axis);
	bool AM8052_1J20_0000_startupList(uint8_t axis);
	
	void uploadMotorConfiguration(uint8_t axisNumber, MotorType type){
		switch(type){
			case MotorType::AM8051_1G20: AM8051_1G20_0000_startupList(axisNumber); break;
			case MotorType::AM8052_1J20: AM8052_1J20_0000_startupList(axisNumber); break;
			default:
			case MotorType::NO_MOTOR:
				break;
		}
	}
	
	OptionParameter::Option option_motor_none = OptionParameter::Option(int(MotorType::NO_MOTOR), "No Motor", "NoMotor");
	OptionParameter::Option option_motor_am8051_1g20 = OptionParameter::Option(int(MotorType::AM8051_1G20), "AM8051-1G20", "AM8051-1G20");
	OptionParameter::Option option_motor_am8052_1j20 = OptionParameter::Option(int(MotorType::AM8052_1J20), "AM8052-1J20", "AM8052-1J20");
	std::vector<OptionParameter::Option*> motorTypeOptions = {
		&option_motor_none,
		&option_motor_am8051_1g20,
		&option_motor_am8052_1j20
	};
	
	class Axis : public ActuatorInterface{
	public:
		Axis(std::shared_ptr<AX5206> d, std::string n, uint8_t c) : drive(d), name(n), channel(c){
			motorType = OptionParameter::make2(drive->option_motor_none, drive->motorTypeOptions, "Motor Type", "MotorType");
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
		OptionParam motorType;
		MotorType getMotorType(){ return MotorType(motorType->value); }
		NumberParam<double> velocityLimit_revps = NumberParameter<double>::make(0.0, "Velocity Limit", "VelocityLimit", "%.1f", Units::AngularDistance::Revolution, false, 0, 0, "", "/s");
		NumberParam<double> accelerationLimit_revps2 = NumberParameter<double>::make(0.0, "Acceleration Limit", "AccelerationLimit", "%.1f", Units::AngularDistance::Revolution, false, 0, 0, "", "/s\xc2\xb2");
		NumberParam<double> positionFollowingErrorLimit_rev = NumberParameter<double>::make(0.0, "Position Following Error Limit", "PositionFollowingErrorLimit", "%.1f", Units::AngularDistance::Revolution, false);
		NumberParam<double> currentLimit_amps = NumberParameter<double>::make(0.0, "Current Limit", "CurrentLimit", "%.3f", Units::Current::Ampere, false);
		BoolParam invertDirection_param = BooleanParameter::make(false, "Invert Direction", "InvertDirection");
		bool save(tinyxml2::XMLElement* xml);
		bool load(tinyxml2::XMLElement* xml);

		std::string name;
		std::shared_ptr<AX5206> drive;
		uint8_t channel;
		const double unitsPerRPM = 17895.697;
		const double unitsPerRev = 1048576.0;
		const uint64_t enableTimeout_nanos = 500'000'000; //300ms
	};
	
	class Gpio : public GpioInterface{
	public:
		Gpio(std::shared_ptr<AX5206> d, std::string n) : drive(d), name(n){}
		virtual std::string getName() override { return name; }
		virtual std::string getStatusString() override { return drive->getGpioStatus(); }
	private:
		std::string name;
		std::shared_ptr<AX5206> drive;
	};
	
	std::shared_ptr<Axis> axis0;
	std::shared_ptr<Axis> axis1;
	std::shared_ptr<Gpio> gpio;
	
	std::shared_ptr<bool> STOValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalin0Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalin1Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalin2Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalin3Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalin4Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalin5Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalin6Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalout7Value = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> actuator0Pin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Actuator 0");
	std::shared_ptr<NodePin> actuator1Pin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Actuator 1");
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


//TODO: Error Logging
//TODO: OpMode Selection Setting
//TODO: Encoder reset

//velocity limit
//S-0-0091 Bipolar velocity limit value

//acceleration limit
//S-0-0136 Positive acceleration limit value
//S-x-0137 Negative acceleration limit value

//following error limit
//S-0-0159 Monitoring Window

//current/torque limit
//AX5206 peak amplifier current 26.0A, rated current 12.0A
//AX5206 channel peak current 13.0A, rated current 9.0A
//P-0-0092 configured channel peak current
//P-0-0093 configured channel current
//S-0-0092 Bipolar torque limit value

//fault reaction / deceleration ramp
//P-0-0350 Error reaction control word
//S-0-0429 Emergency stop deceleration


/*

 AX5206 Driver last features
 Mise en route romaine
 Mise en route tournettes
 Mise en route vols
 Boutons Console
 Tactile Console

 Siegfried, séquences pétales
 
*/
