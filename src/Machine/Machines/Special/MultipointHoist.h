#include "Machine/Machine.h"

#include "Motion/Curve/Curve.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curve/Profile.h"

#include "Gui/Environnement/Dashboard/Widget.h"

#include "Animation/Animatables/AnimatablePosition.h"

#include "Project/Editor/Parameter.h"

class MultipointHoist : public Machine{
	
	DEFINE_MACHINE_NODE(MultipointHoist, "Multipoint Hoist", "MultipointHoist", "Special")

	virtual void onPinUpdate(std::shared_ptr<NodePin> pin) override;
	virtual void onPinConnection(std::shared_ptr<NodePin> pin) override;
	virtual void onPinDisconnection(std::shared_ptr<NodePin> pin) override;

	virtual void onAddToNodeGraph() override { controlWidget->addToDictionnary(); }
	virtual void onRemoveFromNodeGraph() override { controlWidget->removeFromDictionnary(); }
	
	class AxisMapping{
	public:
		std::shared_ptr<NodePin> axisPin = std::make_shared<NodePin>(NodePin::DataType::AXIS_INTERFACE, NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Axis Pin");
		std::shared_ptr<AnimatablePosition> animatablePosition = AnimatablePosition::make("Position", Units::None::None);
		bool save(tinyxml2::XMLElement* parent);
		bool load(tinyxml2::XMLElement* parent);
		double positionOffset = 0.0;
		bool b_invertDirection = false;
		double lowerPositionLimit = 0.0;
		double upperPositionLimit = 0.0;
		void controlGui();
		bool isAxisConnected();
		std::shared_ptr<AxisInterface> getAxis();
		void updateAnimatableParameters();
		void updateRealAnimatableValues();
		void updateAxisCommand(double profileTime_seconds, double profileDeltaTime_seconds);
		void enableAxis();
		void disableAxis();
		double axisPositionToMachinePosition(double axisPosition){
			double output = axisPosition - positionOffset;
			if(b_invertDirection) return -output;
			return output;
		}
		double machinePositionToAxisPosition(double machinePosition){
			if(b_invertDirection) return positionOffset - machinePosition;
			return positionOffset + machinePosition;
		}
		double axisVelocityToMachineVelocity(double axisVelocity){
			if(b_invertDirection) return -axisVelocity;
			return axisVelocity;
		}
		double machineVelocityToAxisVelocity(double machineVelocity){
			if(b_invertDirection) return -machineVelocity;
			return machineVelocity;
		}
		double axisAccelerationToMachineAcceleration(double axisAcceleration){
			if(b_invertDirection) return -axisAcceleration;
			return axisAcceleration;
		}
		double machineAccelerationToAxisAcceleration(double machineAcceleration){
			if(b_invertDirection) return -machineAcceleration;
			return machineAcceleration;
		}
	};
	
	std::vector<std::shared_ptr<AxisMapping>> axisMappings = {};
	
	void addAxisMapping();
	void addAxisMapping(std::shared_ptr<AxisMapping> mapping);
	void removeAxisMapping(std::shared_ptr<AxisMapping> mapping);
	
	double masterVelocityCommand = 0.0;
	bool b_enableGroupSurveillance = true;
	bool b_enableTwoAxisDistanceConstraint = false;
	double maxDistanceBetweenAxes = 0.0;
	
	void setMasterVelocityTarget(double velocityTarget);
	
	
	class ControlWidget : public Widget{
	public:
		ControlWidget(std::shared_ptr<MultipointHoist> machine_) : Widget("Machines"), machine(machine_){}
		std::shared_ptr<MultipointHoist> machine;
		virtual void gui() override;
		virtual std::string getName() override {
			return machine->getName();
		}
	};
	std::shared_ptr<ControlWidget> controlWidget;
	void widgetGui();
	
};
