#include "FsoeConnection.h"

bool FsoeConnection::initialize(Config& config){
	
	//check if configuration parameters actually make sense
	if(config.fsoeAddress == 0x0) return false;
	if(config.watchdogTimeout_ms < 1) return false;
	if(config.connectionID == 0x0) return false;
	if(config.safeInputsSize < 1 || config.safeOutputsSize < 1) return false;
	if(config.safeOutputsSize != 1 && config.safeOutputsSize % 2 != 0) return false;
	if(config.safeInputsSize != 1 && config.safeInputsSize % 2 != 0) return false;
	
	//the number of safety PDU bytes used to transfer configuration at startup
	configurationSafeDataSize = std::min(config.safeInputsSize, config.safeOutputsSize);
	watchdogTimeout_ms = config.watchdogTimeout_ms;
	fsoeAddress = config.fsoeAddress;
	connectionID = config.connectionID;
	safeParameters = config.applicationParameters;
	safeInputsSize = config.safeInputsSize;
	safeOutputsSize = config.safeOutputsSize;
	slaveFrameSize = safeInputsSize == 1 ? 6 : (safeInputsSize * 2) + 3;
	masterFrameSize = safeOutputsSize == 1 ? 6 : (safeOutputsSize * 2) + 3;
	
	masterFrame.reset(masterFrameSize);
	slaveFrame.reset(slaveFrameSize);
	
	connectionData[0] = connectionID & 0xFF;
	connectionData[1] = (connectionID >> 8) & 0xFF;
	connectionData[2] = fsoeAddress & 0xFF;
	connectionData[3] = (fsoeAddress >> 8) & 0xFF;
	
	uint16_t applicationParameterSize = int(config.applicationParameters.size());
	int safeParameterSize = applicationParameterSize + 6;
	safeParameters.resize(safeParameterSize, 0x0);
	safeParameters[0] = 2;
	safeParameters[1] = 0;
	safeParameters[2] = watchdogTimeout_ms & 0xFF;
	safeParameters[3] = (watchdogTimeout_ms >> 8) & 0xFF;
	safeParameters[4] = applicationParameterSize & 0xFF;
	safeParameters[5] = (applicationParameterSize >> 8) & 0xFF;
	for(int i = 0; i < applicationParameterSize; i++) safeParameters[i+6] = config.applicationParameters[i];
	
	reset();
	
	return true;
}





bool FsoeConnection::sendFrame(uint8_t* fsoeMasterFrame, int frameSize, uint8_t* safeOutputs, int outputsSize){
	
	if(!b_refreshOutputFrame) return false;
	
	if(fsoeMasterFrame == nullptr) return false;
	if(frameSize != masterFrameSize) return false;
	if(safeOutputs == nullptr) return false;
	if(outputsSize != safeOutputsSize) return false;
	
	if(masterState == MasterState::DATA){
		std::vector<uint8_t> safeData(outputsSize, 0x0);
		if(!b_sendFailsafeData){
			for(int i = 0; i < outputsSize; i++) safeData[i] = safeOutputs[i];
		}
		masterFrame.setSafeData(safeData);
	}
	
	uint16_t seq = masterSequenceNumber;
	calcCrC(masterFrame, masterSequenceNumber, lastSlaveCRC_0, FrameDirection::SEND);
	lastMasterCRC_0 = masterFrame.getCrc(0);
	
	masterFrame.writeTo(fsoeMasterFrame, frameSize);
	
	/*
	Logger::warn("SEND_{}  {:X} {:X} {:X} {:X} {:X} {:X}", seq,
		fsoeMasterFrame[0],
		fsoeMasterFrame[1],
		fsoeMasterFrame[2],
		fsoeMasterFrame[3],
		fsoeMasterFrame[4],
		fsoeMasterFrame[5]);
	 */
}





