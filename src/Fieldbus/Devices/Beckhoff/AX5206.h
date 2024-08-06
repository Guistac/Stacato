#pragma once

#include "Submodules/EL722xActuator.h"

class AX5206 : public EtherCatDevice{
public:
	DEFINE_ETHERCAT_DEVICE(AX5206, "AX5206", "AX5206", "Beckhoff", "Servo Drives", 0x2, 0x14566012)
	
	struct __attribute__((__packed__)) MasterDataTelegram{
		uint16_t ax0_masterControlWord;
		uint32_t ax0_positionCommandValue;
		int32_t ax0_velocityCommandValue;
		int16_t ax0_torqueCommandValue;
		uint16_t digitalOutput;
		uint16_t ax1_masterControlWord;
		uint32_t ax1_positionCommandValue;
		int32_t ax1_velocityCommandValue;
		int16_t ax1_torqueCommandValue;
	}masterDataTelegram;
	
	struct __attribute__((__packed__)) AcknowledgeTelegram{
		uint16_t ax0_driveStatusWord;
		uint32_t ax0_positionFeedbackValue1;
		int32_t ax0_velocityFeedbackValue;
		int16_t ax0_torqueFeedbackValue;
		int32_t ax0_followingDistance;
		uint16_t digitalInputsState;
		uint16_t ax1_driveStatusWord;
		uint32_t ax1_positionFeedbackValue1;
		int32_t ax1_velocityFeedbackValue;
		int16_t ax1_torqueFeedbackValue;
		int32_t ax1_followingDistance;
	}acknowledgeTelegram;
	
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
	};
	
	struct DriveControl{
		bool realtimeBit1 = false;
		bool realtimeBit2 = false;
		uint8_t operatingMode = 0;
		bool syncBit = false;
		bool haltRestart = false;
		bool enable = false;
		bool driveOnOff = false;
		void update(uint16_t& masterControlWord){
			masterControlWord = 0x0;
			masterControlWord |= realtimeBit1 << 6;
			masterControlWord |= realtimeBit2 << 7;
			masterControlWord |= (operatingMode & 0x3) << 8;
			masterControlWord |= syncBit << 10;
			masterControlWord |= (operatingMode & 0x4) << 11;
			masterControlWord |= haltRestart << 13;
			masterControlWord |= enable << 14;
			masterControlWord |= driveOnOff << 15;
		}
	};
	
	DriveStatus ax0_driveStatus;
	DriveStatus ax1_driveStatus;
	
	DriveControl ax0_driveControl;
	DriveControl ax1_driveControl;
	
	bool AM8051_1G20_0000_startupList(uint8_t axis);
	bool AM8052_1J20_0000_startupList(uint8_t axis);
	
	void getInvalidIDNsForSafeOp();
	void getInvalidIDNsForOp();
	void getShutdownErrorList();
	void getErrorHistory();
	void clearErrorHistory();
	void getDiagnosticsMessage();
	
	std::string getErrorString(uint32_t errorCode);
};
