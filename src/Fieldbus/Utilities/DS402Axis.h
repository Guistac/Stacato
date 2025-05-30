#pragma once

#include <memory>
#include "Fieldbus/EtherCatDevice.h"

struct EtherCatPdoAssignement;

class DS402Axis{
private:
	
	DS402Axis(std::shared_ptr<EtherCatDevice> parentDevice_) : parentDevice(parentDevice_){}
	std::shared_ptr<EtherCatDevice> parentDevice = nullptr;
	bool b_warnPowerStateChanges = true;
	
public:
	
	static std::shared_ptr<DS402Axis> make(std::shared_ptr<EtherCatDevice> parentDevice_){
		return std::shared_ptr<DS402Axis>(new DS402Axis(parentDevice_));
	}
	
	void warnPowerStateChanged(bool warn){ b_warnPowerStateChanges = warn; }

	
	enum OperatingMode{
		NONE = 0,
		PROFILE_POSITION = 1,
		VELOCITY = 2,
		PROFILE_VELOCITY = 3,
		PROFILE_TORQUE = 4,
		HOMING = 6,
		INTERPOLATED_POSITION = 7,
		CYCLIC_SYNCHRONOUS_POSITION = 8,
		CYCLIC_SYNCHRONOUS_VELOCITY = 9,
		CYCLIC_SYNCHRONOUS_TORQUE = 10,
		CYCLIC_SYNCHRONOUS_TORQUE_WITH_COMMUTATION_ANGLE = 11
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
	
	enum class TargetPowerState{
		DISABLED,
		ENABLED,
		QUICKSTOP_ACTIVE
	};
	
	
	struct Configuration{
		
	}configuration;
	
	//—————————————————————————————————————————————————
	//—————————————— PDO Configuration ————————————————
	//—————————————————————————————————————————————————
	
	struct ProcessDataConfiguration{
		
		void enableFrequencyMode(){
			targetFrequency = true;
			frequencyActualValue = true;
			operatingModes.frequency = true;
		}
		
		void enableCyclicSynchronousPositionMode(){
			targetPosition = true;
			positionActualValue = true;
			operatingModes.cyclicSynchronousPosition = true;
		}
		
		void enableCyclicSynchronousVelocityMode(){
			targetVelocity = true;
			velocityActualValue = true;
			operatingModes.cyclicSynchronousVelocity = true;
		}
		
		void enableCyclicSynchronousTorqueMode(){
			targetTorque = true;
			torqueActualValue = true;
			operatingModes.cyclicSynchronousTorque = true;
		}
		
		void enableCyclicSynchronousTorqueWithCommutationAngleMode(){
			assert("Cyclic Synchronous Torque With Commutation Angle is not yet supported");
			operatingModes.cyclicSynchronousTorqueWithCommutationAngle = true;
		}
		
		void enableHomingMode(){
			operatingModes.homing = true;
		}
		
	private:
		
		friend class DS402Axis;
		
		struct OperatingModes{
			bool frequency = false;
			bool cyclicSynchronousPosition = false;
			bool cyclicSynchronousVelocity = false;
			bool cyclicSynchronousTorque = false;
			bool cyclicSynchronousTorqueWithCommutationAngle = false;
			bool homing = false;
		}operatingModes;
		
	public:
		
		//———— Errors
		
		///603F.0 Error Code
		bool errorCode = false;
		
		//———— Frequency Converter
		
		///6042.0 Target Velocity (frequency converter mode)
		bool targetFrequency = false;
		
		///6044.0 Velocity Actual Value (frequency converter mode) (or vl_control_effort)
		bool frequencyActualValue = false;
		
		//———— Position
		
		///607A.0 Target Position
		bool targetPosition = false;
		
		///6064.0 Position Actual Value
		bool positionActualValue = false;
		
		///60F4.0 Following Error Actual Value (int32, position units)
		bool positionFollowingErrorActualValue = false;
		
		//———— Velocity
		
		///60FF.0 Target Velocity
		bool targetVelocity = false;
		
		///606C.0 Velocity Actual Value
		bool velocityActualValue = false;
		