bool FsoeConnection::receiveFrame(uint8_t* fsoeSlaveFrame, int frameSize, uint8_t* safeInputs, int inputsSize){
	
	if(fsoeSlaveFrame == nullptr) return false;
	if(frameSize != slaveFrameSize) return false;
	if(safeInputs == nullptr) return false;
	if(inputsSize != safeInputsSize) return false;
	
	slaveFrame.readFrom(fsoeSlaveFrame, frameSize);
	
	bool wd_expired = false;
	if(previousSlaveFrame.equals(slaveFrame)){
		if(Timing::getProgramTime_seconds() - lastReceiveTime_s > 0.2){
			wd_expired = true;
			lastReceiveTime_s = Timing::getProgramTime_seconds();
			Logger::critical("WD expired");
			b_refreshOutputFrame = true;
			lastSlaveCRC_0 = 0;
			lastMasterCRC_0 = 0;
			masterSequenceNumber = 1;
			slaveSequenceNumber = 1;
			masterFrame.setCommandType(FsoeCommand::RESET);
			masterFrame.setSafeData({0x5});
			masterFrame.setConnectionID(0);
			return;
		}else{
			//if the same frame was previously received we exit out of the reception logic
			//we also just send the same frame again until we get a response or the watchdog triggers
			b_refreshOutputFrame = false;
			return;
		}
	}else{
		lastReceiveTime_s = Timing::getProgramTime_seconds();
		b_refreshOutputFrame = true;
		previousSlaveFrame = slaveFrame;
	}
	
	/*
	Logger::info("	REC_{} {:X} {:X} {:X} {:X} {:X} {:X}", slaveSequenceNumber,
		fsoeSlaveFrame[0],
		fsoeSlaveFrame[1],
		fsoeSlaveFrame[2],
		fsoeSlaveFrame[3],
		fsoeSlaveFrame[4],
		fsoeSlaveFrame[5]);
	*/
	
	lastSlaveCRC_0 = slaveFrame.getCrc(0);
	slaveFrame.crc_ok = calcCrC(slaveFrame, slaveSequenceNumber, lastMasterCRC_0, FrameDirection::RECEIVE);
	if(!slaveFrame.crc_ok) Logger::critical("Received CRC not ok [{}]", slaveSequenceNumber);
	
	bool logState = false;
	
	switch(masterState){
			
		case MasterState::RESET:
			switch(slaveFrame.getCommandType()){
				case FsoeCommand::RESET:
					//RESET_OK
					if(logState) Logger::error("RESET_OK");
					masterState = MasterState::SESSION;
					generateSessionId();
					masterFrame.setCommandType(FsoeCommand::SESSION);
					if(configurationSafeDataSize == 1){
						uint8_t sessionIDLowByte = masterSessionId & 0xFF;
						masterFrame.setSafeData({sessionIDLowByte});
						bytesToBeSent = 1;
					}
					else{
						uint8_t sessionIDLowByte = masterSessionId & 0xFF;
						uint8_t sessionIDHighByte = (masterSessionId >> 8) & 0xFF;
						masterFrame.setSafeData({sessionIDLowByte, sessionIDHighByte});
						bytesToBeSent = 0;
					}
					masterFrame.setConnectionID(0);
					masterSequenceNumber = 1;
					slaveSequenceNumber = 1;
					lastSlaveCRC_0 = 0x0;
					break;
				default:
					//RESET_STAY_1
					if(logState) Logger::error("RESET_STAY_1");
					reset();
					masterFrame.setCommandType(FsoeCommand::RESET);
					masterState = MasterState::RESET;
					masterFrame.setConnectionID(0);
					break;
			}
			break;
			
		case MasterState::SESSION:
			switch(slaveFrame.getCommandType()){
				case FsoeCommand::SESSION:
					if(!slaveFrame.crc_ok){
						//SESSION_FAIL1 ???
						//SESSION_STAY2 ???
						if(logState) Logger::error("SESSION_FAIL1");
						masterFrame.setCommandType(FsoeCommand::RESET);
						masterSequenceNumber = 1;
						slaveSequenceNumber = 1;
						lastSlaveCRC_0 = 0x0;
						lastMasterCRC_0 = 0x0;
						masterFrame.setSafeData({0x4});
						masterState = MasterState::RESET;
					}
					else if(bytesToBeSent == 0){
						//SESSION_OK
						if(logState) Logger::error("SESSION_OK");
						
						int connDataByteCount = std::min(configurationSafeDataSize, 4);
						std::vector<uint8_t> firstConnData(connDataByteCount, 0x0);
						for(int i = 0; i < connDataByteCount; i++) firstConnData[i] = connectionData[i];
						masterFrame.setSafeData(firstConnData);
						bytesToBeSent = 4 - connDataByteCount;
						
						masterFrame.setCommandType(FsoeCommand::CONNECTION);
						masterFrame.setConnectionID(connectionID);
						masterState = MasterState::CONNECTION;
					}else{
						//SESSION_STAY1
						if(logState) Logger::error("SESSION_STAY1");
						//send remaining session ID byte (high byte)
						masterFrame.setCommandType(FsoeCommand::SESSION);
						uint8_t sessionIDHighByte = (masterSessionId >> 8) & 0xFF;
						masterFrame.setSafeData({sessionIDHighByte});
						bytesToBeSent = 0;
						masterState = MasterState::SESSION;
					}
					break;
				case FsoeCommand::RESET:
					//SESSION_RESET1
					lastSlaveCRC_0 = 0x0;
					lastMasterCRC_0 = 0x0;
					masterSequenceNumber = 1;
					slaveSequenceNumber = 1;
					generateSessionId();
					masterFrame.setCommandType(FsoeCommand::SESSION);
					masterState = MasterState::SESSION;
					break;
				case FsoeCommand::CONNECTION:
				case FsoeCommand::PARAMETER:
				case FsoeCommand::PROCESSDATA:
				case FsoeCommand::FAILSAFEDATA:
					//SESSION_FAIL3
					masterFrame.setCommandType(FsoeCommand::RESET);
					masterState = MasterState::RESET;
					break;
				default:
					//SESSION_FAIL4
					masterFrame.setCommandType(FsoeCommand::RESET);
					masterState = MasterState::RESET;
					break;
			}
			break;
			
		case MasterState::CONNECTION:
			switch(slaveFrame.getCommandType()){
				case FsoeCommand::CONNECTION:
					if(!slaveFrame.crc_ok){
						//CONN_FAIL1
						if(logState) Logger::error("CONN_FAIL1");
						masterFrame.setCommandType(FsoeCommand::RESET);
						masterState = MasterState::RESET;
					}
					/*
					else if(!isSafeDataCorrect()){
						//CONN_FAIL2
						Logger::error("CONN_FAIL2");
						masterFrame.setCommandType(FsoeCommand::RESET);
						masterState = MasterState::RESET;
					}
					*/
					else if(slaveFrame.getConnnectionID() != connectionID){
						//CONN_FAIL3
						if(logState) Logger::error("CONN_FAIL3");
						masterFrame.setCommandType(FsoeCommand::RESET);
						masterState = MasterState::RESET;
					}
					else if(bytesToBeSent == 0){
						//CONN_OK
						if(logState) Logger::error("CONN_OK");
						
						bytesToBeSent = int(safeParameters.size());
						int currentBytesToBeSent = std::min(configurationSafeDataSize, bytesToBeSent);
						
						std::vector<uint8_t> currentParameterBytes(currentBytesToBeSent, 0x0);
						for(int i = 0; i < currentBytesToBeSent; i++) currentParameterBytes[i] = safeParameters[i];
						bytesToBeSent -= currentBytesToBeSent;
						
						masterFrame.setSafeData(currentParameterBytes);
						masterFrame.setConnectionID(connectionID);
						masterFrame.setCommandType(FsoeCommand::PARAMETER);
						masterState = MasterState::PARAMETER;
					}
					else{
						//CONN_STAY
						
						if(logState) Logger::error("CONN_STAY");
						
						int currentConnectionDataByteCount = std::min(configurationSafeDataSize, bytesToBeSent);
						int startIndex = 4 - bytesToBeSent;
						int endIndex = startIndex + currentConnectionDataByteCount;
						std::vector<uint8_t> currentConnectionData(currentConnectionDataByteCount, 0x0);
						for(int i = 0; i < currentConnectionDataByteCount; i++) currentConnectionData[i] = connectionData[startIndex+i];
						masterFrame.setSafeData(currentConnectionData);
						bytesToBeSent -= currentConnectionDataByteCount;
						
						masterFrame.setConnectionID(connectionID);
						masterFrame.setCommandType(FsoeCommand::CONNECTION);
						masterState = MasterState::CONNECTION;
					}
					break;
				case FsoeCommand::RESET:
					//CONN_RESET1
					masterFrame.setCommandType(FsoeCommand::SESSION);
					masterState = MasterState::SESSION;
					break;
				case FsoeCommand::SESSION:
				case FsoeCommand::PARAMETER:
				case FsoeCommand::PROCESSDATA:
				case FsoeCommand::FAILSAFEDATA:
					//CONN_FAIL4
					masterFrame.setCommandType(FsoeCommand::RESET);
					masterState = MasterState::RESET;
					break;
				default:
					//CONN_FAIL5
					masterFrame.setCommandType(FsoeCommand::RESET);
					masterState = MasterState::RESET;
					break;
			}
			break;
			
			
		case MasterState::PARAMETER:
			switch(slaveFrame.getCommandType()){
				case FsoeCommand::PARAMETER:
					if(!slaveFrame.crc_ok){
						//PARA_FAIL1
						if(logState) Logger::error("PARA_FAIL1");
						masterFrame.setCommandType(FsoeCommand::RESET);
						masterState = MasterState::RESET;
					}
					/*
					else if(!isSafeDataCorrect()){
						//PARA_FAIL2
						Logger::error("PARA_FAIL2");
						masterFrame.setCommandType(FsoeCommand::RESET);
						masterState = MasterState::RESET;
					}
					*/
					else if(slaveFrame.getConnnectionID() != connectionID){
						//PARA_FAIL3
						if(logState) Logger::error("PARA_FAIL3");
						masterFrame.setCommandType(FsoeCommand::RESET);
						masterState = MasterState::RESET;
					}
					else if(bytesToBeSent == 0){
						//PARA_OK
						if(logState) Logger::error("PARA_OK");
						masterFrame.setCommandType(FsoeCommand::PROCESSDATA);
						masterState = MasterState::DATA;
					}
					else{
						//PARA_STAY
						if(logState) Logger::error("PARA_STAY");
						
						int currentParameterDataByteCount = std::min(configurationSafeDataSize, bytesToBeSent);
						int startIndex = int(safeParameters.size()) - bytesToBeSent;
						int endIndex = startIndex + currentParameterDataByteCount;
						std::vector<uint8_t> currentParameterData(currentParameterDataByteCount, 0x0);
						for(int i = 0; i < currentParameterDataByteCount; i++) currentParameterData[i] = safeParameters[startIndex+i];
						masterFrame.setSafeData(currentParameterData);
						bytesToBeSent -= currentParameterDataByteCount;
						
						masterFrame.setSafeData(currentParameterData);
						masterFrame.setConnectionID(connectionID);
						masterFrame.setCommandType(FsoeCommand::PARAMETER);
						masterState = MasterState::PARAMETER;
					}
					break;
				case FsoeCommand::RESET:
					//PARA_RESET1
					masterFrame.setCommandType(FsoeCommand::SESSION);
					masterState = MasterState::SESSION;
					break;
				case FsoeCommand::SESSION:
				case FsoeCommand::CONNECTION:
				case FsoeCommand::PROCESSDATA:
				case FsoeCommand::FAILSAFEDATA:
					//PARA_FAIL4
					masterFrame.setCommandType(FsoeCommand::RESET);
					masterState = MasterState::RESET;
					break;
				default:
					//PARA_FAIL5
					masterFrame.setCommandType(FsoeCommand::RESET);
					masterState = MasterState::RESET;
					break;
			}
			break;
			
			
		case MasterState::DATA:
			switch(slaveFrame.getCommandType()){
				case FsoeCommand::PROCESSDATA:
				case FsoeCommand::FAILSAFEDATA:
					if(!slaveFrame.crc_ok){
						//DATA_FAIL1
						if(logState) Logger::error("DATA_FAIL1");
						masterFrame.setCommandType(FsoeCommand::RESET);
						masterState = MasterState::RESET;
					}
					else if(slaveFrame.getConnnectionID() != connectionID){
						//DATA_FAIL2
						if(logState) Logger::error("DATA_FAIL2");
						masterFrame.setCommandType(FsoeCommand::RESET);
						masterState = MasterState::RESET;
					}
					else{
						
						std::vector<uint8_t> safeData = slaveFrame.getSafeData();
						for(int i = 0; i < inputsSize; i++) safeInputs[i] = safeData[i];
						
						//DATA_OK2
						if(b_sendFailsafeData) {
							if(logState) Logger::error("DATA_OK2");
							masterFrame.setCommandType(FsoeCommand::FAILSAFEDATA);
						}
						//DATA_OK1
						else{
							if(logState) Logger::error("DATA_OK1");
							masterFrame.setCommandType(FsoeCommand::PROCESSDATA);
						}
						masterState = MasterState::DATA;
					}
					break;
				case FsoeCommand::RESET:
					//DATA_RESET1
					masterFrame.setCommandType(FsoeCommand::SESSION);
					masterState = MasterState::SESSION;
					break;
				case FsoeCommand::SESSION:
				case FsoeCommand::CONNECTION:
				case FsoeCommand::PARAMETER:
					//DATA_FAIL3
					masterFrame.setCommandType(FsoeCommand::RESET);
					masterState = MasterState::RESET;
					break;
				default:
					//DATA_FAIL4
					masterFrame.setCommandType(FsoeCommand::RESET);
					masterState = MasterState::RESET;
					break;
			}
			break;
			
	}
}


