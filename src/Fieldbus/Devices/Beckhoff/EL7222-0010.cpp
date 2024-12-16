#include "EL7222-0010.h"

#include "Fieldbus/EtherCatFieldbus.h"

void EL7222_0010::onDisconnection() {
	actuator1->onDisconnection();
	actuator2->onDisconnection();
	gpio->state = DeviceState::OFFLINE;
}

void EL7222_0010::onConnection() {
	gpio->state = DeviceState::ENABLED;
}

void EL7222_0010::initialize() {
	auto thisEtherCatDevice = std::static_pointer_cast<EtherCatDevice>(shared_from_this());
	actuator1 = std::make_shared<EL722x_Actuator>(thisEtherCatDevice, 1);
	actuator2 = std::make_shared<EL722x_Actuator>(thisEtherCatDevice, 2);
	gpio = std::make_shared<EL7222Gpio>(thisEtherCatDevice);
	actuator1->initialize();
	actuator2->initialize();
	
	pinInversionParameters.push_back(BooleanParameter::make(false, "Invert A1", "InvertA1"));
	pinInversionParameters.push_back(BooleanParameter::make(false, "Invert A2", "InvertA2"));
	pinInversionParameters.push_back(BooleanParameter::make(false, "Invert B1", "InvertB1"));
	pinInversionParameters.push_back(BooleanParameter::make(false, "Invert B2", "InvertB2"));
	
	devicePin = std::make_shared<NodePin>(NodePin::DataType::GPIO_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Device");
	devicePin->assignData(std::static_pointer_cast<GpioInterface>(gpio));
	
	for(int i = 0; i < 4; i++) digitalInputValues.push_back(std::make_shared<bool>(false));
	digitalInputPins.push_back(std::make_shared<NodePin>(digitalInputValues[0], NodePin::Direction::NODE_OUTPUT, "1A"));
	digitalInputPins.push_back(std::make_shared<NodePin>(digitalInputValues[1], NodePin::Direction::NODE_OUTPUT, "2A"));
	digitalInputPins.push_back(std::make_shared<NodePin>(digitalInputValues[2], NodePin::Direction::NODE_OUTPUT, "1B"));
	digitalInputPins.push_back(std::make_shared<NodePin>(digitalInputValues[3], NodePin::Direction::NODE_OUTPUT, "2B"));
	
	addNodePin(actuator1->actuatorPin);
	addNodePin(actuator2->actuatorPin);
	addNodePin(devicePin);
	for(int i = 0; i < 4; i++) addNodePin(digitalInputPins[i]);
	
	rxPdoAssignement.addNewModule(0x1610);
	rxPdoAssignement.addEntry(0x7010, 0x1, 16, "CH1 Control Word", &actuator1->rxPdo.controlWord);
	rxPdoAssignement.addNewModule(0x1611);
	rxPdoAssignement.addEntry(0x7010, 0x5, 32, "CH1 Target Position", &actuator1->rxPdo.targetPosition);
	rxPdoAssignement.addNewModule(0x1612);
	rxPdoAssignement.addEntry(0x7010, 0x6, 32, "CH1 Target Velocity", &actuator1->rxPdo.targetVelocity);
	rxPdoAssignement.addNewModule(0x1613);
	rxPdoAssignement.addEntry(0x7010, 0x9, 16, "CH1 Target Torque", &actuator1->rxPdo.targetTorque);
	rxPdoAssignement.addNewModule(0x161A);
	rxPdoAssignement.addEntry(0x7010, 0x3, 8, "CH1 Modes of Operation", &actuator1->rxPdo.modeOfOperationSelection);
	
	rxPdoAssignement.addNewModule(0x1650);
	rxPdoAssignement.addEntry(0x7110, 0x1, 16, "CH2 Control Word", &actuator2->rxPdo.controlWord);
	rxPdoAssignement.addNewModule(0x1651);
	rxPdoAssignement.addEntry(0x7110, 0x5, 32, "CH2 Target Position", &actuator2->rxPdo.targetPosition);
	rxPdoAssignement.addNewModule(0x1652);
	rxPdoAssignement.addEntry(0x7110, 0x6, 32, "CH2 Target Velocity", &actuator2->rxPdo.targetVelocity);
	rxPdoAssignement.addNewModule(0x1653);
	rxPdoAssignement.addEntry(0x7110, 0x9, 16, "CH2 Target Torque", &actuator2->rxPdo.targetTorque);
	rxPdoAssignement.addNewModule(0x165A);
	rxPdoAssignement.addEntry(0x7110, 0x3, 8, "CH2 Modes of Operation", &actuator2->rxPdo.modeOfOperationSelection);
	
	txPdoAssignement.addNewModule(0x1A00);
	txPdoAssignement.addEntry(0x6000, 0x11, 32, "CH1 FB Position", &actuator1->txPdo.fbPosition);
	txPdoAssignement.addNewModule(0x1A02);
	txPdoAssignement.addEntry(0x6000, 0xE, 16, "CH1 FB Status", &actuator1->txPdo.fbStatus);
	txPdoAssignement.addNewModule(0x1A10);
	txPdoAssignement.addEntry(0x6010, 0x1, 16, "CH1 Statusword", &actuator1->txPdo.statusWord);
	txPdoAssignement.addNewModule(0x1A11);
	txPdoAssignement.addEntry(0x6010, 0x6, 32, "CH1 Following Error Actual Value", &actuator1->txPdo.followingErrorActualValue);
	txPdoAssignement.addNewModule(0x1A12);
	txPdoAssignement.addEntry(0x6010, 0x7, 32, "CH1 Velocity Actual Value", &actuator1->txPdo.velocityActualValue);
	txPdoAssignement.addNewModule(0x1A13);
	txPdoAssignement.addEntry(0x6010, 0x8, 16, "CH1 Torque Actual Value", &actuator1->txPdo.torqueActualValue);
	txPdoAssignement.addNewModule(0x1A14);
	txPdoAssignement.addEntry(0x6010, 0x1, 16, "CH1 Info Data 1", &txPdo.infoData1_digitalInputs);
	txPdoAssignement.addNewModule(0x1A17);
	txPdoAssignement.addEntry(0x6010, 0x3, 8, "CH1 Modes of Operation Display", &actuator1->txPdo.modeOfOperationDisplay);
	
	txPdoAssignement.addNewModule(0x1A40);
	txPdoAssignement.addEntry(0x6100, 0x11, 32, "CH2 FB Position", &actuator2->txPdo.fbPosition);
	txPdoAssignement.addNewModule(0x1A42);
	txPdoAssignement.addEntry(0x6100, 0xE, 16, "CH2 FB Status", &actuator2->txPdo.fbStatus);
	txPdoAssignement.addNewModule(0x1A50);
	txPdoAssignement.addEntry(0x6110, 0x1, 16, "CH2 Statusword", &actuator2->txPdo.statusWord);
	txPdoAssignement.addNewModule(0x1A51);
	txPdoAssignement.addEntry(0x6110, 0x6, 32, "CH2 Following Error Actual Value", &actuator2->txPdo.followingErrorActualValue);
	txPdoAssignement.addNewModule(0x1A52);
	txPdoAssignement.addEntry(0x6110, 0x7, 32, "CH2 Velocity Actual Value", &actuator2->txPdo.velocityActualValue);
	txPdoAssignement.addNewModule(0x1A53);
	txPdoAssignement.addEntry(0x6110, 0x8, 16, "CH2 Torque Actual Value", &actuator2->txPdo.torqueActualValue);
	txPdoAssignement.addNewModule(0x1A57);
	txPdoAssignement.addEntry(0x6110, 0x3, 8, "CH2 Modes of Operation Display", &actuator2->txPdo.modeOfOperationDisplay);
}



bool EL7222_0010::startupConfiguration() {
	if(!rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12, false)) return false;
	if(!txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13, false)) return false;
			
	//LOOK UP: "Distributed clocks - application with subordinate controller cycle"
	//sync1 is fixed, it is the slaves internal control loop interrupt timer : every 62500 nanoseconds as per .xml ESI file (or 16KHz)
	//Here sync0 represents the interrupt timing that latches inputs
	//i don't understand exactly why yet, but the final cycle time stored in 0x1C32:3 is equal to
	//the addition of sync1 and sync2 (reg 0x09A3 and 0x09A7)
	//so we subtract the sync0 time from the sync1 time and everything works well without synchronisation hiccups
	uint32_t cycleTime_nanos = EtherCatFieldbus::processInterval_milliseconds * 1000000;
	uint32_t cycleOffset_nanos = EtherCatFieldbus::processInterval_milliseconds * 500000;
	ec_dcsync01(getSlaveIndex(), true, 62500, cycleTime_nanos - 62500, cycleOffset_nanos);
	return true;
}


