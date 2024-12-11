#pragma once

enum class FsoeCommand{
	UNKNOWN_COMMAND = 0x0,
	RESET = 0x2A,
	SESSION = 0x4E,
	CONNECTION = 0x64,
	PARAMETER = 0x52,
	PROCESSDATA = 0x36,
	FAILSAFEDATA = 0x08
};

struct FsoeFrame{
	
	std::vector<uint8_t> frame;
	
	bool crc_ok;
	int frameSize;
	int safeDataSize;
	
	void reset(int size){
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
		frame.resize(frameSize, 0x0);
		for(int i = 0; i < data.size(); i++){
			if(i % 2 == 0) frame[i*2+1] = data[i];
			else frame[i*2] = data[i];
		}
	}
	std::vector<uint8_t> getSafeData(){
		std::vector<uint8_t> output(safeDataSize, 0x0);
		for(int i = 0; i < safeDataSize; i++){
			if(i % 2 == 0) output[i] = frame[i*2+1];
			else output[i] = frame[i*2];
		}
		return output;
	}
	uint8_t getSafeData(int index){
		if(index % 2 == 0) return frame[index*2+1];
		else return frame[index*2];
	}
	
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
	
	bool equals(FsoeFrame& otherFrame){
		if(frameSize != otherFrame.frameSize) return false;
		for(int i = 0; i < frameSize; i++){
			if(frame[i] != otherFrame.frame[i]) return false;
		}
		return true;
	}
};



class FsoeConnection{
public:
	
	struct Config{
		uint16_t fsoeAddress;
		uint16_t connectionID;
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
	}masterState = MasterState::RESET;
	
	std::string errorCodeToString(uint8_t errorCode);
	
private:
	int configurationSafeDataSize;
	int safeOutputsSize = 0;
	int safeInputsSize = 0;
	int masterFrameSize = 0;
	int slaveFrameSize = 0;
	
	uint16_t watchdogTimeout_ms = 0;
	uint16_t fsoeAddress = 0;
	uint16_t connectionID = 0;
	std::vector<uint8_t> safeParameters;
	
	double lastReceiveTime_s = 0.0;
	bool b_refreshOutputFrame = true;
	uint8_t connectionData[4] = {0,0,0,0};
	
	/*
	uint16_t lastCrc = 0;
	uint16_t oldMasterCrc = 0;
	uint16_t oldSlaveCrc = 0;
	uint16_t masterSeqNo = 0;
	uint16_t slaveSeqNo = 0;
	uint16_t sessionId = 0;
	uint16_t bytesToBeSent = 0;
	uint32_t connData;
	uint16_t safeParaSize = 0;
	uint16_t commFaultReason;
	bool secondSessionFrameSent;
	*/
	 
	FsoeFrame previousSlaveFrame;
	FsoeFrame slaveFrame;
	FsoeFrame masterFrame;

	uint16_t masterSessionId = 0;
	uint16_t masterSequenceNumber = 1;
	uint16_t slaveSequenceNumber = 1;
	uint16_t lastMasterCRC_0 = 0x0;
	uint16_t lastSlaveCRC_0 = 0x0;
	int bytesToBeSent = 0;
	
	bool decodeSlaveFrame(uint8_t* frameRaw, int frameSize);
	bool encodeMasterFrame(uint8_t* output, int frameSize);
	
	enum class FrameDirection{
		RECEIVE,
		SEND
	};
	
	
	
	bool calcCrC(FsoeFrame& frame, uint16_t& sequenceNumber, uint16_t oldCrc, FrameDirection direction);
	void encodeCrc(uint16_t& crc, uint8_t data);
	
	void reset();
	bool b_startup = true;
	
	bool isSafeDataCorrect();
	
	void generateSessionId();
	
	static uint16_t aCrcTab1[256];
	static uint16_t aCrcTab2[256];
};
