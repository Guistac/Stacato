#pragma once

#include "Fieldbus/EtherCatSlave.h"

class LedsAndButtons : public EtherCatSlave {
public:

	SLAVE_DEFINITION(LedsAndButtons, "LedsAndButtons")

	//RxPDO
	uint8_t ui8_led0 = 0;
	uint8_t ui8_led1 = 0;
	uint8_t ui8_led2 = 0;
	uint8_t ui8_led3 = 0;
	uint8_t ui8_led4 = 0;
	
	//TxPDO
	uint8_t buttonWord = 0;

	ioData led0 = ioData(DataType::UINT8_T, DataDirection::NODE_INPUT, "LED-0");
	ioData led1 = ioData(DataType::UINT8_T, DataDirection::NODE_INPUT, "LED-1");
	ioData led2 = ioData(DataType::UINT8_T, DataDirection::NODE_INPUT, "LED-2");
	ioData led3 = ioData(DataType::UINT8_T, DataDirection::NODE_INPUT, "LED-3");
	ioData led4 = ioData(DataType::UINT8_T, DataDirection::NODE_INPUT, "LED-4");

	ioData button0 = ioData(DataType::BOOL_VALUE, DataDirection::NODE_OUTPUT, "Button0");
	ioData button1 = ioData(DataType::BOOL_VALUE, DataDirection::NODE_OUTPUT, "Button1");
	ioData button2 = ioData(DataType::BOOL_VALUE, DataDirection::NODE_OUTPUT, "Button2");
	ioData button3 = ioData(DataType::BOOL_VALUE, DataDirection::NODE_OUTPUT, "Button3");

};