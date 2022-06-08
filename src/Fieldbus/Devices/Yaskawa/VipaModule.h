#pragma once

#define DEFINE_VIPA_MODULE(className, saveName, displayName) public:\
	className() { onConstruction(); }\
	virtual void onConstruction();\
	virtual std::shared_ptr<VipaModule> getInstance(){ return std::make_shared<className>();}\
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

class NodePin;
class VipaBusCoupler_053_1EC01;
namespace tinyxml2{ struct XMLElement; }

class VipaModule {
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
	
	//output pins
	std::shared_ptr<NodePin> encoderPin = std::make_shared<NodePin>(NodePin::DataType::POSITIONFEEDBACK, NodePin::Direction::NODE_OUTPUT, "SSI Encoder");
	std::shared_ptr<NodePin> resetPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Reset Encoder");
	
	std::shared_ptr<bool> resetPinValue = std::make_shared<bool>(false);
	
	virtual void onConnection();
	virtual void onDisconnection();
	
	uint32_t encoderValue;
	uint16_t time_microseconds;
	
	double encoderPosition_revolutions = 0.0;
	double encoderVelocity_revolutionsPerSecond = 0.0;
	
	uint16_t previousReadingTime_microseconds = 0.0;
	double previousEncoderPosition_revolutions = 0.0;
	
	//==== Subdevices ====
	
	void onSetParentBusCoupler(std::shared_ptr<VipaBusCoupler_053_1EC01> busCoupler);
	std::shared_ptr<PositionFeedbackDevice> encoderDevice = std::make_shared<PositionFeedbackDevice>("SSI Encoder", Units::AngularDistance::Revolution, PositionFeedbackType::ABSOLUTE);
	void updateEncoderWorkingRange();
	void updateResetPinVisibility();
	uint64_t resetStartTime_nanoseconds = 0;
	double resetStartTime_seconds = 0.0;
	
	//==== Data Type ====
	
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
	static uint16_t getMeasurementPauseTimeValue(MeasurementPauseTime time);
	
	enum class TransmissionRate{
		_2_MEGAHERTZ,
		_1p5_MEGAHERTZ,
		_1_MEGAHERTZ,
		_500_KILOHERTZ,
		_250_KILOHERTZ,
		_125_KILOHERTZ
	};
	static uint16_t getTransmissionRateValue(TransmissionRate rate);
	
	static int maxNormalisationBits;
	static int minNormalisationBits;
	uint8_t normalisationBitCountToCanValue(int bitCount);
	int canValuetoNormalisationBitCount(uint8_t canValue);
	
	static int maxEncoderBits;
	static int minEncoderBits;
	uint8_t encoderBitCountToCanValue(int bitCount);
	int canValuetoEncoderBitCount(uint8_t canValue);
	
	enum class BitDirection{
		LSB_FIRST,
		MSB_FIRST
	};
	static bool getBitDirectionValue(BitDirection dir);
	
	enum class ClockEdge{
		FALLING_EDGE,
		RISING_EDGE
	};
	static bool getClockEdgeValue(ClockEdge edge);
	
	enum class Encoding{
		BINARY,
		GRAY
	};
	static bool getEncodingValue(Encoding cod);
	
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

inline uint16_t VIPA_050_1BS00::getMeasurementPauseTimeValue(MeasurementPauseTime time){
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


#define VipaSSITransmissionRateTypeStrings \
	{VIPA_050_1BS00::TransmissionRate::_2_MEGAHERTZ,  	"2.0 Mhz", 		"2Mhz"},\
	{VIPA_050_1BS00::TransmissionRate::_1p5_MEGAHERTZ,  "1.5 Mhz", 		"1.5Mhz"},\
	{VIPA_050_1BS00::TransmissionRate::_1_MEGAHERTZ,  	"1.0 Mhz", 		"1.0Mhz"},\
	{VIPA_050_1BS00::TransmissionRate::_500_KILOHERTZ,  "500.0 Khz",	"500Khz"},\
	{VIPA_050_1BS00::TransmissionRate::_250_KILOHERTZ,  "250.0 Khz", 	"250Khz"},\
	{VIPA_050_1BS00::TransmissionRate::_125_KILOHERTZ,  "125.0 Khz", 	"125Khz"}\

DEFINE_ENUMERATOR(VIPA_050_1BS00::TransmissionRate, VipaSSITransmissionRateTypeStrings)

inline uint16_t VIPA_050_1BS00::getTransmissionRateValue(TransmissionRate rate){
	switch(rate){
		case TransmissionRate::_2_MEGAHERTZ: return 0x0018;
		case TransmissionRate::_1p5_MEGAHERTZ: return 0x0020;
		case TransmissionRate::_1_MEGAHERTZ: return 0x0030;
		case TransmissionRate::_500_KILOHERTZ: return 0x0060;
		case TransmissionRate::_250_KILOHERTZ: return 0x00C0;
		case TransmissionRate::_125_KILOHERTZ: return 0x0180;
	}
}


#define VipaSSIBitShiftTypeStrings \
	{VIPA_050_1BS00::BitDirection::LSB_FIRST, "LSB First", "LSBFirst"},\
	{VIPA_050_1BS00::BitDirection::MSB_FIRST,"MSB First (default)", "MSBFirst"}\

DEFINE_ENUMERATOR(VIPA_050_1BS00::BitDirection, VipaSSIBitShiftTypeStrings)

inline bool VIPA_050_1BS00::getBitDirectionValue(BitDirection dir){
	switch(dir){
		case BitDirection::LSB_FIRST: return false;
		case BitDirection::MSB_FIRST: return true;
	}
}


#define VipaSSIClockEdgeTypeStrings \
	{VIPA_050_1BS00::ClockEdge::FALLING_EDGE, "Falling Edge", "FallingEdge"},\
	{VIPA_050_1BS00::ClockEdge::RISING_EDGE, "Rising Edge (default)", "RisingEdge"},\

DEFINE_ENUMERATOR(VIPA_050_1BS00::ClockEdge, VipaSSIClockEdgeTypeStrings)

inline bool VIPA_050_1BS00::getClockEdgeValue(ClockEdge edge){
	switch(edge){
		case ClockEdge::FALLING_EDGE: return false;
		case ClockEdge::RISING_EDGE: return false;
	}
}


#define VipaSSIEncodingTypeStrings \
	{VIPA_050_1BS00::Encoding::BINARY, "Binary", "Binary"},\
	{VIPA_050_1BS00::Encoding::GRAY, "Gray", "Gray"}\

DEFINE_ENUMERATOR(VIPA_050_1BS00::Encoding, VipaSSIEncodingTypeStrings)

inline bool VIPA_050_1BS00::getEncodingValue(Encoding cod){
	switch(cod){
		case Encoding::BINARY: return false;
		case Encoding::GRAY: return true;
	}
}



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
