#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/DS402Axis.h"

#include "Legato/Editor/Parameters.h"

class ATV340 : public EtherCatDevice {
public:

	DEFINE_ETHERCAT_DEVICE(ATV340, "Schneider Electric", "Motor Drives", 0x800005A, 0x12C)
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		EtherCatDevice::onCopyFrom(source);
	};
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	
	//Process Data
	std::shared_ptr<DS402Axis> axis;
	int16_t motorEffort = 0;
	uint16_t logicInputs = 0;
	uint16_t stoState = 0;
	uint16_t lastFaultCode = 0;
	uint16_t logicOutputs = 0;
	int16_t analogInput1 = 0;
	int16_t analogInput2 = 0;
	
	class ATV340_Motor : public ActuatorInterface{
	public:
		ATV340_Motor(std::shared_ptr<ATV340> drive_) : drive(drive_){}
		std::shared_ptr<ATV340> drive;
		
		virtual std::string getName() override { return std::string(drive->getName()) + " Motor"; };
		virtual std::string getStatusString() override { return drive->getStatusString(); }
	};
	
	class ATV340_GPIO : public GpioInterface{
	public:
		ATV340_GPIO(std::shared_ptr<ATV340> drive_) : drive(drive_){}
		std::shared_ptr<ATV340> drive;
		
		virtual std::string getName() override { return std::string(drive->getName()) + " GPIO"; };
		virtual std::string getStatusString() override { return drive->getStatusString(); }
	};
	
	std::string getStatusString(){
		return "not impleted...";
	}
	
	int manualVelocityTarget_rpm = 0;
	bool b_waitingForEnable = false;
	long long enableRequestTime_nanoseconds = 0;
	
	//———— Node Pins
	
	std::shared_ptr<ATV340_Motor> motor;
	std::shared_ptr<ATV340_GPIO> gpio;
	std::shared_ptr<NodePin> motor_pin;
	std::shared_ptr<NodePin> gpio_pin;
	
	std::shared_ptr<double> velocity_Value = std::make_shared<double>(0.0);
	std::shared_ptr<double> load_Value = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> velocity_Pin;
	std::shared_ptr<NodePin> load_Pin;
	
	std::shared_ptr<bool> digitalInput1_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput2_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput3_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput4_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput5_Signal = std::make_shared<bool>(false);
	std::shared_ptr<NodePin> digitalInput1_Pin;
	std::shared_ptr<NodePin> digitalInput2_Pin;
	std::shared_ptr<NodePin> digitalInput3_Pin;
	std::shared_ptr<NodePin> digitalInput4_Pin;
	std::shared_ptr<NodePin> digitalInput5_Pin;
	
	std::shared_ptr<double> analogInput1_value = std::make_shared<double>(0.0);
	std::shared_ptr<double> analogInput2_value = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> analogInput1_pin;
	std::shared_ptr<NodePin> analogInput2_pin;
	
	std::shared_ptr<bool> digitalOutput1_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalOutput2_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> relaisOutput1_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> relaisOutput2_Signal = std::make_shared<bool>(false);
	std::shared_ptr<NodePin> digitalOutput1_Pin;
	std::shared_ptr<NodePin> digitalOutput2_Pin;
	std::shared_ptr<NodePin> relaisOutput1_Pin;
	std::shared_ptr<NodePin> relaisOutput2_Pin;
		
	//———— Parameter for the following parameters
	
	struct ParameterOptions{
		
		//[BFR]
		Legato::Option Hz50 = Legato::Option(0, "50 Hz", "50Hz");
		Legato::Option Hz60 = Legato::Option(1, "60 Hz", "60Hz");
		std::vector<Legato::Option*> motorStandardFrequency_Options = {
			&Hz50,
			&Hz60
		};

		//[MPC]
		Legato::Option NominalPower = Legato::Option(0, "Nominal Power", "NominalPower");
		Legato::Option CosinusPhi = Legato::Option(1, "Cosinus Phi", "CosinusPhi");
		std::vector<Legato::Option*> motorParameterChoice_Options = {
			&NominalPower,
			&CosinusPhi
		};
		
		//[BLC]
		Legato::Option NoDigitalOutput = Legato::Option(0, "None", "None");
		Legato::Option R2 = Legato::Option(2, 	"R2", 	"R2");
		Legato::Option DQ1 = Legato::Option(64, "DQ0", 	"DQ0");
		Legato::Option DQ2 = Legato::Option(65, "DQ1", 	"DQ1");
		std::vector<Legato::Option*> digitalOutput_Options = {
			&NoDigitalOutput,
			&R2,
			&DQ1,
			&DQ2
		};
		
		//[BST]
		Legato::Option HorizontalMovement = Legato::Option(0, "Horizontal Travel", "HorizontalTravel");
		Legato::Option Hoisting = Legato::Option(1, "Hoisting", "Hoisting");
		std::vector<Legato::Option*> brakeMovementType_Options = {
			&HorizontalMovement,
			&Hoisting
		};
		
		//[EECP]
		Legato::Option EmbeddedEncoderTypeNone = Legato::Option(0, "No Embedded Encoder", "None");
		Legato::Option EmbeddedEncoderTypeAB = Legato::Option(1, "AB Incremental Encoder", "AB");
		std::vector<Legato::Option*> embeddedEncoderType_Options = {
			&EmbeddedEncoderTypeNone,
			&EmbeddedEncoderTypeAB
		};
		
		//[EECV]
		Legato::Option EmbeddedEncoder5V = Legato::Option(5, "5V", "5V");
		Legato::Option EmbeddedEncoder12V = Legato::Option(12, "12V", "12V");
		Legato::Option EmbeddedEncoder24V = Legato::Option(24, "24V", "24V");
		std::vector<Legato::Option*> embeddedEncoderVoltage_Options = {
			&EmbeddedEncoder5V,
			&EmbeddedEncoder12V,
			&EmbeddedEncoder24V
		};
		
		//[EENU] embedded encoder usage (0=None, 1=SpeedMonitoring, 2=SpeedRegulation, 3=SpeedReference)
		Legato::Option EmbeddedEncoderNoUsage = Legato::Option(0, "No Usage", "NoUsage");
		Legato::Option EmbeddedEncoderSpeedMonitoring = Legato::Option(1, "Speed Monitoring", "SpeedMonitoring");
		Legato::Option EmbeddedEncoderSpeedRegulation = Legato::Option(2, "Speed Regulation", "SpeedRegulation");
		std::vector<Legato::Option*> embeddedEncoderUsage_Options = {
			&EmbeddedEncoderNoUsage,
			&EmbeddedEncoderSpeedMonitoring,
			&EmbeddedEncoderSpeedRegulation
		};
		
		//[CTT]
		Legato::Option SensorlessFullFlux = Legato::Option(0, "Sensorless Full Flux (Open Loop)", "SensorlessFullFlux");
		Legato::Option FullFluxVector = Legato::Option(2, "Full Flux Vector (Closed Loop)", "FullFluxVector");
		std::vector<Legato::Option*> motorControlType_Options = {
			&SensorlessFullFlux,
			&FullFluxVector
		};
		
		
		//[AIOT]
		Legato::Option AnalogInputTypeVoltage = Legato::Option(1, "Measure Voltage", "MeasureVoltage");
		Legato::Option AnalogInputTypeCurrent = Legato::Option(2, "Measure Current", "MeasureCurrent");
		Legato::Option AnalogInputTypeVoltageBipolar = Legato::Option(5, "Measure Voltage ±10V", "MeasureVoltageBipolar10V");
		std::vector<Legato::Option*> analogInput1Type_Options = {
			&AnalogInputTypeVoltage,
			&AnalogInputTypeCurrent
		};
		std::vector<Legato::Option*> analogInput2Type_Options = {
			&AnalogInputTypeVoltage,
			&AnalogInputTypeVoltageBipolar
		};
		
		//[PSLIN]
		Legato::Option NoDigitalInput = Legato::Option(0, "None", "NoInputAssignement");
		Legato::Option DI1_High = Legato::Option(129, "DI1 (High Level)", "DI1High");
		Legato::Option DI2_High = Legato::Option(130, "DI2 (High Level)", "DI2High");
		Legato::Option DI3_High = Legato::Option(131, "DI3 (High Level)", "DI3High");
		Legato::Option DI4_High = Legato::Option(132, "DI4 (High Level)", "DI4High");
		Legato::Option DI5_High = Legato::Option(133, "DI5 (High Level)", "DI5High");
		Legato::Option DI1_Low = Legato::Option(272, "DI1 (Low Level)", "DI1Low");
		Legato::Option DI2_Low = Legato::Option(273, "DI2 (Low Level)", "DI2Low");
		Legato::Option DI3_Low = Legato::Option(274, "DI3 (Low Level)", "DI3Low");
		Legato::Option DI4_Low = Legato::Option(275, "DI4 (Low Level)", "DI4Low");
		Legato::Option DI5_Low = Legato::Option(276, "DI5 (Low Level)", "DI5Low");
		std::vector<Legato::Option*> digitalInput_Options = {
			&NoDigitalInput,
			&DI1_High,
			&DI1_Low,
			&DI2_High,
			&DI2_Low,
			&DI3_High,
			&DI3_Low,
			&DI4_High,
			&DI4_Low,
			&DI5_High,
			&DI5_Low
		};
		
	};
	static ParameterOptions options;
	
	
	//———— Process Data
	
	Legato::BoolParam pdo_digitalIn;
	Legato::BoolParam pdo_digitalOut;
	Legato::BoolParam pdo_readAnalogIn1;
	Legato::BoolParam pdo_readAnalogIn2;
	Legato::BoolParam pdo_motorVelocity;
	Legato::BoolParam pdo_motorEffort;
	Legato::ParamGroup pdoConfigParameters;
	
	
	Legato::BoolParam invertDigitalInput1_Param;
	Legato::BoolParam invertDigitalInput2_Param;
	Legato::BoolParam invertDigitalInput3_Param;
	Legato::BoolParam invertDigitalInput4_Param;
	Legato::BoolParam invertDigitalInput5_Param;
	Legato::BoolParam invertDigitalOutput1_Param;
	Legato::BoolParam invertDigitalOutput2_Param;
	Legato::BoolParam invertRelay1_Param;
	Legato::BoolParam invertRelay2_Param;
	Legato::ParamGroup digitalSignalInversion;
	
	
	//———— Motor Nameplate
	
	//[bfr] {Async} motor standard frequency
	Legato::OptionParam motorStandartFrequency_Param;
	//[mpc] motor parameter choice (0=NominalPower,1=NominalCosinusPhi)
	Legato::OptionParam motorParameterChoice_Param;
	//[cos] {Async} motor 1 cosinus phi (0.01 increments)
	Legato::NumberParam<double> cosinusPhi_Param;
	//[npr] {Async} nominal motor power (0.01 Watt increments)
	Legato::NumberParam<double> nominalMotorPower_Param;
	//[uns] {Async} nominal motor voltage (1v increments)
	Legato::NumberParam<int> nominalMotorVoltage_Param;
	//[ncr] {Async} nominal motor current (0.01 Ampere increments)
	Legato::NumberParam<double> nominalMotorCurrent_Param;
	//[frs] {Async} nominal motor frequency (0.1Hz increments)
	Legato::NumberParam<double> nominalMotorFrequency_Param;
	//[nsp] {Async} nominal motor speed (rpm)
	Legato::NumberParam<int> nominalMotorSpeed_Param;
	//[ith] Motor Thermal Current (0.01 Ampere increments)
	Legato::NumberParam<double> motorThermalCurrent_Param;
	//[tfr] Motor Maximum Frequency (0.1 Hz increments)
	Legato::NumberParam<double> motorMaximumFrequency_Param;
	Legato::ParamGroup motorNameplateParameters;
	
	//———— Brake Logic
	
	//[blc] brake assignement
	Legato::OptionParam brakeOutputAssignement_Param;
	//[bst] movement type (0= Horizontal Movement, 1=Hoisting)
	Legato::OptionParam brakeMovementType_Param;
	//[brt] brake release time
	Legato::NumberParam<double> brakeReleaseTime_Param;
	//[bet] brake engage time
	Legato::NumberParam<double> brakeEngageTime_Param;
	//[ibr] brake release current (default is nominal motor current)
	Legato::NumberParam<double> brakeReleaseCurrent_Param;
	//[bir] brake release frequency (default is 0Hz)
	Legato::NumberParam<double> brakeReleaseFrequency_Param;
	//[ben] brake engage frequency (default is 0Hz)
	Legato::NumberParam<double> brakeEngageFrequency_Param;
	Legato::ParamGroup brakeLogicParameters;
	
	//———— Embedded Encoder
	
	//[eecp] embedded encoder etype (0=None, 1=AB, 2=SinCos)
	Legato::OptionParam embeddedEncoderType_Param;
	//[eecv] embedded encoder supply voltage (5=5V, 12=12V, 24=24V)
	Legato::OptionParam embeddedEncoderVoltage_Param;
	//[epg] pulses per encoder revolution
	Legato::NumberParam<int> embeddedEncoderPulsesPerRevolution_Param;
	//[eeri] embedded encoder revolution inversion (0=No, 1=Yes)
	Legato::BoolParam embeddedEncoderInvertDirection_Param;
	//[eenu] embedded encoder usage (0=None, 1=SpeedMonitoring, 2=SpeedRegulation, 3=SpeedReference)
	Legato::OptionParam embeddedEncoderUsage_Param;
	Legato::ParamGroup embeddedEncoderParameters;
	
	//———— Motor Control
	
	//[ctt] Motor Control Type
	Legato::OptionParam motorControlType_Param;
	Legato::NumberParam<int> velocityLimitRPM_Param;
	Legato::NumberParam<double> accelerationRampTime_Param;
	Legato::NumberParam<double> decelerationRampTime_Param;
	Legato::NumberParam<double> switchingFrequency_Param;
	Legato::ParamGroup motorControlParameters;
	
	//———— Analog IO Configuration
	
	//[AI1T]
	Legato::OptionParam analogInput1Type_Param;
	//[AI2T]
	Legato::OptionParam analogInput2Type_Param;
	//[CrL1]
	Legato::NumberParam<double> analogInputMinCurrent_Param;
	//[CrH1]
	Legato::NumberParam<double> analogInputMaxCurrent_Param;
	//[UIL1] = [UIL2] if voltage is selected
	Legato::NumberParam<double> analogInputMinVoltage_Param;
	//[UIH1] = [UIH2] if voltage is selected
	Legato::NumberParam<double> analogInputMaxVoltage_Param;
	Legato::ParamGroup analogIoConfigParameters;
	
	//———— Digital IO Configuration
	
	//[LAF] Stop forward limit assignement
	Legato::OptionParam forwardLimitSignal_Param;
	//[LAR] Stop Reverse limit assignement
	Legato::OptionParam reverseLimitSignal_Param;
	Legato::ParamGroup digitalIoConfigParameters;
	
	
	void configureDrive();
	void startStandardTuning();
	void startRotationTuning();
	void resetFactorySettings();
	
	bool saveToEEPROM();
	
	void configureProcessData();
	
	const char* getErrorCodeString(){
		switch(lastFaultCode){
			case 0: return "No error detected";
			case 2: return "Control memory error";
			case 3: return "Incorrect configuration";
			case 4: return "Invalid configuration";
			case 5: return "Modbus communication interruption";
			case 6: return "Internal communication interruption with option module";
			case 7: return "Fieldbus communication interruption";
			case 8: return "External detected error";
			case 9: return "Overcurrent";
			case 10: return "Precharge capacitor";
			case 11: return "Encoder feedback loss";
			case 12: return "Load slipping";
			case 15: return "Input Overheating";
			case 16: return "Device overheating";
			case 17: return "Motor overload";
			case 18: return "DC bus overvoltage";
			case 19: return "Supply mains overvoltage";
			case 20: return "Single output phase loss";
			case 21: return "Input phase loss";
			case 22: return "Supply mains undervoltage";
			case 23: return "Motor short circuit";
			case 24: return "Motor overspeed";
			case 25: return "Autotuning detected error";
			case 26: return "Internal error 1 (Rating)";
			case 27: return "Internal error 2 (Soft)";
			case 28: return "Internal error 3 (Intern Comm)";
			case 29: return "Internal error 4 (Manufacturing)";
			case 30: return "Power memory error";
			case 32: return "Ground short circuit";
			case 33: return "Output phase loss";
			case 34: return "CANopen communication interruption";
			case 35: return "Brake control";
			case 37: return "Internal error 7 (Init)";
			case 38: return "External error detected by Fieldbus";
			case 40: return "Internal error 8 (Switching Supply)";
			case 41: return "Brake feedback";
			case 42: return "PC communication interruption";
			case 43: return "Encoder coupling";
			case 44: return "Torque limitation error";
			case 45: return "HMI communication interruption";
			case 51: return "Internal error 9 (Measure)";
			case 52: return "Internal error 10 (Mains)";
			case 53: return "Internal error 11 (Temperature)";
			case 54: return "IGBT overheating";
			case 55: return "IGBT short circuit";
			case 56: return "Motor short circuit";
			case 57: return "Torque timeout";
			case 58: return "Output contactor closed error";
			case 59: return "Output contactor opened error";
			case 60: return "Internal error 12 (Internal current supply)";
			case 62: return "Encoder";
			case 64: return "input contactor";
			case 65: return "DB unit sh. circuit";
			case 68: return "Internal error 6 (Option)";
			case 69: return "Internal error 14 (CPU)";
			case 70: return "Braking resistor overload";
			case 71: return "AI3 4-20mA loss";
			case 72: return "AI4 4-20mA loss";
			case 73: return "Boards compatibility";
			case 76: return "Dynamic load detected error";
			case 77: return "Configuration transfer error";
			case 79: return "AI5 4-20 mA loss";
			case 82: return "ON lock error";
			case 99: return "Channel switching detected error";
			case 100: return "Process underload";
			case 101: return "Process overload";
			case 105: return "Angle error";
			case 106: return "AI1 4-20 mA loss";
			case 107: return "Safety function detected error";
			case 112: return "AI3 thermal level error";
			case 113: return "Thermal sensor error on AI3";
			case 120: return "AI4 thermal level error";
			case 121: return "Thermal sensor error on AI4";
			case 122: return "AI5 thermal level error";
			case 123: return "Thermal sensor error on AI5";
			case 128: return "Program loading detected error";
			case 129: return "Program running detected error";
			case 142: return "Internal error 16 (IO module - relay)";
			case 143: return "Internal error 17 (IO module - Standard)";
			case 144: return "Internal error 0 (IPC)";
			case 146: return "Internal error 13 (Diff current)";
			case 148: return "Motor stall detected error";
			case 149: return "Internal error 21 (RTC)";
			case 150: return "Embedded Ethernet communication interruption";
			case 151: return "Internal error 15 (Flash)";
			case 152: return "Firmware Update error";
			case 153: return "Internal error 22 (Embedded Ethernet)";
			case 154: return "Internal error 25 (Incompatibility CB & SW)";
			case 155: return "Internal error 20 (option interface PCBA)";
			case 156: return "Internal error 19 (Encoder module)";
			case 157: return "Internal error 27 (Diagnostics CPLD)";
			case 158: return "Internal error 23 (Module link)";
			case 159: return "AFE ShortCircuit error";
			case 160: return "AFE Bus unbalancing";
			case 161: return "Internal error 28 (AFE)";
			case 162: return "Monitoring circuit A error";
			case 163: return "Monitoring circuit B error";
			case 164: return "Monitoring circuit C error";
			case 165: return "Monitoring circuit D error";
			case 173: return "Cabinet overheat  error";
			case 175: return "Internal error 31 (Missing brick)";
			case 176: return "Internal error 29 (Inverter)";
			case 177: return "Internal error 30 (Rectifier)";
			case 179: return "AFE IGBT over-heat error";
			case 180: return "AFE contactor feedback error";
			case 181: return "Pre-settings transfer error";
			case 182: return "Circuit breaker error";
			case 184: return "DB unit open circuit";
			case 185: return "Drive overload";
			case 186: return "MultiDrive Link error";
			case 187: return "AI1 thermal level error";
			case 188: return "Thermal sensor error on AI1";
			case 189: return "Backlash error";
			case 190: return "M/S device error";
			case 194: return "Encoder module thermal level error";
			case 195: return "Thermal sensor error on encoder module";
			case 196: return "Empty configuration";
			case 200: return "FDR Eth embedded error";
			case 201: return "FDR Eth module error";
			case 206: return "DC Bus ripple error";
			case 208: return "Idle mode exit error";
			case 209: return "Load movement error";
			case 211: return "Security files corrupt";
			case 220: return "Advanced function locked error";
			default: return "unknown error";
		}
	}
	
	void controlTab();
	void processDataConfigTab();
	void driveConfigTab();
	
};
