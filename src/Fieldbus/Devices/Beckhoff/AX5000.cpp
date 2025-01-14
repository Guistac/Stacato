#include "AX5000.h"
#include "Fieldbus/EtherCatFieldbus.h"

void AX5103::initialize(){
	ax5000.actuators.push_back(std::make_shared<AX5000::Actuator>(nullptr, "Actuator", 0));
	ax5000.initialize(std::static_pointer_cast<EtherCatDevice>(shared_from_this()));
}
void AX5103::onDisconnection(){ 								ax5000.onDisconnection(); }
void AX5103::onConnection(){ 									ax5000.onConnection(); }
bool AX5103::startupConfiguration(){ return 					ax5000.startupConfiguration(); }
void AX5103::readInputs(){ 										ax5000.readInputs(); }
void AX5103::writeOutputs(){ 									ax5000.writeOutputs(); }
bool AX5103::saveDeviceData(tinyxml2::XMLElement* xml){ return 	ax5000.save(xml); }
bool AX5103::loadDeviceData(tinyxml2::XMLElement* xml){ return 	ax5000.load(xml); }


void AX5203::initialize(){
	ax5000.actuators.push_back(std::make_shared<AX5000::Actuator>(nullptr, "Actuator 1", 0));
	ax5000.actuators.push_back(std::make_shared<AX5000::Actuator>(nullptr, "Actuator 2", 1));
	ax5000.initialize(std::static_pointer_cast<EtherCatDevice>(shared_from_this()));
}
void AX5203::onDisconnection(){ 								ax5000.onDisconnection(); }
void AX5203::onConnection(){ 									ax5000.onConnection(); }
bool AX5203::startupConfiguration(){ return 					ax5000.startupConfiguration(); }
void AX5203::readInputs(){ 										ax5000.readInputs(); }
void AX5203::writeOutputs(){ 									ax5000.writeOutputs(); }
bool AX5203::saveDeviceData(tinyxml2::XMLElement* xml){ return	ax5000.save(xml); }
bool AX5203::loadDeviceData(tinyxml2::XMLElement* xml){ return	ax5000.load(xml); }






void AX5000::initialize(std::shared_ptr<EtherCatDevice> ecatDevice){
	etherCatDevice = ecatDevice;
	//gpio = std::make_shared<Gpio>(<#std::shared_ptr<AX5000> d#>, <#std::string n#>);

	etherCatDevice->addNodePin(gpioPin);
	etherCatDevice->addNodePin(STO_pin);
	etherCatDevice->addNodePin(digitalInput0_pin);
	etherCatDevice->addNodePin(digitalInput1_pin);
	etherCatDevice->addNodePin(digitalInput2_pin);
	etherCatDevice->addNodePin(digitalInput3_pin);
	etherCatDevice->addNodePin(digitalInput4_pin);
	etherCatDevice->addNodePin(digitalInput5_pin);
	etherCatDevice->addNodePin(digitalInput6_pin);
	etherCatDevice->addNodePin(digitalOutput7_pin);
	
	gpioPin->assignData(std::static_pointer_cast<GpioInterface>(gpio));
	
	for(auto actuator : actuators) {
		actuator->initialize();
		etherCatDevice->addNodePin(actuator->actuatorPin);
	}
}

void AX5000::onDisconnection(){
	gpio->state = DeviceState::OFFLINE;
	for(auto actuator : actuators) actuator->state = DeviceState::OFFLINE;
}
void AX5000::onConnection(){
	gpio->state = DeviceState::ENABLED;
}


