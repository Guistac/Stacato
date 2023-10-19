#pragma once


#include "Animation/Animatables/AnimatablePosition.h"
#include "Motion/Interfaces.h"
#include "Environnement/NodeGraph/NodePin.h"

#include "Project/Editor/Parameter.h"

class AxisMapping{
public:
	
	std::shared_ptr<NodePin> axisPin = std::make_shared<NodePin>(NodePin::DataType::AXIS_INTERFACE, NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Axis Pin");
	std::shared_ptr<AnimatablePosition> animatablePosition = AnimatablePosition::make("Position", Units::None::None);

	
	bool isAxisConnected();
	std::shared_ptr<AxisInterface> getAxis();
	
	
	void updateAnimatableParameters();
	void updateRealAnimatableValues();
	void updateAxisCommand(double profileTime_seconds, double profileDeltaTime_seconds);
	void enableAxis();
	void disableAxis();
	
	
	double axisPositionToMachinePosition(double axisPosition){
		double output = axisPosition - userPositionOffset;
		if(b_invertDirection) return -output;
		return output;
	}
	double machinePositionToAxisPosition(double machinePosition){
		if(b_invertDirection) return userPositionOffset - machinePosition;
		return userPositionOffset + machinePosition;
	}
	double axisVelocityToMachineVelocity(double axisVelocity){
		if(b_invertDirection) return -axisVelocity;
		return axisVelocity;
	}
	double machineVelocityToAxisVelocity(double machineVelocity){
		if(b_invertDirection) return -machineVelocity;
		return machineVelocity;
	}
	double axisAccelerationToMachineAcceleration(double axisAcceleration){
		if(b_invertDirection) return -axisAcceleration;
		return axisAcceleration;
	}
	double machineAccelerationToAxisAcceleration(double machineAcceleration){
		if(b_invertDirection) return -machineAcceleration;
		return machineAcceleration;
	}
	
	
	bool save(tinyxml2::XMLElement* parent);
	bool load(tinyxml2::XMLElement* parent);
	
	
	void controlGui();
	void setupGui();
	
	

	double axisLowerPositionLimit = 0.0;
	double axisUpperPositionLimit = 0.0;
	
	void captureUserZero();
	void resetUserZero();
	bool b_userZeroUpdateRequest = false;
	double requestedUserZeroOffset = 0.0;

	void setUserLowerLimit(double lowerLimit);
	void setUserUpperLimit(double upperLimit);
	void captureLowerUserLimit();
	void captureUpperUserLimit();
	void resetLowerUserLimit();
	void resetUpperUserLimit();

	bool b_invertDirection = false;
	double userPositionOffset = 0.0;
	double userLowerPositionLimit = 0.0;
	double userUpperPositionLimit = 0.0;
	
	
};
