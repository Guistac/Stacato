#include "Fieldbus/EtherCatDevice.h"

#include "Fieldbus/Utilities/DS402.h"

class PD4_E : public EtherCatDevice {

	DEFINE_ETHERCAT_DEVICE(PD4_E, "Drive", "PD4-E", "PD4-E", "Nanotec", "Servo Drives")

	std::shared_ptr<ServoActuatorDevice> servoMotor = std::make_shared<ServoActuatorDevice>("Servo", PositionUnit::Unit::REVOLUTION, PositionFeedback::Type::ABSOLUTE_FEEDBACK);

	std::shared_ptr<NodePin> servoActuatorDeviceLink = std::make_shared<NodePin>(NodeData::Type::SERVO_ACTUATOR_DEVICE_LINK, DataDirection::NODE_OUTPUT, "Servo Motor", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> positionPin = std::make_shared<NodePin>(NodeData::Type::REAL_VALUE, DataDirection::NODE_OUTPUT, "Position", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(NodeData::Type::REAL_VALUE, DataDirection::NODE_OUTPUT, "Velocity", NodePinFlags_DisableDataField);


	//settings:
	double maxCurrent_amperes = 2.1;
	double maxFollowingError_revolutions = 1.0;
	double followingErrorTimeout_milliseconds = 10;
	
	//TODO: add drive controller settings

	//constants
	const int encoderSingleTurnResolutionBits = 12;
	const int encoderMultiTurnResolutionBits = 18;
	const double velocityUnitsPerRevolutionPerSecond = 100;


	//commands:
	bool b_enabledOperation = false;
	bool b_disableOperation = false;

	double targetPosition_revolution = 0.0;

	bool performFaultReset = false;
	DS402::PowerState::State requestedPowerState = DS402::PowerState::State::READY_TO_SWITCH_ON;
	DS402::PowerState::State actualPowerState = DS402::PowerState::State::UNKNOWN;

	DS402::OperatingMode::Mode requestedOperatingMode = DS402::OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION;
	DS402::OperatingMode::Mode actualOperatingMode = DS402::OperatingMode::Mode::NONE;

	//PDO OUTPUTS
	DS402::Control ds402control;
	int32_t targetPosition = 0;
	int32_t targetVelocity = 0;

	//PDO INPUTS
	DS402::Status ds402status;
	int32_t actualPosition = 0;
	int32_t actualVelocity = 0;
	int32_t actualCurrent = 0;
	int32_t actualError = 0;

	double followingError_revolutions = 0;

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


	bool b_autoSetup = false;
	bool b_startSetup = false;
};