bool AX5000::startupConfiguration() {
	
	struct IDN_List{
		uint16_t size = 0; //total bytes in idn list (idn count * 2)
		uint16_t maxLength = 0; //max length of array in slave, not useful when uploading
		uint16_t IDNs[10] = {0};
		uint8_t* getPointer(){ return (uint8_t*)this; }
	};
	
	//Master Data Telegram (MDT) [Master->Slave]
	IDN_List axis0_mdtList;
	axis0_mdtList.size = 6;
	//axis0_mdtList.IDNs[0] = 134; 	//(2) Master Control Word		[FIXED]
	axis0_mdtList.IDNs[0] = 47; 	//(4) Position Command Value
	axis0_mdtList.IDNs[1] = 36; 	//(4) Velocity Command Value
	axis0_mdtList.IDNs[2] = 33570;	//(2) Digital Outputs
	
	IDN_List axis1_mdtList;
	axis1_mdtList.size = 4;
	//axis1_mdtList.IDNs[0] = 134; 	//(2) Master Control Word		[FIXED]
	axis1_mdtList.IDNs[0] = 47; 	//(4) Position Command Value
	axis1_mdtList.IDNs[1] = 36; 	//(4) Velocity Command Value

	//Acknowledge Telegram (AT) [Slave->Master]
	IDN_List axis0_atList;
	axis0_atList.size = 12;
	//axis0_atList.IDNs[0] = 135;	//(2) Drive Status Word (u16)						[FIXED]
	axis0_atList.IDNs[0] = 51;		//(4) Position Feedback Value 1 (motor feedback)
	axis0_atList.IDNs[1] = 40;		//(4) Velocity Feedback Value 1
	axis0_atList.IDNs[2] = 84;		//(2) Torque Feedback Value
	axis0_atList.IDNs[3] = 11;		//(2) Class 1 Diagnostics
	axis0_atList.IDNs[4] = 33569; 	//(2) Digital Inputs, state
	axis0_atList.IDNs[5] = 34770; 	//(2) Safety option state
	
	IDN_List axis1_atList;
	axis1_atList.size = 8;
	//axis1_atList.IDNs[0] = 135;	//(2) Drive Status Word (u16)						[FIXED]
	axis1_atList.IDNs[0] = 51;		//(4) Position Feedback Value 1 (motor feedback)
	axis1_atList.IDNs[1] = 40;		//(4) Velocity Feedback Value 1
	axis1_atList.IDNs[2] = 84;		//(2) Torque Feedback Value
	axis1_atList.IDNs[3] = 11;		//(2) Class 1 Diagnostics
	
	//========== STARTUP LIST ==========
	
	//upload process data configuration
	etherCatDevice->writeSercos_Array('S', 16, axis0_atList.getPointer(), sizeof(IDN_List), 0); //AT List [axis0]
	etherCatDevice->writeSercos_Array('S', 24, axis0_mdtList.getPointer(), sizeof(IDN_List), 0); //MDT List [axis0]
	if(actuators.size() > 1){
		etherCatDevice->writeSercos_Array('S', 16, axis1_atList.getPointer(), sizeof(IDN_List), 1); //AT List [axis1]
		etherCatDevice->writeSercos_Array('S', 24, axis1_mdtList.getPointer(), sizeof(IDN_List), 1); //MDT List [axis1]
	}
	
	/*
	writeSercos_U64('P', 10, 0x1FBD, 0);
	writeSercos_U64('P', 10, 0x1FBD, 1);
	
	uploadMotorConfiguration(0, axis0->getMotorType());
	uploadMotorConfiguration(1, axis1->getMotorType());
	
	writeSercos_U16('S', 32, 2, 0); //Operation mode 0 VEL
	writeSercos_U16('S', 32, 2, 1); //Operation mode 0 VEL
	*/
	 
	etherCatDevice->writeSercos_U16('P', 2000, 3); //Configured Safety Option (3 == AX5801-0200) [MANDATORY]
	etherCatDevice->writeSercos_U16('P', 800, 0x80, 0); //Set digital pin 7 to User Output

	/*
	writeSercos_U16('P', 350, 0, 0); //set error reaction to torque off
	writeSercos_U16('P', 350, 0, 1);

	bool b_invert0 = axis0->invertDirection_param->value;
	bool b_invert1 = axis1->invertDirection_param->value;

	writeSercos_U16('S', 43, b_invert0 ? 0xD : 0x0, 0);
	writeSercos_U16('S', 55, b_invert0 ? 0xD : 0x0, 0);
	writeSercos_U16('S', 43, b_invert1 ? 0xD : 0x0, 1);
	writeSercos_U16('S', 55, b_invert1 ? 0xD : 0x0, 1);

	writeSercos_U32('P', 92, axis0->currentLimit_amps->value * 1000, 0);
	writeSercos_U32('P', 92, axis1->currentLimit_amps->value * 1000, 1);

	writeSercos_U32('S', 159, axis0->positionFollowingErrorLimit_rev->value * axis0->unitsPerRev, 0);
	writeSercos_U32('S', 159, axis1->positionFollowingErrorLimit_rev->value * axis1->unitsPerRev, 1);
	*/
	
	//setup cycle times
	uint16_t cycleTime_micros = EtherCatFieldbus::processInterval_milliseconds * 1000;
	uint32_t cycleTime_nanos = cycleTime_micros * 1000;
	uint32_t driveInterruptTime_nanos = 250'000;
	etherCatDevice->writeSercos_U16('S', 1, cycleTime_micros); //Control unit cycle time (TNcyc) [MANDATORY]
	etherCatDevice->writeSercos_U16('S', 2, cycleTime_micros); //Communication cycle time (tScyc) [MANDATORY]
	ec_dcsync01(etherCatDevice->getSlaveIndex(), true, driveInterruptTime_nanos, cycleTime_nanos - driveInterruptTime_nanos, 0); //[MANDATORY]
	
	return true;
}



