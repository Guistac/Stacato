#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Utilities/ScrollingBuffer.h"

class Lexium32 : public EtherCatDevice {
public:

    DEFINE_ETHERCAT_DEVICE(Lexium32, "LXM32M EtherCAT", "Lexium32M", "Lexium32M", "Schneider Electric", "Servo Drives")

    //subdevices
    std::shared_ptr<ServoActuatorDevice> servoMotorDevice = std::make_shared<ServoActuatorDevice>("Servo", PositionUnit::REVOLUTION, PositionFeedbackType::ABSOLUTE);
    std::shared_ptr<GpioDevice> gpioDevice = std::make_shared<GpioDevice>("GPIO");

    //node input data
    std::shared_ptr<NodePin> digitalOut0 = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "DQ0");
    std::shared_ptr<NodePin> digitalOut1 = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "DQ1");
    std::shared_ptr<NodePin> digitalOut2 = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "DQ2");

    //node output data
    std::shared_ptr<NodePin> servoMotorLink = std::make_shared<NodePin>(NodeData::SERVO_ACTUATOR_DEVICE_LINK, DataDirection::NODE_OUTPUT, "Servo Motor");
    std::shared_ptr<NodePin> actualLoad = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "Load", NodePinFlags_DisableDataField | NodePinFlags_HidePin);
    std::shared_ptr<NodePin> actualPosition = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "Position", NodePinFlags_DisableDataField | NodePinFlags_HidePin);
    std::shared_ptr<NodePin> actualVelocity = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "Velocity", NodePinFlags_DisableDataField | NodePinFlags_HidePin);
    std::shared_ptr<NodePin> gpNodeLink = std::make_shared<NodePin>(NodeData::GPIO_DEVICELINK, DataDirection::NODE_OUTPUT, "GPIO");
    std::shared_ptr<NodePin> digitalIn0 = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI0", NodePinFlags_DisableDataField);
    std::shared_ptr<NodePin> digitalIn1 = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI1", NodePinFlags_DisableDataField);
    std::shared_ptr<NodePin> digitalIn2 = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI2", NodePinFlags_DisableDataField);
    std::shared_ptr<NodePin> digitalIn3 = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI3", NodePinFlags_DisableDataField);
    std::shared_ptr<NodePin> digitalIn4 = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI4", NodePinFlags_DisableDataField);
    std::shared_ptr<NodePin> digitalIn5 = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI5", NodePinFlags_DisableDataField);


	void resetData();
	
    //===== drive status =====

    enum class State {
        NotReadyToSwitchOn,
        SwitchOnDisabled,
        ReadyToSwitchOn,
        SwitchedOn,
        OperationEnabled,
        QuickStopActive,
        FaultReactionActive,
        Fault
    };

    State state = State::SwitchOnDisabled;

    const char* getStateChar() {
        switch (state) {
            case State::NotReadyToSwitchOn: return "Restart Needed";
            case State::SwitchOnDisabled: return "Switch On Disabled";
            case State::ReadyToSwitchOn: return "Ready To Switch On";
            case State::SwitchedOn: return "Switched On";
            case State::OperationEnabled: return "Operation Enabled";
            case State::QuickStopActive: return "Quick Stop Active";
            case State::FaultReactionActive: return "Fault Reaction Active";
            case State::Fault: return "Fault";
            default: return "";
        }
    }

    //===== mode display and changing =====

    struct OperatingMode {
        enum class Mode {
            TUNING,
            MOTION_SEQUENCE,
            ELECTRONIC_GEAR,
            JOG,
            PROFILE_POSITION,
            PROFILE_VELOCITY,
            PROFILE_TORQUE,
            HOMING,
            INTERPOLATED_POSITION,
            CYCLIC_SYNCHRONOUS_POSITION,
            CYCLIC_SYNCHRONOUS_VELOCITY,
            CYCLIC_SYNCHRONOUS_TORQUE,
            UNKNOWN
        };
        int id;
        Mode mode;
        const char displayName[64];
    };
    static std::vector<OperatingMode> operatingModes;
    static std::vector<OperatingMode> availableOperatingModes;
    static OperatingMode* getOperatingMode(OperatingMode::Mode operatingMode);
    static OperatingMode* getOperatingMode(const char* displayName);
    static OperatingMode* getOperatingMode(int id);

    OperatingMode::Mode actualOperatingMode = OperatingMode::Mode::UNKNOWN;
    const OperatingMode::Mode requestedOperatingMode = OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION;

    //===== EMERGENCY STOP =====

    bool b_emergencyStopActive = false;
    bool isEmergencyStopActive() { return b_emergencyStopActive; }
    void quickStop() { b_quickStop = true; }

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
	
	double maxFollowingError_revolutions = 0.0;

    void uploadGeneralParameters();
    DataTransferState::State generalParameterUploadState = DataTransferState::State::NO_TRANSFER;
    void downloadGeneralParameters();
    DataTransferState::State generalParameterDownloadState = DataTransferState::State::NO_TRANSFER;

    //======= QuickStop Settings =======
    
    struct QuickStopReaction {
        enum class Type {
            TORQUE_RAMP,
            DECELERATION_RAMP
        };
        Type type;
        const char displayName[64];
        const char saveName[64];
    };
    static std::vector<QuickStopReaction> quickStopReactions;
    std::vector<QuickStopReaction>& getQuickStopReactions();
    QuickStopReaction* getQuickStopReaction(const char* saveName);
    QuickStopReaction* getQuickStopReaction(QuickStopReaction::Type t);

    QuickStopReaction::Type quickstopReaction = QuickStopReaction::Type::TORQUE_RAMP;
    double maxQuickstopCurrent_amps = 0.0;
    double quickStopDeceleration_revolutionsPerSecondSquared = 0.0;

    //===== GPIO settings ======

    struct InputPin {
        enum class Pin {
            DI0,
            DI1,
            DI2,
            DI3,
            DI4,
            DI5,
            NONE
        };
        Pin pin;
        uint8_t CoeSubindex;
        const char displayName[64];
        const char saveName[64];
    };
    static std::vector<InputPin> inputPins;
    static std::vector<InputPin>& getInputPins();
    static InputPin* getInputPin(const char* saveName);
    static InputPin* getInputPin(InputPin::Pin pin);
    static InputPin* getInputPin(uint8_t coeSubindex);

    InputPin::Pin negativeLimitSwitchPin = InputPin::Pin::NONE;
    bool b_negativeLimitSwitchNormallyClosed = false;
    InputPin::Pin positiveLimitSwitchPin = InputPin::Pin::NONE;
    bool b_positiveLimitSwitchNormallyClosed = false;

    bool b_invertDI0 = false;
    bool b_invertDI1 = false;
    bool b_invertDI2 = false;
    bool b_invertDI3 = false;
    bool b_invertDI4 = false;
    bool b_invertDI5 = false;

    void uploadPinAssignements();
    DataTransferState::State pinAssignementUploadState = DataTransferState::State::NO_TRANSFER;
    void downloadPinAssignements();
    DataTransferState::State pinAssignementDownloadState = DataTransferState::State::NO_TRANSFER;

    //===== Encoder Settings =====

    struct EncoderAssignement {
        enum class Type {
            INTERNAL_ENCODER,
            ENCODER_MODULE
        };
        Type type;
        uint16_t CoeData;
        const char displayName[64];
        const char saveName[64];
    };
    static std::vector<EncoderAssignement> encoderAssignements;
    static EncoderAssignement* getEncoderAssignement(const char* saveName);
    static EncoderAssignement* getEncoderAssignement(EncoderAssignement::Type assignementType);
    static EncoderAssignement* getEncoderAssignement(uint16_t CoeData);

    struct EncoderModule {
        enum class Type {
            ANALOG_MODULE,
            DIGITAL_MODULE,
            RESOLVER_MODULE,
            NONE
        };
        Type type;
        uint16_t CoeData;
        const char displayName[64];
        const char saveName[64];
    };
    static std::vector<EncoderModule> encoderModules;
    static EncoderModule* getEncoderModule(const char* saveName);
    static EncoderModule* getEncoderModule(EncoderModule::Type moduleType);
    static EncoderModule* getEncoderModule(uint16_t CoeData);

    struct EncoderType {
        enum class Type {
            NONE,
            SSI_ROTARY
        };
        Type type;
        uint16_t CoeData;
        const char displayName[64];
        const char saveName[64];
    };
    static std::vector<EncoderType> encoderTypes;
    static EncoderType* getEncoderType(const char* saveName);
    static EncoderType* getEncoderType(EncoderType::Type moduleType);
    static EncoderType* getEncoderType(uint16_t CoeData);

    struct EncoderCoding {
        enum class Type {
            BINARY,
            GRAY
        };
        Type type;
        uint16_t CoeData;
        const char displayName[64];
        const char saveName[64];
    };
    static std::vector<EncoderCoding> encoderCodings;
    static EncoderCoding* getEncoderCoding(const char* saveName);
    static EncoderCoding* getEncoderCoding(EncoderCoding::Type encodingType);
    static EncoderCoding* getEncoderCoding(uint16_t CoeData);
    struct EncoderVoltage {
        enum class Voltage {
            V5,
            V12
        };
        Voltage voltage;
        uint16_t CoeData;
        const char displayName[64];
        const char saveName[64];
    };
    static std::vector<EncoderVoltage> encoderVoltages;
    static EncoderVoltage* getEncoderVoltage(const char* saveName);
    static EncoderVoltage* getEncoderVoltage(EncoderVoltage::Voltage v);
    static EncoderVoltage* getEncoderVoltage(uint16_t CoeData);

    EncoderAssignement::Type encoderAssignement = EncoderAssignement::Type::INTERNAL_ENCODER;
    EncoderModule::Type encoderModuleType = EncoderModule::Type::NONE;
    EncoderType::Type encoderType = EncoderType::Type::NONE;
    EncoderCoding::Type encoderCoding = EncoderCoding::Type::BINARY;
    EncoderVoltage::Voltage encoderVoltage = EncoderVoltage::Voltage::V12;

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
    DataTransferState::State encoderSettingsUploadState = DataTransferState::State::NO_TRANSFER;
    void downloadEncoderSettings();
    DataTransferState::State encoderSettingsDownloadState = DataTransferState::State::NO_TRANSFER;

    float manualAbsoluteEncoderPosition_revolutions = 0.0;
    void uploadManualAbsoluteEncoderPosition();
    DataTransferState::State encoderAbsolutePositionUploadState = DataTransferState::State::NO_TRANSFER;

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
    DataTransferState::State autoTuningSaveState = DataTransferState::State::NO_TRANSFER;

    //======== Other Settings =========

    bool saveToEEPROM();

    void factoryReset();
    DataTransferState::State factoryResetTransferState = DataTransferState::State::NO_TRANSFER;

    void setStationAlias(uint16_t a);
    DataTransferState::State stationAliasUploadState = DataTransferState::State::NO_TRANSFER;

    //===== Drive Status Flags ======

    //from _DCOMstatus
    bool motorVoltagePresent = false;
    bool class0error = false;
    bool halted = false;
    bool fieldbusControlActive = false;
    bool targetReached = false;
    bool internalLimitActive = false; //DS402intLim
    bool operatingModeSpecificFlag = false;
    bool stoppedByError = false;
    bool operatingModeFinished = false;
    bool validPositionReference = false;


