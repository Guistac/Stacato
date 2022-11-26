#pragma once

#include "Fieldbus/Utilities/DeviceModule.h"

#include "Motion/SubDevice.h"
#include "Fieldbus/Utilities/EtherCatPDO.h"

#include "Environnement/NodeGraph/NodePin.h"
#include "Project/Editor/Parameter.h"


class IL_EC_BK_BusCoupler;
namespace tinyxml2{ struct XMLElement; }



namespace PhoenixContact::ModuleFactory{
	std::vector<EtherCAT::DeviceModule*>& getModules();
}




//=================================================================
//======================= 4x Digital Input ========================
//=================================================================

namespace PhoenixContact{

class IB_IL_24_DI_4 : public EtherCAT::DeviceModule{
public:
	DEFINE_DEVICE_MODULE(IB_IL_24_DI_4, "IB IL 24 DI 4-PAC", "Digital Input x4 (24V)", 0x41BE)
	
	//pdo data
	uint8_t inputByte;
	
	//pin values
	std::vector<std::shared_ptr<bool>> outputPinValues;
	
	//parameters
	bool invertInputs[4] = {false, false, false, false};
};

}

//=================================================================
//======================= 4x Digital Output =======================
//=================================================================

namespace PhoenixContact{

class IB_IL_24_DO_4 : public EtherCAT::DeviceModule{
public:
	DEFINE_DEVICE_MODULE(IB_IL_24_DO_4, "IB IL 24 DO 4-PAC", "Digital Output x4 (24V 500mA)", 0x41BD)
	
	//pdo data
	uint8_t outputByte;
	
	//pin values
	std::vector<std::shared_ptr<bool>> inputPinValues;
	
	//parameters
	bool invertOutputs[4] = {false, false, false, false};
};

}

//=================================================================
//============== 050-1BS00 Single SSI Encoder Input ===============
//=================================================================


namespace PhoenixContact::SSI{

	enum class Parity{
		NONE,
		EVEN,
		ODD
	};

	enum class Baudrate{
		KHz_100,
		KHz_200,
		KHz_400,
		KHz_800,
		MHz_1
	};

	enum class Code{
		BINARY,
		GRAY
	};

};

#define ParityTypeStrings \
	{PhoenixContact::SSI::Parity::NONE, 	"No Parity",	"None"},\
	{PhoenixContact::SSI::Parity::EVEN, 	"Even",			"Even"},\
	{PhoenixContact::SSI::Parity::ODD, 		"Odd",			"Odd"}\

DEFINE_ENUMERATOR(PhoenixContact::SSI::Parity, ParityTypeStrings)

#define BaudrateTypeStrings \
	{PhoenixContact::SSI::Baudrate::KHz_100, 	"100 KHz",	"100KHz"},\
	{PhoenixContact::SSI::Baudrate::KHz_200, 	"200 KHz",	"200KHz"},\
	{PhoenixContact::SSI::Baudrate::KHz_400, 	"400 KHz", 	"400KHz"},\
	{PhoenixContact::SSI::Baudrate::KHz_800, 	"800 KHz", 	"800KHz"},\
	{PhoenixContact::SSI::Baudrate::MHz_1, 		"1 MHz", 	"1MHz"}\

DEFINE_ENUMERATOR(PhoenixContact::SSI::Baudrate, BaudrateTypeStrings);

#define CodeTypeStrings \
	{PhoenixContact::SSI::Code::BINARY, "Binary",	"Binary"},\
	{PhoenixContact::SSI::Code::GRAY, 	"Gray",		"Gray"}\

DEFINE_ENUMERATOR(PhoenixContact::SSI::Code, CodeTypeStrings)


namespace PhoenixContact{

class IB_IL_SSI_IN : public EtherCAT::DeviceModule{
public:
	DEFINE_DEVICE_MODULE(IB_IL_SSI_IN, "IB IL SSI-IN-PAC", "SSI input", 0x25F)
	
	//————— Node Pins ———————
	std::shared_ptr<NodePin> encoderPin = std::make_shared<NodePin>(NodePin::DataType::POSITION_FEEDBACK, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "SSI Encoder");
	std::shared_ptr<NodePin> resetPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Reset Encoder");
	
	std::shared_ptr<bool> resetPinValue = std::make_shared<bool>(false);
	
