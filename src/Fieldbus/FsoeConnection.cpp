#include "FsoeConnection.h"

bool FsoeConnection::initialize(Config& config){
	
	//check if configuration parameters actually make sense
	if(config.fsoeAddress == 0x0) return false;
	if(config.watchdogTimeout_ms < 1) return false;
	if(config.safeInputsSize < 1 || config.safeOutputsSize < 1) return false;
	if(config.safeOutputsSize != 1 && config.safeOutputsSize % 2 != 0) return false;
	if(config.safeInputsSize != 1 && config.safeInputsSize % 2 != 0) return false;
	
	//the number of safety PDU bytes used to transfer configuration at startup
	configurationSafeDataSize = std::min(config.safeInputsSize, config.safeOutputsSize);
	watchdogTimeout_ms = config.watchdogTimeout_ms;
	fsoeAddress = config.fsoeAddress;
	safeParameters = config.applicationParameters;
	safeInputsSize = config.safeInputsSize;
	safeOutputsSize = config.safeOutputsSize;
	slaveFrameSize = safeInputsSize == 1 ? 6 : (safeInputsSize * 2) + 3;
	masterFrameSize = safeOutputsSize == 1 ? 6 : (safeOutputsSize * 2) + 3;
	
	masterFrame.reset(masterFrameSize);
	slaveFrame.reset(slaveFrameSize);
	
	//generate connection ID.
	//the static variable is increment so every id will be unique
	static uint16_t connectionIdCounter = 0;
	connectionIdCounter++;
	if(connectionIdCounter == 0) connectionIdCounter++;
	connectionID = connectionIdCounter;
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
	
	setResetState(FsoeError::RESET);
	
	return true;
}




bool FsoeConnection::sendFrame(uint8_t* fsoeMasterFrame, int frameSize, uint8_t* safeOutputs, int outputsSize){
	
	if(fsoeMasterFrame == nullptr) return false;
	if(frameSize != masterFrameSize) return false;
	if(safeOutputs == nullptr) return false;
	if(outputsSize != safeOutputsSize) return false;
	
	if(b_refreshOutputFrame){
		//in ProcessData state, copy safeOutputs to safeData
		//in FailSafeData state, copy all zeroes to safeData
		if(masterState == MasterState::DATA){
			std::vector<uint8_t> safeData(outputsSize, 0x0);
			if(!b_sendFailsafeData) for(int i = 0; i < outputsSize; i++) safeData[i] = safeOutputs[i];
			masterFrame.setSafeData(safeData);
		}
		//compute crc, increment sequence number
		computeCrc(masterFrame, masterSequenceNumber, lastSlaveCRC_0, lastMasterCRC_0, true);
		lastMasterCRC_0 = masterFrame.getCrc(0);
		masterSequenceNumber++;
		if(masterSequenceNumber == 0) masterSequenceNumber++;
	}
	
	//write frame to buffer
	masterFrame.writeTo(fsoeMasterFrame, frameSize);
	
	if(true){
		if(frameSize == 6){
			Logger::warn("SEND_{}  {:X} {:X} {:X} {:X} {:X} {:X}", masterSequenceNumber == 0 ? masterSequenceNumber - 2 : masterSequenceNumber - 1,
						 fsoeMasterFrame[0],
						 fsoeMasterFrame[1],
						 fsoeMasterFrame[2],
						 fsoeMasterFrame[3],
						 fsoeMasterFrame[4],
						 fsoeMasterFrame[5]);
		}
		else if(frameSize == 7){
			Logger::warn("SEND_{}  {:X} {:X} {:X} {:X} {:X} {:X} {:X}", masterSequenceNumber == 0 ? masterSequenceNumber - 2 : masterSequenceNumber - 1,
						 fsoeMasterFrame[0],
						 fsoeMasterFrame[1],
						 fsoeMasterFrame[2],
						 fsoeMasterFrame[3],
						 fsoeMasterFrame[4],
						 fsoeMasterFrame[5],
						 fsoeMasterFrame[6]);
		}
	}
	
	return true;
}


