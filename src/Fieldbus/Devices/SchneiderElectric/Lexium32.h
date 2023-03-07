#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Utilities/ScrollingBuffer.h"

#include "Fieldbus/Utilities/DS402.h"

#include "Gui/Assets/Colors.h"

class Lexium32 : public EtherCatDevice {
public:
	
    DEFINE_ETHERCAT_DEVICE(Lexium32, "Lexium32M", "Lexium32M", "Schneider Electric", "Servo Drives", 0x800005A, 0x16440)

	class LexiumServoMotor : public ActuatorInterface{
	public:
		LexiumServoMotor(std::shared_ptr<Lexium32> lexium32) : drive(lexium32){}
		std::shared_ptr<Lexium32> drive;
		
		virtual std::string getName() override { return std::string(drive->getName()) + " Servo Motor"; };
		virtual std::string getStatusString() override { return drive->getStatusString(); }
		
		double positionOffset_revolutions = 0.0;
	};
	
	class LexiumGpio : public GpioInterface{
	public:
		LexiumGpio(std::shared_ptr<Lexium32> lexium32) : drive(lexium32){}
		std::shared_ptr<Lexium32> drive;
		virtual std::string getName() override { return std::string(drive->getName()) + " GPIO"; }
		virtual std::string getStatusString() override { return drive->getStatusString(); }
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
	
    std::shared_ptr<NodePin> servoMotorPin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR_INTERFACE,
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
	
    std::shared_ptr<NodePin> gpioDevicePin = std::make_shared<NodePin>(NodePin::DataType::GPIO_INTERFACE, NodePin::Direction::NODE_OUTPUT, "GPIO");
    std::shared_ptr<NodePin> digitalIn0Pin = std::make_shared<NodePin>(digitalIn0Value, NodePin::Direction::NODE_OUTPUT, "DI0", NodePin::Flags::DisableDataField);
    std::shared_ptr<NodePin> digitalIn1Pin = std::make_shared<NodePin>(digitalIn1Value, NodePin::Direction::NODE_OUTPUT, "DI1", NodePin::Flags::DisableDataField);
    std::shared_ptr<NodePin> digitalIn2Pin = std::make_shared<NodePin>(digitalIn2Value, NodePin::Direction::NODE_OUTPUT, "DI2", NodePin::Flags::DisableDataField);
    std::shared_ptr<NodePin> digitalIn3Pin = std::make_shared<NodePin>(digitalIn3Value, NodePin::Direction::NODE_OUTPUT, "DI3", NodePin::Flags::DisableDataField);
    std::shared_ptr<NodePin> digitalIn4Pin = std::make_shared<NodePin>(digitalIn4Value, NodePin::Direction::NODE_OUTPUT, "DI4", NodePin::Flags::DisableDataField);
    std::shared_ptr<NodePin> digitalIn5Pin = std::make_shared<NodePin>(digitalIn5Value, NodePin::Direction::NODE_OUTPUT, "DI5", NodePin::Flags::DisableDataField);
	
	double cycleTime_milliseconds = 20.0;
	
	//————————— PROCESS DATA ———————————
	
private:

	DS402::PowerState actualPowerState = DS402::PowerState::UNKNOWN;
	DS402::OperatingMode actualOperatingMode = DS402::OperatingMode::UNKNOWN;
	
	DS402::PowerState requestedPowerState = DS402::PowerState::UNKNOWN;
	DS402::OperatingMode requestedOperatingMode = DS402::OperatingMode::UNKNOWN;
	
	//Rx-PDO
	DS402::Control ds402Control;
	int32_t PPp_target = 0;
	int32_t PVv_target = 0;
	int16_t PTtq_target = 0;
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
	uint16_t BRK_release = 0;
	
	long long enableRequestTime_nanoseconds;
	long long enableRequestTimeout_nanoseconds = 250'000'000; //250ms enable timeout
	
	bool b_hasFault = false;
	bool b_isResettingFault = false;
	bool b_faultNeedsRestart = false;
	bool b_motorVoltagePresent = false;
	
	//homing mode commands
	bool b_startHoming = false;
	bool b_isHoming = false;

	void updateEncoderWorkingRange();
	
