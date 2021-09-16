#pragma once

#include "Fieldbus/EtherCatSlave.h"

class LedsAndButtons : public EtherCatSlave {
public:

	SLAVE_DEFINITION(LedsAndButtons, "LedsAndButtons", "Bausano", "I/O")

	//RxPDO
	uint8_t ui8_led0 = 0;
	uint8_t ui8_led1 = 0;
	uint8_t ui8_led2 = 0;
	uint8_t ui8_led3 = 0;
	uint8_t ui8_led4 = 0;
	
	//TxPDO
	uint8_t buttonWord = 0;

	std::shared_ptr<ioData> led0 = std::make_shared<ioData>(DataType::INTEGER_VALUE, DataDirection::NODE_INPUT, "LED-0");
	std::shared_ptr<ioData> led1 = std::make_shared<ioData>(DataType::INTEGER_VALUE, DataDirection::NODE_INPUT, "LED-1");
	std::shared_ptr<ioData> led2 = std::make_shared<ioData>(DataType::INTEGER_VALUE, DataDirection::NODE_INPUT, "LED-2");
	std::shared_ptr<ioData> led3 = std::make_shared<ioData>(DataType::INTEGER_VALUE, DataDirection::NODE_INPUT, "LED-3");
	std::shared_ptr<ioData> led4 = std::make_shared<ioData>(DataType::INTEGER_VALUE, DataDirection::NODE_INPUT, "LED-4");

	std::shared_ptr<ioData> button0		= std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Button0", ioDataFlags_DisableDataField);
	std::shared_ptr<ioData> button1		= std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Button1", ioDataFlags_DisableDataField);
	std::shared_ptr<ioData> button2		= std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Button2", ioDataFlags_DisableDataField);
	std::shared_ptr<ioData> button3		= std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Button3", ioDataFlags_DisableDataField);

};