		//———— Torque
		
		///6071.0 Target Torque
		bool targetTorque = false;
		
		///6077.0 Torque Actual Value
		bool torqueActualValue = false;
		
		///6078.0 Current Actual Value
		bool currentActualValue = false;
		
		///60EA.0 Commutation Angle
		//bool commutationAngle = false;
		
		///60FA Control Effort
		//bool controlEffort = false;
		
		//———— Inputs and Outputs
		
		///60FD.0 Digital Inputs
		bool digitalInputs = false;
		
		///60FE.1 Digital Outputs
		bool digitalOutputs = false;
		
	}processDataConfiguration;
	
	void configureProcessData();
	bool startupConfiguration();
	void updateInputs();
	void updateOutput();

	//—————————————————————————————————————————————————
	//—————————————— SDO Configuration ————————————————
	//—————————————————————————————————————————————————
	
	//=== Drive Operation
	
	bool setOperatingMode(OperatingMode mode){
		return parentDevice->writeSDO_S8(0x6060, 0x0, mode, "DS402 Operating Mode");
	}
	
	bool getOperatingMode(OperatingMode& output){
		int8_t opMode;
		if(!parentDevice->readSDO_S8(0x6061, 0x0, opMode, "DS402 Operating Mode")) return false;
		output = OperatingMode(opMode);
		return true;
	}
	
	///60C2.? Interpolation Time Period
	bool setInterpolationTimePeriod(int milliseconds){
		if(milliseconds < 1 || milliseconds > 255) return false;
		uint8_t base = milliseconds;
		int8_t exponent = -3;
		if(!parentDevice->writeSDO_U8(0x60C2, 0x1, base)) return false;
		if(!parentDevice->writeSDO_S8(0x60C2, 0x2, exponent)) return false;
		return true;
	}
	
	///6065.0 Following Error Window
	bool setFollowingErrorWindow(uint32_t maxFollowingError) { return parentDevice->writeSDO_U32(0x6065, 0x0, maxFollowingError, "DS402 Following Error Window"); }
	bool getFollowingErrorWindow(uint32_t& output){ return parentDevice->readSDO_U32(0x6065, 0x0, output, "DS402 Following Error Window"); }

	///6066.0 Following Error Time Out
	bool setFollowingErrorTimeOut(uint16_t followingErrorTimeout) { return parentDevice->writeSDO_U16(0x6066, 0x0, followingErrorTimeout, "DS402 Following Error Time Out"); }
	bool getFollowingErrorTimeOut(uint16_t& output){ return parentDevice->readSDO_U16(0x6066, 0x0, output, "DS402 Following Error Time Out"); }
	
	///607E.0 Polarity
	bool setPolarity(bool polarity){ return parentDevice->writeSDO_U8(0x607E, 0x0, polarity ? 1 : 0, "DS402 Polarity"); }
	bool getPolarity(bool& output){
		uint8_t polarity;
		if(parentDevice->readSDO_U8(0x607E, 0x0, polarity, "DS402 Polarity")){
			if(polarity == 0x0) output = false;
			else output = true;
			return true;
		}
		return false;
	}

	///6073.0 Max Current
	bool setMaxCurrent(uint16_t current){ return parentDevice->writeSDO_U16(0x6073, 0x0, current, "DS402 Max Current"); }
	bool getMaxCurrent(uint16_t& output){ return parentDevice->readSDO_U16(0x6073, 0x0, output, "DS402 Max Current"); }
	
	///6075.0 Motor Rated Current (mA)
	bool getMotorRatedCurrent(uint32_t& output) { return parentDevice->readSDO_U32(0x6075, 0x0, output, "DS402 Motor Rated Current"); }
	
	///6076.? Motor Rated Torque
	
	///6080.? Max Motor Speed
	
	///6007.9 Abort Connection Option Code
	bool setAbortConnectionOptionCode(int16_t optionCode){ return parentDevice->writeSDO_S16(0x6007, 0x0, optionCode); }
	bool getAbortConnectionOptionCode(int16_t& output){ return parentDevice->readSDO_S16(0x6007, 0x0, output); }
	
