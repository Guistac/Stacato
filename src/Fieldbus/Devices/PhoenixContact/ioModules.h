#pragma once

#include "Fieldbus/Utilities/ModularDeviceProfile.h"

#include "Motion/Interfaces.h"
#include "Fieldbus/Utilities/EtherCatPDO.h"

#include "Environnement/NodeGraph/NodePin.h"

#include "Legato/Editor/Parameters.h"


class IL_EC_BK_BusCoupler;
namespace tinyxml2{ struct XMLElement; }



namespace PhoenixContact::ModuleFactory{
	std::vector<std::shared_ptr<EtherCAT::ModularDeviceProfile::DeviceModule>>& getModules();
}




//=================================================================
//======================= 4x Digital Input ========================
//=================================================================

namespace PhoenixContact{

class IB_IL_24_DI_4 : public EtherCAT::ModularDeviceProfile::DeviceModule{
public:
	DEFINE_MODULAR_DEVICE_MODULE(IB_IL_24_DI_4, "IB IL 24 DI 4", 0x41BE)
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{}
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	
	//pdo data
	uint8_t inputByte;
	
	//pin values
	std::vector<std::shared_ptr<bool>> outputPinValues;
	
	//parameters
	std::vector<Legato::BoolParam> inversionParameters;
};

}

//=================================================================
//======================= 4x Digital Output =======================
//=================================================================

namespace PhoenixContact{

class IB_IL_24_DO_4 : public EtherCAT::ModularDeviceProfile::DeviceModule{
public:
	DEFINE_MODULAR_DEVICE_MODULE(IB_IL_24_DO_4, "IB IL 24 DO 4", 0x41BD)
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{}
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	
	//pdo data
	uint8_t outputByte;
	
	//pin values
	std::vector<std::shared_ptr<bool>> inputPinValues;
	
	//parameters
	std::vector<Legato::BoolParam> inversionParameters;
};

}

//=======================================================
//================== Relais Output x2 ===================
//=======================================================

namespace PhoenixContact{

class IB_IL_24_48_DOR_2 : public EtherCAT::ModularDeviceProfile::DeviceModule{
public:
	DEFINE_MODULAR_DEVICE_MODULE(IB_IL_24_48_DOR_2, "IB IL 24/48 DOR 2", 0x123)
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{}
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	
	//pdo data
	uint8_t outputByte;
	
	//pin value
	std::vector<std::shared_ptr<bool>> inputPinValues;
	
	//parameters
	std::vector<Legato::BoolParam> inversionParameters;
};

}

