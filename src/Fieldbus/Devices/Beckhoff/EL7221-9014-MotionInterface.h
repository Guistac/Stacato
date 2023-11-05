#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Project/Editor/Parameter.h"

class EL7221_9014 : public EtherCatDevice{
public:
	DEFINE_ETHERCAT_DEVICE(EL7221_9014, "EL7221-9014", "EL7221-9014", "Beckhoff", "Servo Drives", 0x2, 0x1c353052)
	
	//rx-pdo
	uint16_t controlWord;				//7010:1
	uint8_t modeOfOperationSelection;	//7010:3
	uint32_t targetPosition;			//7010:5
	int32_t targetVelocity;				//7010:6
	
	//tx-pdo
	uint16_t statusWord;				//6010:1
	uint8_t modeOfOperationDisplay;		//6010:3
	int32_t fbPosition;					//6000:11
	int32_t followingErrorActualValue;	//6010:6
	int32_t velocityActualValue;		//6010:7
	int16_t torqueActualValue;			//6010:8
	//uint16_t infoData1;					//6010:12
	//uint16_t infoData2;					//6010:13
};
