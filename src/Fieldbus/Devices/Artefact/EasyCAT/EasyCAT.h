#pragma once

#include "Fieldbus/EtherCatDevice.h"

class EasyCAT : public EtherCatDevice {
public:

	DEFINE_ETHERCAT_DEVICE(EasyCAT, "Artefact", "EasyCAT", "Artefact", "Bausano", "I/O")

	std::shared_ptr<NodePin> byteOut =		std::make_shared<NodePin>(NodeData::INTEGER_VALUE, DataDirection::NODE_INPUT, "byte0-out");
	std::shared_ptr<NodePin> shortOut =		std::make_shared<NodePin>(NodeData::INTEGER_VALUE, DataDirection::NODE_INPUT, "short0-out");
	std::shared_ptr<NodePin> longOut =		std::make_shared<NodePin>(NodeData::INTEGER_VALUE, DataDirection::NODE_INPUT, "long0-out");
	std::shared_ptr<NodePin> longLongOut =	std::make_shared<NodePin>(NodeData::INTEGER_VALUE, DataDirection::NODE_INPUT, "longLong0-out");
		
	std::shared_ptr<NodePin> byteIn =		std::make_shared<NodePin>(NodeData::INTEGER_VALUE, DataDirection::NODE_OUTPUT, "byte0-in", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> shortIn =		std::make_shared<NodePin>(NodeData::INTEGER_VALUE, DataDirection::NODE_OUTPUT, "short0-in", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> longIn =		std::make_shared<NodePin>(NodeData::INTEGER_VALUE, DataDirection::NODE_OUTPUT, "long0-in", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> longLongIn =	std::make_shared<NodePin>(NodeData::INTEGER_VALUE, DataDirection::NODE_OUTPUT, "longLong0-in", NodePinFlags_DisableDataField);

	uint8_t ui8_byteOut = 0;
	uint16_t ui16_shortOut = 0;
	uint32_t ui32_longOut = 0;
	uint64_t ui64_longLongOut = 0;

	uint8_t ui8_byteIn = 0;
	uint16_t ui16_shortIn = 0;
	uint32_t ui32_longIn = 0;
	uint64_t ui64_longLongIn = 0;
};