bool FsoeConnection::receiveFrame(uint8_t* fsoeSlaveFrame, int frameSize, uint8_t* safeInputs, int inputsSize){
	
	if(fsoeSlaveFrame == nullptr) return false;
	if(frameSize != slaveFrameSize) return false;
	if(safeInputs == nullptr) return false;
	if(inputsSize != safeInputsSize) return false;
	
	//handle frame repetition and watchdog state
	uint64_t now_nanos = Timing::getProgramTime_nanoseconds();
	if(slaveFrame.equals(fsoeSlaveFrame, frameSize)){
		if(now_nanos - lastReceiveTime_nanos > watchdogTimeout_ms * 1000'000){
			b_watchdogExpired = true;
			if(b_connectionUp){
				b_connectionUp = false;
				Logger::warn("Fsoe Watchdog Expired");
			}
			lastReceiveTime_nanos = now_nanos;
			b_refreshOutputFrame = true;
			setResetState(FsoeError::WD_EXPIRED);
			return false;
		}
		else if(b_watchdogExpired){
			setResetState(FsoeError::RESET);
			b_refreshOutputFrame = true;
			return false;
		}
		else{
			//if the same frame was previously received we just send the same frame again
			//until we get a different response or the until watchdog triggers
			b_refreshOutputFrame = false;
			return false;
		}
	}
	else{
		b_watchdogExpired = false;
		b_connectionUp = true;
		lastReceiveTime_nanos = now_nanos;
		b_refreshOutputFrame = true;
	}
		
	if(true){
		if(frameSize == 6){
			Logger::info("	REC_{} {:X} {:X} {:X} {:X} {:X} {:X}", slaveSequenceNumber,
				 fsoeSlaveFrame[0],
				 fsoeSlaveFrame[1],
				 fsoeSlaveFrame[2],
				 fsoeSlaveFrame[3],
				 fsoeSlaveFrame[4],
				 fsoeSlaveFrame[5]);
		}
		else if(frameSize == 7){
			Logger::info("	REC_{} {:X} {:X} {:X} {:X} {:X} {:X} {:X}", slaveSequenceNumber,
				fsoeSlaveFrame[0],
				fsoeSlaveFrame[1],
				fsoeSlaveFrame[2],
				fsoeSlaveFrame[3],
				fsoeSlaveFrame[4],
				fsoeSlaveFrame[5],
				fsoeSlaveFrame[6]);
		}
	}
	
	//read new frame, validate crcs, increment counters
	slaveFrame.readFrom(fsoeSlaveFrame, frameSize);
	lastSlaveCRC_0 = slaveFrame.getCrc(0);
	computeCrc(slaveFrame, slaveSequenceNumber, lastMasterCRC_0, 0x0, false);
	slaveSequenceNumber++;
	if(slaveSequenceNumber == 0) slaveSequenceNumber++;
	
	//prepare next Master PDU
	updateMasterStateMachine();
	
	if(masterState == MasterState::DATA){
		//copy received safeData to SafeInputs
		std::vector<uint8_t> safeData = slaveFrame.getSafeData();
		for(int i = 0; i < inputsSize; i++) safeInputs[i] = safeData[i];
	}
	
	return true;
}


