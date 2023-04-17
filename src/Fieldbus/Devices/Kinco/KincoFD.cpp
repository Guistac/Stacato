#include "KincoFD.h"

#include "Fieldbus/EtherCatFieldbus.h"

void KincoFD::onDisconnection() {}
void KincoFD::onConnection() {}

void KincoFD::onConstruction() {
	EtherCatDevice::onConstruction();
	
	auto thisDevice = std::static_pointer_cast<EtherCatDevice>(shared_from_this());
	axis = DS402Axis::make(thisDevice);
	
	axis->processDataConfiguration.enableCyclicSynchronousPositionMode();
	//axis->processDataConfiguration.enableFrequencyMode();
	//axis->processDataConfiguration.enableCyclicSynchronousVelocityMode();
	//axis->processDataConfiguration.positionFollowingErrorActualValue = true;
	axis->processDataConfiguration.errorCode = true;
	//axis->processDataConfiguration.currentActualValue = true;
	
	rxPdoAssignement.addNewModule(0x1600);
	txPdoAssignement.addNewModule(0x1A00);
	axis->configureProcessData();
}

bool KincoFD::startupConfiguration() {
	//———— PDO Assignement

	if(!rxPdoAssignement.mapToRxPdoSyncManager(getSlaveIndex())) return false;
	if(!txPdoAssignement.mapToTxPdoSyncManager(getSlaveIndex())) return false;
	
	//———— Synchronisation
	
	uint32_t cycleTime_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 1'000'000;
	uint32_t shiftTime_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 500'000;
	ec_dcsync0(getSlaveIndex(), true, cycleTime_nanoseconds, shiftTime_nanoseconds);
	
	return true;
}
void KincoFD::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	axis->updateInputs();
	
	
	
}
void KincoFD::writeOutputs(){
	
	if(b_enable){
		b_enable = false;
		axis->enable();
	}
	
	if(b_disable){
		b_disable = false;
		axis->disable();
	}
	
	axis->setOperatingMode(DS402Axis::OperatingMode::CYCLIC_SYNCHRONOUS_POSITION);
	axis->setPosition(pos);
	
	axis->updateOutput();
	rxPdoAssignement.pushDataTo(identity->outputs);
}


bool KincoFD::onSerialization() {
	bool success = true;
	success &= EtherCatDevice::onSerialization();
	return success;
}
bool KincoFD::onDeserialization() {
	bool success = true;
	success &= EtherCatDevice::onDeserialization();
	return success;
}