//=======================================================
//============== Single SSI Encoder Input ===============
//=======================================================


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

	enum ControlCode{
		NO_ACTION = 0x0,
		READ_POSITION = 0x1,
		OFFLINE = 0x2,
		ACKNOWLEDGE_FAULT = 0x21
	};

	enum class StatusCode{
		UNKNOWN,
		OFFLINE,
		OPERATION,
		ACKNOWLEDGE_FAULT,
		FAULT_ENCODER_SUPPLY_NOT_PRESENT_OR_SHORT_CIRCUIT,
		FAULT_PARITY_ERROR,
		FAULT_INVALID_CONFIGURATION_DATA,
		FAULT_INVALID_CONTROL_CODE
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

class IB_IL_SSI_IN : public EtherCAT::ModularDeviceProfile::DeviceModule{
public:
	DEFINE_MODULAR_DEVICE_MODULE(IB_IL_SSI_IN, "IB IL SSI-IN", 0x25F)
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{}
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	
	//————— Node Pins ———————
	std::shared_ptr<bool> resetPinValue = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> encoderPin;
	std::shared_ptr<NodePin> resetPin;
	
	//————— Process Data —————
	
	//--- rx/tx-PDO
	uint32_t encoderData;
	uint32_t controlData;
	
	//--- cyclic data
	SSI::StatusCode statusCode = SSI::StatusCode::UNKNOWN;
	
	uint64_t previousReadingTime_nanoseconds = 0;
	double previousPosition_revolutions = 0.0;
	uint32_t rawPositionData = 0x0;
	double positionBeforeOffset = 0.0;
	double positionOffset = 0.0;
	
	uint64_t resetStartTime_nanoseconds = 0;
	
	//————— User Parameters ——————
	
	Legato::NumberParam<int> resolutionParameter;
	Legato::NumberParam<int> singleturnResolutionParameter;
	Legato::BoolParam invertDirectionParameter;
	
	enum Parity{
		NONE = 0x0,
		EVEN = 0x1,
		ODD = 0x2
	};
	Legato::Option option_parityNone = Legato::Option(Parity::NONE, "No Parity", "None");
	Legato::Option option_parityEven = Legato::Option(Parity::EVEN, "Even", "Even");
	Legato::Option option_parityOdd = Legato::Option(Parity::ODD, "Odd", "Odd");
	std::vector<Legato::Option*> options_parity = {
		&option_parityNone,
		&option_parityEven,
		&option_parityOdd
	};
	Legato::OptionParam parityParameter;

	enum Baudrate{
		KHz_100 = 0x1,
		KHz_200 = 0x2,
		KHz_400 = 0x3,
		KHz_800 = 0x4,
		MHz_1 = 0x5
	};
	Legato::Option option_baudrate100KHz = Legato::Option(Baudrate::KHz_100, "100 Khz", "100KHz");
	Legato::Option option_baudrate200KHz = Legato::Option(Baudrate::KHz_200, "200 Khz", "200KHz");
	Legato::Option option_baudrate400KHz = Legato::Option(Baudrate::KHz_400, "400 Khz", "400KHz");
	Legato::Option option_baudrate800KHz = Legato::Option(Baudrate::KHz_800, "800 Khz", "800KHz");
	Legato::Option option_baudrate1Mhz = Legato::Option(Baudrate::MHz_1, "1 MHz", "1MHz");
	std::vector<Legato::Option*> options_baudrate = {
		&option_baudrate100KHz,
		&option_baudrate200KHz,
		&option_baudrate400KHz,
		&option_baudrate800KHz,
		&option_baudrate1Mhz
	};
	Legato::OptionParam baudrateParameter;

	enum Code{
		BINARY = 0x0,
		GRAY = 0x1
	};
	Legato::Option option_codeBinary = Legato::Option(Code::BINARY, "Binary", "Binary");
	Legato::Option option_codeGray = Legato::Option(Code::GRAY, "Gray", "Gray");
	std::vector<Legato::Option*> options_code = {
		&option_codeBinary,
		&option_codeGray
	};
	Legato::OptionParam codeParameter;
	
	Legato::BoolParam centerWorkingRangeOnZeroParameter;
	Legato::BoolParam hasResetSignalParameter;
	Legato::NumberParam<double> resetSignalTimeParameter;
	
	//————— SubDevice ——————
	
	class SsiEncoder : public MotionFeedbackInterface{
	public:
		SsiEncoder(std::shared_ptr<IB_IL_SSI_IN> module) : encoderModule(module){}
		
		virtual std::string getName() override {
			return std::string(encoderModule->parentDevice->getName()) + " SSI Encoder";
		};
		
		virtual std::string getStatusString() override {
			if(state == DeviceState::OFFLINE) {
				std::string message = "Encoder is Offline: parent device " + std::string(encoderModule->parentDevice->getName()) + " is Offline";
				return message;
			}
			/*
			 else if(!encoderModule->parentDevice->isStateOperational()){
				std::string message = "Enoder is Not Ready : parent device " + std::string(encoderModule->parentDevice->getName()) + " is not in operational state";
				return message;
			}
			 */
			switch(encoderModule->statusCode){
				case SSI::StatusCode::UNKNOWN:
					return "Encoder is not ready : unknown state";
				case SSI::StatusCode::OFFLINE:
					return "Encoder is offline";
				case SSI::StatusCode::OPERATION:
					return "Encoder is operating normally";
				case SSI::StatusCode::ACKNOWLEDGE_FAULT:
					return "Encoder is not ready : acknowledging fault";
				case SSI::StatusCode::FAULT_ENCODER_SUPPLY_NOT_PRESENT_OR_SHORT_CIRCUIT:
					return "Encoder is not ready : no power supply or short circuit present";
				case SSI::StatusCode::FAULT_PARITY_ERROR:
					return "Encoder is not ready : parity error";
				case SSI::StatusCode::FAULT_INVALID_CONFIGURATION_DATA:
					return "Encoder is not ready : invalid configuration data";
				case SSI::StatusCode::FAULT_INVALID_CONTROL_CODE:
					return "Encoder is not ready : invalid control code";
			}
		}
						
		std::shared_ptr<IB_IL_SSI_IN> encoderModule;
	};
	
	std::shared_ptr<SsiEncoder> encoder;
	
	void updateEncoderWorkingRange();
	
	virtual void onDisconnection() override {
		encoder->state = DeviceState::OFFLINE;
	}
};


};