	///605A.0 Quickstop Option Code
	bool setQuickstopOptionCode(int16_t optionCode){ return parentDevice->writeSDO_S16(0x605A, 0x0, optionCode); }
	bool getQuickstopOptionCode(int16_t& output){ return parentDevice->readSDO_S16(0x605A, 0x0, output); }
	
	///605B.0 Shutdown Option Code
	bool setShutdownOptionCode(int16_t optionCode){ return parentDevice->writeSDO_S16(0x605B, 0x0, optionCode); }
	bool getShutdownOptionCode(int16_t& output){ return parentDevice->readSDO_S16(0x605B, 0x0, output); }
	
	///605C.0 Disable Operation Option Code
	bool setDisableOperationOptionCode(int16_t optionCode){ return parentDevice->writeSDO_S16(0x605C, 0x0, optionCode); }
	bool getDisableOperationOptionCode(int16_t& output){ return parentDevice->readSDO_S16(0x605C, 0x0, output); }
	
	///605D.0 Halt Option Code
	bool setHaltOptionCode(int16_t optionCode){ return parentDevice->writeSDO_S16(0x605D, 0x0, optionCode); }
	bool getHaltOptionCode(int16_t& output){ return parentDevice->readSDO_S16(0x605D, 0x0, output); }
	
	///605E.0 Fault Reaction Option Code
	bool setFaultReactionOptionCode(int16_t optionCode){ return parentDevice->writeSDO_S16(0x605E, 0x0, optionCode); }
	bool getFaultReactionOptionCode(int16_t& output){ return parentDevice->readSDO_S16(0x605E, 0x0, output); }
	
	///6085.0 Quickstop Deceleration
	bool setQuickstopDeceleration(uint32_t deceleration) { return parentDevice->writeSDO_U32(0x6085, 0x0, deceleration); }
	bool getQuickstopDeceleration(uint32_t& output) { return parentDevice->readSDO_U32(0x6085, 0x0, output); }
	
	
	
	//=== Frequency
	
	///6046.0 vl velocity min max amount
	bool setMaxMinFrequency(){
		//return parentDevice->writeSDO_U16(0x6046, 0x0, <#const uint16_t &data#>);
		return false;
	}
	
	///6048.0 velocity acceleration
	bool setFrequencyAcceleration(){
		//return parentDevice->writeSDO_U16(0x6048, 0x0, <#const uint16_t &data#>);
		return false;
	}
	
	///6049.? velocity deceleration
	bool setFrequencyDeceleration(){
		//return parentDevice->writeSDO_U16(0x6049, 0x0, <#const uint16_t &data#>);
		return false;
	}
	
	//=== Position
	
	///60B0.0 Position Offset (position units)
	bool setPositionOffset(int32_t offset){ return parentDevice->writeSDO_S32(0x60B0, 0x0, offset); }
	bool getPositionOffset(int32_t& output){ return parentDevice->readSDO_S32(0x60B0, 0x0, output); }
	
	///6065.0 Following Error Window (position units)
	bool setPositionFollowingErrorWindow(uint32_t window){ return parentDevice->writeSDO_U32(0x6065, 0x0, window); }
	bool getPositionFollowingErrorWindow(uint32_t& output){ return parentDevice->readSDO_U32(0x6065, 0x0, output); }
	
	///6066.0 Following Error Timeout (milliseconds)
	bool setPositionFollowingErrorTimeout(uint16_t timeout){ return parentDevice->writeSDO_U16(0x6066, 0x0, timeout); }
	bool getPositionFollowingErrorTimeout(uint16_t& output){ return parentDevice->readSDO_U16(0x6066, 0x0, output); }
	
	///607D.? position range limit
	
	///607B.? software position limit
	
	//==== Velocity
	
	///60B1.0 Velocity Offset
	bool setVelocityOffset(int32_t offset){ return parentDevice->writeSDO_S32(0x60B1, 0x0, offset); }
	bool getVelocityOffset(int32_t& output){ return parentDevice->readSDO_S32(0x60B1, 0x0, output); }
	
