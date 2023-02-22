#include <pch.h>

#include "BusCouper_IL-EC-BK.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>

#include "ioModules.h"

namespace PhoenixContact{


std::vector<EtherCAT::ModularDeviceProfile::DeviceModule*>& BusCoupler::getModuleFactory(){
	return ModuleFactory::getModules();
}

void BusCoupler::beforeModuleReordering(){
	txPdoAssignement.clear();
	txPdoAssignement.addNewModule(0x1AFF);
	txPdoAssignement.addEntry(0x0, 		0x0, 16, "Mandatory Diagnostics word 0", 			&diagnosticsWord0);
	txPdoAssignement.addEntry(0xF100, 	0x1, 16, "Interbus Diagnotstics State", 			&diagnosticsWord1);
	txPdoAssignement.addEntry(0xF100, 	0x2, 16, "Interbus Diagnotstics Parameter", 		&diagnosticsWord2);
	txPdoAssignement.addEntry(0xF100, 	0x3, 16, "Interbus Diagnotstics Ext. Parameter", 	&diagnosticsWord3);
}

void BusCoupler::onDisconnection() {
	for(auto& module : modules) module->onDisconnection();
	gpioDevice->state = DeviceState::OFFLINE;
}

void BusCoupler::onConnection() {
	for(auto& module : modules) module->onConnection();
	gpioDevice->state = DeviceState::ENABLED;
}

void BusCoupler::initialize() {
	//by default, this node only has one pin
	//no modules are loaded by default
	auto thisCoupler = std::static_pointer_cast<BusCoupler>(shared_from_this());
	gpioDevice = std::make_shared<PhoenixContactGpioDevice>(thisCoupler);
	auto abstractGpioDevice = std::static_pointer_cast<GpioInterface>(gpioDevice);
	gpioDeviceLink->assignData(abstractGpioDevice);
	//gpio device link pin
	addNodePin(gpioDeviceLink);
}


bool BusCoupler::startupConfiguration() {
	
	//no idea why CanOpen SDO data is not available until around 300 milliseconds after transition to PreOp
	//i guess we just need to wait
	while(true){
		double timeSinceFieldbusConfigurationStart = Timing::getProgramTime_seconds() - EtherCatFieldbus::getConfigurationProgramStartTime_seconds();
		if(timeSinceFieldbusConfigurationStart > .5) break;
		else std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	uint8_t resetBehavior = 0x0; //reset all outputs to 0 when fault occurs
	if(!writeSDO_U8(0xF801, 0x0, resetBehavior)) {
		return Logger::error("{} : Could not configure reset behavior", getName());
	}
	
	//object 0xF030 (Configured Module Ident List) gets written in configureModules()
	//not sure if this is working or really necessary after all...
	uint8_t validateModuleConfiguration = 0x1;
	if(!writeSDO_U8(0xF802, 0x0, validateModuleConfiguration)) {
		return Logger::error("{} : Could not configure module configuration validation", getName());
	}
	
	if(!configureModules()) return false;
	return true;
}


void BusCoupler::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	readModuleInputs();
}


void BusCoupler::writeOutputs(){
	writeModuleOutputs();
	rxPdoAssignement.pushDataTo(identity->outputs);
}

bool BusCoupler::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	saveModules(xml);
	return true;
}


bool BusCoupler::loadDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	if(!loadModules(xml)) return Logger::warn("Failed to load modules");
	return true;
}


}
