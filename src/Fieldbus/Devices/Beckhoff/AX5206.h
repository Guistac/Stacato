#pragma once

#include "Submodules/EL722xActuator.h"

class AX5206 : public EtherCatDevice{
public:
	DEFINE_ETHERCAT_DEVICE(AX5206, "AX5206", "AX5206", "Beckhoff", "Servo Drives", 0x2, 0x14566012)
		
	double unitsPerRPM = 17895.697;
	double unitsPerRev = 1048576.0;
	uint64_t enableTimeout_nanos = 300'000'000; //300ms
	
	class AX5206_Servo : public ActuatorInterface{
	public:
		AX5206_Servo(std::shared_ptr<AX5206> drive_, std::string name_, uint8_t channel_){
			drive = drive_;
			name = name_;
			channel = channel_;
		}
		virtual std::string getName() override { return name; }
		virtual std::string getStatusString() override { return drive->getAxisStatus(channel); }
		
		float guiVelocitySliderValue = 0.0;
	private:
		std::string name;
		std::shared_ptr<AX5206> drive;
		uint8_t channel;
	};
	
	class AX5206_Gpio : public GpioInterface{
	public:
		AX5206_Gpio(std::shared_ptr<AX5206> drive_, std::string name_){
			drive = drive_;
			name = name_;
		}
		virtual std::string getName() override { return name; }
		virtual std::string getStatusString() override { return drive->getGpioStatus(); }
	private:
		std::string name;
		std::shared_ptr<AX5206> drive;
	};
	
	struct DriveStatus{
		bool followsCommand;
		bool realtimeBit1;
		bool realtimeBit2;
		uint8_t operatingMode;
		bool infoChange;
		bool warningChange;
		bool shutdownError;
		uint8_t status;
		void update(uint16_t driveStatusWord){
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
	
	struct DriveControl{
		bool b_realtimeBit1 = false;
		bool b_realtimeBit2 = false;
		uint8_t operatingMode = 0;
		bool b_syncBit = false;
		bool b_haltRestart = false;
		bool b_enable = false;
		bool b_driveOnOff = false;
		void update(uint16_t& masterControlWord){
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
		bool requestingEnable(){ return b_haltRestart && b_enable && b_driveOnOff; }
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
		DriveStatus ax0_driveStatus;
		DriveStatus ax1_driveStatus;
		DriveControl ax0_driveControl;
		DriveControl ax1_driveControl;
		bool digitalInput0 = false;
		bool digitalInput1 = false;
		bool digitalInput2 = false;
		bool digitalInput3 = false;
		bool digitalInput4 = false;
		bool digitalInput5 = false;
		bool digitalInput6 = false;
		bool digitalOutput7 = false;
	}processData;
	
	std::shared_ptr<AX5206_Servo> servo0;
	std::shared_ptr<AX5206_Servo> servo1;
	std::shared_ptr<AX5206_Gpio> gpio;
	
	bool AM8051_1G20_0000_startupList(uint8_t axis);
	bool AM8052_1J20_0000_startupList(uint8_t axis);
	
	void getInvalidIDNsForSafeOp();
	void getInvalidIDNsForOp();
	void getShutdownErrorList();
	void getErrorHistory();
	void getDiagnosticsMessage();
	void requestFaultReset(uint8_t axis);
	std::string getAxisStatus(uint8_t axis);
	std::string getGpioStatus();
	
	std::string getErrorString(uint32_t errorCode);
	std::string getClass1Errors(uint16_t class1diagnostics);
};


//Encoder reset
//Current Limit
