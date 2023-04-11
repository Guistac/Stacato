#include "Machine/Machine.h"

#include "Motion/Curve/Curve.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curve/Profile.h"

#include "Gui/Environnement/Dashboard/Widget.h"

#include "Animation/Animatables/AnimatablePosition.h"

#include "Project/Editor/Parameter.h"

#include "Animation/NewAnimation/AnimatableOwner.h"
#include "Animation/NewAnimation/AnimatableRegistry.h"
#include "Animation/NewAnimation/PositionAnimatable.h"


class PositionControlledMachine : public Machine{
public:
	
	DEFINE_MACHINE(PositionControlledMachine)
	
	virtual bool onSerialization() override {
		Machine::onSerialization();
		return true;
	}
	
	virtual bool onDeserialization() override {
		Machine::onDeserialization();
		return true;
	}
	
	virtual void onConstruction() override {
		Machine::onConstruction();
	}
	
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Machine::onCopyFrom(source);
	}
	
	virtual void inputProcess() override {}
	virtual void outputProcess() override {}
	virtual bool needsOutputProcess() override {}
	
};

/*
class PositionControlledMachine : public Machine, public AnimationSystem::AnimatableOwner{
	
	DEFINE_MACHINE_NODE(PositionControlledMachine, "Position Controlled Machine", "PositionControlledMachine", "Basic")

	//std::shared_ptr<AnimatablePosition> animatablePosition = AnimatablePosition::make("Position", Units::None::None);
	std::shared_ptr<AnimationSystem::PositionAnimatable> positionAnimatable = std::make_shared<AnimationSystem::PositionAnimatable>();
	
	//———————— Input Pins ——————————
	
	std::shared_ptr<NodePin> axisPin = std::make_shared<NodePin>(NodePin::DataType::AXIS_INTERFACE, NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Position Controlled Axis");
	bool isAxisConnected();
	std::shared_ptr<AxisInterface> getAxisInterface();
	
	//——————— Output Pins ——————————
	
	std::shared_ptr<double> positionPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> velocityPinValue = std::make_shared<double>(0.0);
	
	std::shared_ptr<NodePin> positionPin = std::make_shared<NodePin>(positionPinValue, NodePin::Direction::NODE_OUTPUT, "Position");
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(velocityPinValue, NodePin::Direction::NODE_OUTPUT, "Velocity");
	
	virtual void onPinUpdate(std::shared_ptr<NodePin> pin) override;
	virtual void onPinConnection(std::shared_ptr<NodePin> pin) override;
	virtual void onPinDisconnection(std::shared_ptr<NodePin> pin) override;
	
	void updateAnimatableParameters();

	//————————— Settings ——————————
	
	Unit positionUnit = Units::None::None;
	
	std::shared_ptr<BooleanParameter> invertAxis = BooleanParameter::make(false, "Invert Axis", "InvertAxis");
	std::shared_ptr<NumberParameter<double>> axisOffset = NumberParameter<double>::make(0.0, "Axis Offset", "AxisOffset", "%.3f");
	std::shared_ptr<NumberParameter<double>> lowerPositionLimit = NumberParameter<double>::make(0.0, "Lower Position Limit", "LowerPositionLimit", "%.3f");
	std::shared_ptr<NumberParameter<double>> upperPositionLimit = NumberParameter<double>::make(0.0, "Upper Position Limit", "UpperPositionLimit", "%.3f");
	
	std::shared_ptr<BooleanParameter> allowUserZeroEdit = BooleanParameter::make(false, "Allow User Zero Edit", "AllowUserZeroEdit");
	std::shared_ptr<BooleanParameter> allowUserLowerLimitEdit = BooleanParameter::make(false, "Allow User Lower Limit Edit", "AllowUserLowerLimitEdit");
	std::shared_ptr<BooleanParameter> allowUserUpperLimitEdit = BooleanParameter::make(false, "Allow User Upper Limit Edit", "AllowUserUpperLimitEdit");
	std::shared_ptr<BooleanParameter> allowUserHoming = BooleanParameter::make(false, "Allow User Homing", "AllowUserHoming");
	std::shared_ptr<BooleanParameter> allowUserEncoderValueOverride = BooleanParameter::make(false, "Allow User Encoder Value Override", "AllowUserEncoderValueOverride");
	std::shared_ptr<BooleanParameter> allowUserEncoderRangeReset = BooleanParameter::make(false, "Allow User Encoder Range Reset", "AllowUserEncoderRangeReset");
	std::shared_ptr<BooleanParameter> invertControlGui = BooleanParameter::make(false, "Invert Control Gui", "InvertControlGui");
	
	//————————— Unit Conversion & Limits ——————————
	
	double getMinPosition();
	double getMaxPosition();
	double getLowerPositionLimit();
	double getUpperPositionLimit();
	
	void captureZero();
	void resetZero();
	void captureLowerLimit();
	void resetLowerLimit();
	void captureUpperLimit();
	void resetUpperLimit();
	
	double axisPositionToMachinePosition(double axisPosition);
	double axisVelocityToMachineVelocity(double axisVelocity);
	double axisAccelerationToMachineAcceleration(double axisAcceleration);
	double machinePositionToAxisPosition(double machinePosition);
	double machineVelocityToAxisVelocity(double machineVelocity);
	double machineAccelerationToAxisAcceleration(double machineAcceleration);
	
	//——————————————— Homing ————————————————
	
	bool canStartHoming();
	bool isHoming();
	void startHoming();
	void stopHoming();
	bool didHomingSucceed();
	bool didHomingFail();
	const char* getHomingString();
	
	//——————————— Control Widget ————————————
		
	virtual void onAddToNodeGraph() override {
		auto thisPositionControlledMachine = std::static_pointer_cast<PositionControlledMachine>(shared_from_this());
		auto thisAnimatableOwner = std::static_pointer_cast<AnimationSystem::AnimatableOwner>(thisPositionControlledMachine);
		Environnement::getAnimatableRegistry()->registerAnimatableOwner(thisAnimatableOwner);
		controlWidget->addToDictionnary();
	}
	virtual void onRemoveFromNodeGraph() override { controlWidget->removeFromDictionnary(); }
	
	void widgetGui();
	virtual bool hasSetupGui() override { return true; }
	virtual void setupGui() override;
	
	class ControlWidget : public Widget{
	public:
		ControlWidget(std::shared_ptr<PositionControlledMachine> machine_) : Widget("Machines"), machine(machine_){}
		std::shared_ptr<PositionControlledMachine> machine;
		virtual void gui() override;
		virtual std::string getName() override {
			return machine->getName();
		}
	};
	std::shared_ptr<ControlWidget> controlWidget;
	double velocitySliderValue = .0f;
	double positionTargetValue = .0f;
};
*/
