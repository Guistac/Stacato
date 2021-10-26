#pragma once

namespace DS402 {

	struct PowerState {
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
	std::vector<PowerState>& getPowerStates();
	PowerState* getPowerState(PowerState::State s);

	struct OperatingMode {
		enum class Mode {
			PROFILE_POSITION,
			VELOCITY,
			PROFILE_VELOCITY,
			PROFILE_TORQUE,
			HOMING,
			INTERPOLATED_POSITION,
			CYCLIC_SYNCHRONOUS_POSITION,
			CYCLIC_SYNCHRONOUS_VELOCITY,
			CYCLIC_SYNCHRONOUS_TORQUE,
			CYCLIC_SYNCHRONOUS_TORQUE_WITH_COMMUTATION_ANGLE,
			UNKNOWN
		};
		Mode mode;
		int8_t value;
		const char displayName[64];
	};
	
	std::vector<OperatingMode>& getOperatingModes();
	OperatingMode* getOperatingMode(OperatingMode::Mode);
	OperatingMode* getOperatingMode(int8_t);


	struct Status {
		uint16_t statusWord = 0x0;
		int8_t operatingModeDisplay = 0x0;
		PowerState::State getPowerState();
		OperatingMode::Mode getOperatingMode();
		bool hasFault();
		bool hasWarning();
		bool isRemoteControlled();
		bool isInternalLimitSet();
		bool getManufacturerSpecificByte8();
		bool getManufacturerSpecificByte14();
		bool getManufacturerSpecificByte15();
		bool getOperationModeSpecificByte10();
		bool getOperationModeSpecificByte12();
		bool getOperationModeSpecificByte13();
	};

	struct Control {
		uint16_t controlWord = 0x0;
		int8_t operatingModeControl = 0x0;

		uint8_t powerStateControlBits = 0x0;
		bool faultResetBit = false;
		bool haltBit = false;
		bool b4 = false;
		bool b5 = false;
		bool b6 = false;
		bool b9 = false;
		bool b11 = false;
		bool b12 = false;
		bool b13 = false;
		bool b14 = false;
		bool b15 = false;

		void setOperatingMode(OperatingMode::Mode m);
		void setPowerState(PowerState::State requestedState, PowerState::State currentState);
		void performFaultReset();
		void performHalt();
		void clearHalt();
		bool isHalted();
		void setOperatingModeSpecificByte4(bool state);
		void setOperatingModeSpecificByte5(bool state);
		void setOperatingModeSpecificByte6(bool state);
		void setOperatingModeSpecificByte9(bool state);
		void setManufacturerSpecificByte11(bool state);
		void setManufacturerSpecificByte12(bool state);
		void setManufacturerSpecificByte13(bool state);
		void setManufacturerSpecificByte14(bool state);
		void setManufacturerSpecificByte15(bool state);
		void updateControlWord();
	};



	extern uint16_t statusWordIndex;
	extern uint16_t controlWordIndex;
	extern uint16_t operatingModeControlIndex;
	extern uint16_t operatingModeDisplayIndex;

	extern uint16_t targetPositionIndex;
	extern uint16_t actualPositionIndex;
	extern uint16_t actualFollowingErrorIndex;
	extern uint16_t followingErrorWindowIndex;
	extern uint16_t followingErrorTimeoutIndex;

	extern uint16_t targetVelocityIndex;
	extern uint16_t actualVelocityIndex;

	extern uint16_t actualTorqueIndex;
	extern uint16_t actualTorqueIndex;

}

