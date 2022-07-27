#pragma once

#define DEFINE_VIPA_MODULE(className, saveName, displayName) public:\
	virtual void onConstruction();\
	virtual std::shared_ptr<VipaModule> getInstance(){\
		auto newModule = std::make_shared<className>();\
		newModule->onConstruction();\
		return newModule;\
	}\
	virtual const char* getSaveName(){ return saveName; }\
	virtual const char* getDisplayName() { return displayName; }\
	virtual void onSetIndex(int i);\
	virtual void addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement);\
	virtual void addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement);\
	virtual bool configureParameters();\
	virtual void readInputs();\
	virtual void writeOutputs();\

#include "Motion/SubDevice.h"
#include "VIPA-053-1EC01.h"
#include "Fieldbus/Utilities/EtherCatPDO.h"

#include "Project/Editor/Parameter.h"

class NodePin;
class VipaBusCoupler_053_1EC01;
namespace tinyxml2{ struct XMLElement; }

class VipaModule : public std::enable_shared_from_this<VipaModule>{
public:

	virtual void onConstruction() = 0;
	virtual std::shared_ptr<VipaModule> getInstance() = 0;

	std::shared_ptr<VipaBusCoupler_053_1EC01> parentBusCoupler = nullptr;
	void setParentBusCoupler(std::shared_ptr<VipaBusCoupler_053_1EC01> c){
		parentBusCoupler = c;
		onSetParentBusCoupler(c);
	}
	virtual void onSetParentBusCoupler(std::shared_ptr<VipaBusCoupler_053_1EC01> c) {}
	
	int moduleIndex = -1;
	void setIndex(int i){
		moduleIndex = i;
		onSetIndex(i);
	}
	virtual void onSetIndex(int i) = 0;
	
	//module name as reported by the bus coupler (also savename for xml)
	virtual const char* getSaveName() = 0;
	//module name for gui display
	virtual const char* getDisplayName() = 0;
	
	//configures parameters and PDO mappings
	virtual bool configureParameters() = 0;
	
	//reads data from input data and assigns it to pins
	virtual void readInputs() = 0;
	int inputByteCount = 0;
	int inputBitCount = 0;
	
	//reads data from pins and output data
	virtual void writeOutputs() = 0;
	int outputByteCount = 0;
	int outputBitCount = 0;
	
	virtual void onConnection(){}
	virtual void onDisconnection(){}
	
	virtual void addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement) = 0;
	virtual void addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement) = 0;
	
	std::vector<std::shared_ptr<NodePin>> inputPins; //input pins are device outputs
	std::vector<std::shared_ptr<NodePin>> outputPins; //output pins are device inputs
	
	//gui stuff
	void moduleGui();
	virtual void moduleParameterGui();
	
	//saving and loading
	virtual bool save(tinyxml2::XMLElement* xml){}
	virtual bool load(tinyxml2::XMLElement* xml){}
};


namespace VipaModuleFactory{
	std::vector<VipaModule*>& getModules();
	std::shared_ptr<VipaModule> getModule(const char* saveName);
};


//=================================================================
//================== 022-1HD10 4x Relais Output ===================
//=================================================================

class VIPA_022_1HD10 : public VipaModule{
public:
	DEFINE_VIPA_MODULE(VIPA_022_1HD10, "VIPA 022-1HD10", "DO4x Relais (1.8A)")
	
	//pdo data
	bool outputs[4] = {false, false, false, false};
	uint8_t pdoSpacerBits = 0;
	
	//pin values
	std::vector<std::shared_ptr<bool>> inputPinValues;
	
	//parameters
	bool invertOutputs[4] = {false, false, false, false};
	
	virtual void moduleParameterGui();
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);
};


//=================================================================
//================== 021-1BF00 8x Digital Input ===================
//=================================================================

class VIPA_021_1BF00 : public VipaModule{
public:
	DEFINE_VIPA_MODULE(VIPA_021_1BF00, "VIPA 021-1BF00", "DI8x (DC24V)")
	
	//pdo data
	bool inputs[8] = {false, false, false, false, false, false, false, false};
	
	//pin values
	std::vector<std::shared_ptr<bool>> outputPinValues;
	
	//parameters
	bool invertInputs[8] = {false, false, false, false, false, false, false};
	
	virtual void moduleParameterGui();
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);
};

//=================================================================
//================== 022-1BF00 8x Digital Output ===================
//=================================================================