void AX5000::readInputs(){
	memcpy(&acknowledgeTelegram, etherCatDevice->identity->inputs, etherCatDevice->identity->Ibytes);
		
	//Drive Data
	bool b_stoActive = acknowledgeTelegram.safetyOptionState & 0x1;
	bool digitalInput0 = acknowledgeTelegram.digitalInputsState & 0x1;
	bool digitalInput1 = acknowledgeTelegram.digitalInputsState & 0x2;
	bool digitalInput2 = acknowledgeTelegram.digitalInputsState & 0x4;
	bool digitalInput3 = acknowledgeTelegram.digitalInputsState & 0x8;
	bool digitalInput4 = acknowledgeTelegram.digitalInputsState & 0x10;
	bool digitalInput5 = acknowledgeTelegram.digitalInputsState & 0x20;
	bool digitalInput6 = acknowledgeTelegram.digitalInputsState & 0x40;
	
	*STOValue = invertSTO_param->value ? !b_stoActive : b_stoActive;
	*digitalin0Value = invertDigitalIn0_param->value ? !digitalInput0 : digitalInput0;
	*digitalin1Value = invertDigitalIn1_param->value ? !digitalInput1 : digitalInput1;
	*digitalin2Value = invertDigitalIn2_param->value ? !digitalInput2 : digitalInput2;
	*digitalin3Value = invertDigitalIn3_param->value ? !digitalInput3 : digitalInput3;
	*digitalin4Value = invertDigitalIn4_param->value ? !digitalInput4 : digitalInput4;
	*digitalin5Value = invertDigitalIn5_param->value ? !digitalInput5 : digitalInput5;
	*digitalin6Value = invertDigitalIn6_param->value ? !digitalInput6 : digitalInput6;
	
	//Axis Data
	actuators[0]->updateInputs(acknowledgeTelegram.ax0_driveStatusWord,
						acknowledgeTelegram.ax0_positionFeedbackValue1,
						acknowledgeTelegram.ax0_velocityFeedbackValue,
						acknowledgeTelegram.ax0_torqueFeedbackValue,
						acknowledgeTelegram.ax0_class1Diagnostics,
						b_stoActive);
	if(actuators.size() > 1){
		actuators[1]->updateInputs(acknowledgeTelegram.ax1_driveStatusWord,
								   acknowledgeTelegram.ax1_positionFeedbackValue1,
								   acknowledgeTelegram.ax1_velocityFeedbackValue,
								   acknowledgeTelegram.ax1_torqueFeedbackValue,
								   acknowledgeTelegram.ax1_class1Diagnostics,
								   b_stoActive);
	}
		
}
void AX5000::writeOutputs(){
	
	//Drive Data
	if(digitalOutput7_pin->isConnected()) digitalOutput7_pin->copyConnectedPinValue();
	bool digitalOutput7 = invertDigitalOut7_param->value ? (*digitalout7Value) : !(*digitalout7Value);
	masterDataTelegram.digitalOutput = digitalOutput7 ? 0x80 : 0x0;

	//Axis Data
	uint16_t previousControlWord0 = masterDataTelegram.ax0_masterControlWord;
	actuators[0]->updateOutputs(masterDataTelegram.ax0_masterControlWord,
								masterDataTelegram.ax0_velocityCommandValue,
								masterDataTelegram.ax0_positionCommandValue);
	if(previousControlWord0 != masterDataTelegram.ax0_masterControlWord)
		Logger::info("Axis 0 ctrlwrd {:x}", masterDataTelegram.ax0_masterControlWord);
	
	if(actuators.size() > 1){
		uint16_t previousControlWord1 = masterDataTelegram.ax1_masterControlWord;
		actuators[1]->updateOutputs(masterDataTelegram.ax1_masterControlWord,
									masterDataTelegram.ax1_velocityCommandValue,
									masterDataTelegram.ax1_positionCommandValue);
		if(previousControlWord1 != masterDataTelegram.ax1_masterControlWord)
			Logger::info("Axis 1 ctrlwrd {:x}", masterDataTelegram.ax1_masterControlWord);
	}

	memcpy(etherCatDevice->identity->outputs, &masterDataTelegram, etherCatDevice->identity->Obytes);
}