void EL7222_0010::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	actuator1->readInputs();
	actuator2->readInputs();
	
	bool digitalInputs[4] = {
		bool(txPdo.infoData1_digitalInputs & 0x1),
		bool(txPdo.infoData1_digitalInputs & 0x2),
		bool(txPdo.infoData1_digitalInputs & 0x100),
		bool(txPdo.infoData1_digitalInputs & 0x200)
	};
	for(int i = 0; i < 4; i++) {
		if(pinInversionParameters[i]->value) digitalInputs[i] = !digitalInputs[i];
		*digitalInputValues[i] = digitalInputs[i];
	}
}


void EL7222_0010::writeOutputs(){
	actuator1->writeOutputs();
	actuator2->writeOutputs();
	rxPdoAssignement.pushDataTo(identity->outputs);
}

bool EL7222_0010::requestStateSafeOp(){
	//first take the drive to preop if it is not already there (like it should after being scanned)
	if(EC_STATE_PRE_OP != ec_statecheck(getSlaveIndex(), EC_STATE_PRE_OP, EC_TIMEOUTSAFE)){
		identity->state = EC_STATE_PRE_OP;
		ec_writestate(getSlaveIndex());
		if(EC_STATE_PRE_OP != ec_statecheck(getSlaveIndex(), EC_STATE_PRE_OP, EC_TIMEOUTSTATE)) return Logger::error("[{}] Failed to take drive to PreOP", getName());
	}
	
	//configure PDOs and Clocks
	startupConfiguration();
	
	//read the length of the configured Process Data
	uint32_t oBits, iBits;
	if(0 == ec_readPDOmap(getSlaveIndex(), &oBits, &iBits)) return Logger::error("[{}] Could not read PDO map", getName());

	//declare a structure to hold sync manager configuration
	struct PACKED SyncManagerConfiguration{
		uint16_t startAddress;
		uint16_t dataLength;
		uint32_t flags;
	};
	
	//addresses and flags are copied from the default SOEM SyncManager configuration
	SyncManagerConfiguration SM2{
		.startAddress = 0x1200,
		.dataLength = uint16_t(oBits / 8),
		.flags = 65572
	};
	SyncManagerConfiguration SM3{
		.startAddress = 0x1600,
		.dataLength = uint16_t(iBits / 8),
		.flags = 65568
	};
	
	//upload pdo syncmanager configuration
	ec_FPWR(identity->configadr, ECT_REG_SM2, sizeof(ec_sm), &SM2, EC_TIMEOUTSAFE);
	ec_FPWR(identity->configadr, ECT_REG_SM3, sizeof(ec_sm), &SM3, EC_TIMEOUTSAFE);
	
	//now we can finally take the drive to safeop, which will trigger the motor nameplate reading
	identity->state = EC_STATE_SAFE_OP;
	ec_writestate(getSlaveIndex());
	if(EC_STATE_SAFE_OP != ec_statecheck(getSlaveIndex(), EC_STATE_SAFE_OP, EC_TIMEOUTSAFE)) return Logger::error("[{}] Failed to take drive to SafeOp", getName());
	
	return true;
}