private:

    //Rx PDO display Data
    uint16_t DCOMcontrol = 0;
    int8_t DCOMopmode = 0;
    int32_t PPp_target = 0;
    uint16_t IO_DQ_set = 0;

    //Tx PDO display Data
    uint16_t _DCOMstatus = 0;
    int8_t _DCOMopmd_act = 0;
    int32_t _p_act = 0;
	int32_t _p_dif_usr = 0;
    int32_t _v_act = 0;
    uint16_t _I_act = 0;
    uint16_t _LastError = 0;
    uint16_t _IO_act = 0;
    uint16_t _IO_STO_act = 0;

    //used to track change in _LastError
    uint16_t previousErrorCode = 0;

    //command flags to control state machine (interface to construct DCOM_control word)
    bool b_disableOperation = false;
    bool b_enableOperation = false;
    bool b_faultReset = false;
    bool b_quickStop = false;

    //bits used to construct DCOM_control word (DS402 state machine control word)
    bool b_switchedOn = false;
    bool b_voltageEnabled = false;
    bool b_quickStopActive = true; //quickstop is active when bit is low
    bool b_operationEnabled = false;
    bool opModeSpec4 = false;
    bool opModeSpec5 = false;
    bool opModeSpec6 = false;
    bool b_faultResetState = false;
    bool b_halted = false;
    bool opModeSpec9 = false;


    //Lexium GUI functions
    void statusGui();
    void controlsGui();
    void generalSettingsGui();
    void gpioGui();
    void encoderGui();
    void tuningGui();
    void miscellaneousGui();
};
