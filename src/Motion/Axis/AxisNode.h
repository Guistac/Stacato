#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Motion/Interfaces.h"
#include "Project/Editor/Parameter.h"
#include "Motion/Curve/Profile.h"

class AxisNode : public Node {
public:
	
	DEFINE_NODE(AxisNode, "Axis", "Axis", Node::Type::AXIS, "")
	
	std::shared_ptr<AxisInterface> axisInterface;
	
	std::shared_ptr<NodePin> actuatorPin;
	std::shared_ptr<NodePin> feedbackPin;
	std::shared_ptr<NodePin> gpioPin;
	std::shared_ptr<NodePin> lowerLimitSignalPin;
	std::shared_ptr<NodePin> upperLimitSignalPin;
	std::shared_ptr<NodePin> lowerSlowdownSignalPin;
	std::shared_ptr<NodePin> upperSlowdownSignalPin;
	std::shared_ptr<NodePin> referenceSignalPin;
	std::shared_ptr<NodePin> surveillanceResetSignalPin;
	
	std::shared_ptr<NodePin> axisPin;
	std::shared_ptr<NodePin> brakeControlSignalPin;
	std::shared_ptr<NodePin> surveillanceValidSignalPin;
	
	std::shared_ptr<bool> lowerLimitSignal;
	std::shared_ptr<bool> upperLimitSignal;
	std::shared_ptr<bool> lowerSlowdownSignal;
	std::shared_ptr<bool> upperSlowdownSignal;
	std::shared_ptr<bool> referenceSignal;
	std::shared_ptr<bool> surveillanceResetSignal;
	
	std::shared_ptr<bool> brakeControlSignal;
	std::shared_ptr<bool> surveillanceValidSignal;
	
	virtual bool prepareProcess() override;
	virtual void inputProcess() override;
	virtual void outputProcess() override;
	virtual bool needsOutputProcess() override;
	
	virtual void onPinUpdate(std::shared_ptr<NodePin> pin) override;
	virtual void onPinConnection(std::shared_ptr<NodePin> pin) override;
	virtual void onPinDisconnection(std::shared_ptr<NodePin> pin) override;
	
	virtual bool load(tinyxml2::XMLElement* xml) override;
	virtual bool save(tinyxml2::XMLElement* xml) override;
	virtual bool loadAfterLinksConnected(tinyxml2::XMLElement* xml) override;
	
	virtual void nodeSpecificGui() override;
	
	void controlTab();
	void configurationTab();
	void devicesTab();
	void positionControlSettingsGui();
	void actuatorControlSettingsGui();
	void limitSettingsGui();
	void motionFeedbackSettingsGui();
	void homingSettingsGui();
	
private:
	 
	static OptionParameter::Option controlModePosition;
	static OptionParameter::Option controlModeVelocity;
	static OptionParameter::Option controlModeNone;
	static std::vector<OptionParameter::Option*> controlModeParameterOptions;
	enum ControlMode{
		POSITION_CONTROL = 0,
		VELOCITY_CONTROL = 1,
		NO_CONTROL = 2
	};
	
	static OptionParameter::Option option_NoLimitSignal;
	static OptionParameter::Option option_SignalAtLowerLimit;
	static OptionParameter::Option option_SignalAtLowerAndUpperLimits;
	static OptionParameter::Option option_SignalAtOrigin;
	static OptionParameter::Option option_LimitAndSlowdownAtLowerAndUpperLimits;
	static std::vector<OptionParameter::Option*> limitSignalTypeOptions;
	enum LimitSignalType{
		NONE = 0,
		SIGNAL_AT_LOWER_LIMIT = 1,
		SIGNAL_AT_LOWER_AND_UPPER_LIMITS = 2,
		SIGNAL_AT_ORIGIN = 3,
		LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS = 4
	};
	
	static OptionParameter::Option option_HomingDirectionNegative;
	static OptionParameter::Option option_HomingDirectionPositive;
	static std::vector<OptionParameter::Option*> homingDirectionOptions;
	enum HomingDirection{
		NEGATIVE = 0,
		POSITIVE = 1
	};
	
