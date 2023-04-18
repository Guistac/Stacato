#include "Machine/Machine.h"

#include "Motion/Curve/Curve.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curve/Profile.h"

#include "Gui/Environnement/Dashboard/Widget.h"

#include "Animation/Animatables/AnimatablePosition.h"

#include "Animation/NewAnimation/AnimatableRegistry.h"
#include "Animation/NewAnimation/PositionAnimatable.h"

#include "Legato/Editor/Parameters.h"


class PositionControlledMachine : public Machine{
	
	DEFINE_MACHINE(PositionControlledMachine)
	
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override;
	
	virtual void inputProcess() override;
	virtual void outputProcess() override;
	virtual bool needsOutputProcess() override { return true; }
	
private:
	
	std::shared_ptr<AnimationSystem::PositionAnimatable> positionAnimatable;
	
	std::shared_ptr<NodePin> axisPin;
	std::shared_ptr<NodePin> positionPin;
	std::shared_ptr<NodePin> velocityPin;
	
	bool isAxisConnected();
	std::shared_ptr<AxisInterface> getAxisInterface();
	
	//——————— Output Pins ——————————
	
	std::shared_ptr<double> positionPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> velocityPinValue = std::make_shared<double>(0.0);
	
	virtual void onPinUpdate(std::shared_ptr<NodePin> pin) override{}
	virtual void onPinConnection(std::shared_ptr<NodePin> pin) override{}
	virtual void onPinDisconnection(std::shared_ptr<NodePin> pin) override{}
	
	void updateAnimatableParameters();

	//————————— Parameters ——————————
	
	Legato::BoolParam invertAxis;
	Legato::NumberParam<double> axisOffset;
	Legato::NumberParam<double> lowerPositionLimit;
	Legato::NumberParam<double> upperPositionLimit;
	Legato::BoolParam allowUserZeroEdit;
	Legato::BoolParam allowUserLowerLimitEdit;
	Legato::BoolParam allowUserUpperLimitEdit;
	Legato::BoolParam allowUserHoming;
	Legato::BoolParam allowUserEncoderValueOverride;
	Legato::BoolParam allowUserEncoderRangeReset;
	Legato::BoolParam invertControlGui;
	
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
		
	/*
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
	 */
	
};

