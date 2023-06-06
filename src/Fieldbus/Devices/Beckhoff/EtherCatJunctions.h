#pragma once

#include "Fieldbus/EtherCatDevice.h"

#include "Project/Editor/Parameter.h"

class CU1128 : public EtherCatDevice {
public:
    DEFINE_ETHERCAT_DEVICE(CU1128, "CU1128 8-Port EtherCAT Junction", "CU1128", "Beckhoff", "Utilities", 0x2, 0x4685432)
};

class CU1124 : public EtherCatDevice {
public:
	DEFINE_ETHERCAT_DEVICE(CU1124, "CU1124 4-Port EtherCAT Junction", "CU1124", "Beckhoff", "Utilities", 0x2, 0x4645432)
};


class EK1100 : public EtherCatDevice{
public:
DEFINE_ETHERCAT_DEVICE(EK1100, "EK1100 Bus Coupler", "EK1100", "Beckhoff", "I/O", 0x2, 0x44c2c52)
};
 
class EL2008 : public EtherCatDevice{
public:
DEFINE_ETHERCAT_DEVICE(EL2008, "EL2008 8x Digital Output", "EL2008", "Beckhoff", "I/O", 0x2, 0x7d83052)
	bool outputs[8] = {0,0,0,0,0,0,0,0};
};

class EL5001 : public EtherCatDevice{
public:
DEFINE_ETHERCAT_DEVICE(EL5001, "EK1100 SSI Input", "EL5001", "Beckhoff", "I/O", 0x2, 0x13893052)
	//txPdo
	uint8_t status;
	uint32_t ssiValue;
	
	NumberParam<int> ssiFrameSize;
	NumberParam<int> multiturnResolution;
	NumberParam<int> singleturnResolution;
	NumberParam<int> inhibitTime;
	OptionParam ssiCoding_parameter;
	OptionParam ssiBaudrate_parameter;
	
	OptionParameter::Option ssiCoding_binary = OptionParameter::Option(0, "Binary", "Binary");
	OptionParameter::Option ssiCoding_gray = OptionParameter::Option(1, "Gray", "Gray");
	std::vector<OptionParameter::Option*> ssiCodingOptions = {
		&ssiCoding_binary,
		&ssiCoding_gray
	};
	
	OptionParameter::Option ssiBaudrate_1250Khz = OptionParameter::Option(1, "1250 Khz", "1250Khz");
	OptionParameter::Option ssiBaudrate_1000Khz = OptionParameter::Option(2, "1000 Khz", "1000Khz");
	OptionParameter::Option ssiBaudrate_500Khz = OptionParameter::Option(3, "500 Khz", "500Khz");
	OptionParameter::Option ssiBaudrate_250Khz = OptionParameter::Option(4, "250 Khz", "250Khz");
	OptionParameter::Option ssiBaudrate_125Khz = OptionParameter::Option(5, "125 Khz", "125Khz");
	std::vector<OptionParameter::Option*> ssiBaudrateOptions = {
		&ssiBaudrate_1250Khz,
		&ssiBaudrate_1000Khz,
		&ssiBaudrate_500Khz,
		&ssiBaudrate_250Khz,
		&ssiBaudrate_125Khz
	};
	
	bool b_dataError = false;		//b0
	bool b_frameError = false;		//b1
	bool b_powerFailure = false;	//b2
	bool b_dataMismatch = false;	//b3
	bool b_syncError = false;		//b5
	bool b_txPdoState = false;		//b6
	bool b_txPdoToggle = false;		//b7
	
	std::string frameFormatString;
	void updateSSIFrameFormat();
};