	static OptionParameter::Option option_FindSignalEdge;
	static OptionParameter::Option option_FindSignalCenter;
	static std::vector<OptionParameter::Option*> signalApproachOptions;
	enum SignalApproachMethod{
		FIND_SIGNAL_EDGE = 0,
		FIND_SIGNAL_CENTER = 1
	};
	
	void updateConnectedModules();
	std::vector<std::shared_ptr<DeviceInterface>> connectedDeviceInterfaces;
	std::vector<std::shared_ptr<ActuatorInterface>> connectedActuatorInterfaces;
	std::vector<std::shared_ptr<MotionFeedbackInterface>> connectedFeedbackInteraces;
	std::vector<std::shared_ptr<GpioInterface>> connectedGpioInterfaces;
	
	//Feedback
	class FeedbackMapping{
	public:
		FeedbackMapping(std::shared_ptr<NodePin> interfacePin) {
			feedbackInterface = interfacePin->getSharedPointer<MotionFeedbackInterface>();
			interfacePinID = interfacePin->getUniqueID();
		}
		std::shared_ptr<MotionFeedbackInterface> feedbackInterface;
		double feedbackUnitsPerAxisUnit = 1.0;
		int interfacePinID = 0;
	};
	std::shared_ptr<FeedbackMapping> positionFeedbackMapping = nullptr;
	std::shared_ptr<FeedbackMapping> velocityFeedbackMapping = nullptr;
	
	//Actuators
	class ActuatorMapping{
	public:
		ActuatorMapping(std::shared_ptr<NodePin> actuatorPin) {
			actuatorInterface = actuatorPin->getSharedPointer<ActuatorInterface>();
			interfacePinID = actuatorPin->getUniqueID();
		}
		ActuatorInterface::ControlMode controlModeSelection = ActuatorInterface::ControlMode::VELOCITY;
		double actuatorUnitsPerAxisUnits = 1.0;
		double actuatorPositionOffset = 0.0;
		std::shared_ptr<ActuatorInterface> actuatorInterface;
		int interfacePinID = 0;
	};
	std::vector<std::shared_ptr<ActuatorMapping>> actuatorMappings;
	
	//General
	OptionParam controlModeParameter;
	OptionParam limitSignalTypeParameter;
	NumberParam<double> maxEnableTimeSeconds;
	
	//Position Control
	NumberParam<double> positionLoop_velocityFeedForward;
	NumberParam<double> positionLoop_proportionalGain;
	NumberParam<double> positionLoop_maxError;
	NumberParam<double> positionLoop_minError;
	
	//Velocity Control
	NumberParam<double> limitSlowdownVelocity;
	
	//Limits
	BoolParam enableLowerPositionLimit;
	BoolParam enableUpperPositionLimit;
	NumberParam<double> lowerPositionLimit;
	NumberParam<double> upperPositionLimit;
	NumberParam<double> lowerPositionLimitClearance;
	NumberParam<double> upperPositionLimitClearance;
	NumberParam<double> velocityLimit;
	NumberParam<double> accelerationLimit;
	
	//Homing
	OptionParam homingDirectionParameter;
	OptionParam signalApproachParameter;
	NumberParam<double> homingVelocityCoarse;
	NumberParam<double> homingVelocityFine;
	NumberParam<double> maxHomingDistanceCoarse;
	NumberParam<double> maxHomingDistanceFine;
	
	//Surveillance
	BoolParam enableSurveillanceParameter = BooleanParameter::make(false, "Enable Surveillance", "EnableSurveillance");
	std::shared_ptr<MotionFeedbackInterface> surveillanceFeedbackModule;
	double surveillanceFeedbackUnitsPerAxisUnits;
	
	
	
	void updateControlMode();
	void updateLimitSignalType();
	void updateAxisConfiguration();
	
	//process data
	Motion::Profile motionProfile;
	double enableRequestTime_seconds;
	bool b_isEnabling;
	
};