void EL7222_0010::readMotorNameplatesAndConfigureDrive(){
	std::thread worker([this](){
	
		auto doFirstSetup = [this]()->bool{
			
			if(isOffline()) return false;
			Logger::info("Starting First Setup.");
			
			
			//Enable all autoconfig options
			//this way the drive configures all parameters for the connected motors
			//we will turn off autoconfig when first setup is done
			if(!writeSDO_U8(0x8008, 0x1, 1, "CH1 Enable Autoconfig")) return false;
			if(!writeSDO_U8(0x8008, 0x2, 1, "CH1 Reconfig Identical Motor")) return false;
			if(!writeSDO_U8(0x8008, 0x3, 1, "CH1 Reconfig Non-Identical Motor")) return false;
			if(!writeSDO_U8(0x8108, 0x1, 1, "CH2 Enable Autoconfig")) return false;
			if(!writeSDO_U8(0x8108, 0x2, 1, "CH2 Reconfig Identical Motor")) return false;
			if(!writeSDO_U8(0x8108, 0x3, 1, "CH2 Reconfig Non-Identical Motor")) return false;
			
			//Now we want to trigger scanning of the drives OCT interfaces to read the motor nameplates
			//To do this we must take the drive through the Ethercat state machine up to Safe-Operational
			//After scanning we're already in PreOp state
			//The following song and dance is to satisfy all requirements to go from PreOp to SafeOp
			//We have to to PDO configuration, DC and Sync configuration as well as SyncManager Configuration
			//This is really stupid and complicated, couldn't you just have implemented a CanOpen command for OCT scanning ?
			//Also your documentation says almost nothing about how to even do this this, you're just trying to force me to use twincat, Thanks Beckhoff...
			
			if(!requestStateSafeOp()) return Logger::warn("Failed to reach state SafeOp");
			
			Logger::info("Starting reading of motor nameplate. (this can take up to 10 seconds)");
			
			double octReadStartTime = Timing::getProgramTime_seconds();
			bool ch1Detected = false;
			bool ch2Detected = false;
			while(true){
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
								
				uint8_t CH1_fbPositionValid;
				uint8_t CH2_fbPositionValid;
				readSDO_U8(0x6000, 0xE, CH1_fbPositionValid);
				readSDO_U8(0x6100, 0xE, CH2_fbPositionValid);
				ch1Detected = CH1_fbPositionValid == 0x0;
				ch2Detected = CH2_fbPositionValid == 0x0;
				if(ch1Detected && ch2Detected) break;
				
				double readTime = Timing::getProgramTime_seconds() - octReadStartTime;
				if(readTime > 10.0){
					if(!ch1Detected && !ch2Detected) {
						Logger::warn("failed to read motor nameplates, request timed out.");
						return false;
					}
					else if(!ch1Detected) Logger::warn("failed to read motor 1 nameplate, request timed out.");
					else if(!ch2Detected) Logger::warn("failed to read motor 2 nameplate, request timed out.");
					break;
				}
			}
			
			if(ch1Detected) {
				if(!actuator1->readMotorNameplate()) return false;
			}
			if(ch2Detected) {
				if(!actuator2->readMotorNameplate()) return false;
			}
			
			//disable autoconfig for future motor connections
			if(!writeSDO_U8(0x8008, 0x1, 0, "CH1 Enable Autoconfig")) return false;
			if(!writeSDO_U8(0x8008, 0x2, 0, "CH1 Reconfig Identical Motor")) return false;
			if(!writeSDO_U8(0x8008, 0x3, 0, "CH1 Reconfig Non-Identical Motor")) return false;
			if(!writeSDO_U8(0x8108, 0x1, 0, "CH2 Enable Autoconfig")) return false;
			if(!writeSDO_U8(0x8108, 0x2, 0, "CH2 Reconfig Identical Motor")) return false;
			if(!writeSDO_U8(0x8108, 0x3, 0, "CH2 Reconfig Non-Identical Motor")) return false;
			
			//go back to PreOp, the state we're in after scanning
			identity->state = EC_STATE_PRE_OP;
			ec_writestate(getSlaveIndex());
			
			return true;
		};
		
		if(doFirstSetup()) Logger::info("First Setup Finished Successfully.");
		else Logger::warn("First Setup Failed");
	});
   
	worker.detach();
}




