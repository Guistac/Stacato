#include "Motion/Machine/Machine.h"

#include "Motion/Curves/Position/D1PositionCurve.h"
#include "Motion/MotionTypes.h"

class SingleAxisMachine : public Machine{
	
	DEFINE_MACHINE_NODE(SingleAxisMachine, "Single Axis Linear Machine", "SingleAxisMachine")

	std::shared_ptr<NodePin> axisLink = std::make_shared<NodePin>(NodeData::Type::AXIS_LINK, DataDirection::NODE_INPUT, "Linear Axis");
	
	std::shared_ptr<AnimatableParameter> positionParameter;

	double actualPosition_machineUnits = 0.0;
	double actualVelocity_machineUnits = 0.0;

	bool b_enabled = false;

	bool isAxisConnected();
	std::shared_ptr<Axis> getAxis();

	ControlMode::Mode controlMode = ControlMode::Mode::MANUAL_VELOCITY_TARGET;

	//Master Acceleration For Manual Controls
	double manualControlAcceleration_machineUnitsPerSecondSquared = 100.0;

	//Manual Velocity Control
	void setVelocity(double velocity_machineUnits);
	void velocityTargetControl();
	float manualVelocityTarget_machineUnitsPerSecond = 0.0;

	//Manual Target Control
	void moveToPositionWithVelocity(double position_axisUnits, double velocity_axisUnits, double acceleration_axisUnits);
	void moveToPositionInTime(double position_axisUnits, double movementTime_seconds, double acceleration_axisUnits);
	void positionTargetControl();
	Motion::PositionCurve::D1::Interpolation targetIntepolation;
	double targetPosition_machineUnits = 0.0;
	double targetVelocity_machineUnitsPerSecond = 0.0;
	double targetTime_seconds = 0.0;
	
	//profile generator
	double profileTime_seconds;
	double profileDeltaTime_seconds;
	double profilePosition_machineUnits;
	double profileVelocity_machineUnitsPerSecond;

};