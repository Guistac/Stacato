#include <pch.h>

#include "MotionTypes.h"



bool isLinearPositionUnit(PositionUnit t){
	switch(t){
		case PositionUnit::DEGREE:
		case PositionUnit::RADIAN:
		case PositionUnit::REVOLUTION:
			return false;
		case PositionUnit::METER:
		case PositionUnit::CENTIMETER:
		case PositionUnit::MILLIMETER:
			return true;
	}
}

bool isAngularPositionUnit(PositionUnit t){
	switch(t){
		case PositionUnit::DEGREE:
		case PositionUnit::RADIAN:
		case PositionUnit::REVOLUTION:
			return true;
		case PositionUnit::METER:
		case PositionUnit::CENTIMETER:
		case PositionUnit::MILLIMETER:
			return false;
	}
}




bool isLinearPositionReferenceSignal(PositionReferenceSignal t){
	switch(t){
		case PositionReferenceSignal::NO_SIGNAL:
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			return false;
		default:
			return true;
	}
}

bool isAngularPositionReferenceSignal(PositionReferenceSignal t){
	return true;
}







/*
std::vector<MotionCommand> motionCommandTypes = {
	{MotionCommand::POSITION_COMMAND, "Position Command", "Position"},
	{MotionCommand::VELOCITY_COMMAND, "Velocity Command", "Velocity"}
};
 */





std::vector<ControlMode> controlModes = {
	{ControlMode::Mode::VELOCITY_TARGET,	"Velocity Target"},
	{ControlMode::Mode::POSITION_TARGET,	"Position Target"},
	{ControlMode::Mode::FAST_STOP,			"Fast Stop"},
	{ControlMode::Mode::MACHINE_CONTROL,	"Machine Control"}
};
std::vector<ControlMode>& getControlModes() {
	return controlModes;
}
ControlMode* getControlMode(ControlMode::Mode m) {
	for (auto& controlMode : controlModes) {
		if (m == controlMode.mode) return &controlMode;
	}
	return nullptr;
}