std::string EL7222_0010::getDiagnosticsStringFromTextID(uint16_t textID){
	switch(textID){
		case 0x0000: return "No Error";
		case 0x1201: return "Communication re-established";
		case 0x4101: return "Terminal-Overtemperature";
		case 0x4102: return "PDO-configuration is incompatible to the selected mode of operation";
		case 0x4107: return "Undervoltage Up";
		case 0x4109: return "Overvoltage Up";
		case 0x410A: return "Fan";
		case 0x410B: return "Error detected, but disabled by suppression mask";
		case 0x4201: return "No communication to field-side (Auxiliary voltage missing)";
		case 0x4301: return "Feedback-Warning";
		case 0x4411: return "DC-Link undervoltage";
		case 0x4412: return "DC-Link overvoltage";
		case 0x4413: return "I2T Amplifier overload";
		case 0x4414: return "I2T Motor overload";
		case 0x4415: return "Speed limitation active";
		case 0x4417: return "Motor-Overtemperature";
		case 0x4418: return "Limit: Current";
		case 0x4419: return "Limit: Amplifier I2T-model exceeds 100%%";
		case 0x441A: return "Limit: Motor I2T-model exceeds 100%%";
		case 0x441B: return "Limit: Velocity limitation";
		case 0x441C: return "Axis disabled via STO";
		case 0x4420: return "Cogging compensation not supported (%u)";
		case 0x4421: return "I2T-Model Brake chopper overload";
		case 0x4422: return "Limit: Brake chopper I2T-model exceeds 100%%";
		case 0x4423: return "Brake resistor not connected";
		case 0x4424: return "Modes of operation invalid";
		case 0x8104: return "Terminal-Overtemperature";
		case 0x8105: return "PD-Watchdog";
		case 0x810A: return "Fan";
		case 0x810B: return "Undervoltage Up";
		case 0x810C: return "Overvoltage Up";
		case 0x8135: return "Cycletime has to be a multiple of 125 �s";
		case 0x8137: return "Electronic name plate: CRC error";
		case 0x8144: return "Hardware fault (%d)";
		case 0x817F: return "Error: 0x%X, 0x%X, 0x%X";
		case 0x81B0: return "Content of PDO 0x%X is invalid: Item 0x%X:%X cannot be mapped";
		case 0x81B1: return "Content of PDO 0x%X is invalid: Item 0x%X:%X has an unsupported length (%d bit)";
		case 0x8201: return "No communication to field-side (Auxiliary voltage missing)";
		case 0x8302: return "Feedback-Error";
		case 0x8304: return "OCT communication error";
		case 0x831A: return "Number of encoder-increments per revolution is not a power of two";
		case 0x8403: return "ADC Error";
		case 0x8404: return "Overcurrent";
		case 0x8405: return "Modulo position invalid (feedback position has changed too much while drive was turned off)";
		case 0x8406: return "Undervoltage DC-Link";
		case 0x8407: return "Overvoltage DC-Link";
		case 0x8408: return "I2T-Model Amplifier overload";
		case 0x8409: return "I2T-Model motor overload";
		case 0x840B: return "Commutation error";
		case 0x840C: return "Motor not connected";
		case 0x8415: return "Invalid modulo range";
		case 0x8416: return "Motor-Overtemperature";
		case 0x8417: return "Maximum rotating field velocity exceeded";
		case 0x841C: return "STO while the axis was enabled";
		case 0x8420: return "Teach-In Process (%d) failed";
		case 0x8421: return "Teach-In Process Timeout (STO, DC-Link, ...)";
		case 0x8422: return "Drive configuration missing";
		case 0x8423: return "Invalid process data format (number of singleturn bits+multiturn bits != 32)";
		case 0x8441: return "Maximum following error distance exceeded";
		case 0x8443: return "Invalid value for Mode of Operation";
		case 0x8445: return "I2T-Model Brake chopper overload";
		case 0x8446: return "Brake chopper overcurrent";
		case 0x8448: return "Drive / axis is not referenced";
		case 0x8449: return "Target position not in modulo range";
		case 0x844A: return "Modulo position: Checksum error";
		case 0x844B: return "Modulo position: Storage not supported for singleturn encoder";
		case 0x844C: return "Position offset cannot be used in the selected configuration";
		case 0x8450: return "Invalid start type 0x%x";
		case 0x8451: return "Invalid limit switch level";
		case 0x8452: return "Drive error during positioning";
		case 0x8453: return "Latch unit will be used by multiple modules";
		case 0x8454: return "Drive not in control";
		case 0x8455: return "Invalid value for Target acceleration";
		case 0x8456: return "Invalid value for Target deceleration";
		case 0x8457: return "Invalid value for Target velocity";
		case 0x8458: return "Invalid value for Target position";
		case 0x8459: return "Emergency stop active";
		case 0x845A: return "Target position exceeds Modulofactor";
		case 0x845B: return "Drive must be disabled";
		case 0x845C: return "No Feedback found";
		case 0x845D: return "Modulo factor invalid";
		case 0x845E: return "Invalid target position window";
		default: return "Unknown Diagnostics Message";
	}
}

void EL7222_0010::downloadCompleteDiagnostics(){
		
	uint8_t newestMessageSubindex;
	if(!readSDO_U8(0x10F3, 0x2, newestMessageSubindex)) return;
	if(newestMessageSubindex == 0x0) {
		Logger::info("No Diagnostics Message Available.");
		return;
	}
	uint8_t messageCount = newestMessageSubindex - 5;
	Logger::info("----- {} Diagnostics Messages -----", messageCount);
	for(int i = 0; i < messageCount; i++){
		uint8_t messageSubindex = 6 + i;
		int size = 64;
		uint8_t buffer[64];
		if(1 == ec_SDOread(getSlaveIndex(), 0x10F3, messageSubindex, false, &size, buffer, EC_TIMEOUTSAFE)){
			uint32_t diagCode = *((uint32_t*)(&buffer[0]));					//4bytes
			uint16_t flags = *((uint16_t*)(&buffer[4]));					//2bytes
			uint16_t textID = *((uint16_t*)(&buffer[6]));					//2bytes
			uint64_t timestamp = *((uint64_t*)(&buffer[8]));				//8bytes
			std::string message = getDiagnosticsStringFromTextID(textID);
			double time_seconds = (double(timestamp) - double(dcStartTime_nanoseconds)) / 1000000000.0;
			
			if(flags == 0x0) 		Logger::info(		"[{}] <Info>    0x{:x} diagCode:{:x} : {} {:.3f}s", i, textID, diagCode, message, time_seconds);
			else if(flags == 0x1) 	Logger::warn(		"[{}] <Warning> 0x{:x} diagCode:{:x} : {} {:.3f}s", i, textID, diagCode, message, time_seconds);
			else if(flags == 0x2) 	Logger::error(		"[{}] <Error>   0x{:x} diagCode:{:x} : {} {:.3f}s", i, textID, diagCode, message, time_seconds);
			else 					Logger::critical(	"[{}] <Message> 0x{:x} diagCode:{:x} : {} {:.3f}s", i, textID, diagCode, message, time_seconds);
		}
	}
}

void EL7222_0010::downloadLatestDiagnosticsMessage(uint16_t* output, bool* b_downloadFinished){
	
	if(output == nullptr || b_downloadFinished == nullptr) return;
	*output = 0x0;
	*b_downloadFinished = false;
	
	
	std::thread worker = std::thread([this, output, b_downloadFinished](){
		uint8_t newestMessageSubindex;
		if(!readSDO_U8(0x10F3, 0x2, newestMessageSubindex)) return;
		uint8_t buffer[64];
		int size = 64;
		if(1 != ec_SDOread(getSlaveIndex(), 0x10F3, newestMessageSubindex, false, &size, &buffer, EC_TIMEOUTSAFE)) return;
		uint16_t textID = *((uint16_t*)(&buffer[6]));					//2bytes
		*output = textID;
		*b_downloadFinished = true;
	});
	
	worker.detach();
	
}



