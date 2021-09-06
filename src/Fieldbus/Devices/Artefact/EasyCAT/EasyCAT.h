#pragma once

#include "Fieldbus/EtherCatSlave.h"

class EasyCAT : public EtherCatSlave {
public:

	SLAVE_DEFINITION(EasyCAT, "Artefact")

	ioData byteOut =		ioData(DataType::INTEGER_VALUE, DataDirection::NODE_INPUT, "byte0-out");
	ioData shortOut =		ioData(DataType::INTEGER_VALUE, DataDirection::NODE_INPUT, "short0-out");
	ioData longOut =		ioData(DataType::INTEGER_VALUE, DataDirection::NODE_INPUT, "long0-out");
	ioData longLongOut =	ioData(DataType::INTEGER_VALUE, DataDirection::NODE_INPUT, "longLong0-out");
		
	ioData byteIn =			ioData(DataType::INTEGER_VALUE, DataDirection::NODE_OUTPUT, "byte0-in");
	ioData shortIn =		ioData(DataType::INTEGER_VALUE, DataDirection::NODE_OUTPUT, "short0-in");
	ioData longIn =			ioData(DataType::INTEGER_VALUE, DataDirection::NODE_OUTPUT, "long0-in");
	ioData longLongIn =		ioData(DataType::INTEGER_VALUE, DataDirection::NODE_OUTPUT, "longLong0-in");

	uint8_t ui8_byteOut = 0;
	uint16_t ui16_shortOut = 0;
	uint32_t ui32_longOut = 0;
	uint64_t ui64_longLongOut = 0;

	uint8_t ui8_byteIn = 0;
	uint16_t ui16_shortIn = 0;
	uint32_t ui32_longIn = 0;
	uint64_t ui64_longLongIn = 0;
};