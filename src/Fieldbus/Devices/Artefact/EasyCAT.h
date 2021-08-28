#pragma once

#include "Fieldbus/EtherCatSlave.h"

class EasyCAT : public EtherCatSlave {
public:

	SLAVE_DEFINITION(EasyCAT, "Artefact")

	EtherCatData byteOut =		EtherCatData(EtherCatData::UINT8_T, "byte0-out");
	EtherCatData shortOut =		EtherCatData(EtherCatData::UINT16_T, "short0-out");
	EtherCatData longOut =		EtherCatData(EtherCatData::UINT32_T, "long0-out");
	EtherCatData longLongOut =	EtherCatData(EtherCatData::UINT64_T, "longLong0-out");

	EtherCatData byteIn =		EtherCatData(EtherCatData::UINT8_T, "byte0-in");
	EtherCatData shortIn =		EtherCatData(EtherCatData::UINT16_T, "short0-in");
	EtherCatData longIn =		EtherCatData(EtherCatData::UINT32_T, "long0-in");
	EtherCatData longLongIn =	EtherCatData(EtherCatData::UINT64_T, "longLong0-in");

};