bool AX5000::save(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	XMLElement* pinInversionXML = xml->InsertNewChildElement("PinInversion");
	invertSTO_param->save(pinInversionXML);
	invertDigitalIn0_param->save(pinInversionXML);
	invertDigitalIn1_param->save(pinInversionXML);
	invertDigitalIn2_param->save(pinInversionXML);
	invertDigitalIn3_param->save(pinInversionXML);
	invertDigitalIn4_param->save(pinInversionXML);
	invertDigitalIn5_param->save(pinInversionXML);
	invertDigitalIn6_param->save(pinInversionXML);
	invertDigitalOut7_param->save(pinInversionXML);

	XMLElement* axis0XML = xml->InsertNewChildElement("Axis0");
	actuators[0]->save(axis0XML);
	
	if(actuators.size() > 1){
		XMLElement* axis1XML = xml->InsertNewChildElement("Axis1");
		actuators[1]->save(axis1XML);
	}
	
	return true;
}
bool AX5000::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	if(XMLElement* pinInversionXML = xml->FirstChildElement("PinInversion")){
		invertSTO_param->load(pinInversionXML);
		invertDigitalIn0_param->load(pinInversionXML);
		invertDigitalIn1_param->load(pinInversionXML);
		invertDigitalIn2_param->load(pinInversionXML);
		invertDigitalIn3_param->load(pinInversionXML);
		invertDigitalIn4_param->load(pinInversionXML);
		invertDigitalIn5_param->load(pinInversionXML);
		invertDigitalIn6_param->load(pinInversionXML);
		invertDigitalOut7_param->load(pinInversionXML);
	}
	
	if(XMLElement* axis0XML = xml->FirstChildElement("Axis0")){
		actuators[0]->load(axis0XML);
	}
	
	if(XMLElement* axis1XML = xml->FirstChildElement("Axis1")){
		if(actuators.size() > 1){
			actuators[1]->load(axis1XML);
		}
	}
	
	return true;
}