class VIPA_022_1BF00 : public VipaModule{
public:
	DEFINE_VIPA_MODULE(VIPA_022_1BF00, "VIPA 022-1BF00", "DO8x (DC24V0.5A)")
	
	//pdo data
	bool outputs[8] = {false, false, false, false, false, false, false, false};
	
	//pin values
	std::vector<std::shared_ptr<bool>> inputPinValues;
	
	//parameters
	bool invertOutputs[8] = {false, false, false, false, false, false, false, false};
	
	virtual void moduleParameterGui();
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);
};

//=================================================================
//============== 050-1BS00 Single SSI Encoder Input ===============
//=================================================================


class VIPA_050_1BS00 : public VipaModule{
public:
	DEFINE_VIPA_MODULE(VIPA_050_1BS00, "VIPA 050-1BS00", "SSI RS422 (DC24V)")
	
	//————— Node Pins ———————
	std::shared_ptr<NodePin> encoderPin = std::make_shared<NodePin>(NodePin::DataType::POSITION_FEEDBACK, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "SSI Encoder");
	std::shared_ptr<NodePin> resetPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Reset Encoder");
	
	std::shared_ptr<bool> resetPinValue = std::make_shared<bool>(false);
	
	//————— PDO DATA —————
	uint32_t encoderValue;
	uint16_t time_microseconds;
	
	double encoderPosition_revolutions = 0.0;
	double encoderVelocity_revolutionsPerSecond = 0.0;
	
	uint16_t previousReadingTime_microseconds = 0.0;
	double previousEncoderPosition_revolutions = 0.0;
	
	uint64_t resetStartTime_nanoseconds = 0;
	double resetStartTime_seconds = 0.0;
	
	//————— SubDevice ——————
	
	class SsiEncoder : public PositionFeedbackDevice{
	public:
		SsiEncoder(std::shared_ptr<VIPA_050_1BS00> module) :
		MotionDevice(name, Units::AngularDistance::Revolution),
		PositionFeedbackDevice("Vipa Module SSI Encoder", Units::AngularDistance::Revolution, PositionFeedbackType::ABSOLUTE),
		encoderModule(module){}
		
		virtual std::string getStatusString() override { return ""; }
		
		virtual bool canHardReset() override { return encoderModule->b_hasResetSignal && encoderModule->resetPin->isConnected(); }
		virtual void executeHardReset() override {
			b_doHardReset = true;
			b_hardResetBusy = true;
		}
		virtual bool isExecutingHardReset() override { return b_hardResetBusy; }
						
		std::shared_ptr<VIPA_050_1BS00> encoderModule;
		bool b_doHardReset = false;
		bool b_hardResetBusy = false;
	};
	
	std::shared_ptr<SsiEncoder> encoder;
	
	void onSetParentBusCoupler(std::shared_ptr<VipaBusCoupler_053_1EC01> busCoupler);
	void updateEncoderWorkingRange();
	void updateResetPinVisibility();
	
	//==== Setting Types ====
	
	enum class MeasurementPauseTime{
		_1_MICROSECONDS,
		_2_MICROSECONDS,
		_4_MICROSECONDS,
		_8_MICROSECONDS,
		_16_MICROSECONDS,
		_32_MICROSECONDS,
		_48_MICROSECONDS,
		_64_MICROSECONDS
	};
	static uint16_t getMeasurementPauseTimeValue(MeasurementPauseTime time){
		switch(time){
			case MeasurementPauseTime::_1_MICROSECONDS: return 0x0030;
			case MeasurementPauseTime::_2_MICROSECONDS: return 0x0060;
			case MeasurementPauseTime::_4_MICROSECONDS: return 0x00C0;
			case MeasurementPauseTime::_8_MICROSECONDS: return 0x0180;
			case MeasurementPauseTime::_16_MICROSECONDS: return 0x0300;
			case MeasurementPauseTime::_32_MICROSECONDS: return 0x0600;
			case MeasurementPauseTime::_48_MICROSECONDS: return 0x0900;
			case MeasurementPauseTime::_64_MICROSECONDS: return 0x0C00;
		}
	}
	
