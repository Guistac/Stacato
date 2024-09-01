#include "Machine/Machine.h"

#include "Motion/Curve/Curve.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curve/Profile.h"

#include "Gui/Environnement/Dashboard/Widget.h"

#include "Animation/Animatables/AnimatableVelocity.h"

#include "Project/Editor/Parameter.h"

class VelocityControlledMachine : public Machine{
	
	DEFINE_MACHINE_NODE(VelocityControlledMachine, "Velocity Controlled Machine", "VelocityControlledMachine", "Basic")

	std::shared_ptr<AnimatableVelocity> animatableVelocity = AnimatableVelocity::make("Velocity", Units::None::None);
	
	//———————— Input Pins ——————————
	
	std::shared_ptr<NodePin> axisPin = std::make_shared<NodePin>(NodePin::DataType::AXIS_INTERFACE, NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Velocity Controlled Axis");
	bool isAxisConnected();
	std::shared_ptr<AxisInterface> getAxisInterface();
	
	//——————— Output Pins ——————————
	
	
	std::shared_ptr<double> velocityPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(velocityPinValue, NodePin::Direction::NODE_OUTPUT, "Velocity");
	
	virtual void onPinUpdate(std::shared_ptr<NodePin> pin) override;
	virtual void onPinConnection(std::shared_ptr<NodePin> pin) override;
	virtual void onPinDisconnection(std::shared_ptr<NodePin> pin) override;

	//————————— Settings ——————————
	
	Unit positionUnit = Units::None::None;
	
	std::shared_ptr<BooleanParameter> invertAxis = BooleanParameter::make(false, "Invert Axis", "InvertAxis");
	std::shared_ptr<BooleanParameter> invertControlGui = BooleanParameter::make(false, "Invert Control Gui", "InvertControlGui");
	
	//std::shared_ptr<NumberParameter<double>> lowerPositionLimit = NumberParameter<double>::make(0.0, "Lower Position Limit", "LowerPositionLimit", "%.3f");
	
	//————————— Unit Conversion & Limits ——————————
	
	double axisVelocityToMachineVelocity(double axisVelocity);
	double machineVelocityToAxisVelocity(double machineVelocity);
	double axisAccelerationToMachineAcceleration(double axisVelocity);
	double machineAccelerationToAxisAcceleration(double machineVelocity);

	
	void updateAnimatableParameters();
	
	bool b_enableRequest = false;
	bool b_disableRequest = false;
	
	//——————————— Control Widget ————————————
		
	virtual void onAddToNodeGraph() override { controlWidget->addToDictionnary(); }
	virtual void onRemoveFromNodeGraph() override { controlWidget->removeFromDictionnary(); }
	
	void verticalWidgetGui();
	
	void widgetGui();
	virtual bool hasSetupGui() override { return false; }
	
	class ControlWidget : public Widget{
	public:
		ControlWidget(std::shared_ptr<VelocityControlledMachine> machine_) : Widget("Machines"), machine(machine_){}
		std::shared_ptr<VelocityControlledMachine> machine;
		virtual void gui() override;
		virtual std::string getName() override {
			return machine->getName();
		}
	};
	std::shared_ptr<ControlWidget> controlWidget;
	double velocitySliderValue = .0f;
	
};
