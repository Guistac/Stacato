#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Utilities/ScrollingBuffer.h"

#include "Fieldbus/Utilities/DS402.h"

#include "Gui/Assets/Colors.h"

class Lexium32 : public EtherCatDevice {
public:
	
    DEFINE_ETHERCAT_DEVICE(Lexium32, "Lexium32M", "Lexium32M", "Schneider Electric", "Servo Drives", 0x800005A, 0x16440)

	class LexiumServoMotor : public ServoActuatorDevice{
	public:
		
		LexiumServoMotor(std::shared_ptr<Lexium32> lexium32) : drive(lexium32){}
		
		virtual MotionState getState() override {
			
		};
		virtual std::string getName() override { return "Servo Motor"; }
		virtual bool hasFault() override { return drive->b_hasFault; }
		virtual std::string getStatusString() override { return drive->getStatusString(); }
		
		virtual Unit getPositionUnit() override { return Units::AngularDistance::Revolution; }
		virtual PositionFeedbackType getPositionFeedbackType() override { return PositionFeedbackType::ABSOLUTE; }
		
		virtual bool isInsideWorkingRange() override { return drive->position >= drive->minWorkingRange && drive->position <= drive->maxWorkingRange; }
		virtual double getPositionInWorkingRange() override { return (drive->position - drive->minWorkingRange) / (drive->maxWorkingRange - drive->minWorkingRange); }
		virtual double getMinPosition() override { return drive->minWorkingRange; }
		virtual double getMaxPosition() override { return drive->maxWorkingRange; }
		
		virtual double getPosition() override { return drive->position - drive->positionOffset; }
		virtual double getVelocity() override { return drive->velocity; }
		virtual bool isMoving() override { return drive->b_isMoving; }
		virtual double getLoad() override { return drive->load; }
		
		virtual bool canHardReset() override { return true; }
		virtual void executeHardReset() override { return b_hardReset; }
		virtual bool isExecutingHardReset() override { return b_hardResetBusy; }
		
		//virtual bool canHardOverride() override { return false; }
		//virtual void executeHardOverride(double overridePosition) override {}
		//virtual bool isExecutingHardOverride() override { return false; }
		
		virtual void softOverridePosition(double position) override {
			//UNIMPLEMENTED
		}
		
		virtual void enable() override { b_enable = true; }
		virtual void disable() override { b_disable = true; }
		virtual void quickstop() override { b_quickstop = true; }

		virtual void setVelocityCommand(double velocityCommand, double accelerationCommand) override { /*UNIMPLEMENTED*/ }
		
		virtual double getVelocityLimit() override { return drive->velocityLimit; }
		virtual double getAccelerationLimit() override { return drive->accelerationLimit; }

		//virtual bool hasManualHoldingBrake() { return false; }
		//virtual bool isHoldingBrakeApplied() { return false; }
		//virtual void releaseHoldingBrake() {}
		//virtual void applyHoldingBrake() {}
		
		
		virtual void setPositionCommand(double positionCommand, double velocityCommand, double accelerationCommand) override {
			drive->targetPosition = positionCommand;
			drive->targetVelocity = velocityCommand;
			drive->targetAcceleration = accelerationCommand;
		};
		
		virtual double getFollowingError() override { return drive->followingError; }
		virtual double getMaxFollowingError() override { return drive->maxFollowingError; }
		virtual double getFollowingErrorInRange() override { return std::abs(drive->followingError / drive->maxFollowingError); }
		
		std::shared_ptr<Lexium32> drive;
		bool b_enable = false;
		bool b_disable = false;
		bool b_quickstop = false;
		bool b_hardReset = false;
		bool b_hardResetBusy = false;
		MotionState state = MotionState::OFFLINE;
	};
	
