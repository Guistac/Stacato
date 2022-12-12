#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/DS402Axis.h"

#include "Project/Editor/Parameter.h"

class ATV340 : public EtherCatDevice {
public:

	DEFINE_ETHERCAT_DEVICE(ATV340, "ATV340", "ATV340", "Schneider Electric", "Servo Drives", 0x800005A, 0x12C)
	
	//Process Data
	std::shared_ptr<DS402Axis> axis;
	uint16_t motorPower;
	uint16_t logicInputs;
	uint16_t stoState;
	uint16_t lastFaultCode;
	uint16_t logicOutputs;
	int16_t analogInput1;
	int16_t analogInput2;
	
	class ATV340_Motor : public ActuatorDevice{
	public:
		
		ATV340_Motor(std::shared_ptr<ATV340> drive_) :
		MotionDevice(Units::AngularDistance::Revolution),
		ActuatorDevice(Units::AngularDistance::Revolution),
		drive(drive_){}
		
		virtual std::string getName() override { return std::string(drive->getName()) + " Motor"; };
		virtual std::string getStatusString() override { return drive->getStatusString(); }
		std::shared_ptr<ATV340> drive;
	};
	
	class ATV340_GPIO : public GpioDevice{
	public:
		
		ATV340_GPIO(std::shared_ptr<ATV340> drive_) :
		GpioDevice(),
		drive(drive_){}
		
		virtual std::string getName() override { return std::string(drive->getName()) + " GPIO"; };
		virtual std::string getStatusString() override { return drive->getStatusString(); }
		std::shared_ptr<ATV340> drive;
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
	std::shared_ptr<NodePin> motor_pin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Actuator");
	std::shared_ptr<NodePin> gpio_pin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Gpio");
	
