#pragma once

#include <memory>
#include "Fieldbus/EtherCatDevice.h"

struct EtherCatPdoAssignement;

class DS402Axis{
private:
	
	DS402Axis(std::shared_ptr<EtherCatDevice> parentDevice_) : parentDevice(parentDevice_){}
	std::shared_ptr<EtherCatDevice> parentDevice = nullptr;
	
public:
	
	static std::shared_ptr<DS402Axis> make(std::shared_ptr<EtherCatDevice> parentDevice_){
		return std::shared_ptr<DS402Axis>(new DS402Axis(parentDevice_));
	}

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
	
	
	//—————————————————————————————————————————————————
	//—————————————— PDO Configuration ————————————————
	//—————————————————————————————————————————————————
	
	struct ProcessDataConfiguration{
		
		struct OperatingModes{
			bool frequency = false;
			bool cyclicSynchronousPosition = false;
			bool cyclicSynchronousVelocity = false;
			bool cyclicSynchronousTorque = false;
			//bool cyclicSycnhronousTorqueWithCommutationAngle = false;
		}operatingModes;
		
		//———— Errors
		
		///603F.0 Error Code
		bool errorCode = false;
		
		//———— Frequency Converter
		
		///6042.0 Target Velocity (frequency converter mode)
		bool targetFrequency = false;
		
		///6044.0 Velocity Actual Value (frequency converter mode)
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
	void updateInputs();
	void updateOutput();

	//—————————————————————————————————————————————————
	//—————————————— SDO Configuration ————————————————
	//—————————————————————————————————————————————————
	
	//=== Drive Operation
	
	///60C2.? Interpolation Time Period

	///607E.0 Polarity
	bool setPolarity(uint8_t polarity){ return parentDevice->writeSDO_U8(0x607E, 0x0, polarity); }
	bool getPolarity(uint8_t& output){ return parentDevice->readSDO_U8(0x607E, 0x0, output); }

	///6073.0 Max Current
	bool setMaxCurrent(uint16_t current){ return parentDevice->writeSDO_U16(0x6073, 0x0, current); }
	bool getMaxCurrent(uint16_t& output){ return parentDevice->readSDO_U16(0x6073, 0x0, output); }
	
	///6075.0 Motor Rated Current
	bool getMotorRatedCurrent(uint32_t& output) { return parentDevice->readSDO_U32(0x6075, 0x0, output); }
	
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
	
	
	
	
	//=== Frequency
	
	///6046.? vl velocity min max amount
	
	///6048.? velocity acceleration
	
	///6049.? velocity deceleration
	
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
	
	bool hasVoltage(){ return b_voltageEnabled; }
	bool hasWarning(){ return b_hasWarning; }
	bool hasFault(){ return b_hasFault; }
	bool isReady(){ return b_isReady; }
	bool isEnabled(){ return b_isEnabled; }
	bool isQuickstopActive(){ return b_isQuickstop; }
	
	void enable(){ b_shouldEnable = true; }
	void disable(){ b_shouldDisable = true; }
	void doFaultReset(){ b_shouldFaultReset = true; }
	void doQuickstop(){ b_shouldQuickstop = true; }
	
	PowerState getActualPowerState(){ return powerStateActual; }
	OperatingMode getActualOperatingMode(){ return operatingModeActual; }
	uint16_t getErrorCode(){ return processData.errorCode; }
	
	int16_t getActualFrequency(){ return processData.frequencyActualValue; }
	int32_t getActualPosition(){ return processData.positionActualValue; }
	int32_t getActualVelocity(){ return processData.velocityActualValue; }
	int16_t getActualTorque(){ return processData.torqueActualValue; }
	
	void setFrequency(int16_t frequency){
		processData.targetFrequency = frequency;
		operatingModeTarget = OperatingMode::VELOCITY;
	}
	void setPosition(int32_t position){
		processData.targetPosition = position;
		operatingModeTarget = OperatingMode::CYCLIC_SYNCHRONOUS_POSITION;
	}
	void setVelocity(int32_t velocity){
		processData.targetVelocity = velocity;
		operatingModeTarget = OperatingMode::CYCLIC_SYNCHRONOUS_VELOCITY;
	}
	void setTorque(int16_t torque){
		processData.targetTorque = torque;
		operatingModeTarget = OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE;
	}
	
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
		b8 = (processData.statusWord >> 8) & 0x1;
		b14 = (processData.statusWord >> 14) & 0x1;
		b15 = (processData.statusWord >> 15) & 0x1;
	}
	
private:
	
	struct ProcessData{
		
		//———— Drive Operation
		uint16_t controlWord;
		uint16_t statusWord;
		int8_t operatingModeSelection;
		int8_t operatingModeDisplay;
		uint16_t errorCode;
		
		//———— Frequency Converter
		int16_t targetFrequency;
		int16_t frequencyActualValue;
		
		//———— Position
		int32_t targetPosition;
		int32_t positionActualValue;
		int32_t positionFollowingErrorActualValue;
		
		//———— Velocity
		int32_t targetVelocity;
		int32_t velocityActualValue;
		
		//———— Torque
		int16_t targetTorque;
		int16_t torqueActualValue;
		int16_t currentActualValue;

		//———— Inputs and Outputs
		uint32_t digitalInputs;
		uint32_t digitalOutputs;
		
	}processData;
	
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
