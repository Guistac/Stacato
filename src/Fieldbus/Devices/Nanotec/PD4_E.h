#include "Fieldbus/EtherCatDevice.h"

#include "Fieldbus/Utilities/DS402.h"

class PD4_E : public EtherCatDevice {

	DEFINE_ETHERCAT_DEVICE(PD4_E, "Drive", "PD4-E", "PD4-E", "Nanotec", "Servo Drives")

	std::shared_ptr<ServoActuatorDevice> servoMotor = std::make_shared<ServoActuatorDevice>("Servo", PositionUnit::Unit::REVOLUTION, PositionFeedback::Type::ABSOLUTE_FEEDBACK);
	std::shared_ptr<GpioDevice> gpioDevice = std::make_shared<GpioDevice>("Gpio");

	std::shared_ptr<NodePin> servoActuatorDeviceLink = std::make_shared<NodePin>(NodeData::Type::SERVO_ACTUATOR_DEVICE_LINK, DataDirection::NODE_OUTPUT, "Servo Motor", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> positionPin = std::make_shared<NodePin>(NodeData::Type::REAL_VALUE, DataDirection::NODE_OUTPUT, "Position", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(NodeData::Type::REAL_VALUE, DataDirection::NODE_OUTPUT, "Velocity", NodePinFlags_DisableDataField);
	
	std::shared_ptr<NodePin> gpioDeviceLink = std::make_shared<NodePin>(NodeData::Type::GPIO_DEVICELINK, DataDirection::NODE_OUTPUT, "Gpio");
	std::shared_ptr<NodePin> digitalIn1Pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Digital In 1", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> digitalIn2Pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Digital In 2", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> digitalIn3Pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Digital In 3", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> digitalIn4Pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Digital In 4", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> digitalIn5Pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Digital In 5", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> digitalIn6Pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Digital In 6", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> analogIn1Pin = std::make_shared<NodePin>(NodeData::Type::REAL_VALUE, DataDirection::NODE_OUTPUT, "Analog In 1", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> digitalOut1Pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Digital Out 1");
	std::shared_ptr<NodePin> digitalOut2Pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Digital Out 2");

	//constants
	const double maxVelocity_revolutionsPerSecond = 42.0;
	const double maxCurrent_amperes = 2.1;
	const int encoderSingleTurnResolutionBits = 12;
	const int encoderMultiTurnResolutionBits = 18;
	const double velocityUnitsPerRevolutionPerSecond = 100;
	const double followingErrorTimeout_milliseconds = 1.0;

	//settings:
	double currentLimit_amperes = 2.1;
	double maxFollowingError_revolutions = 1.0;
	double defaultManualAcceleration_revolutionsPerSecondSquared = 10.0;
	double positionControllerProportionalGain = 500;
	bool negativeLimitSwitchOnDigitalIn1 = false;
	bool positiveLimitSwitchOnDigitalIn2 = false;
	bool invertDigitalInput1 = false;
	bool invertDigitalInput2 = false;
	bool invertDigitalInput3 = false;
	bool invertDigitalInput4 = false;
	bool invertDigitalInput5 = false;
	bool invertDigitalInput6 = false;
	bool invertDirectionOfMotion = false;
	
	//TODO: add drive controller settings

	DS402::PowerState::State requestedPowerState = DS402::PowerState::State::READY_TO_SWITCH_ON;
	DS402::PowerState::State actualPowerState = DS402::PowerState::State::UNKNOWN;

	DS402::OperatingMode::Mode requestedOperatingMode = DS402::OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION;
	DS402::OperatingMode::Mode actualOperatingMode = DS402::OperatingMode::Mode::NONE;

	bool b_directionOfMotionIsInverted = false;
	bool b_startAutoSetup = false;
	bool b_autoSetupActive = false;
	bool b_autoSetupComplete = false;

	struct ControlMode {
		enum class Mode {
			MANUAL_VELOCITY_CONTROL,
			EXTERNAL_CONTROL
		};
		Mode mode;
		const char displayName[64];
	};
    static std::vector<ControlMode> controlModes;
	static std::vector<ControlMode>& getControlModes();
	ControlMode* getControlMode(ControlMode::Mode m);

	ControlMode::Mode actualControlMode = ControlMode::Mode::EXTERNAL_CONTROL;
	double manualVelocityTarget_revolutionsPerSecond = 0.0;
	double manualAcceleration_revolutionPerSecondSquared = defaultManualAcceleration_revolutionsPerSecondSquared;

	//=========== PROCESS DATA ============

	double actualPosition_revolutions = 0.0;
	double actualVelocity_revolutionsPerSecond = 0.0;
	double actualFollowingError_revolutions = 0.0;
	double actualCurrent_amperes = 0.0;

	double profilePosition_revolutions = 0.0;
	double profileVelocity_revolutions = 0.0;
	double profileTime_seconds = 0.0;

	bool digitalIn1 = false;
	bool digitalIn2 = false;
	bool digitalIn3 = false;
	bool digitalIn4 = false;
	bool digitalIn5 = false;
	bool digitalIn6 = false;

	bool digitalOut1 = false;
	bool digitalOut2 = false;

	//==============IO DATA===================

	//PDO OUTPUTS
	DS402::Control ds402control;
	int32_t targetPosition = 0;
	int32_t targetVelocity = 0;
	uint32_t digitalOutputs = 0;

	//PDO INPUTS
	DS402::Status ds402status;
	int32_t actualPosition = 0;
	int32_t actualVelocity = 0;
	int32_t actualCurrent = 0;
	int32_t actualError = 0;
	uint32_t digitalInputs = 0;

	//mandatory DS402 bits
	bool warning;
	bool fault;
	bool isRemoteControlled;
	bool targetReached;
	bool internalLimit;

	//nanotec drive specific bits
	bool controllerSyncedWithFieldbus;
	bool controllerFollowsTarget;
	bool followingError;
	bool closedLoopActive;

	//=========GUI==========

	void statusGui();
	void controlGui();
	void limitsGui();
	void gpioGui();
	void autosetupGui();
};
