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


class EL3078 : public EtherCatDevice{
public:
	DEFINE_ETHERCAT_DEVICE(EL3078, "EL3078 8x Analog Input (12bit)", "EL3078", "Beckhoff", "I/O", 0x2, 0xC063052)
	int16_t analogInputs[8] = {0,0,0,0,0,0,0,0};
	std::vector<std::shared_ptr<double>> pinValues;
	std::vector<std::shared_ptr<NodePin>> pins;
	
	void updloadConfiguration();
	
	enum InputType{
		BIPOLAR_10V = 2,
		UNIPOLAR_10V = 14,
		BIPOLAR_20MA = 17,
		UNIPOLAR_20MA = 18,
		UNIPOLAR_4_20MA = 19,
		UNIPOLAR_4_20MA_NAMUR = 20
	};
	
	Option inputType_Bipolar10V = 			Option(InputType::BIPOLAR_10V, 				"±10V", "±10V");
	Option inputType_Unipolar10V = 			Option(InputType::UNIPOLAR_10V, 			"0-10V", "0-10V");
	Option inputType_Bipolar20ma = 			Option(InputType::BIPOLAR_20MA, 			"±20mA", "±20mA");
	Option inputType_Unipolar20ma = 		Option(InputType::UNIPOLAR_20MA, 			"0-20mA", "0-20mA");
	Option inputType_Unipolar4_20ma = 		Option(InputType::UNIPOLAR_4_20MA, 			"4-20mA", "4-20mA");
	Option inputType_Unipolar4_20ma_NAMUR = Option(InputType::UNIPOLAR_4_20MA_NAMUR, 	"4-20mA NAMUR", "4-20mA_NAMUR");
	std::vector<Option*> inputTypeOptions = {
		&inputType_Bipolar10V,
		&inputType_Unipolar10V,
		&inputType_Bipolar20ma,
		&inputType_Unipolar20ma,
		&inputType_Unipolar4_20ma,
		&inputType_Unipolar4_20ma_NAMUR
	};
	
	enum FilterSetting{
		FIR_50HZ = 0,
		FIR_60HZ = 1,
		IIR_1 = 2,
		IIR_2 = 3,
		IIR_3 = 4,
		IIR_4 = 5,
		IIR_5 = 6,
		IIR_6 = 7,
		IIR_7 = 8,
		IIR_8 = 9
	};
	
	Option filter_FIR_50HZ = Option(FilterSetting::FIR_50HZ, "FIR 50HZ", "FIR_50HZ");
	Option filter_FIR_60HZ = Option(FilterSetting::FIR_60HZ, "FIR 60HZ", "FIR_60HZ");
	Option filter_IIR_1 = Option(FilterSetting::IIR_1, "IIR 1", "IIR_1");
	Option filter_IIR_2 = Option(FilterSetting::IIR_2, "IIR 2", "IIR_2");
	Option filter_IIR_3 = Option(FilterSetting::IIR_3, "IIR 3", "IIR_3");
	Option filter_IIR_4 = Option(FilterSetting::IIR_4, "IIR 4", "IIR_4");
	Option filter_IIR_5 = Option(FilterSetting::IIR_5, "IIR 5", "IIR_5");
	Option filter_IIR_6 = Option(FilterSetting::IIR_6, "IIR 6", "IIR_6");
	Option filter_IIR_7 = Option(FilterSetting::IIR_7, "IIR 7", "IIR_7");
	Option filter_IIR_8 = Option(FilterSetting::IIR_8, "IIR 8", "IIR_8");
	std::vector<Option*> filterOptions = {
		&filter_FIR_50HZ,
		&filter_FIR_60HZ,
		&filter_IIR_1,
		&filter_IIR_2,
		&filter_IIR_3,
		&filter_IIR_4,
		&filter_IIR_5,
		&filter_IIR_6,
		&filter_IIR_7,
		&filter_IIR_8
	};
	
	struct ChannelSettings{
		OptionParam inputType;
		BoolParam enableFilter;
		OptionParam filterSetting;
	};
	std::vector<ChannelSettings> channelSettings = std::vector<ChannelSettings>(8);
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
