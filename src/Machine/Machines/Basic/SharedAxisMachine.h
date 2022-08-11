#include "Machine/Machine.h"

#include "Motion/Curve/Curve.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curve/Profile.h"

#include "Gui/Environnement/Dashboard/Widget.h"

#include "Animation/Animatables/AnimatablePosition.h"

class SharedAxisMachine : public Machine{
	
	DEFINE_MACHINE_NODE(SharedAxisMachine, "Shared Axis Machine", "SharedAxisMachine", "Basic")
	DEFINE_HOMEABLE_MACHINE

	std::shared_ptr<AnimatablePosition> axis1Position = AnimatablePosition::make("Axis 1", Units::None::None);
	std::shared_ptr<AnimatablePosition> axis2Position = AnimatablePosition::make("Axis 2", Units::None::None);
	
	//———————— Input Pins ——————————
	
	std::shared_ptr<NodePin> axis1Pin = std::make_shared<NodePin>(NodePin::DataType::POSITION_CONTROLLED_AXIS,
																  NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Position Controlled Axis 1");
	std::shared_ptr<NodePin> axis2Pin = std::make_shared<NodePin>(NodePin::DataType::POSITION_CONTROLLED_AXIS,
																  NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Position Controlled Axis 2");
	bool areAxesConnected();
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
	
	void updateAnimatableParameters();
	
	virtual std::vector<std::shared_ptr<PositionControlledAxis>> getPositionControlledAxes() override {
		std::vector<std::shared_ptr<PositionControlledAxis>> output;
		if(areAxesConnected()) {
			output.push_back(getAxis1());
			output.push_back(getAxis2());
		}
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
		ControlWidget(std::shared_ptr<SharedAxisMachine> machine_, std::string name) : Widget(name, "Machines"), machine(machine_){}
		std::shared_ptr<SharedAxisMachine> machine;
		virtual void gui() override;
		virtual bool hasFixedContentSize() override{ return true; }
		virtual glm::vec2 getFixedContentSize() override;
	};
	std::shared_ptr<ControlWidget> controlWidget;
	double velocitySliderValue = .0f;
	double positionTargetValue = .0f;
};
