#pragma once

namespace DS402 {

	enum class PowerState{
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

	enum class OperatingMode {
		NONE,
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
	
	OperatingMode getOperatingModeInteger(int8_t modeNumber);
	int8_t getOperatingModeFromInteger(OperatingMode mode);

	struct Status {
		uint16_t statusWord = 0x0;
		int8_t operatingMode = 0x0;
		PowerState getPowerState();
		OperatingMode getOperatingMode();
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
		int8_t operatingMode = 0x0;

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

		void setOperatingMode(OperatingMode m);
		void setPowerState(PowerState requestedState, PowerState currentState);
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


#define PowerStateStrings \
	{DS402::PowerState::NOT_READY_TO_SWITCH_ON, "Not ready to switch on"},\
	{DS402::PowerState::SWITCH_ON_DISABLED, 	"Switch on disabled"},\
	{DS402::PowerState::READY_TO_SWITCH_ON, 	"Ready to switch on"},\
	{DS402::PowerState::SWITCHED_ON, 			"Switched on"},\
	{DS402::PowerState::OPERATION_ENABLED, 		"Operation Enabled"},\
	{DS402::PowerState::QUICKSTOP_ACTIVE, 		"Quickstop Active"},\
	{DS402::PowerState::FAULT_REACTION_ACTIVE, 	"Fault reaction active"},\
	{DS402::PowerState::FAULT, 					"Fault"},\
	{DS402::PowerState::UNKNOWN, 				"Unknown power state"}\

DEFINE_ENUMERATOR(DS402::PowerState, PowerStateStrings)

#define OperatingModeStrings \
	{DS402::OperatingMode::NONE, 												"No operating mode"},\
	{DS402::OperatingMode::PROFILE_POSITION, 									"Profile Position"},\
	{DS402::OperatingMode::VELOCITY, 											"Velocity"},\
	{DS402::OperatingMode::PROFILE_VELOCITY, 									"Profile Velocity"},\
	{DS402::OperatingMode::PROFILE_TORQUE, 										"Profile Torque"},\
	{DS402::OperatingMode::HOMING, 												"Homing"},\
	{DS402::OperatingMode::INTERPOLATED_POSITION, 								"Interpolated position"},\
	{DS402::OperatingMode::CYCLIC_SYNCHRONOUS_POSITION, 						"Cyclic synchronous position"},\
	{DS402::OperatingMode::CYCLIC_SYNCHRONOUS_VELOCITY, 						"Cyclic synchronous velocity"},\
	{DS402::OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE, 							"Cyclic synchronous torque"},\
	{DS402::OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE_WITH_COMMUTATION_ANGLE, 	"Cyclic synchronous torque with communcation angle"},\
	{DS402::OperatingMode::UNKNOW, 												"Unknown operating mode"}\