	///6083.0 Profile Acceleration
	bool setProfileAcceleration(uint32_t acceleration){ return parentDevice->writeSDO_U32(0x6083, 0x0, acceleration, "DS402 Profile Acceleration"); }
	bool getProfileAcceleration(uint32_t& output){ return parentDevice->readSDO_U32(0x6083, 0x0, output, "DS402 Profile Acceleration"); }
	
	///6084.0 Profile Deceleration
	bool setProfileDeceleration(uint32_t deceleration){ return parentDevice->writeSDO_U32(0x6084, 0x0, deceleration, "DS402 Profile Deceleration"); }
	bool getProfileDeceleration(uint32_t& output){ return parentDevice->readSDO_U32(0x6084, 0x0, output, "DS402 Profile Deceleration"); }
	
	//==== Torque
	
	///60B2.0 Torque Offset
	bool setTorqueOffset(int16_t offset){ return parentDevice->writeSDO_S16(0x60B2, 0x0, offset); }
	bool getTorqueOffset(int16_t& output){ return parentDevice->readSDO_S16(0x60B2, 0x0, output); }
	
	///6072.? Max Torque
	
	///60E0.0 Max Positive Torque
	bool setMaxPositiveTorque(uint16_t torque){ return parentDevice->writeSDO_U16(0x60E0, 0x0, torque); }
	bool getMaxPositiveTorque(uint16_t& output){ return parentDevice->readSDO_U16(0x60E0, 0x0, output); }
	
	///60E1.0 Max Negative Torque
	bool setMaxNegativeTorque(uint16_t torque){ return parentDevice->writeSDO_U16(0x60E1, 0x0, torque); }
	bool getMaxNegativeTorque(uint16_t& output){ return parentDevice->readSDO_U16(0x60E1, 0x0, output); }
	
	
	
	//—————————————————————————————————————————————————
	//———————————————— Axis Control ———————————————————
	//—————————————————————————————————————————————————
	
	bool hasWarning(){ return b_hasWarning; }
	bool hasFault(){ return b_hasFault; }
	uint16_t getErrorCode(){ return processData.errorCode; }
	
	bool hasVoltage(){ return b_voltageEnabled; }
	bool isRemoteControlActive(){ return b_remoteControlActive; }
	bool isInternalLimitReached(){ return b_internalLimitReached; }
	
	bool isReady(){
		switch(powerStateActual){
			case PowerState::READY_TO_SWITCH_ON:
			case PowerState::SWITCHED_ON:
			case PowerState::OPERATION_ENABLED:
			case PowerState::QUICKSTOP_ACTIVE:
				return true;
			default:
				return false;
		}
	}
	bool isNotReady(){
		switch(powerStateActual){
			case PowerState::NOT_READY_TO_SWITCH_ON:
			case PowerState::SWITCH_ON_DISABLED:
				return true;
			default:
				return false;
		}
	}
	bool isEnabled(){ return powerStateActual == PowerState::OPERATION_ENABLED; }
	bool isQuickstopActive(){ return powerStateActual == PowerState::QUICKSTOP_ACTIVE; }
	
	void enable(){ powerStateTarget = TargetPowerState::ENABLED; }
	void disable(){ powerStateTarget = TargetPowerState::DISABLED; }
	void quickstop(){ powerStateTarget = TargetPowerState::QUICKSTOP_ACTIVE; }
	void doFaultReset(){ b_doFaultReset = true; }
	