bool EL7222_0010::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* actuator1XML = xml->InsertNewChildElement("Actuator1");
	XMLElement* actuator2XML = xml->InsertNewChildElement("Actuator2");
	actuator1->save(actuator1XML);
	actuator2->save(actuator2XML);
	XMLElement* pinInversionsXML = xml->InsertNewChildElement("PinInversion");
	for(auto parameter : pinInversionParameters) parameter->save(pinInversionsXML);
	return true;
}


bool EL7222_0010::loadDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	bool b_success = true;
	
	if(XMLElement* actuatorXML = xml->FirstChildElement("Actuator1")){
		if(!actuator1->load(actuatorXML)) b_success = false;
	}else b_success = false;
	
	if(XMLElement* actuatorXML = xml->FirstChildElement("Actuator2")){
		if(!actuator2->load(actuatorXML)) b_success = false;
	}else b_success = false;
	
	if(XMLElement* pinInversionsXML = xml->FirstChildElement("PinInversion")){
		for(auto parameter : pinInversionParameters) parameter->load(pinInversionsXML);
	}
	
	return b_success;
}





void ELM7231_9016::onDisconnection() { actuator->onDisconnection(); }
void ELM7231_9016::onConnection() {}
void ELM7231_9016::initialize() {
	auto thisEtherCatDevice = std::static_pointer_cast<EtherCatDevice>(shared_from_this());
	actuator = std::make_shared<EL722x_Actuator>(thisEtherCatDevice, 1);
	actuator->initialize();
	
	addNodePin(actuator->actuatorPin);
	
	rxPdoAssignement.addNewModule(0x1610);
	rxPdoAssignement.addEntry(0x7010, 0x1, 16, "CH1 Control Word", &actuator->rxPdo.controlWord);
	rxPdoAssignement.addNewModule(0x1611);
	rxPdoAssignement.addEntry(0x7010, 0x5, 32, "CH1 Target Position", &actuator->rxPdo.targetPosition);
	rxPdoAssignement.addNewModule(0x1612);
	rxPdoAssignement.addEntry(0x7010, 0x6, 32, "CH1 Target Velocity", &actuator->rxPdo.targetVelocity);
	rxPdoAssignement.addNewModule(0x1613);
	rxPdoAssignement.addEntry(0x7010, 0x9, 16, "CH1 Target Torque", &actuator->rxPdo.targetTorque);
	rxPdoAssignement.addNewModule(0x161A);
	rxPdoAssignement.addEntry(0x7010, 0x3, 8, "CH1 Modes of Operation", &actuator->rxPdo.modeOfOperationSelection);
	
	txPdoAssignement.addNewModule(0x1A00);
	txPdoAssignement.addEntry(0x6000, 0x11, 32, "CH1 FB Position", &actuator->txPdo.fbPosition);
	txPdoAssignement.addNewModule(0x1A02);
	txPdoAssignement.addEntry(0x6000, 0x14, 16, "CH1 FB Status", &actuator->txPdo.fbStatus); //not 1 object but 3, subindeces 13 14 15
	txPdoAssignement.addNewModule(0x1A10);
	txPdoAssignement.addEntry(0x6010, 0x1, 16, "CH1 Statusword", &actuator->txPdo.statusWord);
	txPdoAssignement.addNewModule(0x1A11);
	txPdoAssignement.addEntry(0x6010, 0x6, 32, "CH1 Following Error Actual Value", &actuator->txPdo.followingErrorActualValue);
	txPdoAssignement.addNewModule(0x1A12);
	txPdoAssignement.addEntry(0x6010, 0x7, 32, "CH1 Velocity Actual Value", &actuator->txPdo.velocityActualValue);
	txPdoAssignement.addNewModule(0x1A13);
	txPdoAssignement.addEntry(0x6010, 0x8, 16, "CH1 Torque Actual Value", &actuator->txPdo.torqueActualValue);
	txPdoAssignement.addNewModule(0x1A17);
	txPdoAssignement.addEntry(0x6010, 0x3, 8, "CH1 Modes of Operation Display", &actuator->txPdo.modeOfOperationDisplay);
	 
	fsoeRxPdoAssignement.addNewModule(0x16A0);
	fsoeRxPdoAssignement.addEntry(0x7200, 0x1, 8, "Fsoe Receive CMD", fsoeMasterFrame);
	fsoeRxPdoAssignement.addEntry(0xF701, 0x1, 8, "STO ChA", fsoeMasterFrame + 1);
	fsoeRxPdoAssignement.addEntry(0xF701, 0x2, 8, "STO ChB", fsoeMasterFrame + 2);
	fsoeRxPdoAssignement.addEntry(0x7200, 0x3, 16, "FSOE Receive CRC_0", fsoeMasterFrame + 3);
	fsoeRxPdoAssignement.addEntry(0x7200, 0x2, 16, "FSOE Receive ConnID", fsoeMasterFrame + 5);
	fsoeRxPdoAssignement.addNewModule(0x16B0);
	fsoeRxPdoAssignement.addEntry(0x7200, 0x2, 8, "???", &weird1);
	fsoeRxPdoAssignement.addNewModule(0x16BF);
	fsoeRxPdoAssignement.addEntry(0x0, 0x0, 16, "FSLOGIC Outputs", fslogicMaster);
	
	fsoeTxPdoAssignement.addNewModule(0x1AA0);
	fsoeTxPdoAssignement.addEntry(0x6200, 0x1, 8, "FSOE Send CMD", fsoeSlaveFrame);
	fsoeTxPdoAssignement.addEntry(0xF601, 0x1, 8, "STO Active ChA", fsoeSlaveFrame + 1);
	fsoeTxPdoAssignement.addEntry(0xF601, 0x2, 8, "STO Active ChB", fsoeSlaveFrame + 2);
	fsoeTxPdoAssignement.addEntry(0x6200, 0x3, 16, "FSOE Receive CRC_0", fsoeSlaveFrame + 3);
	fsoeTxPdoAssignement.addEntry(0x6200, 0x2, 16, "FSOE Receive ConnID", fsoeSlaveFrame + 5);
	fsoeTxPdoAssignement.addNewModule(0x1AB0);
	fsoeTxPdoAssignement.addEntry(0x6200, 0x2, 8, "???", &weird2);
	fsoeTxPdoAssignement.addNewModule(0x1ABF);
	fsoeTxPdoAssignement.addEntry(0xF100, 0x1, 8, "FSOE Safe Logic State", fsLogicSlave);
	fsoeTxPdoAssignement.addEntry(0xF100, 0x2, 8, "FSOE Cycle Counter", fsLogicSlave + 1);
	
}
bool ELM7231_9016::startupConfiguration() {
	if(!rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12, false)) return false;
	if(!txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13, false)) return false;
	if(!fsoeRxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C16, false)) return false;
	if(!fsoeTxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C17, false)) return false;
		
	//LOOK UP: "Distributed clocks - application with subordinate controller cycle"
	//sync1 is fixed, it is the slaves internal control loop interrupt timer : every 62500 nanoseconds as per .xml ESI file (or 16KHz)
	//Here sync0 represents the interrupt timing that latches inputs
	//i don't understand exactly why yet, but the final cycle time stored in 0x1C32:3 is equal to
	//the addition of sync1 and sync2 (reg 0x09A3 and 0x09A7)
	//so we subtract the sync0 time from the sync1 time and everything works well without synchronisation hiccups
	uint32_t cycleTime_nanos = EtherCatFieldbus::processInterval_milliseconds * 1000000;
	uint32_t cycleOffset_nanos = EtherCatFieldbus::processInterval_milliseconds * 500000;
	ec_dcsync01(getSlaveIndex(), true, 62500, cycleTime_nanos - 62500, cycleOffset_nanos);
	
	FsoeConnection::Config fsoeConfig;
	fsoeConfig.watchdogTimeout_ms = 100;
	fsoeConfig.fsoeAddress = 1;
	fsoeConfig.safeInputsSize = 2;
	fsoeConfig.safeOutputsSize = 2;
	fsoeConfig.applicationParameters = {
		0x0, 0x0,	//store code
		0x0, 0x0	//project crc
	};
	fsoeConnection.initialize(fsoeConfig);
	
	b_initialized = false;
	
	return true;
}
void ELM7231_9016::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	fsoeTxPdoAssignement.pullDataFrom(identity->inputs + 19);
	fsoeConnection.receiveFrame(fsoeSlaveFrame, 7, safeInputs, 2);
	
	actuator->readInputs();
}
void ELM7231_9016::writeOutputs(){
	
	if(!b_initialized){
		b_initialized = true;
		fixFMMUs();
	}
	
	//always acknowledge fsoeConnErr
	if(weird1 == 0x0) weird1 = 0x1;
	else weird1 = 0x0;
	
	fsoeConnection.b_sendFailsafeData = b_sto;
	safeOutputs[0] = b_sto ? 0x0 : 0x1;
	
	actuator->writeOutputs();
	
	fsoeConnection.sendFrame(fsoeMasterFrame, 7, safeOutputs, 2);
	rxPdoAssignement.pushDataTo(identity->outputs);
	fsoeRxPdoAssignement.pushDataTo(identity->outputs + 13);
}
bool ELM7231_9016::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* actuatorXML = xml->InsertNewChildElement("Actuator");
	actuator->save(actuatorXML);
	//XMLElement* pinInversionsXML = xml->InsertNewChildElement("PinInversion");
	//for(auto parameter : pinInversionParameters) parameter->save(pinInversionsXML);
	return true;
}
bool ELM7231_9016::loadDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	bool b_success = true;
	
	if(XMLElement* actuatorXML = xml->FirstChildElement("Actuator")){
		if(!actuator->load(actuatorXML)) b_success = false;
	}else b_success = false;
	
	/*
	if(XMLElement* pinInversionsXML = xml->FirstChildElement("PinInversion")){
		for(auto parameter : pinInversionParameters) parameter->load(pinInversionsXML);
	}
	*/
	
	return true;
}


