#include "Fieldbus/EtherCatDevice.h"

class PD4_E : public EtherCatDevice {

	DEFINE_ETHERCAT_DEVICE(PD4_E, "Drive", "PD4-E", "PD4-E", "Nanotec", "Servo Drives")


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


	uint16_t DS402controlWord = 0;
	int8_t operatingModeControl = 0;


	uint16_t DS402statusWord = 0;
	int8_t operatingModeDisplay = 0;
	int32_t actualPosition = 0;
};