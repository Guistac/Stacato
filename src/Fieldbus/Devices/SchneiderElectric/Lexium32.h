#pragma once

#include "Fieldbus/EtherCatSlave.h"
#include "Utilities/ScrollingBuffer.h"

#include <map>
#include <string>

class Lexium32 : public EtherCatSlave {
public:

    SLAVE_DEFINITION(Lexium32, "LXM32M EtherCAT", "Schneider Electric", "Servo Drives")

    //fieldbus commands
    bool setStartupParameters();
    bool assignPDOs();

    //state machine commands
    void enableVoltage() { b_enableVoltage = true; }
    void disableVoltage() { b_disableVoltage = true; }
    void switchOn() { b_switchOn = true; }
    void shutDown() { b_shutdown = true; }
    void enableOperation() { b_enableOperation = true; }
    void disableOperation() { b_disableOperation = true; }
    void faultReset() { b_faultReset = true; }
    void quickStop() { b_quickStop = true; }

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
    uint16_t lastErrorCode = 0;

    const char* getStateChar() {
        switch (state) {
            case State::NotReadyToSwitchOn: return "Not Ready To Switch On";
            case State::SwitchOnDisabled: return "Switch On Disabled";
            case State::ReadyToSwitchOn: return "Ready To Switch On";
            case State::SwitchedOn: return "Switched On";
            case State::OperationEnabled: return "Operation Enabled";
            case State::QuickStopActive: return "Quick Stop Active";
            case State::FaultReactionActive: return "Fault Reaction Active";
            case State::Fault: return "Fault";
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
    OperatingMode::Mode requestedOperatingMode = OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION;

    //===== INTERNAL MOTION PROFILE GENERATOR =====
    
    double profileVelocity_rpm = 0.0;
    double profilePosition_r = 0.0;

    //===== Manual Controls =====

    float manualVelocityCommand_rpm = 0;
    float manualAcceleration_rpm2 = 100.0;

    //===== Drive Settings =====

    double maxVelocity_rpm = 5000.0;
    double maxAcceleration_rpm2 = 100.0;
    double maxCurrent_amps = 5.0;
    int encoderIncrementsPerShaftRotation = 131072;
    int encoderMultiturnResolution = 4096;

    bool b_uploadingMaxCurrent = true;
    bool b_uploadMaxCurrentSuccess = false;

    //===== Limit Switch Assignement ======

    struct InputPinFunction {
        enum class Type {
            UNASSIGNED,
            NEGATIVE_LIMIT,
            POSITIVE_LIMIT
        };
        Type type;
        const char displayName[64];
        const char saveName[64];
    };
    static std::vector<InputPinFunction> inputPinFunctions;
    static InputPinFunction* getInputPinFunction(const char* saveName);
    static InputPinFunction* getInputPinFunction(InputPinFunction::Type type);

    InputPinFunction::Type inputPin0Function = InputPinFunction::Type::UNASSIGNED;
    InputPinFunction::Type inputPin1Function = InputPinFunction::Type::UNASSIGNED;
    InputPinFunction::Type inputPin2Function = InputPinFunction::Type::UNASSIGNED;
    InputPinFunction::Type inputPin3Function = InputPinFunction::Type::UNASSIGNED;
    InputPinFunction::Type inputPin4Function = InputPinFunction::Type::UNASSIGNED;
    InputPinFunction::Type inputPin5Function = InputPinFunction::Type::UNASSIGNED;
    bool b_uploadingPinAssignements = true;
    bool b_uploadPinAssignementSuccess = false;

    //===== Encoder Settings =====

    struct EncoderAssignement {
        enum class Type {
            INTERNAL_ENCODER,
            ENCODER_MODULE
        };
        Type type;
        const char displayName[64];
        const char saveName[64];
    };
    static std::vector<EncoderAssignement> encoderAssignements;
    static EncoderAssignement* getEncoderAssignement(const char* saveName);
    static EncoderAssignement* getEncoderAssignement(EncoderAssignement::Type assignementType);

    struct EncoderModule {
        enum class Type {
            ANALOG_MODULE,
            DIGITAL_MODULE,
            RESOLVER_MODULE,
            NONE
        };
        Type type;
        const char displayName[64];
        const char saveName[64];
    };
    static std::vector<EncoderModule> encoderModules;
    static EncoderModule* getEncoderModule(const char* saveName);
    static EncoderModule* getEncoderModule(EncoderModule::Type moduleType);

    struct EncoderType {
        enum class Type {
            NONE,
            SSI_ROTARY
        };
        Type type;
        const char displayName[64];
        const char saveName[64];
    };
    static std::vector<EncoderType> encoderTypes;
    static EncoderType* getEncoderType(const char* saveName);
    static EncoderType* getEncoderType(EncoderType::Type moduleType);

    EncoderAssignement::Type encoderAssignement = EncoderAssignement::Type::INTERNAL_ENCODER;
    EncoderModule::Type encoderModuleType = EncoderModule::Type::NONE;
    EncoderType::Type encoderType = EncoderType::Type::NONE;
    uint8_t singleTurnResolutionBits = 0;
    uint8_t multiTurnResoltuionBits = 0;
    double encoderRevolutionsPerMotorShaftRevolution = 1.0;

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
    int32_t PVv_target = 0;
    int16_t PTtq_target = 0;
    uint16_t IO_DQ_set = 0;

    //Tx PDO display Data
    uint16_t _DCOMstatus = 0;
    int8_t _DCOMopmd_act = 0;
    int32_t _p_act = 0;
    int32_t _v_act = 0;
    int16_t _tq_act = 0;
    uint16_t _LastError = 0;
    uint16_t _IO_act = 0;

    //subdevices
    std::shared_ptr<ActuatorDevice> motorDevice = std::make_shared<ActuatorDevice>("Motor");
    std::shared_ptr<FeedbackDevice> encoderDevice = std::make_shared<FeedbackDevice>("Encoder");
    std::shared_ptr<GpioDevice> gpioDevice = std::make_shared<GpioDevice>("GPIO");

    //node input data
    std::shared_ptr<ioData> positionCommand =   std::make_shared<ioData>(DataType::REAL_VALUE,    DataDirection::NODE_INPUT, "Position");
    std::shared_ptr<ioData> velocityCommand =   std::make_shared<ioData>(DataType::REAL_VALUE,    DataDirection::NODE_INPUT, "Velocity");
    std::shared_ptr<ioData> digitalOut0 =       std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "DQ0");
    std::shared_ptr<ioData> digitalOut1 =       std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "DQ1");
    std::shared_ptr<ioData> digitalOut2 =       std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "DQ2");

    //node output data
    std::shared_ptr<ioData> motorLink =         std::make_shared<ioData>(DataType::ACTUATOR_DEVICELINK, DataDirection::NODE_OUTPUT, "Motor");
    std::shared_ptr<ioData> actualTorque =      std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "Torque", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> actualLoad =        std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "Load", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> encoderLink =       std::make_shared<ioData>(DataType::FEEDBACK_DEVICELINK, DataDirection::NODE_OUTPUT, "Encoder");
    std::shared_ptr<ioData> actualPosition =    std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "Position", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> actualVelocity =    std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "Velocity", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> gpioLink =          std::make_shared<ioData>(DataType::GPIO_DEVICELINK, DataDirection::NODE_OUTPUT, "GPIO");
    std::shared_ptr<ioData> digitalIn0 =        std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI0", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> digitalIn1 =        std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI1", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> digitalIn2 =        std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI2", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> digitalIn3 =        std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI3", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> digitalIn4 =        std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI4", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> digitalIn5 =        std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI5", ioDataFlags_DisableDataField);

    //command flags to control state machine (interface to construct DCOM_control word)
    bool b_enableVoltage = false;
    bool b_disableVoltage = false;
    bool b_switchOn = false;
    bool b_shutdown = false;
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


    void statusGui();
    void controlsGui();
    void limitsGui();
    void feedbackConfigurationGui();
};