void AX5000::Actuator::updateInputs(uint16_t statusw, int32_t pos, int32_t vel, int16_t tor, uint16_t err, bool sto){

   statusWord.decode(statusw);

   bool previousEstop = actuatorProcessData.b_isEmergencyStopActive;
   if(previousEstop != sto){
	   //if(sto) Logger::warn("[{}] Axis {} : STO Active", drive->getName(), channel);
	   //else Logger::info("[{}] Axis {} : STO Cleared", drive->getName(), channel);
   }

   if(statusWord.shutdownError != b_hasFault){
	   //if(!statusWord.shutdownError) Logger::info("[{}] Axis {} : Fault Cleared", drive->getName(), channel);
	   //else Logger::error("[{}] Axis {} : Fault !", drive->getName(), channel);
   }
   if(statusWord.warningChange != b_warning){
	   //Logger::critical("[{}] Axis {} : Warning {}", drive->getName(), channel, statusWord.warningChange);
   }
   if(statusWord.infoChange != b_info){
	   //Logger::critical("[{}] Axis {} : Info {}", drive->getName(), channel, statusWord.infoChange);
   }



   if(class1Errors != err){
	   if(err != 0x0){
		   //Logger::error("[{}] Axis {} Errors:", drive->getName(), channel);
		   //Logger::error("{}", drive->getClass1Errors(err));
	   }
	   //else Logger::error("[{}] Axis {} : Error cleared !", drive->getName(), channel);
   }
   class1Errors = err;

   if(actuatorProcessData.b_enabling){
	   if(statusWord.isEnabled()){
		   actuatorProcessData.b_enabling = false;
		   //Logger::info("[{}] Axis {} Enabled", drive->getName(), channel);
	   }
	   else if(EtherCatFieldbus::getCycleProgramTime_nanoseconds() - actuatorProcessData.enableRequest_nanos > enableTimeout_nanos){
		   actuatorProcessData.b_enabling = false;
		   controlWord.disable();
		   //Logger::warn("[{}] Axis {} Enable timeout", drive->getName(), channel);
	   }
   }
   else if(isEnabled() && !statusWord.isEnabled()){
	   //Logger::error("[{} Axis {}] isEnabled: {}", drive->getName(), channel, isEnabled());
	   actuatorProcessData.b_enabling = false;
	   controlWord.disable();
	   //Logger::error("[{} Axis {}] Disabled (st:{} fl:{})", drive->getName(), channel, statusWord.status, statusWord.followsCommand);
   }


   //update interface state
   if(statusWord.isEnabled()) {
	   if(controlWord.isRequestingEnable()) state = DeviceState::ENABLED;
	   else state = DeviceState::DISABLING;
   }
   else{
	   if(actuatorProcessData.b_enabling) state = DeviceState::ENABLING;
	   else if(sto || !statusWord.isReady()) state = DeviceState::NOT_READY;
	   //else if(!drive->isStateOperational()) state = DeviceState::NOT_READY;
	   else state = DeviceState::READY;
   }
   feedbackProcessData.positionActual = double(pos) / unitsPerRev;
   feedbackProcessData.velocityActual = double(vel) / (unitsPerRPM * 60.0);
   actuatorProcessData.effortActual = std::abs(double(tor) / 1000.0);
   actuatorProcessData.b_isEmergencyStopActive = sto;
   b_hasFault = statusWord.shutdownError;
   b_warning = statusWord.warningChange;
   b_info = statusWord.infoChange;
}
void AX5000::Actuator::updateOutputs(uint16_t& controlw, int32_t& vel, uint32_t& pos){
   //controlWord.toggleSyncBit();
   if(actuatorProcessData.b_enable){
	   actuatorProcessData.b_enable = false;
	   actuatorProcessData.b_enabling = true;
	   actuatorProcessData.enableRequest_nanos = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
	   //if(statusWord.shutdownError) drive->requestFaultReset(channel);
   }
   if(actuatorProcessData.b_enabling){
	   if(!statusWord.shutdownError) controlWord.enable();
   }
   if(actuatorProcessData.b_disable){
	   actuatorProcessData.b_disable = false;
	   actuatorProcessData.b_enabling = false;
	   controlWord.disable();
	   //Logger::error("[{} Axis {}] Disable request", drive->getName(), channel);
   }
   
   vel = actuatorProcessData.velocityTarget * unitsPerRPM * 60.0;
   pos = actuatorProcessData.positionTarget * unitsPerRev;

   controlWord.encode(controlw);
}
bool AX5000::Actuator::save(tinyxml2::XMLElement* xml){
   //motorType->save(xml);
   velocityLimit_revps->save(xml);
   accelerationLimit_revps2->save(xml);
   positionFollowingErrorLimit_rev->save(xml);
   currentLimit_amps->save(xml);
   invertDirection_param->save(xml);
   return true;
}
bool AX5000::Actuator::load(tinyxml2::XMLElement* xml){
   //motorType->load(xml);
   velocityLimit_revps->load(xml);
   accelerationLimit_revps2->load(xml);
   positionFollowingErrorLimit_rev->load(xml);
   currentLimit_amps->load(xml);
   invertDirection_param->load(xml);
   return true;
}
std::string AX5000::Actuator::getStatusString(){
   if(!isOnline()) return "Actuator Offline";
   else if(isEnabled()) return "Actuator Enabled";
   else if(isEnabling()) return "Actuator Enabling...";
   else if(isReady()) return "Actuator Ready";
   else{
	   if(isEmergencyStopActive()) return "STO is active";
	   //if(!drive->isStateOperational()) return "Drive is not in operational state";
	   if(hasFault()) {
		   std::string output = "Axis has errors:\n";
		   //return output + drive->getClass1Errors(class1Errors);
	   }
	   else return "Actuator not ready";
   }
}



