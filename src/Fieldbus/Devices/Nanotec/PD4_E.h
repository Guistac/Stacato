#include "Fieldbus/EtherCatDevice.h"

#include "Fieldbus/Utilities/DS402.h"

class PD4_E : public EtherCatDevice {

	DEFINE_ETHERCAT_DEVICE(PD4_E, "PD4-E", "PD4-E", "Nanotec", "Servo Drives", 0x26C, 0xC9);
	
	class PD4EServoMotor : public ActuatorInterface{
	public:
		virtual void enable() override { b_enable = true; }
		virtual void disable() override { b_disable = true; }
		bool b_enable = false;
		bool b_disable = false;
		virtual std::string getName() override {
			return "PD4E Servo Motor";
		}
		virtual std::string getStatusString() override {
			return "No Status";
		}
	};
	
	class PD4EGpio : public GpioInterface{
	public:
		virtual std::string getName() override {
			return "PD4E Gpio";
		}
		virtual std::string getStatusString() override {
			return "No Status";
		}
	};
	
	std::shared_ptr<PD4EServoMotor> servoMotor = std::make_shared<PD4EServoMotor>();
	std::shared_ptr<PD4EGpio> gpioDevice = std::make_shared<PD4EGpio>();

	std::shared_ptr<NodePin> servoActuatorDeviceLink = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR_INTERFACE, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Servo Motor", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> positionPin = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "Position", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "Velocity", NodePin::Flags::DisableDataField);
	std::shared_ptr<double> positionPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> velocityPinValue = std::make_shared<double>(0.0);
	
	std::shared_ptr<NodePin> gpioDeviceLink = std::make_shared<NodePin>(NodePin::DataType::GPIO_INTERFACE, NodePin::Direction::NODE_OUTPUT, "Gpio");
	std::shared_ptr<NodePin> digitalIn1Pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Digital In 1", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalIn2Pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Digital In 2", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalIn3Pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Digital In 3", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalIn4Pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Digital In 4", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalIn5Pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Digital In 5", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalIn6Pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Digital In 6", NodePin::Flags::DisableDataField);
	std::shared_ptr<bool> digitalIn1PinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn2PinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn3PinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn4PinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn5PinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalIn6PinValue = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> analogIn1Pin = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "Analog In 1", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalOut1Pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "Digital Out 1");
	std::shared_ptr<NodePin> digitalOut2Pin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "Digital Out 2");
	std::shared_ptr<double> analogIn1PinValue = std::make_shared<double>(0.0);
	std::shared_ptr<bool> digitalOut1PinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalOut2PinValue = std::make_shared<bool>(false);

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

	DS402::PowerState requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
	DS402::PowerState actualPowerState = DS402::PowerState::UNKNOWN;

	DS402::OperatingMode requestedOperatingMode = DS402::OperatingMode::CYCLIC_SYNCHRONOUS_POSITION;
	DS402::OperatingMode actualOperatingMode = DS402::OperatingMode::NONE;

	bool b_directionOfMotionIsInverted = false;
	bool b_startAutoSetup = false;
	bool b_autoSetupActive = false;
	bool b_autoSetupComplete = false;
	
	void resetData();

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
