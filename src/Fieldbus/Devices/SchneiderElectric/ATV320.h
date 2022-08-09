#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/DS402.h"

#include "Motion/SubDevice.h"

class ATV320 : public EtherCatDevice{
public:
	
	DEFINE_ETHERCAT_DEVICE(ATV320, "ATV320", "ATV320", "Schneider Electric", "Motor Drives", 0x800005A, 0x389)
	
	class ATV_Motor : public ActuatorDevice{
	public:
		
		ATV_Motor(std::shared_ptr<ATV320> drive_) :
		MotionDevice("ATV320 Motor", Units::AngularDistance::Revolution),
		ActuatorDevice("ATV320 Motor", Units::AngularDistance::Revolution),
		drive(drive_){}
		
		virtual std::string getStatusString() override { return drive->getStatusString(); }
		std::shared_ptr<ATV320> drive;
	};
	
	class ATV_GPIO : public GpioDevice{
	public:
		
		ATV_GPIO(std::shared_ptr<ATV320> drive_) :
		GpioDevice("ATV320 Gpio"),
		drive(drive_){}
		
		virtual std::string getStatusString() override { return drive->getStatusString(); }
		std::shared_ptr<ATV320> drive;
	};
	
	std::shared_ptr<ATV_Motor> actuator;
	std::shared_ptr<ATV_GPIO> gpio;
	std::shared_ptr<bool> digitalInput1 = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput2 = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput3 = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput4 = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput5 = std::make_shared<bool>(false);
	std::shared_ptr<bool> digitalInput6 = std::make_shared<bool>(false);
	std::shared_ptr<double> actualVelocity = std::make_shared<double>(0.0);
	
	std::shared_ptr<NodePin> digitalInput1Pin = std::make_shared<NodePin>(digitalInput1, NodePin::Direction::NODE_OUTPUT, "DI1", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalInput2Pin = std::make_shared<NodePin>(digitalInput1, NodePin::Direction::NODE_OUTPUT, "DI2", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalInput3Pin = std::make_shared<NodePin>(digitalInput1, NodePin::Direction::NODE_OUTPUT, "DI3", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalInput4Pin = std::make_shared<NodePin>(digitalInput1, NodePin::Direction::NODE_OUTPUT, "DI4", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalInput5Pin = std::make_shared<NodePin>(digitalInput1, NodePin::Direction::NODE_OUTPUT, "DI5", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> digitalInput6Pin = std::make_shared<NodePin>(digitalInput1, NodePin::Direction::NODE_OUTPUT, "DI6", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> actuatorPin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Actuator");
	std::shared_ptr<NodePin> gpioPin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Gpio");
	std::shared_ptr<NodePin> actualVelocityPin = std::make_shared<NodePin>(actualVelocity, NodePin::Direction::NODE_OUTPUT, "Velocity");
	
	//————— Drive State —————
	DS402::PowerState requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
	DS402::PowerState actualPowerState = DS402::PowerState::UNKNOWN;
	bool b_resetFaultCommand = false;
	bool b_velocityTargetReached = false;
	bool b_hasFault = false;
	long long enableRequestTime_nanoseconds;
	
	//————— RX PDO —————
	DS402::Control ds402Control;
	int16_t velocityTarget_rpm = 0;
	
	//————— TX PDO —————
	DS402::Status ds402Status;
	int16_t velocityActual_rpm = 0;
	uint16_t logicInputs = 0;
	uint16_t stoState = 0;
	int16_t motorPower = 0;
	
	//————— Driver Settings —————
	float accelerationTime_seconds = 3.0;
	float decelerationTime_seconds = 3.0;
	
	//————— General Settings —————
	long long enableRequestTimeout_nanoseconds = 250'000'000; //250ms enable timeout
	
	std::string getStatusString(){
		return "";
	}
	
};