	enum class TransmissionRate{
		_2_MEGAHERTZ,
		_1p5_MEGAHERTZ,
		_1_MEGAHERTZ,
		_500_KILOHERTZ,
		_250_KILOHERTZ,
		_125_KILOHERTZ
	};
	static uint16_t getTransmissionRateValue(TransmissionRate rate){
		switch(rate){
			case TransmissionRate::_2_MEGAHERTZ: return 0x0018;
			case TransmissionRate::_1p5_MEGAHERTZ: return 0x0020;
			case TransmissionRate::_1_MEGAHERTZ: return 0x0030;
			case TransmissionRate::_500_KILOHERTZ: return 0x0060;
			case TransmissionRate::_250_KILOHERTZ: return 0x00C0;
			case TransmissionRate::_125_KILOHERTZ: return 0x0180;
		}
	}
	
	constexpr static int maxNormalisationBits = 15;
	constexpr static int minNormalisationBits = 0;
	static uint8_t normalisationBitCountToCanValue(int bitCount){
		uint8_t output = std::max(minNormalisationBits, bitCount);
		output = std::min(maxNormalisationBits, bitCount);
		return output;
	}
	static int canValuetoNormalisationBitCount(uint8_t canValue){ return canValue; }
	
	constexpr static int maxEncoderBits = 32;
	constexpr static int minEncoderBits = 8;
	static uint8_t encoderBitCountToCanValue(int bitCount){
		bitCount = std::max(minEncoderBits, bitCount);
		bitCount = std::min(maxEncoderBits, bitCount);
		uint8_t output = bitCount - 1;
		return output;
	}
	static int canValuetoEncoderBitCount(uint8_t canValue){ return canValue + 1; }
	
	enum class BitDirection{
		LSB_FIRST,
		MSB_FIRST
	};
	static bool getBitDirectionValue(BitDirection dir){
		switch(dir){
			case BitDirection::LSB_FIRST: return false;
			case BitDirection::MSB_FIRST: return true;
		}
	}
	
	enum class ClockEdge{
		FALLING_EDGE,
		RISING_EDGE
	};
	inline bool getClockEdgeValue(ClockEdge edge){
		switch(edge){
			case ClockEdge::FALLING_EDGE: return false;
			case ClockEdge::RISING_EDGE: return false;
		}
	}
	
	enum class Encoding{
		BINARY,
		GRAY
	};
	inline bool getEncodingValue(Encoding cod){
		switch(cod){
			case Encoding::BINARY: return false;
			case Encoding::GRAY: return true;
		}
	}
	
	//==== Settings Data ====
	MeasurementPauseTime pausetime = MeasurementPauseTime::_32_MICROSECONDS;
	TransmissionRate transmissionFrequency = TransmissionRate::_500_KILOHERTZ;
	int normalisationBitCount = 0;
	int encoderBitCount = 24;
	int singleTurnBitCount = 12;
	BitDirection bitshiftDirection = BitDirection::MSB_FIRST;
	ClockEdge clockEdge = ClockEdge::RISING_EDGE;
	Encoding encodingFormat = Encoding::BINARY;
	bool b_centerRangeOnZero = false;
	bool b_hasResetSignal = false;
	float resetTime_milliseconds = 1.0;
	
	//==== Gui Stuff ====
	virtual void moduleParameterGui();
	
	//==== Parameter Saving & Loading ====
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);
	
};

#define VipaSSIPauseTimeTypeStrings \
	{VIPA_050_1BS00::MeasurementPauseTime::_1_MICROSECONDS, "1 µs", 	"1"},\
	{VIPA_050_1BS00::MeasurementPauseTime::_2_MICROSECONDS, "2 µs", 	"2"},\
	{VIPA_050_1BS00::MeasurementPauseTime::_4_MICROSECONDS, "4 µs", 	"4"},\
	{VIPA_050_1BS00::MeasurementPauseTime::_8_MICROSECONDS, "8 µs", 	"8"},\
	{VIPA_050_1BS00::MeasurementPauseTime::_16_MICROSECONDS, "16 µs", 	"16"},\
	{VIPA_050_1BS00::MeasurementPauseTime::_32_MICROSECONDS, "32 µs", 	"32"},\
	{VIPA_050_1BS00::MeasurementPauseTime::_48_MICROSECONDS, "48 µs", 	"48"},\
	{VIPA_050_1BS00::MeasurementPauseTime::_64_MICROSECONDS, "64 µs", 	"64"}\

DEFINE_ENUMERATOR(VIPA_050_1BS00::MeasurementPauseTime, VipaSSIPauseTimeTypeStrings)