void FsoeConnection::reset(){
	masterFrame.setCommandType(FsoeCommand::RESET);
	masterState = MasterState::RESET;
	lastSlaveCRC_0 = 0x0;
	lastMasterCRC_0 = 0x0;
	masterSequenceNumber = 1;
	slaveSequenceNumber = 1;
	b_startup = true;
}





//compare slaveFrame.safeData with lastMasterFrame.safeData
//if they are identical return true
bool FsoeConnection::isSafeDataCorrect(){
	return false;
}





std::string FsoeConnection::errorCodeToString(uint8_t errorCode){
	switch(errorCode){
		case 0:		return "Local reset or acknowledgement of a RESET command";
		case 1:		return "Unexpected command (INVALID_CMD)";
		case 2:		return "Unknown command (UNKNOWN_CMD)";
		case 3:		return "Invalid connection ID (INVALID_CONNID)";
		case 4:		return "CRC error (INVALID_CRC)";
		case 5:		return "Watchdog has expired (WD_EXPIRED)";
		case 6:		return "Invalid FSoE Slave Address (INVALID_ADDRESS)";
		case 7:		return "Invalid safety data (INVALID_DATA)";
		case 8:		return "Invalid communication parameter length (INVALID_COMMPARALEN)";
		case 9:		return "Invalid communication parameter data (INVALID_COMPARA)";
		case 10:	return "Invalid application parameter length (INVALID_USERPARALEN)";
		case 11:	return "Invalid application parameter data (INVALID_USERPARA)";
		default:	return "Invalid SafePara (device-specific)";
	}
}





