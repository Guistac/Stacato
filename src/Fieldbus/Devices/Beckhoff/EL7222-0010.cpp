#include "EL7222-0010.h"

#include "Fieldbus/EtherCatFieldbus.h"

void EL7222_0010::onDisconnection() {
	actuator1->onDisconnection();
	actuator2->onDisconnection();
	gpio->state = DeviceState::OFFLINE;
}

void EL7222_0010::onConnection() {}

void EL7222_0010::initialize() {
	auto thisEtherCatDevice = std::static_pointer_cast<EtherCatDevice>(shared_from_this());
	actuator1 = std::make_shared<EL722x_Actuator>(thisEtherCatDevice, 1);
	actuator2 = std::make_shared<EL722x_Actuator>(thisEtherCatDevice, 2);
	gpio = std::make_shared<EL7222Gpio>(thisEtherCatDevice);
	actuator1->initialize();
	actuator2->initialize();
	
	gpioPin->assignData(std::static_pointer_cast<GpioInterface>(gpio));
	
	addNodePin(actuator1->actuatorPin);
	addNodePin(actuator2->actuatorPin);
	addNodePin(gpioPin);
	addNodePin(digitalInput1A_pin);
	addNodePin(digitalInput2A_pin);
	addNodePin(digitalInput1B_pin);
	addNodePin(digitalInput2B_pin);
	
	
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
	txPdoAssignement.addEntry(0x6010, 0x12, 16, "CH1 Info Data 1", &actuator1->txPdo.infoData1_errors);
	txPdoAssignement.addNewModule(0x1A15);
	txPdoAssignement.addEntry(0x6010, 0x13, 16, "CH1 Info Data 2", &actuator1->txPdo.infoData2_digitalInputs);
	txPdoAssignement.addNewModule(0x1A16);
	txPdoAssignement.addEntry(0x6010, 0x14, 16, "CH1 Info Data 3", &actuator1->txPdo.infoData3);
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
	txPdoAssignement.addNewModule(0x1A54);
	txPdoAssignement.addEntry(0x6110, 0x12, 16, "CH2 Info Data 1", &actuator2->txPdo.infoData1_errors);
	txPdoAssignement.addNewModule(0x1A55);
	txPdoAssignement.addEntry(0x6110, 0x13, 16, "CH2 Info Data 2", &actuator2->txPdo.infoData2_digitalInputs);
	txPdoAssignement.addNewModule(0x1A56);
	txPdoAssignement.addEntry(0x6110, 0x14, 16, "CH2 Info Data 3", &actuator2->txPdo.infoData3);
	txPdoAssignement.addNewModule(0x1A57);
	txPdoAssignement.addEntry(0x6110, 0x3, 8, "CH2 Modes of Operation Display", &actuator2->txPdo.modeOfOperationDisplay);
	
}



bool EL7222_0010::startupConfiguration() {
	if(!rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12, false)) return false;
	if(!txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13, false)) return false;
	
	double cycleTime_millis = EtherCatFieldbus::processInterval_milliseconds;
	uint32_t cycleTime_nanos = cycleTime_millis * 1000000;
	uint32_t cycleOffset_nanos = EtherCatFieldbus::processInterval_milliseconds * 500000;
	
	//ec_dcsync01(getSlaveIndex(), true, 62500, 0, 0); //why does this work ?
	//ec_dcsync01(getSlaveIndex(), true, 62500, 2000000, 2000000); this starts the network successfully with 2ms cycle time
	ec_dcsync01(getSlaveIndex(), true, 62500, cycleTime_nanos, cycleOffset_nanos); //setting CycleTime0 to 62500 (as per .xml ESI file) and cyclTime1 equal to the cycle time seems to work, no idea about cycleShift
	return true;
}


void EL7222_0010::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	
	//*digitalInput1_Value 									= bool(txPdo.infoData2_digitalInputs & (0x1 << 0));
	//*digitalInput2_Value									= bool(txPdo.infoData2_digitalInputs & (0x1 << 1));
	
	actuator1->readInputs();
	actuator2->readInputs();
}


void EL7222_0010::writeOutputs(){
	actuator1->writeOutputs();
	actuator2->writeOutputs();
	rxPdoAssignement.pushDataTo(identity->outputs);
}




bool EL7222_0010::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* actuator1XML = xml->InsertNewChildElement("Actuator1");
	XMLElement* actuator2XML = xml->InsertNewChildElement("Actuator2");
	actuator1->save(actuator1XML);
	actuator2->save(actuator2XML);
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
	
	return b_success;
}


void EL7222_0010::downloadDiagnostics(){
		auto getDiagnosticsStringFromTextID = [](uint16_t textID) -> std::string{
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
		};
		
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
				
				if(flags == 0x0) 		Logger::info(		"[{}] <Info>    0x{:x} : {}", i, textID, message);
				else if(flags == 0x1) 	Logger::warn(		"[{}] <Warning> 0x{:x} : {}", i, textID, message);
				else if(flags == 0x2) 	Logger::error(		"[{}] <Error>   0x{:x} : {}", i, textID, message);
				else 					Logger::critical(	"[{}] <Message> 0x{:x} : {}", i, textID, message);
			}
		}
		
}
