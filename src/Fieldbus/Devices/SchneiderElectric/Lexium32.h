#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Utilities/ScrollingBuffer.h"

#include "Fieldbus/Utilities/DS402.h"

class Lexium32 : public EtherCatDevice {
public:

    DEFINE_ETHERCAT_DEVICE(Lexium32, "Lexium32M", "Lexium32M", "Schneider Electric", "Servo Drives", 0x800005A, 0x16440)

    //node input data
    std::shared_ptr<NodePin> digitalOut0 = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "DQ0");
    std::shared_ptr<NodePin> digitalOut1 = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "DQ1");
    std::shared_ptr<NodePin> digitalOut2 = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "DQ2");
	
	std::shared_ptr<bool> digitalOut0PinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalOut1PinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalOut2PinValue = std::make_shared<bool>(false);

    //node output data
	std::shared_ptr<ServoActuatorDevice> servoMotorDevice = std::make_shared<ServoActuatorDevice>("Servo", Units::AngularDistance::Revolution, PositionFeedbackType::ABSOLUTE);
	std::shared_ptr<GpioDevice> gpioDevice = std::make_shared<GpioDevice>("GPIO");
	std::shared_ptr<double> loadPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> positionPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> velocityPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<bool> digitalIn0PinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn1PinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn2PinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn3PinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn4PinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn5PinValue = std::make_shared<bool>(false);
	
    std::shared_ptr<NodePin> servoMotorLink = std::make_shared<NodePin>(servoMotorDevice, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Servo Motor");
    std::shared_ptr<NodePin> actualLoad = std::make_shared<NodePin>(loadPinValue, NodePin::Direction::NODE_OUTPUT, "Load", NodePin::Flags::DisableDataField | NodePin::Flags::HidePin);
    std::shared_ptr<NodePin> actualPosition = std::make_shared<NodePin>(positionPinValue, NodePin::Direction::NODE_OUTPUT, "Position", NodePin::Flags::DisableDataField | NodePin::Flags::HidePin);
    std::shared_ptr<NodePin> actualVelocity = std::make_shared<NodePin>(velocityPinValue, NodePin::Direction::NODE_OUTPUT, "Velocity", NodePin::Flags::DisableDataField | NodePin::Flags::HidePin);
    std::shared_ptr<NodePin> gpioDeviceLink = std::make_shared<NodePin>(gpioDevice, NodePin::Direction::NODE_OUTPUT, "GPIO");
    std::shared_ptr<NodePin> digitalIn0 = std::make_shared<NodePin>(digitalIn0PinValue, NodePin::Direction::NODE_OUTPUT, "DI0", NodePin::Flags::DisableDataField);
    std::shared_ptr<NodePin> digitalIn1 = std::make_shared<NodePin>(digitalIn1PinValue, NodePin::Direction::NODE_OUTPUT, "DI1", NodePin::Flags::DisableDataField);
    std::shared_ptr<NodePin> digitalIn2 = std::make_shared<NodePin>(digitalIn2PinValue, NodePin::Direction::NODE_OUTPUT, "DI2", NodePin::Flags::DisableDataField);
    std::shared_ptr<NodePin> digitalIn3 = std::make_shared<NodePin>(digitalIn3PinValue, NodePin::Direction::NODE_OUTPUT, "DI3", NodePin::Flags::DisableDataField);
    std::shared_ptr<NodePin> digitalIn4 = std::make_shared<NodePin>(digitalIn4PinValue, NodePin::Direction::NODE_OUTPUT, "DI4", NodePin::Flags::DisableDataField);
    std::shared_ptr<NodePin> digitalIn5 = std::make_shared<NodePin>(digitalIn5PinValue, NodePin::Direction::NODE_OUTPUT, "DI5", NodePin::Flags::DisableDataField);


	void resetData();
	
    //===== drive status =====


    //===== INTERNAL MOTION PROFILE GENERATOR =====

    double profileVelocity_rps = 0.0;
    double profilePosition_r = 0.0;
	double actualFollowingError_r = 0.0;
	
    //===== Manual Controls =====

    float manualVelocityCommand_rps = 0.0;
    float manualAcceleration_rpsps = 1.0;
    float defaultManualAcceleration_rpsps = 1.0;

    //===== Unit Settings =====

    const int positionUnitsPerRevolution = 131072;
    const int velocityUnitsPerRpm = 100;
    const int accelerationUnitsPerRpmps = 1;
    const int currentUnitsPerAmp = 100;

    //===== General Settings =====

    bool b_invertDirectionOfMotorMovement = false;

    double maxCurrent_amps = 0.0;
    
    float maxMotorVelocity_rps = 0.0;

    void uploadGeneralParameters();
    DataTransferState generalParameterUploadState = DataTransferState::NO_TRANSFER;
    void downloadGeneralParameters();
    DataTransferState generalParameterDownloadState = DataTransferState::NO_TRANSFER;

    //======= QuickStop Settings =======
    
    enum class QuickStopReaction {
		TORQUE_RAMP,
		DECELERATION_RAMP
    };

    QuickStopReaction quickstopReaction = QuickStopReaction::TORQUE_RAMP;
    double maxQuickstopCurrent_amps = 0.0;
    double quickStopDeceleration_revolutionsPerSecondSquared = 0.0;

    //===== GPIO settings ======

    enum class InputPin {
		DI0,
		DI1,
		DI2,
		DI3,
		DI4,
		DI5,
		NONE
    };
	
	uint8_t getInputPinSubindex(InputPin pin){
		switch(pin){
			case InputPin::DI0: return 0x1;
			case InputPin::DI1: return 0x2;
			case InputPin::DI2: return 0x3;
			case InputPin::DI3: return 0x4;
			case InputPin::DI4: return 0x5;
			case InputPin::DI5: return 0x6;
			default: return 0x0;
		}
	}

    InputPin negativeLimitSwitchPin = InputPin::NONE;
    bool b_negativeLimitSwitchNormallyClosed = false;
    InputPin positiveLimitSwitchPin = InputPin::NONE;
    bool b_positiveLimitSwitchNormallyClosed = false;

    bool b_invertDI0 = false;
    bool b_invertDI1 = false;
    bool b_invertDI2 = false;
    bool b_invertDI3 = false;
    bool b_invertDI4 = false;
    bool b_invertDI5 = false;

    void uploadPinAssignements();
    DataTransferState pinAssignementUploadState = DataTransferState::NO_TRANSFER;
    void downloadPinAssignements();
    DataTransferState pinAssignementDownloadState = DataTransferState::NO_TRANSFER;

    //===== Encoder Settings =====

	
    enum class EncoderAssignement {
		INTERNAL_ENCODER,
		ENCODER_MODULE
    };
	uint16_t getEncoderAssignementValue(EncoderAssignement assignement){
		switch(assignement){
			case EncoderAssignement::INTERNAL_ENCODER: return 0;
			case EncoderAssignement::ENCODER_MODULE: return 1;
		}
	}
	EncoderAssignement getEncoderAssignement(uint16_t val){
		switch(val){
			case 0: return EncoderAssignement::INTERNAL_ENCODER;
			case 1: return EncoderAssignement::ENCODER_MODULE;
		}
	}

	
    enum class EncoderModule {
		ANALOG_MODULE,
		DIGITAL_MODULE,
		RESOLVER_MODULE,
		NONE
    };
	uint16_t getEncoderModuleValue(EncoderModule module){
		switch(module){
			case Lexium32::EncoderModule::ANALOG_MODULE: return 769;
			case Lexium32::EncoderModule::DIGITAL_MODULE: return 770;
			case Lexium32::EncoderModule::RESOLVER_MODULE: return 771;
			case Lexium32::EncoderModule::NONE: return 0;
		}
	}
	EncoderModule getEncoderModule(uint16_t val){
		switch(val){
			case 769: return EncoderModule::ANALOG_MODULE;
			case 770: return EncoderModule::DIGITAL_MODULE;
			case 771: return EncoderModule::RESOLVER_MODULE;
			case 0: return EncoderModule::NONE;
			default: return EncoderModule::NONE;
		}
	}
	


    enum class EncoderType {
		NONE,
		SSI_ROTARY
    };
	uint16_t getEncoderTypeValue(EncoderType type){
		switch(type){
			case EncoderType::NONE: return 0;
			case EncoderType::SSI_ROTARY: return 10;
		}
	}
	EncoderType getEncoderType(uint16_t val){
		switch(val){
			case 0: return EncoderType::NONE;
			case 10: return EncoderType::SSI_ROTARY;
		}
	}
	

    enum class EncoderCoding {
		BINARY,
		GRAY
    };
	uint16_t getEncoderEncodingValue(EncoderCoding coding){
		switch(coding){
			case EncoderCoding::BINARY: return 0;
			case EncoderCoding::GRAY: return 1;
		}
	}
	EncoderCoding getEncoderCoding(uint16_t val){
		switch(val){
			case 0: return EncoderCoding::BINARY;
			case 1: return EncoderCoding::GRAY;
		}
	}
    
	enum class EncoderVoltage {
		V5,
		V12
    };
	uint16_t getEncoderVoltageValue(EncoderVoltage voltage){
		switch(voltage){
			case EncoderVoltage::V5: return 5;
			case EncoderVoltage::V12: return 12;
		}
	}
	EncoderVoltage getEncoderVoltage(uint16_t id){
		switch(id){
			case 5: return EncoderVoltage::V5;
			case 12: return EncoderVoltage::V12;
		}
	}

    EncoderAssignement encoderAssignement = EncoderAssignement::INTERNAL_ENCODER;
    EncoderModule encoderModuleType = EncoderModule::NONE;
    EncoderType encoderType = EncoderType::NONE;
    EncoderCoding encoderCoding = EncoderCoding::BINARY;
    EncoderVoltage encoderVoltage = EncoderVoltage::V12;

    //internal encoder settings are constant
    const int encoder1_singleTurnResolutionBits = 17;
    const int encoder1_multiTurnResolutionBits = 12;

    int encoder2_singleTurnResolutionBits = 17;
    int encoder2_multiTurnResolutionBits = 12;
    int encoder2_EncoderToMotorRatioNumerator = 1;   //integer amount of encoder revolutions per ->
    int encoder2_EncoderToMotorRatioDenominator = 1;   //-> per integer amount of motor revolutions
    bool encoder2_invertDirection = false;
    double encoder2_maxDifferenceToMotorEncoder_rotations = 0.5;
    bool b_encoderRangeShifted = false;

    void detectEncoderModule();
    void uploadEncoderSettings();
    DataTransferState encoderSettingsUploadState = DataTransferState::NO_TRANSFER;
    void downloadEncoderSettings();
    DataTransferState encoderSettingsDownloadState = DataTransferState::NO_TRANSFER;

    float manualAbsoluteEncoderPosition_revolutions = 0.0;
    void uploadManualAbsoluteEncoderPosition();
    DataTransferState encoderAbsolutePositionUploadState = DataTransferState::NO_TRANSFER;

    void getEncoderWorkingRange(float& low, float& high);

    //display variable for the new encoder position setting
    double newEncoderPosition = 0.0;

    //====== AUTO TUNING ========

    std::thread autoTuningHandler;
    void startAutoTuning();
    void stopAutoTuning();
    bool isAutoTuning() { return b_isAutoTuning; }
    bool b_isAutoTuning = false;
    bool b_autoTuningSucceeded = false;
    float tuningProgress = 0.0;
    float tuning_frictionTorque_amperes = 0.0;
    float tuning_constantLoadTorque_amperes = 0.0;
    float tuning_momentOfInertia_kilogramcentimeter2 = 0.0;
    DataTransferState autoTuningSaveState = DataTransferState::NO_TRANSFER;

    //======== Other Settings =========

    bool saveToEEPROM();

    void factoryReset();
    DataTransferState factoryResetTransferState = DataTransferState::NO_TRANSFER;

    void setStationAlias(uint16_t a);
    DataTransferState stationAliasUploadState = DataTransferState::NO_TRANSFER;


private:

	DS402::PowerState actualPowerState = DS402::PowerState::UNKNOWN;
	DS402::OperatingMode actualOperatingMode = DS402::OperatingMode::UNKNOWN;
	
	DS402::PowerState requestedPowerState = DS402::PowerState::UNKNOWN;
	DS402::OperatingMode requestedOperatingMode = DS402::OperatingMode::UNKNOWN;
	
	//Rx-PDO
	DS402::Control ds402Control;
    int32_t PPp_target = 0;
    uint16_t IO_DQ_set = 0;

	//Tx-PDO
	DS402::Status ds402Status;
    int32_t _p_act = 0;
	int32_t _p_dif_usr = 0;
    int32_t _v_act = 0;
    uint16_t _I_act = 0;
    uint16_t _LastError = 0;
    uint16_t _IO_act = 0;
    uint16_t _IO_STO_act = 0;
	
	bool b_hasFault = false;
	bool b_isResettingFault = false;
	
	
	static std::string getErrorCodeString(uint16_t errorCode){
		switch(errorCode){
			case 0x0: return "No Error";
			case 0xA320: return "A320 : Max Following Error Exceeded";
			case 0xB122: return "B122 : Communication Synchronization Error";
			case 0xB610: return "B610 : Fieldbus Watchdog";
			case 0xB103: return "B103 : Fieldbus Communication Closed";
			case 0X733F: return "733F : Encoder Connection Error";
			default: {
				static char hexString[16];
				sprintf(hexString, "%X", errorCode);
				std::string output = std::string(hexString) + " (Unknown Error Code)";
				return output;
			}
		}
	}
	
	/*
	bool b_resetEncoder = false;
	bool b_resettingEncoder = false;
	
	bool b_homingMode = false;
	bool b_sendHomingMode = false;
	bool b_doHoming = false;
	
	bool b_homingCompleted = false;
	bool b_homingSuccessful = false;
	*/
	 
	bool b_startHoming = false;
	bool b_isHoming = false;
	
    //Lexium GUI functions
    void statusGui();
    void controlsGui();
    void generalSettingsGui();
    void gpioGui();
    void encoderGui();
    void tuningGui();
    void miscellaneousGui();
};


