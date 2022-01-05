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
	
	virtual void addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement) = 0;
	virtual void addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement) = 0;
	
	std::vector<std::shared_ptr<NodePin>> inputPins; //input pins are device outputs
	std::vector<std::shared_ptr<NodePin>> outputPins; //output pins are device inputs
	
	//gui stuff
	void moduleGui();
	void informationGui();
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
	bool outputs[4] = {false, false, false, false};
	uint8_t pdoSpacerBits = 0;
};


//=================================================================
//================== 021-1BF00 8x Digital Input ===================
//=================================================================

class VIPA_021_1BF00 : public VipaModule{
public:
	DEFINE_VIPA_MODULE(VIPA_021_1BF00, "VIPA 021-1BF00", "DI8x (DC24V)")
	bool inputs[8] = {false, false, false, false, false, false, false, false};
};

//=================================================================
//================== 022-1BF00 8x Digital Output ===================
//=================================================================

class VIPA_022_1BF00 : public VipaModule{
public:
	DEFINE_VIPA_MODULE(VIPA_022_1BF00, "VIPA 022-1BF00", "DO8x (DC24V0.5A)")
	bool outputs[8] = {false, false, false, false, false, false, false, false};
};

//=================================================================
//============== 050-1BS00 Single SSI Encoder Input ===============
//=================================================================


class VIPA_050_1BS00 : public VipaModule{
public:
	DEFINE_VIPA_MODULE(VIPA_050_1BS00, "VIPA 050-1BS00", "SSI RS422 (DC24V)")
	
	std::shared_ptr<NodePin> encoderPin = std::make_shared<NodePin>(NodeData::Type::POSITIONFEEDBACK_DEVICELINK, DataDirection::NODE_OUTPUT, "SSI Encoder");
	std::shared_ptr<NodePin> resetPin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Reset Encoder");
	
	uint32_t encoderValue;
	uint16_t time_microseconds;
	
	double encoderPosition_revolutions = 0.0;
	double encoderVelocity_revolutionsPerSecond = 0.0;
	
	uint16_t previousReadingTime_microseconds = 0.0;
	double previousEncoderPosition_revolutions = 0.0;
	
	//==== Subdevices ====
	
	void onSetParentBusCoupler(std::shared_ptr<VipaBusCoupler_053_1EC01> busCoupler);
	std::shared_ptr<PositionFeedbackDevice> encoderDevice = std::make_shared<PositionFeedbackDevice>("SSI Encoder", PositionUnit::Unit::REVOLUTION, PositionFeedback::Type::ABSOLUTE_FEEDBACK);
	void updateEncoderWorkingRange();
	void updateResetPinVisibility();
	bool b_isResetting = false;
	
	//==== Data Type ====
	
	struct PauseTime{
		enum class Microseconds{
			_1_MICROSECONDS,
			_2_MICROSECONDS,
			_4_MICROSECONDS,
			_8_MICROSECONDS,
			_16_MICROSECONDS,
			_32_MICROSECONDS,
			_48_MICROSECONDS,
			_64_MICROSECONDS
		};
		Microseconds microseconds;
		uint16_t valueSetting;
		const char saveName[64];
		const char displayName[64];
	};
	static std::vector<PauseTime> pauseTimes;
	static PauseTime* getPauseTime(const char* saveName);
	static PauseTime* getPauseTime(PauseTime::Microseconds microseconds);
	static PauseTime* getPauseTime(uint16_t valueSetting);
	
	struct TransmissionRate{
		enum class Frequency{
			_2_MEGAHERTZ,
			_1p5_MEGAHERTZ,
			_1_MEGAHERTZ,
			_500_KILOHERTZ,
			_250_KILOHERTZ,
			_125_KILOHERTZ
		};
		Frequency frequency;
		uint16_t valueSetting;
		const char saveName[64];
		const char displayName[64];
	};
	static std::vector<TransmissionRate> transmissionRates;
	static TransmissionRate* getTransmissionRate(const char* saveName);
	static TransmissionRate* getTransmissionRate(TransmissionRate::Frequency frequency);
	static TransmissionRate* getTransmissionRate(uint16_t valueSetting);
	
