#pragma once

#include <vector>
#include <ethercat.h>

enum class FsoeCommand{
	UNKNOWN_COMMAND = 0x0,
	RESET = 0x2A,
	SESSION = 0x4E,
	CONNECTION = 0x64,
	PARAMETER = 0x52,
	PROCESSDATA = 0x36,
	FAILSAFEDATA = 0x08
};

enum class FsoeError{
	RESET = 0,
	INVALID_CMD = 1,
	UNKNOWN_CMD = 2,
	INVALID_CONNID = 3,
	INVALID_CRC = 4,
	WD_EXPIRED = 5,
	INVALID_ADDRESS = 6,
	INVALID_DATA = 7,
	INVALID_COMMPARALEN = 8,
	INVALID_COMPARA = 9,
	INVALID_USERPARALEN = 10,
	INVALID_USERPARA = 11,
	DEVICE_SPECIFIC_ERROR = 0x80
};

struct FsoeFrame{
	
	std::vector<uint8_t> frame;
	bool crc_ok;
	int frameSize;
	int safeDataSize;
	
	void reset(int size){
		frame.clear();
		frame.resize(size, 0x0);
		frameSize = size;
		if(size == 6) safeDataSize = 1;
		else safeDataSize = (size - 3) / 2;
		crc_ok = false;
	}
	
	void readFrom(uint8_t* sourceBuffer, int size){
		for(int i = 0; i < size; i++) frame[i] = sourceBuffer[i];
	}
	void writeTo(uint8_t* outputBuffer, int size){
		for(int i = 0; i < size; i++) outputBuffer[i] = frame[i];
	}
	
	FsoeCommand getCommandType(){
		switch(frame[0]){
			case uint8_t(FsoeCommand::RESET): return FsoeCommand::RESET;
			case uint8_t(FsoeCommand::SESSION): return FsoeCommand::SESSION;
			case uint8_t(FsoeCommand::CONNECTION): return FsoeCommand::CONNECTION;
			case uint8_t(FsoeCommand::PARAMETER): return FsoeCommand::PARAMETER;
			case uint8_t(FsoeCommand::PROCESSDATA): return FsoeCommand::PROCESSDATA;
			case uint8_t(FsoeCommand::FAILSAFEDATA): return FsoeCommand::FAILSAFEDATA;
			default: return FsoeCommand::UNKNOWN_COMMAND;
		}
	}
	void setCommandType(FsoeCommand cmd){ frame[0] = uint8_t(cmd); }
	uint8_t getCommand(){ return frame[0]; }
	
	void setSafeData(std::vector<uint8_t> data){
		for(int i = 0; i < safeDataSize; i++){
			uint8_t newData;
			if(i < data.size()) newData = data[i];
			else newData = 0x0;
			if(i % 2 == 0) frame[i*2+1] = newData;
			else frame[i*2] = newData;
		}
	}
	uint8_t getSafeData(int index){
		if(index % 2 == 0) return frame[index*2+1];
		else return frame[index*2];
	}
	std::vector<uint8_t> getSafeData(){
		std::vector<uint8_t> output(safeDataSize, 0x0);
		for(int i = 0; i < safeDataSize; i++) output[i] = getSafeData(i);
		return output;
	}
	
	void setErrorType(FsoeError error){ frame[1] = uint8_t(error); }
	FsoeError getErrorType(){
		switch(frame[1]){
			case uint8_t(FsoeError::RESET): return FsoeError::RESET;
			case uint8_t(FsoeError::INVALID_CMD): return FsoeError::INVALID_CMD;
			case uint8_t(FsoeError::UNKNOWN_CMD): return FsoeError::UNKNOWN_CMD;
			case uint8_t(FsoeError::INVALID_CONNID): return FsoeError::INVALID_CONNID;
			case uint8_t(FsoeError::INVALID_CRC): return FsoeError::INVALID_CRC;
			case uint8_t(FsoeError::WD_EXPIRED): return FsoeError::WD_EXPIRED;
			case uint8_t(FsoeError::INVALID_ADDRESS): return FsoeError::INVALID_ADDRESS;
			case uint8_t(FsoeError::INVALID_DATA): return FsoeError::INVALID_DATA;
			case uint8_t(FsoeError::INVALID_COMMPARALEN): return FsoeError::INVALID_COMMPARALEN;
			case uint8_t(FsoeError::INVALID_COMPARA): return FsoeError::INVALID_COMPARA;
			case uint8_t(FsoeError::INVALID_USERPARALEN): return FsoeError::INVALID_USERPARALEN;
			case uint8_t(FsoeError::INVALID_USERPARA): return FsoeError::INVALID_USERPARA;
			default: return FsoeError::DEVICE_SPECIFIC_ERROR;
		};
	}
	uint8_t getErrorCode(){ return frame[1]; }
	