	void setTargetPowerState(TargetPowerState target){ powerStateTarget = target; }
	TargetPowerState getTargetPowerState(){ return powerStateTarget; }
	PowerState getActualPowerState(){ return powerStateActual; }
	static std::string getPowerStateString(PowerState state){
		switch(state){
			case PowerState::NOT_READY_TO_SWITCH_ON: return "Not ready to switch on";
			case PowerState::SWITCH_ON_DISABLED: return "Switch on disabled";
			case PowerState::READY_TO_SWITCH_ON: return "Ready to switch on";
			case PowerState::SWITCHED_ON: return "Switched on";
			case PowerState::OPERATION_ENABLED: return "Operation enabled";
			case PowerState::QUICKSTOP_ACTIVE: return "Quickstop active";
			case PowerState::FAULT_REACTION_ACTIVE: return "Fault reaction active";
			case PowerState::FAULT: return "Fault";
		}
	}
	static std::string getTargetPowerStateString(TargetPowerState state){
		switch(state){
			case TargetPowerState::DISABLED: return "Disabled";
			case TargetPowerState::ENABLED: return "Enabled";
			case TargetPowerState::QUICKSTOP_ACTIVE: return "Quickstop Active";
		}
	}
	static std::string getOperatingModeString(OperatingMode mode){
		switch(mode){
			case OperatingMode::NONE: return "None";
			case OperatingMode::PROFILE_POSITION: return "Profile position";
			case OperatingMode::VELOCITY: return "Velocity";
			case OperatingMode::PROFILE_VELOCITY: return "Profile Velocity";
			case OperatingMode::PROFILE_TORQUE: return "Profile Torque";
			case OperatingMode::HOMING: return "Homing";
			case OperatingMode::INTERPOLATED_POSITION: return "Interpolated position";
			case OperatingMode::CYCLIC_SYNCHRONOUS_POSITION: return "Cyclic Synchronous Position";
			case OperatingMode::CYCLIC_SYNCHRONOUS_VELOCITY: return "Cyclic Synchronous Velocity";
			case OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE: return "Cyclic Synchronous Torque";
			case OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE_WITH_COMMUTATION_ANGLE: return "Cyclic Synchronous Torque With Communtation Angle";
		}
	}
	
	OperatingMode getOperatingModeActual(){ return OperatingMode(processData.operatingModeDisplay); }
	OperatingMode getOperatingModeTarget(){ return OperatingMode(processData.operatingModeSelection); }
	
	int16_t getActualFrequency(){ return processData.frequencyActualValue; }
	int32_t getActualPosition(){ return processData.positionActualValue; }
	int32_t getActualVelocity(){ return processData.velocityActualValue; }
	int16_t getActualTorque(){ return processData.torqueActualValue; }
	int16_t getActualCurrent(){ return processData.currentActualValue; }
	int32_t getActualPositionFollowingError(){ return processData.positionFollowingErrorActualValue; }
	
	uint32_t getDigitalInputs(){ return processData.digitalInputs; }
	void setDigitalOutputs(uint32_t digitalOutputs){ processData.digitalOutputs = digitalOutputs; }
	
	void setManufacturerSpecificOperatingMode(int8_t opModeCode){
		processData.operatingModeSelection = opModeCode;
	}
	void setFrequency(int16_t frequency){
		processData.targetFrequency = frequency;
		processData.operatingModeSelection = OperatingMode::VELOCITY;
	}
	void setPosition(int32_t position){
		processData.targetPosition = position;
		processData.operatingModeSelection = OperatingMode::CYCLIC_SYNCHRONOUS_POSITION;
	}
	void setVelocity(int32_t velocity){
		processData.targetVelocity = velocity;
		processData.operatingModeSelection = OperatingMode::CYCLIC_SYNCHRONOUS_VELOCITY;
	}
	void setTorque(int16_t torque){
		processData.targetTorque = torque;
		processData.operatingModeSelection = OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE;
	}
		
	bool doHoming(){
		processData.operatingModeSelection = OperatingMode::HOMING;
		if(processData.operatingModeDisplay == OperatingMode::HOMING){
			if(processData.positionActualValue == 0x0){
				//homing is finished, reset the start flag
				setOperatingModeSpecificControlWorldBit_4(false);
				return true;
			}else{
				//homing is not done, set the start flag
				setOperatingModeSpecificControlWorldBit_4(true);
			}
		}
		return false;
	}
	
	void setOperatingModeSpecificControlWorldBit_4(bool bit){ controlWord_OpSpecBit_4 = bit; }
	void setOperatingModeSpecificControlWorldBit_5(bool bit){ controlWord_OpSpecBit_5 = bit; }
	void setOperatingModeSpecificControlWorldBit_6(bool bit){ controlWord_OpSpecBit_6 = bit; }
	void setOperatingModeSpecificControlWorldBit_9(bool bit){ controlWord_OpSpecBit_9 = bit; }
	
