#include <pch.h>
#include "LedsAndButtons.h"

#include "Fieldbus/EtherCatFieldbus.h"

LedsAndButtons::LedsAndButtons() {

	addIoData(&led0);
	addIoData(&led1);
	addIoData(&led2);
	addIoData(&led3);
	addIoData(&led4);

	addIoData(&button0);
	addIoData(&button1);
	addIoData(&button2);
	addIoData(&button3);

	txPdoAssignement.addNewModule(0x0);
	txPdoAssignement.addEntry(0x0, 0x0, 1, "buttonWord", &buttonWord);

	rxPdoAssignement.addNewModule(0x0);
	rxPdoAssignement.addEntry(0x0, 0x0, 1, "led0", &ui8_led0);
	rxPdoAssignement.addEntry(0x0, 0x0, 1, "led1", &ui8_led1);
	rxPdoAssignement.addEntry(0x0, 0x0, 1, "led2", &ui8_led2);
	rxPdoAssignement.addEntry(0x0, 0x0, 1, "led3", &ui8_led3);
	rxPdoAssignement.addEntry(0x0, 0x0, 1, "led4", &ui8_led4);
}

bool LedsAndButtons::startupConfiguration() {
	uint32_t sync0Interval_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 1000000.0L;
	uint32_t sync0offset_nanoseconds = 0;
	ec_dcsync0(getSlaveIndex(), true, sync0Interval_nanoseconds, sync0offset_nanoseconds);
	return true;
}

void LedsAndButtons::readInputs() {
	uint8_t* inputData = identity->inputs;
	buttonWord = inputData[0];
	button0 = (buttonWord & 0x1) != 0;
	button1 = (buttonWord & 0x2) != 0;
	button2 = (buttonWord & 0x4) != 0;
	button3 = (buttonWord & 0x8) != 0;
}

void LedsAndButtons::process(bool b_processDataValid) {}

void LedsAndButtons::prepareOutputs() {
	uint8_t* outputData = identity->outputs;

	ui8_led0 = led0.getUnsignedByte();
	ui8_led1 = led1.getUnsignedByte();
	ui8_led2 = led2.getUnsignedByte();
	ui8_led3 = led3.getUnsignedByte();
	ui8_led4 = led4.getUnsignedByte();

	outputData[0] = ui8_led0;
	outputData[1] = ui8_led1;
	outputData[2] = ui8_led2;
	outputData[3] = ui8_led3;
	outputData[4] = ui8_led4;
}