	std::shared_ptr<bool> digitalInput1_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput2_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput3_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput4_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput5_Signal = std::make_shared<bool>(false);
	std::shared_ptr<NodePin> digitalInput1_Pin = std::make_shared<NodePin>(digitalInput1_Signal, NodePin::Direction::NODE_OUTPUT, "Digital Input 1", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalInput2_Pin = std::make_shared<NodePin>(digitalInput2_Signal, NodePin::Direction::NODE_OUTPUT, "Digital Input 2", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalInput3_Pin = std::make_shared<NodePin>(digitalInput3_Signal, NodePin::Direction::NODE_OUTPUT, "Digital Input 3", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalInput4_Pin = std::make_shared<NodePin>(digitalInput4_Signal, NodePin::Direction::NODE_OUTPUT, "Digital Input 4", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalInput5_Pin = std::make_shared<NodePin>(digitalInput5_Signal, NodePin::Direction::NODE_OUTPUT, "Digital Input 5", NodePin::Flags::DisableDataField);
	
	std::shared_ptr<double> analogInput1_value = std::make_shared<double>(0.0);
	std::shared_ptr<double> analogInput2_value = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> analogInput1_pin = std::make_shared<NodePin>(analogInput1_value, NodePin::Direction::NODE_OUTPUT, "Analog Input 1", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> analogInput2_pin = std::make_shared<NodePin>(analogInput2_value, NodePin::Direction::NODE_OUTPUT, "Analog Input 2", NodePin::Flags::DisableDataField);
	
	std::shared_ptr<bool> digitalOutput1_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalOutput2_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> relaisOutput1_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> relaisOutput2_Signal = std::make_shared<bool>(false);
	std::shared_ptr<NodePin> digitalOutput1_Pin = std::make_shared<NodePin>(digitalOutput1_Signal, NodePin::Direction::NODE_INPUT, "Digital Output 1");
	std::shared_ptr<NodePin> digitalOutput2_Pin = std::make_shared<NodePin>(digitalOutput2_Signal, NodePin::Direction::NODE_INPUT, "Digital Output 2");
	std::shared_ptr<NodePin> relaisOutput1_Pin = std::make_shared<NodePin>(relaisOutput1_Signal, NodePin::Direction::NODE_INPUT, "Relais Output 1");
	std::shared_ptr<NodePin> relaisOutput2_Pin = std::make_shared<NodePin>(relaisOutput2_Signal, NodePin::Direction::NODE_INPUT, "Relais Output 2");
	
	//———— PDO Parameters
	
	BoolParam pdo_digitalIn = BooleanParameter::make(false, "Read Digital Inputs", "ConfigDigitalInputs");
	BoolParam pdo_digitalOut = BooleanParameter::make(false, "Write Digital Outputs", "ConfigDigitalOutputs");
	BoolParam pdo_motorPower = BooleanParameter::make(false, "Read Motor Power", "ConfigMotorPower");
	BoolParam pdo_readMotorSpeed = BooleanParameter::make(false, "Read Motor Speed", "ConfigMotorSpeed");
	BoolParam pdo_readAnalogIn1 = BooleanParameter::make(false, "Read Analog Input 1", "ConfigAnalogInput1");
	BoolParam pdo_readAnalogIn2 = BooleanParameter::make(false, "Read Analog Input 2", "ConfigAnalogInput2");
	ParameterGroup pdoConfigParameters = ParameterGroup("PDOconfig",{
		pdo_digitalIn,
		pdo_digitalOut,
		pdo_readAnalogIn1,
		pdo_readAnalogIn2,
		pdo_motorPower,
		pdo_readMotorSpeed
	});
	
	
	//———— Parameter for the following parameters
	
	struct ParameterOptions{
		//[BFR]
		Option Hz50 = Option(0, "50 Hz", "50Hz");
		Option Hz60 = Option(1, "60 Hz", "60Hz");
		std::vector<Option*> motorStandardFrequency_Options = {
			&Hz50,
			&Hz60
		};

		//[MPC]
		Option NominalPower = Option(0, "Nominal Power", "NominalPower");
		Option CosinusPhi = Option(1, "Cosinus Phi", "CosinusPhi");
		std::vector<Option*> motorParameterChoice_Options = {
			&NominalPower,
			&CosinusPhi
		};
		
		//[BLC]
		Option NoDigitalOutput = Option(0, "None", "None");
		Option R2 = Option(2, 	"R2", 	"R2");
		Option DQ1 = Option(64, "DQ0", 	"DQ0");
		Option DQ2 = Option(65, "DQ1", 	"DQ1");
		std::vector<Option*> digitalOutput_Options = {
			&NoDigitalOutput,
			&R2,
			&DQ1,
			&DQ2
		};
		
		//[BST]
		Option HorizontalMovement = Option(0, "Horizontal Travel", "HorizontalTravel");
		Option Hoisting = Option(1, "Hoisting", "Hoisting");
		std::vector<Option*> brakeMovementType_Options = {
			&HorizontalMovement,
			&Hoisting
		};
		
		//[EECP]
		Option EmbeddedEncoderTypeNone = Option(0, "No Embedded Encoder", "None");
		Option EmbeddedEncoderTypeAB = Option(1, "AB Incremental Encoder", "AB");
		std::vector<Option*> embeddedEncoderType_Options = {
			&EmbeddedEncoderTypeNone,
			&EmbeddedEncoderTypeAB
		};
		
		//[EECV]
		Option EmbeddedEncoder5V = Option(5, "5V", "5V");
		Option EmbeddedEncoder12V = Option(12, "12V", "12V");
		Option EmbeddedEncoder24V = Option(24, "24V", "24V");
		std::vector<Option*> embeddedEncoderVoltage_Options = {
			&EmbeddedEncoder5V,
			&EmbeddedEncoder12V,
			&EmbeddedEncoder24V
		};
		
		//[EENU] embedded encoder usage (0=None, 1=SpeedMonitoring, 2=SpeedRegulation, 3=SpeedReference)
		Option EmbeddedEncoderNoUsage = Option(0, "No Usage", "NoUsage");
		Option EmbeddedEncoderSpeedMonitoring = Option(1, "Speed Monitoring", "SpeedMonitoring");
		Option EmbeddedEncoderSpeedRegulation = Option(2, "Speed Regulation", "SpeedRegulation");
		std::vector<Option*> embeddedEncoderUsage_Options = {
			&EmbeddedEncoderNoUsage,
			&EmbeddedEncoderSpeedMonitoring,
			&EmbeddedEncoderSpeedRegulation
		};
		
		//[CTT]
		Option SensorlessFullFlux = Option(0, "Sensorless Full Flux (Open Loop)", "SensorlessFullFlux");
		Option FullFluxVector = Option(2, "Full Flux Vector (Closed Loop)", "FullFluxVector");
		std::vector<Option*> motorControlType_Options = {
			&SensorlessFullFlux,
			&FullFluxVector
		};
		
		
		//[AIOT]
		Option AnalogInputTypeVoltage = Option(1, "Measure Voltage", "MeasureVoltage");
		Option AnalogInputTypeCurrent = Option(2, "Measure Current", "MeasureCurrent");
		Option AnalogInputTypeVoltageBipolar = Option(5, "Measure Voltage ±10V", "MeasureVoltageBipolar10V");
		std::vector<Option*> analogInput1Type_Options = {
			&AnalogInputTypeVoltage,
			&AnalogInputTypeCurrent
		};
		std::vector<Option*> analogInput2Type_Options = {
			&AnalogInputTypeVoltage,
			&AnalogInputTypeVoltageBipolar
		};
		
		
		
		
	};
	static ParameterOptions options;
	
	
	
	
	//———— Motor Nameplate
	
	//[bfr] {Async} motor standard frequency
	OptionParam motorStandartFrequency_Param = OptionParameter::make(options.Hz50, options.motorStandardFrequency_Options, "Motor Standard Frequency", "MotorStandardFrequency");
	
	//[mpc] motor parameter choice (0=NominalPower,1=NominalCosinusPhi)
	OptionParam motorParameterChoice_Param = OptionParameter::make(options.NominalPower, options.motorParameterChoice_Options, "Motor Parameter Choice", "MotorParameterChoice");
	
	//[cos] {Async} motor 1 cosinus phi (0.01 increments)
	NumberParam<double> cosinusPhi_Param = NumberParameter<double>::make(0.0, "Cosinus Phi", "CosinusPhi", "%.2f", Units::None::None, false);
	
	//[npr] {Async} nominal motor power (0.01 Watt increments)
	NumberParam<double> nominalMotorPower_Param = NumberParameter<double>::make(0.0, "Nominal Motor Power", "NominalMotorPower", "%.2f", Units::Power::KiloWatt, false);
	
	//[uns] {Async} nominal motor voltage (1v increments)
	NumberParam<int> nominalMotorVoltage_Param = NumberParameter<int>::make(0, "Nominal Motor Voltage", "NominalMotorVoltage", "%i", Units::Voltage::Volt, false);
	
	//[ncr] {Async} nominal motor current (0.01 Ampere increments)
	NumberParam<double> nominalMotorCurrent_Param = NumberParameter<double>::make(0.0, "Nominal Motor Current", "NominalMotorCurrent", "%.2f", Units::Current::Ampere, false);

	//[frs] {Async} nominal motor frequency (0.1Hz increments)
	NumberParam<double> nominalMotorFrequency_Param = NumberParameter<double>::make(0.0, "Nominal Motor Frequency", "NominalMotorFrequency", "%.2f", Units::Frequency::Hertz, false);
	
	//[nsp] {Async} nominal motor speed (rpm)
	NumberParam<int> nominalMotorSpeed_Param = NumberParameter<int>::make(0, "Nominal Motor Speed", "NominalMotorSpeed", "%i rpm", Units::None::None, false);
	
	//[ith] Motor Thermal Current (0.01 Ampere increments)
	NumberParam<double> motorThermalCurrent_Param = NumberParameter<double>::make(0.0, "Motor Thermal Current", "MotorThermalCurrent", "%.2f", Units::Current::Ampere, false);
	
	//[tfr] Motor Maximum Frequency (0.1 Hz increments)
	NumberParam<double> motorMaximumFrequency_Param = NumberParameter<double>::make(0.0, "Motor Maximum Frequency", "MotorMaximumFrequency", "%.2f", Units::Frequency::Hertz, false);
	
	ParameterGroup motorNameplateParameters = ParameterGroup("MotorNameplate", {
		motorStandartFrequency_Param,
		motorParameterChoice_Param,
		cosinusPhi_Param,
		nominalMotorPower_Param,
		nominalMotorVoltage_Param,
		nominalMotorCurrent_Param,
		nominalMotorFrequency_Param,
		nominalMotorSpeed_Param,
		motorThermalCurrent_Param,
		motorMaximumFrequency_Param
	});
	
	//———— Brake Logic
	
	//[blc] brake assignement
	OptionParam brakeOutputAssignement_Param = OptionParameter::make(options.NoDigitalOutput, options.digitalOutput_Options, "Brake Output Assignement", "BrakeOutputAssignement");
	
	//[bst] movement type (0= Horizontal Movement, 1=Hoisting)
	OptionParam brakeMovementType_Param = OptionParameter::make(options.HorizontalMovement, options.brakeMovementType_Options, "Brake Movement Type", "BrakeMovementType");
	
	ParameterGroup brakeLogicParameters = ParameterGroup("BrakeLogic", {
		brakeOutputAssignement_Param,
		brakeMovementType_Param
	});
	
	//———— Embedded Encoder
	
	//[eecp] embedded encoder etype (0=None, 1=AB, 2=SinCos)
	OptionParam embeddedEncoderType_Param = OptionParameter::make(options.EmbeddedEncoderTypeNone, options.embeddedEncoderType_Options, "Embedded Encoder Type", "EmbeddedEncoderType");
	
	//[eecv] embedded encoder supply voltage (5=5V, 12=12V, 24=24V)
	OptionParam embeddedEncoderVoltage_Param = OptionParameter::make(options.EmbeddedEncoder24V, options.embeddedEncoderVoltage_Options, "Embedded Encoder Voltage", "EmbeddedEncoderVoltage");
	
	//[epg] pulses per encoder revolution
	NumberParam<int> embeddedEncoderPulsesPerRevolution_Param = NumberParameter<int>::make(0, "Embedded Encoder Pulses Per revolution", "EmbeddedEncoderPulsesPerRevolution", "%i", Units::None::None, false);
	
	//[eeri] embedded encoder revolution inversion (0=No, 1=Yes)
	BoolParam embeddedEncoderInvertDirection_Param = BooleanParameter::make(false, "Invert Embedded Encoder Direction", "InvertEmbeddedEncoderDirection");
	
	//[eenu] embedded encoder usage (0=None, 1=SpeedMonitoring, 2=SpeedRegulation, 3=SpeedReference)
	OptionParam embeddedEncoderUsage_Param = OptionParameter::make(options.EmbeddedEncoderNoUsage, options.embeddedEncoderUsage_Options, "Embedded Encoder Usage", "EmbeddedEncoderUsage");
	
	ParameterGroup embeddedEncoderParameters = ParameterGroup("EmbeddedEncoder", {
		embeddedEncoderType_Param,
		embeddedEncoderVoltage_Param,
		embeddedEncoderPulsesPerRevolution_Param,
		embeddedEncoderInvertDirection_Param,
		embeddedEncoderUsage_Param
	});
	
	//———— Motor Control
	
	//[ctt] Motor Control Type
	OptionParam motorControlType_Param = OptionParameter::make(options.SensorlessFullFlux, options.motorControlType_Options, "Motor Control Type", "MotorControlType");
	
	NumberParam<int> velocityLimitRPM_Param = NumberParameter<int>::make(1000, "Velocity Limit", "VelocityLimit", "%i rpm", Units::None::None, false);
	
	NumberParam<double> accelerationRampTime_Param = NumberParameter<double>::make(3.0, "Acceleration Ramp Time", "AccelerationRampTime", "%.1f", Units::Time::Second, false);
	
	NumberParam<double> decelerationRampTime_Param = NumberParameter<double>::make(3.0, "Deceleration Ramp Time", "DecelerationRampTime", "%.1f", Units::Time::Second, false);
	
	NumberParam<double> switchingFrequency_Param = NumberParameter<double>::make(16, "Switching Frequency", "SwitchingFrequeny", "%.1f", Units::Frequency::Kilohertz, false);
	
	ParameterGroup motorControlParameters = ParameterGroup("MotorControl", {
		motorControlType_Param,
		velocityLimitRPM_Param,
		accelerationRampTime_Param,
		decelerationRampTime_Param,
		switchingFrequency_Param
	});
	
	//———— IO Configuration
	
	//[AI1T]
	OptionParam analogInput1Type_Param = OptionParameter::make(options.AnalogInputTypeVoltage, options.analogInput1Type_Options, "Analog Input 1 Type", "AnalogInput1Type");
	
	//[AI2T]
	OptionParam analogInput2Type_Param = OptionParameter::make(options.AnalogInputTypeVoltage, options.analogInput2Type_Options, "Analog Input 2 Type", "AnalogInput2Type");
	
	//[CrL1]
	NumberParam<double> analogInputMinCurrent_Param = NumberParameter<double>::make(0.0, "Analog Input Minimum Current Value", "AnalogInputMinimumCurrentValue", "%.1f", Units::Current::Milliampere, false);
	
	//[CrH1]
	NumberParam<double> analogInputMaxCurrent_Param = NumberParameter<double>::make(10.0, "Analog Input Maximum Current Value", "AnalogInputMaximumCurrentValue", "%.1f", Units::Current::Milliampere, false);
	
	//[UIL1] = [UIL2] if voltage is selected
	NumberParam<double> analogInputMinVoltage_Param = NumberParameter<double>::make(0.0, "Analog Input Minimum Voltage Value", "AnalogInputMinimumVoltageValue", "%.1f", Units::Voltage::Volt, false);
	
	//[UIH1] = [UIH2] if voltage is selected
	NumberParam<double> analogInputMaxVoltage_Param = NumberParameter<double>::make(10.0, "Analog Input Maximum Voltage Value", "AnalogInputMaximumVoltageValue", "%.1f", Units::Voltage::Volt, false);
	
	ParameterGroup ioConfigParameters = ParameterGroup("IOConfig", {
		analogInput1Type_Param,
		analogInput2Type_Param,
		analogInputMinCurrent_Param,
		analogInputMaxCurrent_Param,
		analogInputMinVoltage_Param,
		analogInputMaxVoltage_Param
	});
	
	void configureDrive();
	void startMotorTuning();
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