void FsoeConnection::generateSessionId(){
	//secondSessionFrameSent = false;
	masterSessionId = rand();
}




bool FsoeConnection::calcCrC(FsoeFrame& frame, uint16_t& sequenceNumber, uint16_t oldCrc, FrameDirection direction){
	
	uint16_t crc_0;
	uint16_t crcCommon;
	
	uint8_t command = frame.getCommand();
	std::vector<uint8_t> safeData = frame.getSafeData();
	int safeDataSize = frame.safeDataSize;
	uint16_t connectionID = frame.getConnnectionID();
	
	int repeatedCrcs = 0;
	
	//Compute CRC_0, using 1 or 2 data bytes
	do{
		
		crc_0 = 0x0;
		uint8_t oldCrc_L = oldCrc & 0xFF;
		uint8_t oldCrc_H = (oldCrc >> 8) & 0xFF;
		uint8_t connectionID_L = connectionID & 0xFF;
		uint8_t connectionID_H = (connectionID >> 8) & 0xFF;
		uint8_t sequenceNo_L = sequenceNumber & 0xFF;
		uint8_t sequenceNo_H = (sequenceNumber >> 8) & 0xFF;
		uint8_t data_0_L = safeData[0];
		
		//encode common crc part
		encodeCrc(crc_0, oldCrc_L);
		encodeCrc(crc_0, oldCrc_H);
		encodeCrc(crc_0, connectionID_L);
		encodeCrc(crc_0, connectionID_H);
		encodeCrc(crc_0, sequenceNo_L);
		encodeCrc(crc_0, sequenceNo_H);
		encodeCrc(crc_0, command);
		crcCommon = crc_0;
		
		//encode safeData
		encodeCrc(crc_0, data_0_L);
		if(safeDataSize > 1){
			uint8_t data_0_H = safeData[1];
			encodeCrc(crc_0, data_0_H);
		}
		
		//increment sequence number
		sequenceNumber++;
		if(sequenceNumber == 0) sequenceNumber++;
		
		if(repeatedCrcs > 0) Logger::warn("Repeated crc [{}]", repeatedCrcs);
		repeatedCrcs++;
	}
	while(crc_0 == oldCrc);
	
	//if receiving, match computed crc_0 against received
	//if sending, store computed crc_0 in frame
	switch(direction){
		case FrameDirection::RECEIVE:
			if(crc_0 != frame.getCrc(0)) return false;
			break;
		case FrameDirection::SEND:
			frame.setCrc(0, crc_0);
			break;
	}
	
	//check if additional CRC_i need to be computed
	//this is not the case for fsoe frames which have 2 or less safedata bytes
	if(safeDataSize <= 2) return true;
	
	//calculate additional crc_i starting from crc_common
	int maxIndex = int(safeDataSize) / 2;
	for(uint16_t i = 1; i < maxIndex; i++){
		
		uint8_t i_L = i & 0xFF;
		uint8_t i_H = (i >> 8) & 0xFF;
		uint8_t data_i_L = safeData[i*2];
		uint8_t data_i_H = safeData[i*2+1];
		
		uint16_t crc_i = crcCommon;
		encodeCrc(crc_i, i_L);
		encodeCrc(crc_i, i_H);
		encodeCrc(crc_i, data_i_L);
		encodeCrc(crc_i, data_i_H);
		
		//if receiving, match computed crc_i against received
		//if sending, store computed crc_i in frame
		switch(direction){
			case FrameDirection::RECEIVE:
				if(crc_i != frame.getCrc(i)) return false;
				break;
			case FrameDirection::SEND:
				frame.setCrc(i, crc_i);
				break;
		}
	}
	
	//if receiving, returns crc match
	//else it always returns true
	return true;
}