/*
void AX5206::requestFaultReset(uint8_t axis){
	std::thread faultResetThread([this,axis](){
		//set & enable command
		if(writeSercos_U16('S', 99, 3, axis)) Logger::info("Requested fault reset");
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		//cancel command
		if(writeSercos_U16('S', 99, 0, axis)) Logger::info("Fault reset request finished");
	});
	faultResetThread.detach();
}


void AX5206::getInvalidIDNsForSafeOp(){
	struct IDN_List{
		uint16_t actualLength = 0; //number of bytes in the idns array
		uint16_t maxLength = 0;
		uint16_t idns[10000] = {0};
	}invalidIDNs;
	int size = 100;
	
	if(!readSercos_Array('S', 21, (uint8_t*)&invalidIDNs, size)) Logger::warn("Failed to download IDN list");
	
	int invalidIDNcount = invalidIDNs.actualLength / 2;
	Logger::info("Invalid IDNs for PreOp->SafeOp Transition ({})", invalidIDNcount);
	for(int i = 0; i < invalidIDNcount; i++){
		uint16_t idn = invalidIDNs.idns[i];
		if(idn > 32768) Logger::info("    P-0-{}", idn - 32768);
		else Logger::info("    S-0-{}", idn);
	}
}

void AX5206::getInvalidIDNsForOp(){
	struct IDN_List{
		uint16_t actualLength = 0; //number of bytes in the idns array
		uint16_t maxLength = 0;
		uint16_t idns[10000] = {0};
	}invalidIDNs;
	int size = 100;
	
	if(!readSercos_Array('S', 22, (uint8_t*)&invalidIDNs, size)) Logger::warn("Failed to download IDN list");
	
	int invalidIDNcount = invalidIDNs.actualLength / 2;
	Logger::info("Invalid IDNs for SafeOp->Op Transition ({})", invalidIDNcount);
	for(int i = 0; i < invalidIDNcount; i++){
		uint16_t idn = invalidIDNs.idns[i];
		if(idn > 32768) Logger::info("    P-0-{}", idn - 32768);
		else Logger::info("    S-0-{}", idn);
	}
}

std::string AX5206::getClass1Errors(uint16_t c1diag){
	if(c1diag == 0x0) return "No Errors";
	std::string output;
	if(c1diag & 0x1) output += "Motor overload shut down\n";
	if(c1diag & 0x2) output += "Heatsink overtemperature shutdown\n";
	if(c1diag & 0x4) output += "Motor overtemperature shut down\n";
	if(c1diag & 0x8) output += "Drive overtemperature shut down\n";
	if(c1diag & 0x10) output += "Control voltage error\n";
	if(c1diag & 0x20) output += "Feedback error\n";
	if(c1diag & 0x40) output += "Commutation error\n";
	if(c1diag & 0x80) output += "Overcurrent error\n";
	if(c1diag & 0x100) output += "Overvoltage error\n";
	if(c1diag & 0x200) output += "Undervoltage error\n";
	if(c1diag & 0x400) output += "Power supply phase error\n";
	if(c1diag & 0x800) output += "Excessive position deviation\n";
	if(c1diag & 0x1000) output += "communication error\n";
	if(c1diag & 0x2000) output += "Overtravel limit exceeded\n";
	if(c1diag & 0x4000) output += "Unknown error\n";
	if(c1diag & 0x8000) output += "Manufacturer specific error\n";
	output.pop_back(); //remove last return character
	return output;
}

void AX5206::getShutdownErrorList(){
	uint16_t err0, err1;
	bool ret0 = readSercos_U16('S', 11, err0);
	bool ret1 = readSercos_U16('S', 11, err1);
	Logger::warn("Axis 0 Errors: ");
	Logger::error("{}", getClass1Errors(err0));
	//Logger::warn("Axis 1 Errors: "); //this does not seem to show anything
	//Logger::error("{}", getClass1Errors(err1));
}

void AX5206::getErrorHistory(){
	struct ErrorList{
		uint16_t actualSize;
		uint16_t maxSize;
		uint32_t errors[110];
	}errorList;
	int errorListSize = sizeof(ErrorList);
	struct ErrorTimeList{
		uint16_t actualSize;
		uint16_t maxSize;
		uint32_t errorTimes[110];
	}errorTimeList;
	int errorTimeListSize = sizeof(ErrorTimeList);
	
	auto secondsToTimeString = [](int total_seconds) -> std::string {
		int hours = total_seconds / 3600;
		int minutes = (total_seconds % 3600) / 60;
		int seconds = total_seconds % 60;
		char buffer[128];
		snprintf(buffer, 128, "%02d:%02d:%02d", hours, minutes, seconds);
		return std::string(buffer);
	};
	
	auto showErrorHistory = [&](uint8_t axis){
		bool ret1 = readSercos_Array('P', 300, (uint8_t*)&errorList, errorListSize, axis);
		bool ret2 = readSercos_Array('P', 301, (uint8_t*)&errorTimeList, errorTimeListSize, axis);
		Logger::info("{} {}", errorListSize, errorTimeListSize);
		if(ret1 && ret2){
			int errorCount = errorList.actualSize / 4;
			Logger::warn("Error History Axis [{}] : {} Errors", axis, errorCount);
			for(int i = 0; i < errorCount; i++){
				uint32_t errorCode = errorList.errors[i];
				std::string errorString = getErrorString(errorCode);
				std::string timeString = secondsToTimeString(errorTimeList.errorTimes[i]);
				if(errorString == "") Logger::error("    {:X} ({})", errorCode, timeString);
				else Logger::warn("    [Axis{}] {:X} : {} ({})", axis, errorCode, errorString, timeString);
			}
		}
	};
	
	showErrorHistory(0);
	showErrorHistory(1);
}

void AX5206::getDiagnosticsMessage(){
	std::string ax0diag;
	bool ret0 = readSercos_String('S', 95, ax0diag, 0); //same idn for axis 1 never displays anything
	Logger::warn("{}", ax0diag);
}

std::string AX5206::getErrorString(uint32_t errorCode){
	switch(errorCode){
		case 0xF100: return "Axis state machine: Communication error";
		case 0xF101: return "Axis state machine: Initialize error (selected uninitialized operating mode)";
		case 0xF106: return "Axis state machine: No motor configured";
		case 0xF107: return "Axis state machine: Current control not ready to enable";
		case 0xF152: return "Initialization of the feedback: Command failed";
		case 0xF166: return "Process data mapping: MDT - S-0-0024";
		case 0xF2A0: return "COMMUTATION ERROR (very rare)";
		case 0xF2A7: return "Torque off triggered from \"shorted coils brake\" or \"DC brake\"";
		case 0xF414: return "Distributed clocks: hardware sync";
		case 0xF415: return "Distributed clocks: Process data synchronization lost";
		case 0xF4A1: return "SoE Communication: Internal error";
		case 0xF4A3: return "SoE Communication: Internal error";
		case 0xF4A5: return "SoE Communication: Parameter error (invalid data for transition to safeop)";
		case 0xF720: return "Feedback \"general\": Initialization failed";
		case 0xF850: return "One cable feedback: Link missing";
		case 0xF851: return "One cable feedback: Initialization failed";
		case 0xF852: return "One cable feedback: Position error limit exceeded";
		case 0xF857: return "One cable feedback: Read of the cyclic data failed";
		case 0xF859: return "One cable feedback: Write system control failed";
		case 0xF85F: return "One cable feedback: Read of the cyclic data failed";
		case 0xFC03: return "Control voltage error: undervoltage";
		case 0xFD08: return "Motor management: Drive type don't match";
		case 0xFD09: return "Motor management: Motor type don't match";
		case 0xFD0A: return "Configured channel peak current is greater than the motor peak current";
		case 0xFD0C: return "Configured channel current is greater than the motor continuous stall current";
		case 0xFD11: return "Periphery voltage too low";
		case 0xFD1A: return "A/D-Converter: Motor temperature - measuring error";
		case 0xFD43: return "Mains supply: Power down";
		case 0xFD44: return "Mains supply: Phase error";
		case 0xFD47: return "Power management error";
		case 0xFDD3: return "Safety switch off while the axis was enabled";
		default: return "";
	};
}

std::string AX5206::getGpioStatus(){
	return "No status string yet :(";
}

*/