	void setCrc(int index, uint16_t crc_i){
		if(safeDataSize == 1){
			frame[2] = crc_i & 0xFF;
			frame[3] = (crc_i >> 8) & 0xFF;
		}
		else{
			frame[index*2+3] = crc_i & 0xFF;
			frame[index*2+4] = (crc_i >> 8) & 0xFF;
		}
	}
	uint16_t getCrc(int index){
		uint16_t crc;
		if(safeDataSize == 1) {
			crc = frame[2] | (frame[3] << 8);
		}
		else{
			crc = frame[index*2+3] | (frame[index*2+4] << 8);
		}
		return crc;
	}
	
	
	void setConnectionID(uint16_t connID){
		frame[frameSize-2] = connID & 0xFF;
		frame[frameSize-1] = (connID >> 8) & 0xFF;
	}
	uint16_t getConnnectionID(){
		uint16_t connID = frame[frameSize-2] | (frame[frameSize-1] << 8);
		return connID;
	}
	
	bool equals(uint8_t* otherFrame, int otherFrameSize){
		if(frameSize != otherFrameSize) return false;
		for(int i = 0; i < frameSize; i++){
			if(frame[i] != otherFrame[i]) return false;
		}
		return true;
	}
	
	bool isSafeDataEqual(FsoeFrame& otherFrame){
		if(safeDataSize != otherFrame.safeDataSize) return false;
		for(int i = 0; i < safeDataSize; i++){
			if(getSafeData(i) != otherFrame.getSafeData(i)) return false;
		}
		return true;
	}
};



class FsoeConnection{
public:
	
	struct Config{
		uint16_t fsoeAddress;
		uint16_t watchdogTimeout_ms;
		std::vector<uint8_t> applicationParameters;
		uint8_t safeOutputsSize;
		uint8_t safeInputsSize;
	};
	
	bool initialize(Config& config_);
	bool sendFrame(uint8_t* fsoeMasterFrame, int frameSize, uint8_t* safeOutputs, int safeOutputsSize);
	bool receiveFrame(uint8_t* fsoeSlaveFrame, int frameSize, uint8_t* safeInputs, int safeInputsSize);
	bool b_sendFailsafeData = false;
	
	enum class MasterState{
		RESET,
		SESSION,
		CONNECTION,
		PARAMETER,
		DATA
	};

	MasterState getState(){ return masterState; }

private:

	MasterState masterState = MasterState::RESET;

	int configurationSafeDataSize;
	int safeOutputsSize = 0;
	int safeInputsSize = 0;
	int masterFrameSize = 0;
	int slaveFrameSize = 0;
		 
	FsoeFrame slaveFrame;
	FsoeFrame masterFrame;

	uint16_t masterSessionId = 0;
	uint16_t fsoeAddress = 0;
	uint8_t connectionData[4];
	std::vector<uint8_t> safeParameters;
	std::vector<uint8_t> safeDataToVerify;
	int bytesToBeSent = 0;
	
	uint64_t lastReceiveTime_nanos = 0;
	bool b_refreshOutputFrame = true;
	
	uint16_t watchdogTimeout_ms = 0;
	uint16_t connectionID = 0;
	uint16_t masterSequenceNumber = 1;
	uint16_t slaveSequenceNumber = 1;
	uint16_t lastMasterCRC_0 = 0x0;
	uint16_t lastSlaveCRC_0 = 0x0;
	uint8_t lastErrorCode = 0;
	bool b_watchdogExpired = false;
	bool b_connectionUp = false;
	
	void updateMasterStateMachine();
	void setResetState(FsoeError error);
	void setSessionState();
	
	std::string errorToString(FsoeError error);
	
	void computeCrc(FsoeFrame& frame, uint16_t& sequenceNumber, uint16_t startCrc, uint16_t oldCrc, bool b_writeCrc);
	void encodeCrcByte(uint16_t& crc, uint8_t data);
	static uint16_t aCrcTab1[256];
	static uint16_t aCrcTab2[256];

};