void FsoeConnection::encodeCrc(uint16_t& crc, uint8_t data){
	uint8_t crc_H = (crc >> 8) & 0xFF;
	uint8_t crc_L = crc & 0xFF;
	
	uint16_t w1 = aCrcTab1[crc_H];
	uint16_t w2 = aCrcTab2[data];
	w1 ^= w2;
	
	uint8_t w1_H = (w1 >> 8) & 0xFF;
	uint8_t w1_L = w1 & 0xFF;
	
	w1_H ^= crc_L;
	crc = (w1_H << 8) | w1_L;
}

uint16_t FsoeConnection::aCrcTab1[256] = {
	0x0000, 0x39B7, 0x736E, 0x4AD9, 0xE6DC, 0xDF6B, 0x95B2, 0xAC05, 0xF40F, 0xCDB8,
	0x8761, 0xBED6, 0x12D3, 0x2B64, 0x61BD, 0x580A, 0xD1A9, 0xE81E, 0xA2C7, 0x9B70,
	0x3775, 0x0EC2, 0x441B, 0x7DAC, 0x25A6, 0x1C11, 0x56C8, 0x6F7F, 0xC37A, 0xFACD,
	0xB014, 0x89A3, 0x9AE5, 0xA352, 0xE98B, 0xD03C, 0x7C39, 0x458E, 0x0F57, 0x36E0,
	0x6EEA, 0x575D, 0x1D84, 0x2433, 0x8836, 0xB181, 0xFB58, 0xC2EF, 0x4B4C, 0x72FB,
	0x3822, 0x0195, 0xAD90, 0x9427, 0xDEFE, 0xE749, 0xBF43, 0x86F4, 0xCC2D, 0xF59A,
	0x599F, 0x6028, 0x2AF1, 0x1346, 0x0C7D, 0x35CA, 0x7F13, 0x46A4, 0xEAA1, 0xD316,
	0x99CF, 0xA078, 0xF872, 0xC1C5, 0x8B1C, 0xB2AB, 0x1EAE, 0x2719, 0x6DC0, 0x5477,
	0xDDD4, 0xE463, 0xAEBA, 0x970D, 0x3B08, 0x02BF, 0x4866, 0x71D1, 0x29DB, 0x106C,
	0x5AB5, 0x6302, 0xCF07, 0xF6B0, 0xBC69, 0x85DE, 0x9698, 0xAF2F, 0xE5F6, 0xDC41,
	0x7044, 0x49F3, 0x032A, 0x3A9D, 0x6297, 0x5B20, 0x11F9, 0x284E, 0x844B, 0xBDFC,
	0xF725, 0xCE92, 0x4731, 0x7E86, 0x345F, 0x0DE8, 0xA1ED, 0x985A, 0xD283, 0xEB34,
	0xB33E, 0x8A89, 0xC050, 0xF9E7, 0x55E2, 0x6C55, 0x268C, 0x1F3B, 0x18FA, 0x214D,
	0x6B94, 0x5223, 0xFE26, 0xC791, 0x8D48, 0xB4FF, 0xECF5, 0xD542, 0x9F9B, 0xA62C,
	0x0A29, 0x339E, 0x7947, 0x40F0, 0xC953, 0xF0E4, 0xBA3D, 0x838A, 0x2F8F, 0x1638,
	0x5CE1, 0x6556, 0x3D5C, 0x04EB, 0x4E32, 0x7785, 0xDB80, 0xE237, 0xA8EE, 0x9159,
	0x821F, 0xBBA8, 0xF171, 0xC8C6, 0x64C3, 0x5D74, 0x17AD, 0x2E1A, 0x7610, 0x4FA7,
	0x057E, 0x3CC9, 0x90CC, 0xA97B, 0xE3A2, 0xDA15, 0x53B6, 0x6A01, 0x20D8, 0x196F,
	0xB56A, 0x8CDD, 0xC604, 0xFFB3, 0xA7B9, 0x9E0E, 0xD4D7, 0xED60, 0x4165, 0x78D2,
	0x320B, 0x0BBC, 0x1487, 0x2D30, 0x67E9, 0x5E5E, 0xF25B, 0xCBEC, 0x8135, 0xB882,
	0xE088, 0xD93F, 0x93E6, 0xAA51, 0x0654, 0x3FE3, 0x753A, 0x4C8D, 0xC52E, 0xFC99,
	0xB640, 0x8FF7, 0x23F2, 0x1A45, 0x509C, 0x692B, 0x3121, 0x0896, 0x424F, 0x7BF8,
	0xD7FD, 0xEE4A, 0xA493, 0x9D24, 0x8E62, 0xB7D5, 0xFD0C, 0xC4BB, 0x68BE, 0x5109,
	0x1BD0, 0x2267, 0x7A6D, 0x43DA, 0x0903, 0x30B4, 0x9CB1, 0xA506, 0xEFDF, 0xD668,
	0x5FCB, 0x667C, 0x2CA5, 0x1512, 0xB917, 0x80A0, 0xCA79, 0xF3CE, 0xABC4, 0x9273,
	0xD8AA, 0xE11D, 0x4D18, 0x74AF, 0x3E76, 0x07C1
};