void FsoeConnection::updateMasterStateMachine(){
	bool logState = true;
	bool logStateError = true;
	switch(masterState){
		case MasterState::RESET:
			switch(slaveFrame.getCommandType()){
				case FsoeCommand::RESET:
					if(logState) Logger::error("RESET_OK");
					setSessionState();
					break;
				default:
					if(logState) Logger::error("RESET_STAY_1");
					setResetState(FsoeError::RESET);
					break;
			}
			break;
		case MasterState::SESSION:
			switch(slaveFrame.getCommandType()){
				case FsoeCommand::SESSION:
					if(!slaveFrame.crc_ok){
						if(logStateError) Logger::error("SESSION_FAIL1/STAY2");
						setResetState(FsoeError::INVALID_CRC);
					}
					else if(bytesToBeSent == 0){
						if(logState) Logger::error("SESSION_OK"); //send and receive sessionIDs
						int connDataByteCount = std::min(configurationSafeDataSize, 4);
						std::vector<uint8_t> firstConnData(connDataByteCount, 0x0);
						for(int i = 0; i < connDataByteCount; i++) firstConnData[i] = connectionData[i];
						safeDataToVerify = firstConnData;
						masterFrame.setSafeData(firstConnData);
						bytesToBeSent = 4 - connDataByteCount;
						masterFrame.setCommandType(FsoeCommand::CONNECTION);
						masterFrame.setConnectionID(connectionID);
						masterState = MasterState::CONNECTION;
					}else{
						if(logState) Logger::error("SESSION_STAY1"); //send remaining sessionID byte (high byte)
						masterFrame.setCommandType(FsoeCommand::SESSION);
						uint8_t sessionIDHighByte = (masterSessionId >> 8) & 0xFF;
						safeDataToVerify = {sessionIDHighByte};
						masterFrame.setSafeData(safeDataToVerify);
						bytesToBeSent = 0;
						masterState = MasterState::SESSION;
					}
					break;
				case FsoeCommand::RESET:
					if(logStateError) Logger::error("SESSION_RESET1");
					lastErrorCode = slaveFrame.getErrorCode();
					Logger::error("FSOE Reset by Slave during Session State : {}", errorToString(slaveFrame.getErrorType()));
					setSessionState();
					break;
				case FsoeCommand::CONNECTION:
				case FsoeCommand::PARAMETER:
				case FsoeCommand::PROCESSDATA:
				case FsoeCommand::FAILSAFEDATA:
					if(logStateError) Logger::error("SESSION_FAIL3");
					setResetState(FsoeError::INVALID_CMD);
					break;
				default:
					if(logStateError) Logger::error("SESSION_FAIL4");
					setResetState(FsoeError::UNKNOWN_CMD);
					break;
			}
			break;
		case MasterState::CONNECTION:
			switch(slaveFrame.getCommandType()){
				case FsoeCommand::CONNECTION:
					if(!slaveFrame.crc_ok){
						if(logStateError) Logger::error("CONN_FAIL1");
						setResetState(FsoeError::INVALID_CRC);
					}
					else if(!slaveFrame.isSafeDataEqual(masterFrame)){
						if(logStateError) Logger::error("CONN_FAIL2");
						setResetState(FsoeError::INVALID_DATA);
					}
					else if(slaveFrame.getConnnectionID() != connectionID){
						if(logStateError) Logger::error("CONN_FAIL3");
						setResetState(FsoeError::INVALID_CONNID);
					}
					else if(bytesToBeSent == 0){
						if(logState) Logger::error("CONN_OK"); //Start uploading application parameters
						bytesToBeSent = int(safeParameters.size());
						int currentBytesToBeSent = std::min(configurationSafeDataSize, bytesToBeSent);
						std::vector<uint8_t> currentParameterBytes(currentBytesToBeSent, 0x0);
						for(int i = 0; i < currentBytesToBeSent; i++) currentParameterBytes[i] = safeParameters[i];
						bytesToBeSent -= currentBytesToBeSent;
						safeDataToVerify = currentParameterBytes;
						masterFrame.setSafeData(currentParameterBytes);
						masterFrame.setConnectionID(connectionID);
						masterFrame.setCommandType(FsoeCommand::PARAMETER);
						masterState = MasterState::PARAMETER;
					}
					else{
						if(logState) Logger::error("CONN_STAY"); //Keep uploading connectionData
						int currentConnectionDataByteCount = std::min(configurationSafeDataSize, bytesToBeSent);
						int startIndex = 4 - bytesToBeSent;
						int endIndex = startIndex + currentConnectionDataByteCount;
						std::vector<uint8_t> currentConnectionData(currentConnectionDataByteCount, 0x0);
						for(int i = 0; i < currentConnectionDataByteCount; i++) currentConnectionData[i] = connectionData[startIndex+i];
						safeDataToVerify = currentConnectionData;
						masterFrame.setSafeData(currentConnectionData);
						bytesToBeSent -= currentConnectionDataByteCount;
						masterFrame.setConnectionID(connectionID);
						masterFrame.setCommandType(FsoeCommand::CONNECTION);
						masterState = MasterState::CONNECTION;
					}
					break;
				case FsoeCommand::RESET:
					if(logStateError) Logger::error("CONN_RESET1");
					lastErrorCode = slaveFrame.getErrorCode();
					Logger::error("FSOE Reset by Slave during Connection State : {}", errorToString(slaveFrame.getErrorType()));
					setSessionState();
					break;
				case FsoeCommand::SESSION:
				case FsoeCommand::PARAMETER:
				case FsoeCommand::PROCESSDATA:
				case FsoeCommand::FAILSAFEDATA:
					if(logStateError) Logger::error("CONN_FAIL4");
					setResetState(FsoeError::INVALID_CMD);
					break;
				default:
					if(logStateError) Logger::error("CONN_FAIL5");
					setResetState(FsoeError::UNKNOWN_CMD);
					break;
			}
			break;
		case MasterState::PARAMETER:
			switch(slaveFrame.getCommandType()){
				case FsoeCommand::PARAMETER:
					if(!slaveFrame.crc_ok){
						if(logStateError) Logger::error("PARA_FAIL1");
						setResetState(FsoeError::INVALID_CRC);
					}
					else if(!slaveFrame.isSafeDataEqual(masterFrame)){
						if(logStateError) Logger::error("PARA_FAIL2");
						setResetState(FsoeError::INVALID_DATA);
					}
					else if(slaveFrame.getConnnectionID() != connectionID){
						if(logStateError) Logger::error("PARA_FAIL3");
						setResetState(FsoeError::INVALID_CONNID);
					}
					else if(bytesToBeSent == 0){
						if(logState) Logger::error("PARA_OK"); //start processData transmission
						masterState = MasterState::DATA;
						masterFrame.setCommandType(FsoeCommand::PROCESSDATA);
					}
					else{
						if(logState) Logger::error("PARA_STAY"); //Keep transmitting application data
						int currentParameterDataByteCount = std::min(configurationSafeDataSize, bytesToBeSent);
						int startIndex = int(safeParameters.size()) - bytesToBeSent;
						int endIndex = startIndex + currentParameterDataByteCount;
						std::vector<uint8_t> currentParameterData(currentParameterDataByteCount, 0x0);
						for(int i = 0; i < currentParameterDataByteCount; i++) currentParameterData[i] = safeParameters[startIndex+i];
						bytesToBeSent -= currentParameterDataByteCount;
						safeDataToVerify = currentParameterData;
						masterFrame.setSafeData(currentParameterData);
						masterFrame.setConnectionID(connectionID);
						masterFrame.setCommandType(FsoeCommand::PARAMETER);
						masterState = MasterState::PARAMETER;
					}
					break;
				case FsoeCommand::RESET:
					if(logStateError) Logger::error("PARA_RESET1");
					lastErrorCode = slaveFrame.getErrorCode();
					Logger::error("FSOE Reset by Slave during Parameter State : {}", errorToString(slaveFrame.getErrorType()));
					setSessionState();
					break;
				case FsoeCommand::SESSION:
				case FsoeCommand::CONNECTION:
				case FsoeCommand::PROCESSDATA:
				case FsoeCommand::FAILSAFEDATA:
					//PARA_FAIL4
					if(logStateError) Logger::error("PARA_FAIL4");
					setResetState(FsoeError::INVALID_CMD);
					break;
				default:
					//PARA_FAIL5
					if(logStateError) Logger::error("PARA_FAIL5");
					setResetState(FsoeError::UNKNOWN_CMD);
					break;
			}
			break;
		case MasterState::DATA:
			switch(slaveFrame.getCommandType()){
				case FsoeCommand::PROCESSDATA:
				case FsoeCommand::FAILSAFEDATA:
					if(!slaveFrame.crc_ok){
						if(logStateError) Logger::error("DATA_FAIL1");
						setResetState(FsoeError::INVALID_CRC);
					}
					else if(slaveFrame.getConnnectionID() != connectionID){
						if(logStateError) Logger::error("DATA_FAIL2");
						setResetState(FsoeError::INVALID_CONNID);
					}
					else{
						masterState = MasterState::DATA;
						if(b_sendFailsafeData) {
							if(logState) Logger::error("DATA_OK2");
							masterFrame.setCommandType(FsoeCommand::FAILSAFEDATA);
						}
						else{
							if(logState) Logger::error("DATA_OK1");
							masterFrame.setCommandType(FsoeCommand::PROCESSDATA);
						}
					}
					break;
				case FsoeCommand::RESET:
					if(logStateError) Logger::error("DATA_RESET1");
					lastErrorCode = slaveFrame.getErrorCode();
					Logger::error("FSOE Reset by Slave during Data State : {}", errorToString(slaveFrame.getErrorType()));
					setSessionState();
					break;
				case FsoeCommand::SESSION:
				case FsoeCommand::CONNECTION:
				case FsoeCommand::PARAMETER:
					if(logStateError) Logger::error("DATA_FAIL3");
					setResetState(FsoeError::INVALID_CMD);
					break;
				default:
					if(logStateError) Logger::error("DATA_FAIL4");
					setResetState(FsoeError::UNKNOWN_CMD);
					break;
			}
			break;
	}
}