#define VipaSSITransmissionRateTypeStrings \
	{VIPA_050_1BS00::TransmissionRate::_2_MEGAHERTZ,  	"2.0 Mhz", 		"2Mhz"},\
	{VIPA_050_1BS00::TransmissionRate::_1p5_MEGAHERTZ,  "1.5 Mhz", 		"1.5Mhz"},\
	{VIPA_050_1BS00::TransmissionRate::_1_MEGAHERTZ,  	"1.0 Mhz", 		"1.0Mhz"},\
	{VIPA_050_1BS00::TransmissionRate::_500_KILOHERTZ,  "500.0 Khz",	"500Khz"},\
	{VIPA_050_1BS00::TransmissionRate::_250_KILOHERTZ,  "250.0 Khz", 	"250Khz"},\
	{VIPA_050_1BS00::TransmissionRate::_125_KILOHERTZ,  "125.0 Khz", 	"125Khz"}\

DEFINE_ENUMERATOR(VIPA_050_1BS00::TransmissionRate, VipaSSITransmissionRateTypeStrings)

#define VipaSSIBitShiftTypeStrings \
	{VIPA_050_1BS00::BitDirection::LSB_FIRST, "LSB First", "LSBFirst"},\
	{VIPA_050_1BS00::BitDirection::MSB_FIRST,"MSB First (default)", "MSBFirst"}\

DEFINE_ENUMERATOR(VIPA_050_1BS00::BitDirection, VipaSSIBitShiftTypeStrings)

#define VipaSSIClockEdgeTypeStrings \
	{VIPA_050_1BS00::ClockEdge::FALLING_EDGE, "Falling Edge", "FallingEdge"},\
	{VIPA_050_1BS00::ClockEdge::RISING_EDGE, "Rising Edge (default)", "RisingEdge"},\

DEFINE_ENUMERATOR(VIPA_050_1BS00::ClockEdge, VipaSSIClockEdgeTypeStrings)


#define VipaSSIEncodingTypeStrings \
	{VIPA_050_1BS00::Encoding::BINARY, "Binary", "Binary"},\
	{VIPA_050_1BS00::Encoding::GRAY, "Gray", "Gray"}\

DEFINE_ENUMERATOR(VIPA_050_1BS00::Encoding, VipaSSIEncodingTypeStrings)



//=================================================================
//================= 032-1BD70 4x 12 Analog Output =================
//=================================================================


class VIPA_032_1BD70 : public VipaModule{
public:
	DEFINE_VIPA_MODULE(VIPA_032_1BD70, "VIPA 032-1BD70", "AO4x 12bit (±10V)")
	
	//pdo data
	int16_t outputs[4] = { 0, 0, 0, 0};
	
	//pin values
	std::vector<std::shared_ptr<double>> inputPinValues;
	
	//parameters
	enum class VoltageRange{
		ZERO_TO_10V,
		NEGATIVE_TO_POSITIVE_10V
	};
	static uint8_t getVoltageRangeValue(VoltageRange range);
	
	VoltageRange voltageRangeSettings[4] = {
		VoltageRange::NEGATIVE_TO_POSITIVE_10V,
		VoltageRange::NEGATIVE_TO_POSITIVE_10V,
		VoltageRange::NEGATIVE_TO_POSITIVE_10V,
		VoltageRange::NEGATIVE_TO_POSITIVE_10V
	};
	
	bool shortCircuitDetectionSettings[4] = {false, false, false, false};
	 
	//==== Parameter Gui ====
	void moduleParameterGui();
	
	//==== Parameter Saving & Loading ====
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);
};

#define VipaAnalogVoltageRangeTypeString \
	{VIPA_032_1BD70::VoltageRange::ZERO_TO_10V, 				"0-10V", 			"ZeroToPositive10V"},\
	{VIPA_032_1BD70::VoltageRange::NEGATIVE_TO_POSITIVE_10V, 	"\xc2\xb1 10V", 	"NegativeToPositive10V"}\

DEFINE_ENUMERATOR(VIPA_032_1BD70::VoltageRange, VipaAnalogVoltageRangeTypeString)

inline uint8_t VIPA_032_1BD70::getVoltageRangeValue(VoltageRange range){
	switch(range){
		case VoltageRange::ZERO_TO_10V: return 0x20;
		case VoltageRange::NEGATIVE_TO_POSITIVE_10V: return 0x22;
	}
}