	void setManufacturerSpecificControlWordBit_11(bool bit){ controlWord_ManSpecBit_11 = bit; }
	void setManufacturerSpecificControlWordBit_12(bool bit){ controlWord_ManSpecBit_12 = bit; }
	void setManufacturerSpecificControlWordBit_13(bool bit){ controlWord_ManSpecBit_13 = bit; }
	void setManufacturerSpecificControlWordBit_14(bool bit){ controlWord_ManSpecBit_14 = bit; }
	void setManufacturerSpecificControlWordBit_15(bool bit){ controlWord_ManSpecBit_15 = bit; }
	
	bool getOperatingModeSpecificStatusWordBit_10(){ return statusWord_OpSpecBit_10; }
	bool getOperatingModeSpecificStatusWordBit_12(){ return statusWord_OpSpecBit_12; }
	bool getOperatingModeSpecificStatusWordBit_13(){ return statusWord_OpSpecBit_13; }
	
	bool getManufacturerSpecificStatusWordBit_8(){ return statusWord_ManSpecBit_8; }
	bool getManufacturerSpecificStatusWordBit_14(){ return statusWord_ManSpecBit_14; }
	bool getManufacturerSpecificStatusWordBit_15(){ return statusWord_ManSpecBit_15; }
	
private:
	
	static int8_t getOperatingModeCode(OperatingMode mode);
	static OperatingMode getOperatingMode(int8_t code);
	
	struct ProcessData{
		
		//———— Drive Operation
		uint16_t controlWord = 0;
		uint16_t statusWord = 0;
		int8_t operatingModeSelection = 0;
		int8_t operatingModeDisplay = 0;
		uint16_t errorCode = 0;
		
		//———— Frequency Converter
		int16_t targetFrequency = 0;
		int16_t frequencyActualValue = 0;
		
		//———— Position
		int32_t targetPosition = 0;
		int32_t positionActualValue = 0;
		int32_t positionFollowingErrorActualValue = 0;
		
		//———— Velocity
		int32_t targetVelocity = 0;
		int32_t velocityActualValue = 0;
		
		//———— Torque
		int16_t targetTorque = 0;
		int16_t torqueActualValue = 0;
		int16_t currentActualValue = 0;
		
		//———— Homing
		int32_t homeOffset = 0;

		//———— Inputs and Outputs
		uint32_t digitalInputs = 0;
		uint32_t digitalOutputs = 0;
		
	}processData;
	
	
	//FAULT RESET
	bool b_doFaultReset = false;
	bool b_faultResetBusy = false;
	
	//HOMING
	//bool b_isHoming = false;
	//bool b_didHomingSucceed = false;
	
	//CONTROL WORLD:
	//b0: Switch On
	//b1: Enable Voltage
	//b2: Quick Stop (inverted)
	//b3: Enable Operation
	//b7: Fault Reset
	//b8: Halt
	
	TargetPowerState powerStateTarget = TargetPowerState::DISABLED;
	
	bool controlWord_OpSpecBit_4 = false;	//(start homing)
	bool controlWord_OpSpecBit_5 = false;
	bool controlWord_OpSpecBit_6 = false;
	bool controlWord_OpSpecBit_9 = false;
	bool controlWord_ManSpecBit_11 = false;
	bool controlWord_ManSpecBit_12 = false;
	bool controlWord_ManSpecBit_13 = false;
	bool controlWord_ManSpecBit_14 = false;
	bool controlWord_ManSpecBit_15 = false;
	
	//STATUS WORD
	//b0: Ready To Switch On
	//b1: Switched On
	//b2: Operation Enabled
	//b3: Fault
	//b4: Voltage Enabled
	//b5: Quick Stop (inverted)
	//b6: Switch On Disabled
	//b7: Warning
	//b9: Remote Control Active
	//b11: Internal Limit Reached
	
