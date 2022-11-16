#pragma once

struct EtherCatPdoAssignement;

class DS402Axis{
public:

	enum class OperatingMode{
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
		CYCLIC_SYNCHRONOUS_TORQUE_WITH_COMMUTATION_ANGLE
	};

	enum class PowerState{
		NOT_READY_TO_SWITCH_ON,
		SWITCH_ON_DISABLED,
		READY_TO_SWITCH_ON,
		SWITCHED_ON,
		OPERATION_ENABLED,
		QUICKSTOP_ACTIVE,
		FAULT_REACTION_ACTIVE,
		FAULT
	};
	
	//=== Axis Control ===
	
	void setPosition(int32_t position);
	void setVelocity(int32_t velocity);
	void setTorque(int32_t torque);
	void setFrequency(int16_t frequency);
	
	int32_t getActualPosition(){ return csp_positionActual; }
	int32_t getActualVelocity(){ return csv_velocityActual; }
	int32_t getActualTorque(){ return cst_torqueActual; }
	int16_t getActualFrequency(){ return vl_velocityActual; }
	
	bool hasWarning(){ return b_hasWarning; }
	bool hasFault(){ return b_hasFault; }
	bool isReady(){ return b_isReady; }
	bool isEnabled(){ return b_isEnabled; }
	bool isQuickstop(){ return b_isQuickstop; }
	
	bool hasVoltage(){ return b_voltageEnabled; }
	
	void enable(){ b_shouldEnable = true; }
	void disable(){ b_shouldDisable = true; }
	void doFaultReset(){ b_shouldFaultReset = true; }
	void doQuickstop(){ b_shouldQuickstop = true; }
	
	void startHoming();
	bool isHoming();
	bool didHomingSucceed();
	
	//manufacturer specific control/statusword bits
	void updateControlWordBits(bool b11, bool b12, bool b13, bool b14, bool b15){
		statusManB11 = b11;
		statusManB12 = b12;
		statusManB13 = b13;
		statusManB14 = b14;
		statusManB15 = b15;
	}
	void getStatusWordBits(bool& b8, bool& b14, bool& b15){
		b8 = (statusWord >> 8) & 0x1;
		b14 = (statusWord >> 14) & 0x1;
		b15 = (statusWord >> 15) & 0x1;
	}
	
	//=== Axis Configuration ===
	
	bool configure_Velocity = false;
	bool configure_Homing = false;
	bool configure_CyclicSynchronousVelocity = false;
	bool configure_CyclicSynchronousPosition = true;
	bool configure_CyclicSynchronousTorque = false;
	
	void configureProcessData(EtherCatPdoAssignement& rxPdo, EtherCatPdoAssignement& txPdo);
	void updateInputs();
	void updateOutput();
	
private:
	
	PowerState powerStateActual = PowerState::NOT_READY_TO_SWITCH_ON;
	PowerState powerStateTarget = PowerState::READY_TO_SWITCH_ON;
	
	OperatingMode operatingModeActual = OperatingMode::NONE;
	OperatingMode operatingModeTarget = OperatingMode::CYCLIC_SYNCHRONOUS_POSITION;
	
	bool b_isControlledRemotely = false;
	bool b_internalLimitActive = false;
	bool b_isFollowingCommandValue = false;
	bool b_voltageEnabled = false;
	bool b_hasWarning = false;
	bool b_hasFault = false;
	bool b_isReady = false;
	bool b_isEnabled = false;
	bool b_isQuickstop = false;
	
	bool b_shouldFaultReset = false;
	bool b_faultResetBusy = false;
	
	bool b_isHoming = false;
	bool b_didHomingSucceed = false;
	
	bool b_shouldEnable = false;
	bool b_shouldDisable = false;
	bool b_shouldQuickstop = false;
	
	//=== Power Control
	uint16_t controlWord;				//0x6040:0
	uint16_t statusWord;				//0x6041:0
	
	//=== Operating mode selection
	int8_t operatingModeSelection;		//0x6060:0
	int8_t operatingModeDisplay;		//0x6061:0
	
	//=== Cyclic Position Control
	int32_t csp_positionTarget;		//0x607A:0
	uint32_t csp_followingErrorWindow;	//0x6065:0 Optional
	uint32_t csp_followingErrorTimeout;	//0x6066:0 Optional
	
	//=== Cyclic Velocity Control
	int32_t csv_velocityTarget;		//0x60FF:0
	
	//=== Cyclic Torque Control
	int32_t cst_torqueTarget;			//0x6071:0
	uint32_t maxTorque;					//0x6072:0
	uint32_t maxPositionTorque;			//0x60E0:0
	uint32_t maxNegativeTorque;			//0x60E1:0
	
	//=== Realtime Drive State
	uint32_t csp_positionActual;		//0x6064:0
	uint32_t csv_velocityActual;		//0x606C:0
	uint32_t cst_torqueActual;			//0x6077:0
	uint32_t csp_followingErrorActual;	//0x60F4:0 Optional
	uint16_t motorCurrentActual;		//0x6078:0 Optional
	uint32_t controlEfforAtual;			//0x60FA:0 Optional
	
	//=== Drive Info
	uint16_t motorRatedCurrent;			//0x6075:0 Optional
	uint16_t motorRatedTorque;			//0x6076:0 Optional
	uint16_t polarity;					//0x607E:0 Optional
	uint16_t maxMotorSpeed;				//0x6080:0 Optional
	uint32_t interpolationTimePeriod;	//0x60C2:0
	
	//=== Homing
	uint16_t homingMethod;				//0x6098:0
	
	
	//=== Velocity Control
	uint16_t vl_velocityTarget;			//0x6042:0
	uint16_t vl_velocityActual;			//0x6044:0
	uint16_t vl_velocityLimit;			//0x6046:0
	uint16_t vl_acceleration;			//0x6048:0
	uint16_t vl_deceleration;			//0x6049:0
	
	
	
	
	//control world bits specific to operation mode: b4 b5 b6 b9
	//b4: homing operation start
	//b11 b12 b13 b14 b15: manufacturer specific
	bool statusOpB4 = false;
	bool statusOpB5 = false;
	bool statusOpB6 = false;
	bool statusOpB9 = false;
	bool statusManB11 = false;
	bool statusManB12 = false;
	bool statusManB13 = false;
	bool statusManB14 = false;
	bool statusManB15 = false;
	
	//STATUS WORD
	//b7: warning
	//b9: remote
	//b11: internal limit reached
	//b12: drive follows the command value (mandatory for csp csv cst cstca)
	//b10: status toggle (in csp csv cst cstca)
	//b13: extended 2nd bit for status toggle (in csp csv cst cstca)
	//b8 b14 b15: manufacturer specific
	
	//=== Supported Drive Modes
	uint32_t supportedDriveModes; //0x6502:0
	//b0: profile position
	//b1: velocity (frequency converter)
	//b2: profile velocity
	//b3: profile torque
	//b5: homing
	//b6: interpolated position
	//b7: cyclic synchronous position
	//b8: cyclic synchronous velocity
	//b9: cyclic synchronous torque
	//b10: cyclic synchrous torque with commutation angle
	
};
