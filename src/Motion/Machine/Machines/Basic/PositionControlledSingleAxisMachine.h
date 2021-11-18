#include "Motion/Machine/Machine.h"

#include "Motion/Curve/Curve.h"
#include "Motion/MotionTypes.h"

class PositionControlledAxis;

class PositionControlledSingleAxisMachine : public Machine{
	
	DEFINE_MACHINE_NODE(PositionControlledSingleAxisMachine, "Position Controlled Single Axis Machine", "PositionControlledPositionControlledSingleAxisMachine")

	std::shared_ptr<NodePin> positionControlledAxisLink = std::make_shared<NodePin>(NodeData::Type::POSITION_CONTROLLED_AXIS_LINK, DataDirection::NODE_INPUT, "Position Controlled Axis");
	
	std::shared_ptr<AnimatableParameter> positionParameter;

	double actualPosition_machineUnits = 0.0;
	double actualVelocity_machineUnits = 0.0;

	bool b_enabled = false;

	bool isAxisConnected();
	std::shared_ptr<PositionControlledAxis> getAxis();

	double getLowPositionLimit();
	double getHighPositionLimit();
	double getVelocityLimit();
	double getAccelerationLimit();

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
	std::shared_ptr<Motion::Interpolation> targetIntepolation = std::make_shared<Motion::Interpolation>();
	double targetPosition_machineUnits = 0.0;
	double targetVelocity_machineUnitsPerSecond = 0.0;
	double targetTime_seconds = 0.0;
	
	//profile generator
	double profileTime_seconds;
	double profileDeltaTime_seconds;
	double profilePosition_machineUnits;
	double profileVelocity_machineUnitsPerSecond;

};