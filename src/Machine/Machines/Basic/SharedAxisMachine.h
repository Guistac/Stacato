#include "Machine/Machine.h"

#include "Motion/Curve/Curve.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curve/Profile.h"

#include "Gui/Environnement/Dashboard/Widget.h"

#include "Animation/Animatables/AnimatablePosition.h"

#include "Project/Editor/Parameter.h"

/*

class SharedAxisMachine : public Machine{
	
	DEFINE_MACHINE_NODE(SharedAxisMachine, "Shared Axis Machine", "SharedAxisMachine", "Basic")

	std::shared_ptr<AnimatablePosition> axis1Animatable = AnimatablePosition::make("Axis 1 Position", Units::None::None);
	std::shared_ptr<AnimatablePosition> axis2Animatable = AnimatablePosition::make("Axis 2 Position", Units::None::None);
	std::shared_ptr<AnimatablePosition> synchronizedAnimatable = AnimatablePosition::make("Synchronized Position", Units::None::None);
	
	//———————— Input Pins ——————————
	
	std::shared_ptr<NodePin> axis1Pin = std::make_shared<NodePin>(NodePin::DataType::POSITION_CONTROLLED_AXIS,
																  NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Position Controlled Axis 1");
	std::shared_ptr<NodePin> axis2Pin = std::make_shared<NodePin>(NodePin::DataType::POSITION_CONTROLLED_AXIS,
																  NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Position Controlled Axis 2");
	bool areAxesConnected();
	bool axesHaveSamePositionUnit();
	std::shared_ptr<PositionControlledAxis> getAxis1();
	std::shared_ptr<PositionControlledAxis> getAxis2();
	
	//——————— Output Pins ——————————
	
	std::shared_ptr<double> position1PinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> velocity1PinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> position2PinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> velocity2PinValue = std::make_shared<double>(0.0);
	
	std::shared_ptr<NodePin> position1Pin = std::make_shared<NodePin>(position1PinValue, NodePin::Direction::NODE_OUTPUT, "Position 1");
	std::shared_ptr<NodePin> velocity1Pin = std::make_shared<NodePin>(velocity1PinValue, NodePin::Direction::NODE_OUTPUT, "Velocity 1");
	std::shared_ptr<NodePin> position2Pin = std::make_shared<NodePin>(position2PinValue, NodePin::Direction::NODE_OUTPUT, "Position 2");
	std::shared_ptr<NodePin> velocity2Pin = std::make_shared<NodePin>(velocity2PinValue, NodePin::Direction::NODE_OUTPUT, "Velocity 2");
	
	virtual void onPinUpdate(std::shared_ptr<NodePin> pin) override;
	virtual void onPinConnection(std::shared_ptr<NodePin> pin) override;
	virtual void onPinDisconnection(std::shared_ptr<NodePin> pin) override;
	
	void updateAxisParameters();
	void updateAnimatableParameters();
	virtual void addConstraints() override;
	
	std::shared_ptr<AnimatablePosition_KeepoutConstraint> axis1AnticollisionConstraint;
	std::shared_ptr<AnimatablePosition_KeepoutConstraint> axis2AnticollisionConstraint;
	std::shared_ptr<AnimatablePosition_KeepoutConstraint> synchronousLowerKeepoutConstraint;
	std::shared_ptr<AnimatablePosition_KeepoutConstraint> synchronousUpperKeepoutConstraint;
		
	virtual std::vector<std::shared_ptr<PositionControlledAxis>> getPositionControlledAxes() override {
		std::vector<std::shared_ptr<PositionControlledAxis>> output;
		if(areAxesConnected()) {
			output.push_back(getAxis1());
			output.push_back(getAxis2());
		}
		return output;
	}

	//————————— Settings ——————————
	
	Unit positionUnit = Units::None::None;
	bool b_forceSynchronousControl = false;
	
	enum class ControlMode{
		NONE,
		INDIVIDUAL,
		SYNCHRONOUS
	};
	ControlMode controlMode = ControlMode::NONE;

	std::shared_ptr<BooleanParameter> horizontalControls = BooleanParameter::make(false, "Horizontal Controls", "HorizontalControls");
	
	std::shared_ptr<BooleanParameter> invertAxis1 = BooleanParameter::make(false, "Invert Axis 1", "InvertAxis1");
	std::shared_ptr<BooleanParameter> invertAxis2 = BooleanParameter::make(false, "Invert Axis 2", "InvertAxis2");
	std::shared_ptr<NumberParameter<double>> axis1Offset = NumberParameter<double>::make(0.0, "Axis 1 Offset", "Axis1Offset");
	std::shared_ptr<NumberParameter<double>> axis2Offset = NumberParameter<double>::make(0.0, "Axis 2 Offset", "Axis2Offset");
	
	std::shared_ptr<NumberParameter<double>> velocityLimit = NumberParameter<double>::make(0.0, "Velocity Limit", "VelocityLimit", "%.3f", Units::None::None, false);
	std::shared_ptr<NumberParameter<double>> accelerationLimit = NumberParameter<double>::make(0.0, "Acceleration Limit", "AccelerationLimit", "%.3f", Units::None::None, false);
	
	std::shared_ptr<BooleanParameter> enableAntiCollision = BooleanParameter::make(false, "Enabled Anticollision", "EnableAnticollision");
	std::shared_ptr<NumberParameter<double>> minimumDistanceBetweenAxes = NumberParameter<double>::make(0.0, "Minimum Distance Between Axes", "MinimumAxisBetweenAxes", "%.3f", Units::None::None, false);
	std::shared_ptr<BooleanParameter> axis1isAboveAxis2 = BooleanParameter::make(true, "Axis Order", "AxisOrder");
	
	std::shared_ptr<BooleanParameter> enableSynchronousControl = BooleanParameter::make(false, "Enable Synchronous Control", "EnableSynchronousControl");
	std::shared_ptr<BooleanParameter> axis1isMaster = BooleanParameter::make(true, "Synchronization Master Selection", "Axis1isMaster");
	
	std::shared_ptr<BooleanParameter> allowUserHoming = BooleanParameter::make(false, "Allow User Homing", "AllowUserHoming");
	
	
	bool b_disableAntiCollision = false;
	double axis1OffsetEditDisplay = 0.0;
	double axis2OffsetEditDisplay = 0.0;
	
	//————————— Unit Conversion ——————————
	
	void captureAxis1PositionToOffset(double machinePosition);
	void captureAxis2PositionToOffset(double machinePosition);
	double axis1PositionToMachinePosition(double axis1Position);
	double axis2PositionToMachinePosition(double axis2Position);
	double machinePositionToAxis1Position(double machinePosition);
	double machinePositionToAxis2Position(double machinePosition);
	double axis1ToMachineConversion(double axis1Value);
	double machineToAxis1Conversion(double machineAxis1Value);
	double axis2ToMachineConversion(double axis2Value);
	double machineToAxis2Conversion(double machineAxis2Value);
	
	//——————————————— Homing ————————————————
	
	bool canStartHoming();
	void startHomingAxis1();
	void startHomingAxis2();
	void startHomingBothAxes();
	
	//——————————— Control Widget ————————————
		
	virtual void onAddToNodeGraph() override { controlWidget->addToDictionnary(); }
	virtual void onRemoveFromNodeGraph() override { controlWidget->removeFromDictionnary(); }
	
	void widgetGui();
	virtual bool hasSetupGui() override;
	virtual void setupGui() override;
	
	class ControlWidget : public Widget{
	public:
		ControlWidget(std::shared_ptr<SharedAxisMachine> machine_) : Widget("Machines"), machine(machine_){}
		std::shared_ptr<SharedAxisMachine> machine;
		virtual void gui() override;
		virtual std::string getName() override { return machine->getName(); }
	};
	std::shared_ptr<ControlWidget> controlWidget;
};


*/
