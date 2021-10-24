#include "Fieldbus/EtherCatDevice.h"

class PD4_E : public EtherCatDevice {

	DEFINE_ETHERCAT_DEVICE(PD4_E, "Drive", "PD4-E", "PD4-E", "Nanotec", "Servo Drives")

	std::shared_ptr<ServoActuatorDevice> servoActuatorDevice = std::make_shared<ServoActuatorDevice>("Servo", PositionUnit::Unit::REVOLUTION, PositionFeedback::Type::ABSOLUTE_FEEDBACK);

	struct DS402PowerState {
		enum class State {
			NOT_READY_TO_SWITCH_ON,
			SWITCH_ON_DISABLED,
			READY_TO_SWITCH_ON,
			SWITCHED_ON,
			OPERATION_ENABLED,
			QUICKSTOP_ACTIVE,
			FAULT_REACTION_ACTIVE,
			FAULT,
			UNKNOWN
		};
		State state;
		const char displayName[64];
	};
	std::vector<DS402PowerState>& getDS402PowerStates();
	DS402PowerState* getDS402PowerState(DS402PowerState::State);

	struct OperatingMode {
		enum class Mode {
			AUTO_SETUP,
			NO_MODE_CHANGE_OR_NONE_ASSIGNED,
			PROFILE_POSITION,
			VELOCITY,
			PROFILE_VELOCITY,
			PROFILE_TORQUE,
			HOMING,
			INTERPOLATED_POSITION,
			CYCLIC_SYNCHRONOUS_POSITION,
			CYCLIC_SYNCHRONOUS_VELOCITY,
			CYCLIC_SYNCHRONOUS_TORQUE
		};
		Mode mode;
		int8_t value;
		const char displayName[64];
	};
	std::vector<OperatingMode>& getOperatingModes();
	OperatingMode* getOperatingMode(int8_t value);
	OperatingMode* getOperatingMode(OperatingMode::Mode);

	bool performFaultReset = false;
	bool b_faultReset = false;
	DS402PowerState::State requestedPowerState = DS402PowerState::State::READY_TO_SWITCH_ON;
	DS402PowerState::State powerState = DS402PowerState::State::UNKNOWN;

	OperatingMode::Mode requestedOperatingMode = OperatingMode::Mode::NO_MODE_CHANGE_OR_NONE_ASSIGNED;
	OperatingMode::Mode currentOperatingMode = OperatingMode::Mode::NO_MODE_CHANGE_OR_NONE_ASSIGNED;

	//PDO OUTPUTS
	uint16_t DS402controlWord = 0;
	int8_t operatingModeControl = 0;
	int32_t targetVelocity = 0;

	//PDO INPUTS
	uint16_t DS402statusWord = 0;
	int8_t operatingModeDisplay = 0;
	int32_t actualPosition = 0;
	int32_t actualVelocity = 0;
};