#pragma once

#include "Fieldbus/EtherCatDevice.h"

class LedsAndButtons : public EtherCatDevice {
public:

	ETHERCAT_DEVICE_DEFINITION(LedsAndButtons, "LedsAndButtons", "Bausano", "I/O")

	//RxPDO
	uint8_t ui8_led0 = 0;
	uint8_t ui8_led1 = 0;
	uint8_t ui8_led2 = 0;
	uint8_t ui8_led3 = 0;
	uint8_t ui8_led4 = 0;
	
	//TxPDO
	uint8_t buttonWord = 0;

	std::shared_ptr<NodePin> led0 = std::make_shared<NodePin>(NodeData::INTEGER_VALUE, DataDirection::NODE_INPUT, "LED-0");
	std::shared_ptr<NodePin> led1 = std::make_shared<NodePin>(NodeData::INTEGER_VALUE, DataDirection::NODE_INPUT, "LED-1");
	std::shared_ptr<NodePin> led2 = std::make_shared<NodePin>(NodeData::INTEGER_VALUE, DataDirection::NODE_INPUT, "LED-2");
	std::shared_ptr<NodePin> led3 = std::make_shared<NodePin>(NodeData::INTEGER_VALUE, DataDirection::NODE_INPUT, "LED-3");
	std::shared_ptr<NodePin> led4 = std::make_shared<NodePin>(NodeData::INTEGER_VALUE, DataDirection::NODE_INPUT, "LED-4");

	std::shared_ptr<NodePin> button0		= std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Button0", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> button1		= std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Button1", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> button2		= std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Button2", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> button3		= std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Button3", NodePinFlags_DisableDataField);

};