//=================================================================
//============ 050-1BB40 Two Channel Frequency Counter ============
//=================================================================


class VIPA_050_1BB40 : public VipaModule{
public:
	DEFINE_VIPA_MODULE(VIPA_050_1BB40, "VIPA 050-1BB40", "2x Frequency Counter")
	
	//————— Node Pins ———————
	std::shared_ptr<double> frequency0Value = std::make_shared<double>(0.0);
	std::shared_ptr<double> frequency1Value = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> frequency0Pin = std::make_shared<NodePin>(frequency0Value, NodePin::Direction::NODE_OUTPUT, "Frequency 0");
	std::shared_ptr<NodePin> frequency1Pin = std::make_shared<NodePin>(frequency1Value, NodePin::Direction::NODE_OUTPUT, "Frequency 1");
	
	//————— PDO DATA —————
	
	//txpdo (inputs)
	uint32_t fm_period_ch0;
	uint32_t fm_rising_edges_ch0;
	uint32_t fm_period_ch1;
	uint32_t fm_rising_edges_ch1;
	uint16_t fm_status_ch0;
	uint16_t fm_status_ch1;
	
	//rxpdo (outputs)
	uint32_t fm_preset_period_ch0;
	uint32_t fm_preset_period_ch1;
	uint16_t fm_control_ch0;
	uint16_t fm_control_ch1;
	
	//bool ch0Control = false;
	//bool ch1Control = false;
	
	enum class InputFilter{
		KHZ_600,
		KHZ_250,
		KHZ_100,
		KHZ_60,
		KHZ_30,
		KHZ_10,
		KHZ_5,
		KHZ_2,
		KHZ_1
	};
	
	uint8_t getInputFilterCode(InputFilter filter){
		switch(filter){
			case InputFilter::KHZ_600: return 0x0;
			case InputFilter::KHZ_250: return 0x1;
			case InputFilter::KHZ_100: return 0x2;
			case InputFilter::KHZ_60: return 0x3;
			case InputFilter::KHZ_30: return 0x4;
			case InputFilter::KHZ_10: return 0x6;
			case InputFilter::KHZ_5: return 0x7;
			case InputFilter::KHZ_2: return 0x8;
			case InputFilter::KHZ_1: return 0x9;
		}
	}
	
	std::shared_ptr<EnumeratorParameter<InputFilter>> channel0InputFilter = std::make_shared<EnumeratorParameter<InputFilter>>(InputFilter::KHZ_600,
																															   "Channel 0 Input Filter",
																															   "Ch0InputFilter");
	std::shared_ptr<EnumeratorParameter<InputFilter>> channel1InputFilter = std::make_shared<EnumeratorParameter<InputFilter>>(InputFilter::KHZ_600,
																															   "Channel 1 Input Filter",
																															   "Ch1InputFilter");
	std::shared_ptr<NumberParameter<double>> channel0MeasurementPeriod;
	std::shared_ptr<NumberParameter<double>> channel1MeasurementPeriod;
	
	
	//==== Gui Stuff ====
	virtual void moduleParameterGui();
	
	//==== Parameter Saving & Loading ====
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);
	
};

#define VipaFrequencyInputFilterStrings \
{VIPA_050_1BB40::InputFilter::KHZ_600, 	"600 KHz", 	"600KHz"},\
{VIPA_050_1BB40::InputFilter::KHZ_250, 	"250 KHz", 	"250KHz"},\
{VIPA_050_1BB40::InputFilter::KHZ_100, 	"100 KHz", 	"100KHz"},\
{VIPA_050_1BB40::InputFilter::KHZ_60, 	"60 KHz", 	"60KHz"},\
{VIPA_050_1BB40::InputFilter::KHZ_30, 	"30 KHz", 	"30KHz"},\
{VIPA_050_1BB40::InputFilter::KHZ_10, 	"10 KHz", 	"10KHz"},\
{VIPA_050_1BB40::InputFilter::KHZ_5, 	"5 KHz", 	"5KHz"},\
{VIPA_050_1BB40::InputFilter::KHZ_2, 	"2 KHz",	"2KHz"},\
{VIPA_050_1BB40::InputFilter::KHZ_1, 	"1 KHz", 	"1KHz"}\

DEFINE_ENUMERATOR(VIPA_050_1BB40::InputFilter, VipaFrequencyInputFilterStrings)
