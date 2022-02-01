#include "Motion/Machine/Machine.h"

#include "Motion/Curve/Curve.h"
#include "Motion/MotionTypes.h"
#include "Motion/Profile/Profile.h"

class PositionControlledSingleAxisMachine : public Machine{
	
	DEFINE_MACHINE_NODE(PositionControlledSingleAxisMachine, "Position Controlled Single Axis Machine", "PositionControlledSingleAxisMachine", "Basic")
	DEFINE_HOMEABLE_MACHINE
	
	std::shared_ptr<NodePin> positionControlledAxisPin = std::make_shared<NodePin>(NodePin::DataType::POSITION_CONTROLLED_AXIS, NodePin::Direction::NODE_INPUT, "Position Controlled Axis");
	
	std::shared_ptr<double> positionPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> positionPin = std::make_shared<NodePin>(positionPinValue, NodePin::Direction::NODE_OUTPUT, "Position");
	
	std::shared_ptr<double> velocityPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(velocityPinValue, NodePin::Direction::NODE_OUTPUT, "Velocity");

	std::shared_ptr<AnimatableParameter> positionParameter = std::make_shared<AnimatableParameter>("Position", ParameterDataType::KINEMATIC_POSITION_CURVE, "units");

	bool isAxisConnected();
	std::shared_ptr<PositionControlledAxis> getAxis();

	//======= MANUAL CONTROLS =========

	double manualVelocityTarget_machineUnitsPerSecond = 0.0;
	double manualPositionTarget_machineUnits = 0.0;

	void setVelocityTarget(double velocity_machineUnitsPerSecond);
	void moveToPosition(double target_machineUnits);

	//======== MACHINE SETTINGS =========

	double rapidVelocity_machineUnitsPerSecond = 0.0;
	double rapidAcceleration_machineUnitsPerSecond = 0.0;
	
	double machineZero_axisUnits = 0.0;
	bool b_invertDirection = false;
	void captureMachineZero();
	
	//======== MACHINE TO AXIS CONVERSION ========
	
	double axisPositionToMachinePosition(double axisPosition);
	double axisVelocityToMachineVelocity(double axisVelocity);
	double machinePositionToAxisPosition(double machinePosition);
	double machineVelocityToAxisVelocity(double machineVelocity);
	
	double getLowPositionLimit();
	double getHighPositionLimit();
	double getPositionNormalized();
	double getVelocityNormalized();
	
	//=========== SIMULATION ==========
	
	enum class ControlMode{
		VELOCITY_TARGET,
		POSITION_TARGET,
		PARAMETER_TRACK
	};
	
	ControlMode controlMode = ControlMode::VELOCITY_TARGET;
	Motion::Profile motionProfile;
	std::shared_ptr<Motion::Interpolation> targetInterpolation = std::make_shared<Motion::Interpolation>();
	
	bool hasManualPositionTarget();
	double getManualPositionTarget();
	
	double getActualPosition();
	double getActualVelocitu();
};
