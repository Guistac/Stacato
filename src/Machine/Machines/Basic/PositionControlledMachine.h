#include "Machine/Machine.h"

#include "Motion/Curve/Curve.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curve/Profile.h"

#include "Gui/Environnement/Dashboard/Widget.h"

#include "Animation/Animatables/AnimatablePosition.h"

#include "Project/Editor/Parameter.h"

class PositionControlledMachine : public Machine{
	
	DEFINE_MACHINE_NODE(PositionControlledMachine, "Position Controlled Machine", "PositionControlledMachine", "Basic")

	std::shared_ptr<AnimatablePosition> animatablePosition = AnimatablePosition::make("Position", Units::None::None);
	
	//———————— Input Pins ——————————
	
	std::shared_ptr<NodePin> axisPin = std::make_shared<NodePin>(NodePin::DataType::AXIS_INTERFACE, NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Position Controlled Axis");
	std::shared_ptr<bool> referenceSignal = std::make_shared<bool>(false);
	std::shared_ptr<NodePin> referenceSignalPin = std::make_shared<NodePin>(referenceSignal, NodePin::Direction::NODE_INPUT, "Reference Signal", "ReferenceSignal");
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
	std::shared_ptr<BooleanParameter> allowModuloPositionShifting = BooleanParameter::make(false, "Allow Modulo Position Shifting", "AllowModulePositionShifting");
	
	OptionParameter::Option linearWidgetOrientation_vertical = OptionParameter::Option(0, "Vertical", "Vertical");
	OptionParameter::Option linearWidgetOrientation_horizontal = OptionParameter::Option(1, "Horizontal", "Horizontal");
	std::vector<OptionParameter::Option*> linearWidgetOrientation_options = {
		&linearWidgetOrientation_vertical,
		&linearWidgetOrientation_horizontal
	};
	std::shared_ptr<OptionParameter> linearWidgetOrientation_parameter = OptionParameter::make2(linearWidgetOrientation_vertical, linearWidgetOrientation_options, "Widget Orientation", "WidgetOrientation");
	
	BoolParam displayModuloturns_param = BooleanParameter::make(false, "Display Modulo Turns", "DisplayModuleTurns");
	
	//————————— Unit Conversion & Limits ——————————
	
	double getMinPosition();
	double getMaxPosition();
	double getLowerPositionLimit();
	double getUpperPositionLimit();
	
	double axisPositionToMachinePosition(double axisPosition);
	double axisVelocityToMachineVelocity(double axisVelocity);
	double axisAccelerationToMachineAcceleration(double axisAcceleration);
	double machinePositionToAxisPosition(double machinePosition);
	double machineVelocityToAxisVelocity(double machineVelocity);
	double machineAccelerationToAxisAcceleration(double machineAcceleration);
	
	
	
	void updateAnimatableParameters();
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
	
	
	
	
	//——————————————— Homing ————————————————
	
	bool canStartHoming();
	bool isHoming();
	void startHoming();
	void stopHoming();
	bool didHomingSucceed();
	bool didHomingFail();
	std::string getHomingString();
	
	//——————————— Control Widget ————————————
		
	virtual void onAddToNodeGraph() override { controlWidget->addToDictionnary(); }
	virtual void onRemoveFromNodeGraph() override { controlWidget->removeFromDictionnary(); }
	
	void verticalWidgetGui();
	void horizontalWidgetGui();
	void angularWidgetGui();
	
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
	
	//MODULA HACK
	int turnOffset = 0;
	int requestedTurnOffset = 0;
	bool b_applyTurnOffset = false;
	void setTurnOffset(int offset);
	bool canSetTurnOffset(int offset);
};