	PowerState powerStateActual = PowerState::NOT_READY_TO_SWITCH_ON;
	bool b_hasFault = false;
	bool b_voltageEnabled = false;
	bool b_hasWarning = false;
	bool b_remoteControlActive = false;
	bool b_internalLimitReached = false;
	
	bool statusWord_ManSpecBit_8 = false;
	bool statusWord_OpSpecBit_10 = false; //status toggle (in csp csv cst cstca)
	bool statusWord_OpSpecBit_12 = false; //drive follows the command value (mandatory for csp csv cst cstca)
	bool statusWord_OpSpecBit_13 = false; //extended 2nd bit for status toggle (in csp csv cst cstca)
	bool statusWord_ManSpecBit_14 = false;
	bool statusWord_ManSpecBit_15 = false;
	
};

#define DS402AxisOperatingModeStrings \
	{DS402Axis::OperatingMode::NONE, 												"None", "None"},\
	{DS402Axis::OperatingMode::PROFILE_POSITION, 									"Profile Position", "ProfilePosition"},\
	{DS402Axis::OperatingMode::VELOCITY, 											"Velocity (Frequency Converter)", "Velocity"},\
	{DS402Axis::OperatingMode::PROFILE_VELOCITY, 									"Profile Velocity", "ProfileVelocity"},\
	{DS402Axis::OperatingMode::PROFILE_TORQUE, 										"Profile Torque", "ProfileTorque"},\
	{DS402Axis::OperatingMode::HOMING, 												"Homing", "Homing"},\
	{DS402Axis::OperatingMode::INTERPOLATED_POSITION, 								"Interpolated Position", "InterpolatedPosition"},\
	{DS402Axis::OperatingMode::CYCLIC_SYNCHRONOUS_POSITION, 						"Cyclic Synchronous Position", "CyclicSynchronousPosition"},\
	{DS402Axis::OperatingMode::CYCLIC_SYNCHRONOUS_VELOCITY, 						"Cyclic Synchronous Velocity", "CyclicSynchronousVelocity"},\
	{DS402Axis::OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE, 							"Cyclic Sychronous Torque", "CyclicSychronousTorque"},\
	{DS402Axis::OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE_WITH_COMMUTATION_ANGLE, 	"Cyclic Sychronous Torque with Commutation Angle", "CyclicSychronousTorquewithCommutationAngle"}\

DEFINE_ENUMERATOR(DS402Axis::OperatingMode, DS402AxisOperatingModeStrings)

#define DS402AxisPowerStateStrings \
	{DS402Axis::PowerState::NOT_READY_TO_SWITCH_ON, "Not Ready to Switch On", "NotReadytoSwitchOn"},\
	{DS402Axis::PowerState::SWITCH_ON_DISABLED, 	"Switch On Disabled", "SwitchOnDisabled"},\
	{DS402Axis::PowerState::READY_TO_SWITCH_ON, 	"Ready to Switch On", "ReadytoSwitchOn"},\
	{DS402Axis::PowerState::SWITCHED_ON, 			"Switched On", "SwitchedOn"},\
	{DS402Axis::PowerState::OPERATION_ENABLED, 		"Operation Enabled", "OperationEnabled"},\
	{DS402Axis::PowerState::QUICKSTOP_ACTIVE, 		"Quickstop Active", "QuickstopActive"},\
	{DS402Axis::PowerState::FAULT_REACTION_ACTIVE,	"Fault Reaction Active", "FaultReactionActive"},\
	{DS402Axis::PowerState::FAULT, 					"Fault", "Fault"}\

DEFINE_ENUMERATOR(DS402Axis::PowerState, DS402AxisPowerStateStrings)

#define DS402AxisTargetPowerStateStrings \
	{DS402Axis::TargetPowerState::ENABLED,			"Enabled",			"Enabled"},\
	{DS402Axis::TargetPowerState::DISABLED,			"Disabled",			"Disabled"},\
	{DS402Axis::TargetPowerState::QUICKSTOP_ACTIVE,	"Quickstop Active",	"QuickstopActive"}\

DEFINE_ENUMERATOR(DS402Axis::TargetPowerState, DS402AxisTargetPowerStateStrings)
