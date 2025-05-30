#include "Machine/Machine.h"

#include "Motion/Curve/Curve.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curve/Profile.h"

#include "Gui/Environnement/Dashboard/Widget.h"

#include "Animation/Animatables/AnimatablePosition.h"

#include "Project/Editor/Parameter.h"

#include "AxisMapping.h"

class MultiAxisMachine : public Machine{
	
	DEFINE_MACHINE_NODE(MultiAxisMachine, "Multi Axis Machine", "MultiAxisMachine", "Basic")

	
	virtual void onPinUpdate(std::shared_ptr<NodePin> pin) override;
	virtual void onPinConnection(std::shared_ptr<NodePin> pin) override;
	virtual void onPinDisconnection(std::shared_ptr<NodePin> pin) override;
	virtual void onAddToNodeGraph() override { controlWidget->addToDictionnary(); }
	virtual void onRemoveFromNodeGraph() override { controlWidget->removeFromDictionnary(); }
	virtual bool hasSetupGui() override { return true; }
	virtual void setupGui() override;
	
	
	std::vector<std::shared_ptr<AxisMapping>> axisMappings = {};
	void addAxisMapping();
	void addAxisMapping(std::shared_ptr<AxisMapping> mapping);
	void removeAxisMapping(std::shared_ptr<AxisMapping> mapping);
	
	
	double masterVelocityCommand = 0.0;
	
	bool b_enableGroupSurveillance = true;
	bool b_enableUnderloadSurveillance = true;
	
	bool b_showLoad = true;
	bool b_allowUserHoming = false;
	bool b_allowUserLimitSettings = true;
	bool b_allowUserUnderloadSurveillanceToggle = true;
	
	void setMasterVelocityTarget(double velocityTarget);
	
	
	class ControlWidget : public Widget{
	public:
		ControlWidget(std::shared_ptr<MultiAxisMachine> machine_) : Widget("Machines"), machine(machine_){}
		std::shared_ptr<MultiAxisMachine> machine;
		virtual void gui() override;
		virtual std::string getName() override {
			return machine->getName();
		}
	};
	std::shared_ptr<ControlWidget> controlWidget;
	void widgetGui();
	
};



//implement max axis distance constraints
//implement emergency mode for ATV340
//debug rapid movement overshoot
//reimplement axis surveillance and emergency states
