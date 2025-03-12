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

class EK1310 : public EtherCatDevice{
public:
	DEFINE_ETHERCAT_DEVICE(EK1310, "EK1310 EtherCAT-P extension with feed-in", "EK1310", "Beckhoff", "I/O", 0x2, 0x51E2C52)
};


class EK1122 : public EtherCatDevice {
public:
	DEFINE_ETHERCAT_DEVICE(EK1122, "EK1122 2-Port EtherCAT Junction", "EK1122", "Beckhoff", "Utilities", 0x2, 0x4622c52)
};

class EK1521_0010 : public EtherCatDevice {
public:
	DEFINE_ETHERCAT_DEVICE(EK1521_0010, "EK1521-0010 1-port EtherCAT fiber-optic junction", "EK1521-0010", "Beckhoff", "Utilities", 0x2, 0x5f12c52)
};

class CU1521_0010 : public EtherCatDevice {
public:
	DEFINE_ETHERCAT_DEVICE(CU1521_0010, "CU1521-0010 Ethernet / Single-mode fiber converter", "CU1521-0010", "Beckhoff", "Utilities", 0x2, 0x5f15432)
};

class EL2008 : public EtherCatDevice{
public:
DEFINE_ETHERCAT_DEVICE(EL2008, "EL2008 8x Digital Output", "EL2008", "Beckhoff", "I/O", 0x2, 0x7d83052)
	//rxPdo
	bool outputs[8] = {0,0,0,0,0,0,0,0};
	
	std::vector<std::shared_ptr<bool>> pinValues;
	std::vector<std::shared_ptr<NodePin>> pins;
	std::vector<BoolParam> signalInversionParams;
	
};

class EL1008 : public EtherCatDevice{
public:
DEFINE_ETHERCAT_DEVICE(EL1008, "EL1008 8x Digital Input", "EL1008", "Beckhoff", "I/O", 0x2, 0x3f03052)
	//txPdo
	bool inputs[8] = {0,0,0,0,0,0,0,0};
	std::vector<std::shared_ptr<bool>> pinValues;
	std::vector<std::shared_ptr<NodePin>> pins;
	std::vector<BoolParam> signalInversionParams;
};

class EPP1008_0001 : public EtherCatDevice{
public:
DEFINE_ETHERCAT_DEVICE(EPP1008_0001, "EPP1008-0001 8x Digital Input", "EPP1008-0001", "Beckhoff", "I/O", 0x2, 0x64760509)
	//txPdo
	bool inputs[8] = {0,0,0,0,0,0,0,0};
	std::vector<std::shared_ptr<bool>> pinValues;
	std::vector<std::shared_ptr<NodePin>> pins;
	std::vector<BoolParam> signalInversionParams;

	std::shared_ptr<GpioInterface> gpio = std::make_shared<GpioInterface>();
};

class EL2624 : public EtherCatDevice{
public:
DEFINE_ETHERCAT_DEVICE(EL2624, "EL2624 4x Relay Output", "EL2624", "Beckhoff", "I/O", 0x2, 0xa403052)
	//rxPdo
	bool outputs[4] = {0,0,0,0};
	std::vector<std::shared_ptr<bool>> pinValues;
	std::vector<std::shared_ptr<NodePin>> pins;
	std::vector<BoolParam> signalInversionParams;
};

class EL5001 : public EtherCatDevice{
public:
DEFINE_ETHERCAT_DEVICE(EL5001, "EL5001 SSI Input", "EL5001", "Beckhoff", "I/O", 0x2, 0x13893052)
	
	
	std::shared_ptr<bool> resetPinValue = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> encoderPin = std::make_shared<NodePin>(NodePin::DataType::MOTIONFEEDBACK_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "SSI Encoder");
	std::shared_ptr<NodePin> resetPin = std::make_shared<NodePin>(resetPinValue, NodePin::Direction::NODE_OUTPUT, "Encoder Reset");
	
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
	
	OptionParameter::Option ssiBaudrate_1000Khz = OptionParameter::Option(2, "1000 Khz", "1000Khz");
	OptionParameter::Option ssiBaudrate_500Khz = OptionParameter::Option(3, "500 Khz", "500Khz");
	OptionParameter::Option ssiBaudrate_250Khz = OptionParameter::Option(4, "250 Khz", "250Khz");
	OptionParameter::Option ssiBaudrate_125Khz = OptionParameter::Option(5, "125 Khz", "125Khz");
	std::vector<OptionParameter::Option*> ssiBaudrateOptions = {
		&ssiBaudrate_1000Khz,
		&ssiBaudrate_500Khz,
		&ssiBaudrate_250Khz,
		&ssiBaudrate_125Khz
	};
	
	BoolParam centerOnZero_Param;
	BoolParam invertDirection_Param;
	BoolParam hasResetSignal_Param;
	NumberParam<double> resetSignalTime_Param;
	BoolParam reportOfflineState_Param;
	
	bool b_dataError = false;		//b0
	bool b_frameError = false;		//b1
	bool b_powerFailure = false;	//b2
	bool b_dataMismatch = false;	//b3
	bool b_syncError = false;		//b5
	bool b_txPdoState = false;		//b6
	bool b_txPdoToggle = false;		//b7
	
	uint64_t resetStartTime_nanoseconds = 0;
	
	double positionBeforeOffset_rev = 0.0;
	double positionOffset_rev = 0.0;
	
	double previousPosition_rev = 0.0;
	uint64_t previousReadingTime_nanoseconds = 0;
	
	//txPdo
	uint8_t status;
	uint32_t ssiValue;
	
	std::string frameFormatString;
	void updateSSIFrameFormat();
	void updateEncoderWorkingRange();
	
	
	//————— SubDevice ——————
	
	class SsiEncoder : public MotionFeedbackInterface{
	public:
		SsiEncoder(std::shared_ptr<EL5001> parentDevice) : encoderModule(parentDevice){}
		
		virtual std::string getName() override {
			return std::string(encoderModule->getName());
		};
		
		virtual std::string getStatusString() override {
			if(state == DeviceState::OFFLINE) {
				std::string message = std::string(encoderModule->getName()) + " is Offline";
				return message;
			}
			else if(state == DeviceState::NOT_READY){
				std::string message = "Encoder has connection or configuration issue";
				return message;
			}
			else return "Encoder is Operating Nominally";
		}
						
		std::shared_ptr<EL5001> encoderModule;
	};
	std::shared_ptr<SsiEncoder> encoder;

	
	
};
