#include "AX5206.h"
#include "Fieldbus/EtherCatFieldbus.h"

void AX5206::onDisconnection(){}
void AX5206::onConnection(){}
void AX5206::initialize(){}

bool AX5206::startupConfiguration() {
	
	struct IDN_List{
		uint16_t size = 0; //total bytes in idn list (idn count * 2)
		uint16_t maxLength = 0; //max length of array in slave, not useful when uploading
		uint16_t IDNs[10] = {0};
		uint8_t* getPointer(){ return (uint8_t*)this; }
	};
	
	//Master Data Telegram (MDT) [Master->Slave]
	IDN_List axis0_mdtList;
	axis0_mdtList.size = 8;
	//14 bytes axis 1
	//axis0_mdtList.IDNs[0] = 134; 	//(2) Master Control Word		[FIXED]
	axis0_mdtList.IDNs[0] = 47; 	//(4) Position Command Value
	axis0_mdtList.IDNs[1] = 36; 	//(4) Velocity Command Value
	axis0_mdtList.IDNs[2] = 80; 	//(2) Torque Command Value
	axis0_mdtList.IDNs[3] = 33570;	//(2) Digital Outputs
	IDN_List axis1_mdtList;
	axis1_mdtList.size = 6;
	//12 bytes axis 2
	//axis1_mdtList.IDNs[0] = 134; 	//(2) Master Control Word		[FIXED]
	axis1_mdtList.IDNs[0] = 47; 	//(4) Position Command Value
	axis1_mdtList.IDNs[1] = 36; 	//(4) Velocity Command Value
	axis1_mdtList.IDNs[2] = 80; 	//(2) Torque Command Value
	//26 bytes total for MDT

	//Acknowledge Telegram (AT) [Slave->Master]
	IDN_List axis0_atList;
	axis0_atList.size = 10;
	//18 bytes axis 1
	//axis0_atList.IDNs[0] = 135;	//(2) Drive Status Word (u16)						[FIXED]
	axis0_atList.IDNs[0] = 51;	//(4) Position Feedback Value 1 (motor feedback)
	axis0_atList.IDNs[1] = 40;	//(4) Velocity Feedback Value 1
	axis0_atList.IDNs[2] = 84;	//(2) Torque Feedback Value
	axis0_atList.IDNs[3] = 189;	//(4) Following Distance
	axis0_atList.IDNs[4] = 33569; //(2) Digital Inputs, state
	IDN_List axis1_atList;
	axis1_atList.size = 8;
	//16 bytes axis 2
	//axis1_atList.IDNs[0] = 135;	//(2) Drive Status Word (u16)						[FIXED]
	axis1_atList.IDNs[0] = 51;	//(4) Position Feedback Value 1 (motor feedback)
	axis1_atList.IDNs[1] = 40;	//(4) Velocity Feedback Value 1
	axis1_atList.IDNs[2] = 84;	//(2) Torque Feedback Value
	axis1_atList.IDNs[3] = 189;	//(4) Following Distance
	//34 bytes total for AT
	
	
	
	//upload process data configuration
	writeSercos_Array(24, axis0_mdtList.getPointer(), sizeof(IDN_List), 0); //MDT List [axis0]
	writeSercos_Array(24, axis1_mdtList.getPointer(), sizeof(IDN_List), 1); //MDT List [axis1]
	writeSercos_Array(16, axis0_atList.getPointer(), sizeof(IDN_List), 0); //AT List [axis0]
	writeSercos_Array(16, axis1_atList.getPointer(), sizeof(IDN_List), 1); //AT List [axis1]
	
	writeSercos_U16(34768, 1); //Configured Safety Option (1 == AX5801 Card) [MANDATORY]
	
	//setup cycle times
	uint16_t cycleTime_micros = EtherCatFieldbus::processInterval_milliseconds * 1000;
	uint32_t cycleTime_nanos = cycleTime_micros * 1000;
	uint32_t driveInterruptTime_nanos = 250'000;
	writeSercos_U16(1, cycleTime_micros); //Control unit cycle time (TNcyc) [MANDATORY]
	writeSercos_U16(2, cycleTime_micros); //Communication cycle time (tScyc) [MANDATORY]
	ec_dcsync01(getSlaveIndex(), true, driveInterruptTime_nanos, cycleTime_nanos - driveInterruptTime_nanos, 0); //[MANDATORY]
	
	//TODO: figure out shitftime in dc function
	//TODO: how to change operating mode during operation
	
	return true;
}

void AX5206::readInputs(){
	memcpy(&acknowledgeTelegram, identity->inputs, identity->Ibytes);
	
	ax0_driveStatus.update(acknowledgeTelegram.ax0_driveStatusWord);
	ax1_driveStatus.update(acknowledgeTelegram.ax1_driveStatusWord);
	
}
void AX5206::writeOutputs(){
	ax0_driveControl.update(masterDataTelegram.ax0_masterControlWord);
	ax1_driveControl.update(masterDataTelegram.ax1_masterControlWord);
	
	memcpy(identity->outputs, &masterDataTelegram, identity->Obytes);
}

bool AX5206::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	return true;
}

bool AX5206::loadDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	return true;
}


/*
writeSercos_U16(32, 11, 0);	//Primary Operation Mode [axis0]
writeSercos_U16(32, 11, 1);	//Primary Operation Mode [axis1]

writeSercos_U16(15, 7, 0); //Telegram Type, [axis0]
writeSercos_U16(15, 7, 1); //Telegram Type [axis1]

writeSercos_U64(32778, 0, 0); //P-0010
writeSercos_U16(33072, 1, 0); //P-0304
writeSercos_U64(32778, 0, 1); //P-0010
writeSercos_U16(33072, 1, 1); //P-0304

writeSercos_U16(33072, 1, 0); //P-0-304 Report diagnostics information [axis0]
writeSercos_U16(33072, 1, 1); //P-0-304 Report diagnostics information [axis1]
writeSercos_U16(32969, 4000); //P-0-201 Nominal Mains Voltage
writeSercos_U16(33324, 0, 0); //position interpolation type
writeSercos_U16(33324, 0, 1); //position interpolation type
writeSercos_U16(0104, 100, 0); //position loop Kv
writeSercos_U16(0104, 100, 1); //position loop Kv
*/
