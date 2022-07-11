#include "Machine/Machine.h"

#include "Motion/Curve/Curve.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curve/Profile.h"

#include "Gui/Environnement/Dashboard/Widget.h"

#include "Animation/Animatables/AnimatablePosition.h"

class PositionControlledMachine : public Machine{
	
	DEFINE_MACHINE_NODE(PositionControlledMachine, "Position Controlled Machine", "PositionControlledMachine", "Basic")
	DEFINE_HOMEABLE_MACHINE

	std::shared_ptr<AnimatablePosition> animatablePosition = AnimatablePosition::make("Position", Units::None::None);
	
	//———————— Input Pins ——————————
	
	std::shared_ptr<NodePin> positionControlledAxisPin = std::make_shared<NodePin>(NodePin::DataType::POSITION_CONTROLLED_AXIS, NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Position Controlled Axis");
	bool isAxisConnected();
	std::shared_ptr<PositionControlledAxis> getAxis();
	
	//——————— Output Pins ——————————
	
	std::shared_ptr<double> positionPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> velocityPinValue = std::make_shared<double>(0.0);
	
	std::shared_ptr<NodePin> positionPin = std::make_shared<NodePin>(positionPinValue, NodePin::Direction::NODE_OUTPUT, "Position");
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(velocityPinValue, NodePin::Direction::NODE_OUTPUT, "Velocity");
	
	virtual void onPinUpdate(std::shared_ptr<NodePin> pin) override;
	virtual void onPinConnection(std::shared_ptr<NodePin> pin) override;
	virtual void onPinDisconnection(std::shared_ptr<NodePin> pin) override;
	
	void updateAnimatableParameters();
	
	virtual std::vector<std::shared_ptr<PositionControlledAxis>> getPositionControlledAxes() override {
		std::vector<std::shared_ptr<PositionControlledAxis>> output;
		if(isAxisConnected()) output.push_back(getAxis());
		return output;
	}

	//————————— Settings ——————————

	double machineZero_axisUnits = 0.0;
	bool b_invertDirection = false;
	
	//————————— Unit Conversion ——————————
	
	void captureMachineZero();
	double axisPositionToMachinePosition(double axisPosition);
	double axisVelocityToMachineVelocity(double axisVelocity);
	double axisAccelerationToMachineAcceleration(double axisAcceleration);
	double machinePositionToAxisPosition(double machinePosition);
	double machineVelocityToAxisVelocity(double machineVelocity);
	double machineAccelerationToAxisAcceleration(double machineAcceleration);
	
	//——————————— Control Widget ————————————
		
	virtual void onAddToNodeGraph() override { controlWidget->addToDictionnary(); }
	virtual void onRemoveFromNodeGraph() override { controlWidget->removeFromDictionnary(); }
	
	void widgetGui();
	
	class ControlWidget : public Widget{
	public:
		ControlWidget(std::shared_ptr<PositionControlledMachine> machine_, std::string name) : Widget(name, "Machines"), machine(machine_){}
		std::shared_ptr<PositionControlledMachine> machine;
		virtual void gui() override;
		virtual bool hasFixedContentSize() override{ return true; }
		virtual glm::vec2 getFixedContentSize() override;
	};
	std::shared_ptr<ControlWidget> controlWidget;
	double velocitySliderValue = .0f;
	double positionTargetValue = .0f;
};