void FsoeConnection::setResetState(FsoeError error){
	masterState = MasterState::RESET;
	masterFrame.setCommandType(FsoeCommand::RESET);
	masterFrame.setErrorType(error);
	masterFrame.setConnectionID(0);
	masterSequenceNumber = 1;
	slaveSequenceNumber = 1;
	lastSlaveCRC_0 = 0x0;
	lastMasterCRC_0 = 0x0;
	b_sendFailsafeData = true;
}


void FsoeConnection::setSessionState(){
	masterState = MasterState::SESSION;
	masterFrame.setCommandType(FsoeCommand::SESSION);
	masterSessionId = rand();
	uint8_t sessionIDLowByte = masterSessionId & 0xFF;
	uint8_t sessionIDHighByte = (masterSessionId >> 8) & 0xFF;
	if(configurationSafeDataSize == 1){
		safeDataToVerify = {sessionIDLowByte};
		bytesToBeSent = 1;
	}
	else{
		safeDataToVerify = {sessionIDLowByte, sessionIDHighByte};
		bytesToBeSent = 0;
	}
	masterFrame.setSafeData(safeDataToVerify);
	masterFrame.setConnectionID(0);
	masterSequenceNumber = 1;
	slaveSequenceNumber = 1;
	lastSlaveCRC_0 = 0x0;
	lastMasterCRC_0 = 0x0;
	b_sendFailsafeData = true;
}