uint16_t FsoeConnection::aCrcTab2[256] = {
	0x0000, 0x7648, 0xEC90, 0x9AD8, 0xE097, 0x96DF, 0x0C07, 0x7A4F, 0xF899, 0x8ED1,
	0x1409, 0x6241, 0x180E, 0x6E46, 0xF49E, 0x82D6, 0xC885, 0xBECD, 0x2415, 0x525D,
	0x2812, 0x5E5A, 0xC482, 0xB2CA, 0x301C, 0x4654, 0xDC8C, 0xAAC4, 0xD08B, 0xA6C3,
	0x3C1B, 0x4A53, 0xA8BD, 0xDEF5, 0x442D, 0x3265, 0x482A, 0x3E62, 0xA4BA, 0xD2F2,
	0x5024, 0x266C, 0xBCB4, 0xCAFC, 0xB0B3, 0xC6FB, 0x5C23, 0x2A6B, 0x6038, 0x1670,
	0x8CA8, 0xFAE0, 0x80AF, 0xF6E7, 0x6C3F, 0x1A77, 0x98A1, 0xEEE9, 0x7431, 0x0279,
	0x7836, 0x0E7E, 0x94A6, 0xE2EE, 0x68CD, 0x1E85, 0x845D, 0xF215, 0x885A, 0xFE12,
	0x64CA, 0x1282, 0x9054, 0xE61C, 0x7CC4, 0x0A8C, 0x70C3, 0x068B, 0x9C53, 0xEA1B,
	0xA048, 0xD600, 0x4CD8, 0x3A90, 0x40DF, 0x3697, 0xAC4F, 0xDA07, 0x58D1, 0x2E99,
	0xB441, 0xC209, 0xB846, 0xCE0E, 0x54D6, 0x229E, 0xC070, 0xB638, 0x2CE0, 0x5AA8,
	0x20E7, 0x56AF, 0xCC77, 0xBA3F, 0x38E9, 0x4EA1, 0xD479, 0xA231, 0xD87E, 0xAE36,
	0x34EE, 0x42A6, 0x08F5, 0x7EBD, 0xE465, 0x922D, 0xE862, 0x9E2A, 0x04F2, 0x72BA,
	0xF06C, 0x8624, 0x1CFC, 0x6AB4, 0x10FB, 0x66B3, 0xFC6B, 0x8A23, 0xD19A, 0xA7D2,
	0x3D0A, 0x4B42, 0x310D, 0x4745, 0xDD9D, 0xABD5, 0x2903, 0x5F4B, 0xC593, 0xB3DB,
	0xC994, 0xBFDC, 0x2504, 0x534C, 0x191F, 0x6F57, 0xF58F, 0x83C7, 0xF988, 0x8FC0,
	0x1518, 0x6350, 0xE186, 0x97CE, 0x0D16, 0x7B5E, 0x0111, 0x7759, 0xED81, 0x9BC9,
	0x7927, 0x0F6F, 0x95B7, 0xE3FF, 0x99B0, 0xEFF8, 0x7520, 0x0368, 0x81BE, 0xF7F6,
	0x6D2E, 0x1B66, 0x6129, 0x1761, 0x8DB9, 0xFBF1, 0xB1A2, 0xC7EA, 0x5D32, 0x2B7A,
	0x5135, 0x277D, 0xBDA5, 0xCBED, 0x493B, 0x3F73, 0xA5AB, 0xD3E3, 0xA9AC, 0xDFE4,
	0x453C, 0x3374, 0xB957, 0xCF1F, 0x55C7, 0x238F, 0x59C0, 0x2F88, 0xB550, 0xC318,
	0x41CE, 0x3786, 0xAD5E, 0xDB16, 0xA159, 0xD711, 0x4DC9, 0x3B81, 0x71D2, 0x079A,
	0x9D42, 0xEB0A, 0x9145, 0xE70D, 0x7DD5, 0x0B9D, 0x894B, 0xFF03, 0x65DB, 0x1393,
	0x69DC, 0x1F94, 0x854C, 0xF304, 0x11EA, 0x67A2, 0xFD7A, 0x8B32, 0xF17D, 0x8735,
	0x1DED, 0x6BA5, 0xE973, 0x9F3B, 0x05E3, 0x73AB, 0x09E4, 0x7FAC, 0xE574, 0x933C,
	0xD96F, 0xAF27, 0x35FF, 0x43B7, 0x39F8, 0x4FB0, 0xD568, 0xA320, 0x21F6, 0x57BE,
	0xCD66, 0xBB2E, 0xC161, 0xB729, 0x2DF1, 0x5BB9
};
