#pragma once

#include "Fieldbus/EtherCatSlave.h"

class EasyCAT : public EtherCatSlave {
public:

	SLAVE_DEFINITION(EasyCAT, "Artefact", "LeoBecker")

	std::shared_ptr<ioData> byteOut =		std::make_shared<ioData>(DataType::UINT8_T, DataDirection::NODE_INPUT, "byte0-out");
	std::shared_ptr<ioData> shortOut =		std::make_shared<ioData>(DataType::UINT16_T, DataDirection::NODE_INPUT, "short0-out");
	std::shared_ptr<ioData> longOut =		std::make_shared<ioData>(DataType::UINT32_T, DataDirection::NODE_INPUT, "long0-out");
	std::shared_ptr<ioData> longLongOut =	std::make_shared<ioData>(DataType::UINT64_T, DataDirection::NODE_INPUT, "longLong0-out");
		
	std::shared_ptr<ioData> byteIn =		std::make_shared<ioData>(DataType::UINT8_T, DataDirection::NODE_OUTPUT, "byte0-in");
	std::shared_ptr<ioData> shortIn =		std::make_shared<ioData>(DataType::UINT16_T, DataDirection::NODE_OUTPUT, "short0-in");
	std::shared_ptr<ioData> longIn =		std::make_shared<ioData>(DataType::UINT32_T, DataDirection::NODE_OUTPUT, "long0-in");
	std::shared_ptr<ioData> longLongIn =	std::make_shared<ioData>(DataType::UINT64_T, DataDirection::NODE_OUTPUT, "longLong0-in");

	uint8_t ui8_byteOut = 0;
	uint16_t ui16_shortOut = 0;
	uint32_t ui32_longOut = 0;
	uint64_t ui64_longLongOut = 0;

	uint8_t ui8_byteIn = 0;
	uint16_t ui16_shortIn = 0;
	uint32_t ui32_longIn = 0;
	uint64_t ui64_longLongIn = 0;
};