bool ELM7231_9016::requestStateSafeOp(){
	//first take the drive to preop if it is not already there (like it should after being scanned)
	if(EC_STATE_PRE_OP != ec_statecheck(getSlaveIndex(), EC_STATE_PRE_OP, EC_TIMEOUTSAFE)){
		identity->state = EC_STATE_PRE_OP;
		ec_writestate(getSlaveIndex());
		if(EC_STATE_PRE_OP != ec_statecheck(getSlaveIndex(), EC_STATE_PRE_OP, EC_TIMEOUTSTATE)) return Logger::error("[{}] Failed to take drive to PreOP", getName());
	}
	
	//configure PDOs and Clocks
	startupConfiguration();
	
	//read the length of the configured Process Data
	uint32_t oBits, iBits;
	if(0 == ec_readPDOmap(getSlaveIndex(), &oBits, &iBits)) return Logger::error("[{}] Could not read PDO map", getName());

	//declare a structure to hold sync manager configuration
	struct PACKED SyncManagerConfiguration{
		uint16_t startAddress;
		uint16_t dataLength;
		uint32_t flags;
	};
	
	//addresses and flags are copied from the default SOEM SyncManager configuration
	SyncManagerConfiguration SM2{
		.startAddress = 0x1200,
		.dataLength = uint16_t(oBits / 8),
		.flags = 65572
	};
	SyncManagerConfiguration SM3{
		.startAddress = 0x1600,
		.dataLength = uint16_t(iBits / 8),
		.flags = 65568
	};
	SM2.dataLength -= 10;
	SM3.dataLength -= 10;
	
	SyncManagerConfiguration SM6{
		.startAddress = 0x1e00,
		.dataLength = 10,
		.flags = 65572
	};
	SyncManagerConfiguration SM7{
		.startAddress = 0x1f00,
		.dataLength = 10,
		.flags = 65568
	};
	
	//upload pdo syncmanager configuration
	ec_FPWR(identity->configadr, ECT_REG_SM2, sizeof(ec_sm), &SM2, EC_TIMEOUTSAFE);
	ec_FPWR(identity->configadr, ECT_REG_SM3, sizeof(ec_sm), &SM3, EC_TIMEOUTSAFE);
	ec_FPWR(identity->configadr, ECT_REG_SM0 + 6 * 0x8, sizeof(ec_sm), &SM6, EC_TIMEOUTSAFE);
	ec_FPWR(identity->configadr, ECT_REG_SM0 + 7 * 0x8, sizeof(ec_sm), &SM7, EC_TIMEOUTSAFE);
	
	//now we can finally take the drive to safeop, which will trigger the motor nameplate reading
	identity->state = EC_STATE_SAFE_OP;
	ec_writestate(getSlaveIndex());
	if(EC_STATE_SAFE_OP != ec_statecheck(getSlaveIndex(), EC_STATE_SAFE_OP, EC_TIMEOUTSAFE)) return Logger::error("[{}] Failed to take drive to SafeOp", getName());
	
	return true;
}