std::string FsoeConnection::errorToString(FsoeError error){
	switch(error){
		case FsoeError::RESET:					return "Local reset or acknowledgement of a RESET command";
		case FsoeError::INVALID_CMD:			return "Unexpected command (INVALID_CMD)";
		case FsoeError::UNKNOWN_CMD:			return "Unknown command (UNKNOWN_CMD)";
		case FsoeError::INVALID_CONNID:			return "Invalid connection ID (INVALID_CONNID)";
		case FsoeError::INVALID_CRC:			return "CRC error (INVALID_CRC)";
		case FsoeError::WD_EXPIRED:				return "Watchdog has expired (WD_EXPIRED)";
		case FsoeError::INVALID_ADDRESS:		return "Invalid FSoE Slave Address (INVALID_ADDRESS)";
		case FsoeError::INVALID_DATA:			return "Invalid safety data (INVALID_DATA)";
		case FsoeError::INVALID_COMMPARALEN:	return "Invalid communication parameter length (INVALID_COMMPARALEN)";
		case FsoeError::INVALID_COMPARA:		return "Invalid communication parameter data (INVALID_COMPARA)";
		case FsoeError::INVALID_USERPARALEN:	return "Invalid application parameter length (INVALID_USERPARALEN)";
		case FsoeError::INVALID_USERPARA:		return "Invalid application parameter data (INVALID_USERPARA)";
		default: 								return "Invalid SafePara (device-specific)";
	}
}