	class LexiumGpio : public GpioDevice{
	public:
		LexiumGpio(std::shared_ptr<Lexium32> lexium32) : drive(lexium32){}
		virtual MotionState getState() override { return state; }
		virtual std::string getName() override { return "Lexium32 Gpio"; }
		virtual bool hasFault() override { return drive->b_hasFault; }
		virtual std::string getStatusString() override { return ""; }
		std::shared_ptr<Lexium32> drive;
		MotionState state = MotionState::OFFLINE;
	};
	
	
    //node input data
    std::shared_ptr<NodePin> digitalOut0Pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "DQ0");
    std::shared_ptr<NodePin> digitalOut1Pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "DQ1");
    std::shared_ptr<NodePin> digitalOut2Pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "DQ2");
	
	std::shared_ptr<bool> digitalOut0Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalOut1Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalOut2Value = std::make_shared<bool>(false);

    //node output data
	std::shared_ptr<LexiumServoMotor> servoMotor;
	std::shared_ptr<double> actualLoadValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> actualPositionValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> actualVelocityValue = std::make_shared<double>(0.0);
	
	std::shared_ptr<LexiumGpio> gpioDevice;
	std::shared_ptr<bool> digitalIn0Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn1Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn2Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn3Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn4Value = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn5Value = std::make_shared<bool>(false);
	
    std::shared_ptr<NodePin> servoMotorPin = std::make_shared<NodePin>(NodePin::DataType::SERVO_ACTUATOR,
																	   NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL,
																	   "Servo Motor");
    std::shared_ptr<NodePin> actualLoadPin = std::make_shared<NodePin>(actualLoadValue,
																	   NodePin::Direction::NODE_OUTPUT,
																	   "Load",
																	   NodePin::Flags::DisableDataField | NodePin::Flags::HidePin);
    std::shared_ptr<NodePin> actualPositionPin = std::make_shared<NodePin>(actualPositionValue,
																		   NodePin::Direction::NODE_OUTPUT,
																		   "Position",
																		   NodePin::Flags::DisableDataField | NodePin::Flags::HidePin);
    std::shared_ptr<NodePin> actualVelocityPin = std::make_shared<NodePin>(actualVelocityValue,
																		   NodePin::Direction::NODE_OUTPUT,
																		   "Velocity",
																		   NodePin::Flags::DisableDataField | NodePin::Flags::HidePin);
	
    std::shared_ptr<NodePin> gpioDevicePin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_OUTPUT, "GPIO");
    std::shared_ptr<NodePin> digitalIn0Pin = std::make_shared<NodePin>(digitalIn0Value, NodePin::Direction::NODE_OUTPUT, "DI0", NodePin::Flags::DisableDataField);
    std::shared_ptr<NodePin> digitalIn1Pin = std::make_shared<NodePin>(digitalIn1Value, NodePin::Direction::NODE_OUTPUT, "DI1", NodePin::Flags::DisableDataField);
    std::shared_ptr<NodePin> digitalIn2Pin = std::make_shared<NodePin>(digitalIn2Value, NodePin::Direction::NODE_OUTPUT, "DI2", NodePin::Flags::DisableDataField);
    std::shared_ptr<NodePin> digitalIn3Pin = std::make_shared<NodePin>(digitalIn3Value, NodePin::Direction::NODE_OUTPUT, "DI3", NodePin::Flags::DisableDataField);
    std::shared_ptr<NodePin> digitalIn4Pin = std::make_shared<NodePin>(digitalIn4Value, NodePin::Direction::NODE_OUTPUT, "DI4", NodePin::Flags::DisableDataField);
    std::shared_ptr<NodePin> digitalIn5Pin = std::make_shared<NodePin>(digitalIn5Value, NodePin::Direction::NODE_OUTPUT, "DI5", NodePin::Flags::DisableDataField);


	void resetData();
	
	//————————— PROCESS DATA ———————————
	
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
	uint16_t _actionStatus = 0;
	uint16_t _IO_act = 0;
	uint16_t _IO_STO_act = 0;
	
	long long enableRequestTime_nanoseconds;
	long long enableRequestTimeout_nanoseconds = 250'000'000; //250ms enable timeout
	
	bool b_hasFault = false;
	bool b_isResettingFault = false;
	bool b_faultNeedsRestart = false;
	bool b_motorVoltagePresent = false;
	bool b_stoActive = false;
	
	bool b_startHoming = false;
	bool b_isHoming = false;
	bool b_isMoving = false;
	
	//all units are in revolutions and seconds
	
	double position;
	double velocity;
	double load;
	double followingError;
	
	double positionOffset = 0.0;
	
	void updateEncoderWorkingRange();
	double minWorkingRange;
	double maxWorkingRange;
	double velocityLimit;
	double accelerationLimit;
	
	double targetPosition;
	double targetVelocity;
	double targetAcceleration;

	static std::string getErrorCodeString(uint16_t errorCode){
		switch(errorCode){
			case 0x0: return "No Error";
			case 0xA320: return "A320 : Max Following Error Exceeded";
			case 0xB122: return "B122 : Cyclic communication: Incorrect synchronization";
			case 0xB610: return "B610 : Fieldbus Watchdog";
			case 0xB103: return "B103 : Fieldbus Communication Closed";
			case 0x733F: return "733F : Amplitude of encoder analog signals too low";
			case 0x5200: return "5200 : Error detected at connection to motor encoder";
			case 0x1300: return "1300 : Safety Function STO was Enabled";
			case 0x1301: return "1301 : STO_A and STO_B different level";
			case 0x3100: return "3100 : Missing mains supply, undervoltage mains supply or overvoltage mains supply";
			default: {
				static char hexString[16];
				sprintf(hexString, "%X", errorCode);
				std::string output = std::string(hexString) + " (Unknown Error Code)";
				return output;
			}
		}
	}
	
	std::string getStatusString(){
		if(!isConnected()) return "Device Offline";
		else if(!b_motorVoltagePresent) return "Motor voltage is not present, check power connections.";
		else if(b_hasFault){
			if(b_faultNeedsRestart) return getErrorCodeString(_LastError) + "\nDrive Restart needed to reset fault.";
			else return getErrorCodeString(_LastError) + "\nFault will be cleared when enabling.";
		}else if(b_stoActive) return "STO Active";
		else if(actualPowerState == DS402::PowerState::NOT_READY_TO_SWITCH_ON) return "Drive Restart Needed.";
		else return "No Issues";
	}
	
	std::string getShortStatusString(){
		if(!isConnected()) return "Device Offline";
		else if(!b_motorVoltagePresent) return "No Motor Voltage";
		else if(b_stoActive) return "STO Active";
		else if(b_hasFault) {
			static char hexFaultCodeString[16];
			sprintf(hexFaultCodeString, "%X", _LastError);
			if(b_faultNeedsRestart) return "Critical Fault : " + std::string(hexFaultCodeString);
			else return "Fault : " + std::string(hexFaultCodeString);
		}
		else if(actualPowerState == DS402::PowerState::NOT_READY_TO_SWITCH_ON) return "Restart Needed";
		else return Enumerator::getDisplayString(actualPowerState);
	}
	 
	glm::vec4 getStatusColor(){
		if(!isConnected()) return Colors::blue;
		else if(!b_motorVoltagePresent) return Colors::red;
		else if(b_hasFault) {
			if(b_faultNeedsRestart) return Colors::red;
			else return Colors::orange;
		}
		else if(!servoMotor->isReady()) return Colors::red;
		else if(servoMotor->isEnabled()) return Colors::green;
		else return Colors::yellow;
	}
	
	//———————————————————————————————————————
	
public:


    //===== INTERNAL MOTION PROFILE GENERATOR =====

    double profileVelocity_rps = 0.0;
    double profilePosition_r = 0.0;
	
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
    
    double maxMotorVelocity = 0.0;
	double maxFollowingError = 0.0;

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