void ELM7231_9016::downloadCompleteDiagnostics(){
		
	uint8_t newestMessageSubindex;
	if(!readSDO_U8(0x10F3, 0x2, newestMessageSubindex)) return;
	if(newestMessageSubindex == 0x0) {
		Logger::info("No Diagnostics Message Available.");
		return;
	}
	uint8_t messageCount = newestMessageSubindex - 5;
	Logger::info("----- {} Diagnostics Messages -----", messageCount);
	for(int i = 0; i < messageCount; i++){
		uint8_t messageSubindex = 6 + i;
		int size = 64;
		uint8_t buffer[64];
		if(1 == ec_SDOread(getSlaveIndex(), 0x10F3, messageSubindex, false, &size, buffer, EC_TIMEOUTSAFE)){
			uint32_t diagCode = *((uint32_t*)(&buffer[0]));					//4bytes
			uint16_t flags = *((uint16_t*)(&buffer[4]));					//2bytes
			uint16_t textID = *((uint16_t*)(&buffer[6]));					//2bytes
			uint64_t timestamp = *((uint64_t*)(&buffer[8]));				//8bytes
			std::string message = getDiagnosticsStringFromTextID(textID);
			double time_seconds = (double(timestamp) - double(dcStartTime_nanoseconds)) / 1000000000.0;
			
			if(flags == 0x0) 		Logger::info(		"[{}] <Info>    0x{:x} diagCode:{:x} : {} {:.3f}s", i, textID, diagCode, message, time_seconds);
			else if(flags == 0x1) 	Logger::warn(		"[{}] <Warning> 0x{:x} diagCode:{:x} : {} {:.3f}s", i, textID, diagCode, message, time_seconds);
			else if(flags == 0x2) 	Logger::error(		"[{}] <Error>   0x{:x} diagCode:{:x} : {} {:.3f}s", i, textID, diagCode, message, time_seconds);
			else 					Logger::critical(	"[{}] <Message> 0x{:x} diagCode:{:x} : {} {:.3f}s", i, textID, diagCode, message, time_seconds);
		}
	}
}


void ELM7231_9016::readMotorNameplatesAndConfigureDrive(){
	std::thread worker([this](){
	
		auto doFirstSetup = [this]()->bool{
			
			if(isOffline()) return false;
			Logger::info("Starting First Setup.");
			
			
			//Enable all autoconfig options
			//this way the drive configures all parameters for the connected motors
			//we will turn off autoconfig when first setup is done
			if(!writeSDO_U8(0x8008, 0x1, 1, "CH1 Enable Autoconfig")) return false;
			if(!writeSDO_U8(0x8008, 0x2, 1, "CH1 Reconfig Identical Motor")) return false;
			if(!writeSDO_U8(0x8008, 0x3, 1, "CH1 Reconfig Non-Identical Motor")) return false;
			
			//Now we want to trigger scanning of the drives OCT interfaces to read the motor nameplates
			//To do this we must take the drive through the Ethercat state machine up to Safe-Operational
			//After scanning we're already in PreOp state
			//The following song and dance is to satisfy all requirements to go from PreOp to SafeOp
			//We have to to PDO configuration, DC and Sync configuration as well as SyncManager Configuration
			//This is really stupid and complicated, couldn't you just have implemented a CanOpen command for OCT scanning ?
			//Also your documentation says almost nothing about how to even do this this, you're just trying to force me to use twincat, Thanks Beckhoff...
			
			if(!requestStateSafeOp()) return Logger::warn("Failed to reach state SafeOp");
			
			Logger::info("Starting reading of motor nameplate. (this can take up to 10 seconds)");
			
			double octReadStartTime = Timing::getProgramTime_seconds();
			bool ch1Detected = false;
			while(true){
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
								
				uint8_t CH1_fbPositionValid;
				readSDO_U8(0x6000, 0xE, CH1_fbPositionValid);
				ch1Detected = CH1_fbPositionValid == 0x0;
				if(ch1Detected) break;
				
				double readTime = Timing::getProgramTime_seconds() - octReadStartTime;
				if(readTime > 10.0){
					if(!ch1Detected) {
						Logger::warn("failed to read motor nameplates, request timed out.");
						return false;
					}
					else if(!ch1Detected) Logger::warn("failed to read motor 1 nameplate, request timed out.");
					break;
				}
			}
			
			if(ch1Detected) {
				if(!actuator->readMotorNameplate()) return false;
			}
			
			//disable autoconfig for future motor connections
			if(!writeSDO_U8(0x8008, 0x1, 0, "CH1 Enable Autoconfig")) return false;
			if(!writeSDO_U8(0x8008, 0x2, 0, "CH1 Reconfig Identical Motor")) return false;
			if(!writeSDO_U8(0x8008, 0x3, 0, "CH1 Reconfig Non-Identical Motor")) return false;
			
			//go back to PreOp, the state we're in after scanning
			identity->state = EC_STATE_PRE_OP;
			ec_writestate(getSlaveIndex());
			
			return true;
		};
		
		if(doFirstSetup()) Logger::info("First Setup Finished Successfully.");
		else Logger::warn("First Setup Failed");
	});
   
	worker.detach();
}