	static std::string getErrorCodeString(uint16_t errorCode){
		switch(errorCode){
			case 0x0: return "No Error";
			case 0xA302: return "A302 : Stop by positive limit switch";
			case 0xA303: return "A303 : Stop by negative limit switch";
			case 0xA320: return "A320 : Max Following Error Exceeded";
			case 0xB121: return "B121 : Cyclic communication: Synchronization signal missing";
			case 0xB122: return "B122 : Cyclic communication: Incorrect synchronization";
			case 0xB610: return "B610 : Fieldbus Watchdog";
			case 0xB103: return "B103 : Fieldbus Communication Closed";
			case 0x733F: return "733F : Amplitude of encoder analog signals too low";
			case 0x7344: return "7344 : Absolute position is different from incremental position";
			case 0x5200: return "5200 : Error detected at connection to motor encoder";
			case 0x1300: return "1300 : Safety Function STO was Enabled";
			case 0x1301: return "1301 : STO_A and STO_B different level";
			case 0x3100: return "3100 : Missing mains supply, undervoltage mains supply or overvoltage mains supply";
			case 0x1A01: return "1A01 : Detected motor type is different from previously detected motor.";
			case 0x7607: return "7607 : Encoder module cannot be identified";
			default: {
				static char hexString[16];
				sprintf(hexString, "%X", errorCode);
				std::string output = std::string(hexString) + " (Unknown Error Code)";
				return output;
			}
		}
	}
	
	std::string getStatusString(){
		std::string status;
		if(!isConnected()) {
			status = "Device is Offline.\n";
			return status;
		}
		if(!b_motorVoltagePresent) status += "Motor voltage is not present, check power connections.\n";
		if(b_hasFault){
			status += "Fault : ";
			if(b_faultNeedsRestart) status += getErrorCodeString(_LastError) + "(Drive Restart needed to reset fault.)\n";
			else status += getErrorCodeString(_LastError) + "(Fault will be cleared when enabling.)\n";
		}
		if(servoMotor->isEmergencyStopActive()) return "STO is Active\n";
		if(servoMotor->isHoldingBrakeReleased()) return "Motor holding brake is manually released.\n";
		if(actualPowerState == DS402::PowerState::NOT_READY_TO_SWITCH_ON) return "Drive Restart Needed.\n";
		return status;
	}
	
	std::string getShortStatusString(){
		if(!isConnected()) return "Device Offline";
		else if(!b_motorVoltagePresent) return "No Motor Voltage";
		else if(servoMotor->isEmergencyStopActive()) return "STO Active";
		else if(b_hasFault) {
			static char hexFaultCodeString[16];
			sprintf(hexFaultCodeString, "%X", _LastError);
			if(b_faultNeedsRestart) return "Critical Fault : " + std::string(hexFaultCodeString);
			else return "Fault : " + std::string(hexFaultCodeString);
		}
		else if(servoMotor->isHoldingBrakeReleased()) return "Holding Brake Released";
		else if(actualPowerState == DS402::PowerState::NOT_READY_TO_SWITCH_ON) return "Restart Needed";
		else return Enumerator::getDisplayString(actualPowerState);
	}
	 
	glm::vec4 getStatusColor(){
		if(!isConnected()) return Colors::blue;
		else if(servoMotor->isEmergencyStopActive()) return Colors::red;
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

    //===== Unit Settings =====

    const int positionUnitsPerRevolution = 131072;
    const int velocityUnitsPerRpm = 100;
    const int accelerationUnitsPerRpmps = 1;
    const int currentUnitsPerAmp = 100;

    //===== General Settings =====

    bool b_invertDirectionOfMotorMovement = false;
    double maxCurrent_amps = 0.0;
    
	//===== Drive Properties
    double maxMotorVelocity = 0.0;
	double maxMotorCurrent_amps = 0.0;

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

    //===== GPIO Input settings ======

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
	
	InputPin holdingBrakeReleasePin = InputPin::NONE;

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
		
	//internal encoder settings are constant
	const int encoder1_singleTurnResolutionBits = 17;
	const int encoder1_multiTurnResolutionBits = 12;

	bool b_encoderRangeShifted = false;
	bool b_encoderIsMultiturn = false;

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