void FsoeConnection::computeCrc(FsoeFrame& frame, uint16_t& sequenceNumber, uint16_t startCrc, uint16_t oldCrc, bool b_writeCrc){
	
	uint16_t crc_0;
	uint16_t crcCommon;
	
	uint8_t command = frame.getCommand();
	std::vector<uint8_t> safeData = frame.getSafeData();
	int safeDataSize = frame.safeDataSize;
	uint16_t connectionID = frame.getConnnectionID();
	
	bool sequenceIncremented;
	int receiveTries = 0;
	
	//Compute CRC_0, using 1 or 2 data bytes
	do{
		
		crc_0 = 0x0;
		uint8_t startCrc_L = startCrc & 0xFF;
		uint8_t startCrc_H = (startCrc >> 8) & 0xFF;
		uint8_t connectionID_L = connectionID & 0xFF;
		uint8_t connectionID_H = (connectionID >> 8) & 0xFF;
		uint8_t sequenceNo_L = sequenceNumber & 0xFF;
		uint8_t sequenceNo_H = (sequenceNumber >> 8) & 0xFF;
		uint8_t data_0_L = safeData[0];
		
		//encode common crc part
		encodeCrcByte(crc_0, startCrc_L);
		encodeCrcByte(crc_0, startCrc_H);
		encodeCrcByte(crc_0, connectionID_L);
		encodeCrcByte(crc_0, connectionID_H);
		encodeCrcByte(crc_0, sequenceNo_L);
		encodeCrcByte(crc_0, sequenceNo_H);
		encodeCrcByte(crc_0, command);
		crcCommon = crc_0;
		
		//encode crc_0
		encodeCrcByte(crc_0, data_0_L);
		if(safeDataSize > 1){
			uint8_t data_0_H = safeData[1];
			encodeCrcByte(crc_0, data_0_H);
		}
		
		//if we're sending and the new crc equals the old one:
		//we need to increment the sequence number until the new crc is different.
		if(b_writeCrc && crc_0 == oldCrc){
			sequenceNumber++;
			if(sequenceNumber == 0) sequenceNumber++;
			sequenceIncremented = true;
			//Logger::trace("Repeated CRC {:X} on send, incrementing sequence to {}", crc_0, sequenceNumber);
		}
		//if we're receiving and the computed crc doesn't match the received one:
		//we need to increment the slave sequence number until the crcs match.
		//we only do this two times until abandoning (1 should be enough but you never know)
		else if(!b_writeCrc && crc_0 != frame.getCrc(0)){
			receiveTries++;
			if(receiveTries > 2) {
				frame.crc_ok = false;
				return;
			}
			sequenceNumber++;
			if(sequenceNumber == 0) sequenceNumber++;
			sequenceIncremented = true;
			//Logger::trace("no crc match on receive, incrementing sequence [{}] to {}", receiveTries, sequenceNumber);
		}
		//if none of these apply, we don't increment the sequence number
		//it will be done outside of this function
		else sequenceIncremented = false;
		
	}
	while(sequenceIncremented);
	
	//if sending, store computed crc_0 in frame
	if(b_writeCrc) frame.setCrc(0, crc_0);
	
	//check if additional CRC_i need to be computed
	//this is not the case for fsoe frames which have 2 or less safedata bytes
	if(safeDataSize <= 2) {
		if(!b_writeCrc) frame.crc_ok = true;
		return true;
	}
	
	//calculate additional crc_i starting from crc_common
	int maxIndex = int(safeDataSize) / 2;
	for(uint16_t i = 1; i < maxIndex; i++){
		
		uint8_t i_L = i & 0xFF;
		uint8_t i_H = (i >> 8) & 0xFF;
		uint8_t data_i_L = safeData[i*2];
		uint8_t data_i_H = safeData[i*2+1];
		
		uint16_t crc_i = crcCommon;
		encodeCrcByte(crc_i, i_L);
		encodeCrcByte(crc_i, i_H);
		encodeCrcByte(crc_i, data_i_L);
		encodeCrcByte(crc_i, data_i_H);
		
		//if sending, store computed crc_i in frame
		//if receiving, match computed crc_i against received
		if(b_writeCrc) frame.setCrc(i, crc_i);
		else if(crc_i != frame.getCrc(i)) {
			frame.crc_ok = false;
			return;
		}
	}
	
	frame.crc_ok = true;
}


//encode one byte into the crc
void FsoeConnection::encodeCrcByte(uint16_t& crc, uint8_t data){
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