void ELM7231_9016::fixFMMUs(){
	//correct FMMU[1,3] physical start address for SM[6,7]
	uint16_t FMMU1_physicalStartAddress = 0x1E00;
	uint16_t FMMU3_physicalStartAddress = 0x1F00;
	ec_FPWR(identity->configadr, 0x618, 2, &FMMU1_physicalStartAddress, EC_TIMEOUTSAFE);
	ec_FPWR(identity->configadr, 0x638, 2, &FMMU3_physicalStartAddress, EC_TIMEOUTSAFE);
}

std::string ELM7231_9016::getDiagnosticsStringFromTextID(uint16_t textID){
	switch(textID){
		case 0x0000: return "No Error";
		case 0x1201: return "Communication re-established";
		case 0x4101: return "Terminal-Overtemperature";
		case 0x4102: return "PDO-configuration is incompatible to the selected mode of operation";
		case 0x4107: return "Undervoltage Up";
		case 0x4109: return "Overvoltage Up";
		case 0x410A: return "Fan";
		case 0x410B: return "Error detected, but disabled by suppression mask";
		case 0x4201: return "No communication to field-side (Auxiliary voltage missing)";
		case 0x4301: return "Feedback-Warning";
		case 0x4411: return "DC-Link undervoltage";
		case 0x4412: return "DC-Link overvoltage";
		case 0x4413: return "I2T Amplifier overload";
		case 0x4414: return "I2T Motor overload";
		case 0x4415: return "Speed limitation active";
		case 0x4417: return "Motor-Overtemperature";
		case 0x4418: return "Limit: Current";
		case 0x4419: return "Limit: Amplifier I2T-model exceeds 100%%";
		case 0x441A: return "Limit: Motor I2T-model exceeds 100%%";
		case 0x441B: return "Limit: Velocity limitation";
		case 0x441C: return "Axis disabled via STO";
		case 0x4420: return "Cogging compensation not supported (%u)";
		case 0x4421: return "I2T-Model Brake chopper overload";
		case 0x4422: return "Limit: Brake chopper I2T-model exceeds 100%%";
		case 0x4423: return "Brake resistor not connected";
		case 0x4424: return "Modes of operation invalid";
		case 0x8104: return "Terminal-Overtemperature";
		case 0x8105: return "PD-Watchdog";
		case 0x810A: return "Fan";
		case 0x810B: return "Undervoltage Up";
		case 0x810C: return "Overvoltage Up";
		case 0x8135: return "Cycletime has to be a multiple of 125 �s";
		case 0x8137: return "Electronic name plate: CRC error";
		case 0x8144: return "Hardware fault (%d)";
		case 0x817F: return "Error: 0x%X, 0x%X, 0x%X";
		case 0x81B0: return "Content of PDO 0x%X is invalid: Item 0x%X:%X cannot be mapped";
		case 0x81B1: return "Content of PDO 0x%X is invalid: Item 0x%X:%X has an unsupported length (%d bit)";
		case 0x8201: return "No communication to field-side (Auxiliary voltage missing)";
		case 0x8302: return "Feedback-Error";
		case 0x8304: return "OCT communication error";
		case 0x831A: return "Number of encoder-increments per revolution is not a power of two";
		case 0x8403: return "ADC Error";
		case 0x8404: return "Overcurrent";
		case 0x8405: return "Modulo position invalid (feedback position has changed too much while drive was turned off)";
		case 0x8406: return "Undervoltage DC-Link";
		case 0x8407: return "Overvoltage DC-Link";
		case 0x8408: return "I2T-Model Amplifier overload";
		case 0x8409: return "I2T-Model motor overload";
		case 0x840B: return "Commutation error";
		case 0x840C: return "Motor not connected";
		case 0x8415: return "Invalid modulo range";
		case 0x8416: return "Motor-Overtemperature";
		case 0x8417: return "Maximum rotating field velocity exceeded";
		case 0x841C: return "STO while the axis was enabled";
		case 0x8420: return "Teach-In Process (%d) failed";
		case 0x8421: return "Teach-In Process Timeout (STO, DC-Link, ...)";
		case 0x8422: return "Drive configuration missing";
		case 0x8423: return "Invalid process data format (number of singleturn bits+multiturn bits != 32)";
		case 0x8441: return "Maximum following error distance exceeded";
		case 0x8443: return "Invalid value for Mode of Operation";
		case 0x8445: return "I2T-Model Brake chopper overload";
		case 0x8446: return "Brake chopper overcurrent";
		case 0x8448: return "Drive / axis is not referenced";
		case 0x8449: return "Target position not in modulo range";
		case 0x844A: return "Modulo position: Checksum error";
		case 0x844B: return "Modulo position: Storage not supported for singleturn encoder";
		case 0x844C: return "Position offset cannot be used in the selected configuration";
		case 0x8450: return "Invalid start type 0x%x";
		case 0x8451: return "Invalid limit switch level";
		case 0x8452: return "Drive error during positioning";
		case 0x8453: return "Latch unit will be used by multiple modules";
		case 0x8454: return "Drive not in control";
		case 0x8455: return "Invalid value for Target acceleration";
		case 0x8456: return "Invalid value for Target deceleration";
		case 0x8457: return "Invalid value for Target velocity";
		case 0x8458: return "Invalid value for Target position";
		case 0x8459: return "Emergency stop active";
		case 0x845A: return "Target position exceeds Modulofactor";
		case 0x845B: return "Drive must be disabled";
		case 0x845C: return "No Feedback found";
		case 0x845D: return "Modulo factor invalid";
		case 0x845E: return "Invalid target position window";
		default: return "Unknown Diagnostics Message";
	}
}