	static int maxNormalisationBits;
	static int minNormalisationBits;
	uint8_t normalisationBitCountToCanValue(int bitCount);
	int canValuetoNormalisationBitCount(uint8_t canValue);
	
	static int maxEncoderBits;
	static int minEncoderBits;
	uint8_t encoderBitCountToCanValue(int bitCount);
	int canValuetoEncoderBitCount(uint8_t canValue);
	
	struct BitShift{
		enum class Direction{
			LSB_FIRST,
			MSB_FIRST
		};
		Direction direction;
		bool bitValue;
		const char saveName[64];
		const char displayName[64];
	};
	static std::vector<BitShift> bitShifts;
	static BitShift* getBitShift(const char* saveName);
	static BitShift* getBitShift(BitShift::Direction direction);
	static BitShift* getBitShift(bool valueSetting);
	
	struct ClockEdge{
		enum class Edge{
			FALLING_EDGE,
			RISING_EDGE
		};
		Edge edge;
		bool bitValue;
		const char saveName[64];
		const char displayName[64];
	};
	static std::vector<ClockEdge> clockEdges;
	static ClockEdge* getClockEdge(const char* saveName);
	static ClockEdge* getClockEdge(ClockEdge::Edge direction);
	static ClockEdge* getClockEdge(bool valueSetting);
	
	struct Encoding{
		enum Format{
			BINARY,
			GRAY
		};
		Format format;
		bool bitValue;
		const char saveName[64];
		const char displayName[64];
	};
	static std::vector<Encoding> encodings;
	static Encoding* getEncoding(const char* saveName);
	static Encoding* getEncoding(Encoding::Format format);
	static Encoding* getEncoding(bool valueSetting);
	
	//==== Settings Data ====
	PauseTime::Microseconds pausetime = PauseTime::Microseconds::_32_MICROSECONDS;
	TransmissionRate::Frequency transmissionFrequency = TransmissionRate::Frequency::_500_KILOHERTZ;
	int normalisationBitCount = 0;
	int encoderBitCount = 24;
	int singleTurnBitCount = 12;
	BitShift::Direction bitshiftDirection = BitShift::Direction::MSB_FIRST;
	ClockEdge::Edge clockEdge = ClockEdge::Edge::RISING_EDGE;
	Encoding::Format encodingFormat = Encoding::Format::BINARY;
	bool b_centerRangeOnZero = false;
	bool b_hasResetSignal = false;
	
	//==== Gui Stuff ====
	virtual void moduleParameterGui();
	
	//==== Parameter Saving & Loading ====
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);
	
};


//=================================================================
//================= 032-1BD70 4x 12 Analog Output =================
//=================================================================


class VIPA_032_1BD70 : public VipaModule{
public:
	DEFINE_VIPA_MODULE(VIPA_032_1BD70, "VIPA 032-1BD70", "AO4x 12bit (±10V)")
	
	int16_t outputs[4] = { 0, 0, 0, 0};
	
	struct VoltageRange{
		enum class Range{
			ZERO_TO_10V,
			NEGATIVE_TO_POSITIVE_10V
		};
		Range range;
		uint8_t valueSetting;
		const char saveName[64];
		const char displayName[64];
	};
	
	static std::vector<VoltageRange> voltageRanges;
	static VoltageRange* getVoltageRange(const char* saveName);
	static VoltageRange* getVoltageRange(VoltageRange::Range range);
	static VoltageRange* getVoltageRange(uint8_t valueSetting);
	
	VoltageRange::Range voltageRangeSettings[4] = {
		VoltageRange::Range::NEGATIVE_TO_POSITIVE_10V,
		VoltageRange::Range::NEGATIVE_TO_POSITIVE_10V,
		VoltageRange::Range::NEGATIVE_TO_POSITIVE_10V,
		VoltageRange::Range::NEGATIVE_TO_POSITIVE_10V
	};
	
	bool shortCircuitDetectionSettings[4] = {false, false, false, false};
	 
	//==== Parameter Gui ====
	void moduleParameterGui();
	
	//==== Parameter Saving & Loading ====
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);
};