#define QuickstopRectionTypeString \
	{Lexium32::QuickStopReaction::TORQUE_RAMP, "Torque Ramp", "TorqueRamp"},\
	{Lexium32::QuickStopReaction::DECELERATION_RAMP, "Deceleration Ramp", "DecelerationRamp"}\

DEFINE_ENUMERATOR(Lexium32::QuickStopReaction, QuickstopRectionTypeString)


#define InputPinStrings \
	{Lexium32::InputPin::DI0, "Digital Input 0", "DI0"},\
	{Lexium32::InputPin::DI1, "Digital Input 1", "DI1"},\
	{Lexium32::InputPin::DI2, "Digital Input 2", "DI2"},\
	{Lexium32::InputPin::DI3, "Digital Input 3", "DI3"},\
	{Lexium32::InputPin::DI4, "Digital Input 4", "DI4"},\
	{Lexium32::InputPin::DI5, "Digital Input 5", "DI5"},\
	{Lexium32::InputPin::NONE, "Unassigned", "Unassigned"}\

DEFINE_ENUMERATOR(Lexium32::InputPin, InputPinStrings)


#define EncoderAssignementTypeStrings \
	{Lexium32::EncoderAssignement::INTERNAL_ENCODER, "Internal Motor Encoder", "Internal"},\
	{Lexium32::EncoderAssignement::ENCODER_MODULE, "Encoder Module", "Module"}\