	//————— PDO DATA —————
	
	uint32_t encoderData;
	uint32_t controlData;
	
	double encoderPosition_revolutions = 0.0;
	double encoderVelocity_revolutionsPerSecond = 0.0;
	
	uint16_t previousReadingTime_microseconds = 0.0;
	double previousEncoderPosition_revolutions = 0.0;
	
	uint64_t resetStartTime_nanoseconds = 0;
	double resetStartTime_seconds = 0.0;
	
	
	
	enum ControlCode{
		NO_ACTION 			= 0x0,
		READ_POSITION 		= 0x1,
		OFFLINE 			= 0x2,
		ACKNOWLEDGE_FAULT 	= 0x21
	};
	
	enum Parity{
		NONE	= 0x0,
		ODD		= 0x1,
		EVEN	= 0x2
	};
	
	
	enum Rev{
		OFF	= 0x0,
		ON	= 0x1
	};
	
	const int minResolution = 8;
	const int maxResolution = 25;
	int resolution = 25;
	uint8_t resolutionCode;
	
	uint8_t getResolutionCode(int res){
		return std::clamp(res, minResolution, maxResolution) - 7;
	};
	
	enum Speed{
		KHz_100 = 0x1,
		KHz_200 = 0x2,
		KHz_400 = 0x3,
		KHz_800 = 0x4,
		MHz_1	= 0x5
	};
	
	enum Code{
		BINARY 	= 0x0,
		GRAY	= 0x1
	};
	
	std::shared_ptr<NumberParameter<int>> resolutionParameter = NumberParameter<int>::make(25,
																						   "Resolution", "Resolution", "%i",
																						   Units::Data::Bit,
																						   false);
	
	std::shared_ptr<NumberParameter<int>> singleturnResolutionParameter = NumberParameter<int>::make(12,
																									"Singleturn Resolution", "SingleturnResolution", "%i",
																									 Units::Data::Bit, false);
	
	std::shared_ptr<EnumeratorParameter<SSI::Parity>> parityParameter = EnumeratorParameter<SSI::Parity>::make(SSI::Parity::NONE, "Parity", "Parity");
	
	std::shared_ptr<BooleanParameter> invertDirectionParameter = BooleanParameter::make(false, "Invert Direction", "Invert");
	
	std::shared_ptr<EnumeratorParameter<SSI::Baudrate>> baudrateParameter = EnumeratorParameter<SSI::Baudrate>::make(SSI::Baudrate::KHz_100, "Baudrate", "Baudrate");
	
	std::shared_ptr<EnumeratorParameter<SSI::Code>> codeParameter = EnumeratorParameter<SSI::Code>::make(SSI::Code::GRAY, "Code", "Code");
	
	std::shared_ptr<BooleanParameter> centerWorkingRangeOnZeroParameter = BooleanParameter::make(true, "Center working range on zero", "CenterWorkingRangeOnZero");
	
	//————— SubDevice ——————
	
	class SsiEncoder : public PositionFeedbackDevice{
	public:
		SsiEncoder(std::shared_ptr<IB_IL_SSI_IN> module) :
		MotionDevice(Units::AngularDistance::Revolution),
		PositionFeedbackDevice(Units::AngularDistance::Revolution, PositionFeedbackType::ABSOLUTE),
		encoderModule(module){}
		
		virtual std::string getName() override {
			//return std::string(encoderModule->parentBusCoupler->getName()) + " SSI Encoder";
		};
		
		virtual std::string getStatusString() override { return ""; }
		
		virtual bool canHardReset() override {
			//return encoderModule->b_hasResetSignal && encoderModule->resetPin->isConnected();
		}
		virtual void executeHardReset() override {
			b_doHardReset = true;
			b_hardResetBusy = true;
		}
		virtual bool isExecutingHardReset() override { return b_hardResetBusy; }
						
		std::shared_ptr<IB_IL_SSI_IN> encoderModule;
		bool b_doHardReset = false;
		bool b_hardResetBusy = false;
	};
	
	std::shared_ptr<SsiEncoder> encoder;
	
	//virtual void onSetParentDevice(std::shared_ptr<EtherCAT::ModularDevice> busCoupler) override;
	void updateEncoderWorkingRange();
	void updateResetPinVisibility();
	
	//virtual void onDisconnection() override;
};


};