DEFINE_ENUMERATOR(Lexium32::EncoderAssignement, EncoderAssignementTypeStrings)


#define EncoderModuleTypeStrings \
	{Lexium32::EncoderModule::ANALOG_MODULE, "Analog Encoder Module", "Analog"},\
	{Lexium32::EncoderModule::DIGITAL_MODULE, "Digital Encoder Module", "Digital"},\
	{Lexium32::EncoderModule::RESOLVER_MODULE, "Resolver Encoder Module", "Resolver"},\
	{Lexium32::EncoderModule::NONE, "No Encoder Module", "None"}\

DEFINE_ENUMERATOR(Lexium32::EncoderModule, EncoderModuleTypeStrings)


#define EncoderTypeStrings \
	{Lexium32::EncoderType::NONE, "None", "None"},\
	{Lexium32::EncoderType::SSI_ROTARY, "SSI Absolute Rotary", "SSIRotary"}\

DEFINE_ENUMERATOR(Lexium32::EncoderType, EncoderTypeStrings)


#define EncoderCodingTypeStrings \
	{Lexium32::EncoderCoding::BINARY, "Binary", "Binary"},\
	{Lexium32::EncoderCoding::GRAY, "Gray", "Gray"}\

DEFINE_ENUMERATOR(Lexium32::EncoderCoding, EncoderCodingTypeStrings)


#define EncoderVoltageTypeStrings \
	{Lexium32::EncoderVoltage::V5, "5V", "5V"},\
	{Lexium32::EncoderVoltage::V12, "12V", "12V"}\

DEFINE_ENUMERATOR(Lexium32::EncoderVoltage, EncoderVoltageTypeStrings)


