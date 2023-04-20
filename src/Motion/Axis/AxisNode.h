#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Motion/Interfaces.h"
#include "Motion/Curve/Profile.h"

#include "Legato/Editor/Parameters.h"

class FeedbackMapping;
class ActuatorMapping;

class AxisNode : public Node {
public:
	
	DEFINE_NODE(AxisNode, Node::Type::PROCESSOR, "")
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override {
		Node::onCopyFrom(source);
	};
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	virtual bool loadAfterPinConnection() override;
	
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
	
	virtual void nodeSpecificGui() override;
	
	void controlTab();
	void configurationTab();
	void devicesTab();
	void positionControlSettingsGui();
	void actuatorControlSettingsGui();
	void limitSettingsGui();
	void motionFeedbackSettingsGui();
	void homingSettingsGui();
	void axisInterfaceTab();
	
private:
	
	friend class ActuatorMapping;
	friend class FeedbackMapping;
	
	//——— Parameters
	enum MovementType{
		LINEAR = 0,
		ANGULAR = 1
	}movementType = LINEAR;
	Legato::Option option_movementType_Linear = 	Legato::Option(MovementType::LINEAR, "Linear Motion", "LinearMotion");
	Legato::Option option_movementType_Angular = 	Legato::Option(MovementType::ANGULAR, "Angular Motion", "AngularMotion");
	Legato::OptionParam movementTypeParameter;
	 
	Legato::Option option_positionUnit_Millimeter = Legato::Option(0, "Millimeter", "Millimeter");
	Legato::Option option_positionUnit_Centimeter = Legato::Option(1, "Centimeter", "Centimeter");
	Legato::Option option_positionUnit_Meter = 		Legato::Option(2, "Meter", "Meter");
	Legato::Option option_positionUnit_Degree = 	Legato::Option(3, "Degree", "Degree");
	Legato::Option option_positionUnit_Radian = 	Legato::Option(4, "Radian", "Radian");
	Legato::Option option_positionUnit_Revolution = Legato::Option(5, "Revolution", "Revolution");
	Legato::OptionParam positionUnitParameter;
	
	enum ControlMode{
		POSITION_CONTROL = 0,
		VELOCITY_CONTROL = 1,
		NO_CONTROL = 2
	}controlMode = NO_CONTROL;
	Legato::Option option_controlMode_Position =	Legato::Option(ControlMode::POSITION_CONTROL, "Position Control", "PositionControl");
	Legato::Option option_controlMode_Velocity =	Legato::Option(ControlMode::VELOCITY_CONTROL, "Velocity Control", "VelocityControl");
	Legato::Option option_controlMode_None =		Legato::Option(ControlMode::NO_CONTROL, "No Control", "No Control");
	Legato::OptionParam controlModeParameter;
	
	enum LimitSignalType{
		NONE = 0,
		SIGNAL_AT_LOWER_LIMIT = 1,
		SIGNAL_AT_LOWER_AND_UPPER_LIMITS = 2,
		SIGNAL_AT_ORIGIN = 3,
		LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS = 4
	}limitSignalType = NONE;
	Legato::Option option_LimitSignalType_NoLimitSignal = Legato::Option(LimitSignalType::NONE,
																		 "No limit signal", "NoLimitSignal");
	Legato::Option option_LimitSignalType_SignalAtLowerLimit = Legato::Option(LimitSignalType::SIGNAL_AT_LOWER_LIMIT,
																			  "Signal at lower limit", "SignalAtLowerLimit");
	Legato::Option option_LimitSignalType_SignalAtLowerAndUpperLimits = Legato::Option(LimitSignalType::SIGNAL_AT_LOWER_AND_UPPER_LIMITS,
																					   "Signal at lower and upper limit", "SignalAtLowerAndUpperLimit");
	Legato::Option option_LimitSignalType_SignalAtOrigin = Legato::Option(LimitSignalType::SIGNAL_AT_ORIGIN,
																		  "Signal at origin", "SignalAtOrigin");
	Legato::Option option_LimitSignalType_LimitAndSlowdownAtLowerAndUpperLimits = Legato::Option(LimitSignalType::LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS,
																								 "Limit and slowdown signals", "LimitAndSlowdownSignals");
	Legato::OptionParam limitSignalTypeParameter;
	
	enum HomingDirection{
		NEGATIVE = 0,
		POSITIVE = 1
	}homingDirection = NEGATIVE;
	Legato::Option option_HomingDirection_Negative = Legato::Option(HomingDirection::NEGATIVE, "Negative", "Negative");
	Legato::Option option_HomingDirection_Positive = Legato::Option(HomingDirection::POSITIVE, "Positive", "Positive");
	
	enum SignalApproachMethod{
		FIND_SIGNAL_EDGE = 0,
		FIND_SIGNAL_CENTER = 1
	}signalApproachMethod = FIND_SIGNAL_EDGE;
	Legato::Option option_SignalApproachMethod_FindSignalEdge = 	Legato::Option(SignalApproachMethod::FIND_SIGNAL_EDGE,
																									"Find signal edge", "FindSignalEdge");
	Legato::Option option_SignalApproachMethod_FindSignalCenter = 	Legato::Option(SignalApproachMethod::FIND_SIGNAL_CENTER,
																									"Find signal center", "FindSignalCenter");
	
	//General
	Legato::NumberParam<double> maxEnableTimeSeconds;
	
	//Position Control
	Legato::NumberParam<double> positionLoop_velocityFeedForward;
	Legato::NumberParam<double> positionLoop_proportionalGain;
	Legato::NumberParam<double> positionLoop_maxError;
	Legato::NumberParam<double> positionLoop_minError;
	
	//Velocity Control
	Legato::NumberParam<double> limitSlowdownVelocity;
	Legato::NumberParam<double> velocityLoop_maxError;
	
	//Limits
	double actuatorVelocityLimit;
	double actuatorAccelerationLimit;
	double feedbackLowerPositionLimit;
	double feedbackUpperPositionLimit;
	Legato::BoolParam enableLowerPositionLimit;
	Legato::BoolParam enableUpperPositionLimit;
	Legato::BoolParam limitPositionToFeedbackWorkingRange;
	Legato::NumberParam<double> lowerPositionLimit;
	Legato::NumberParam<double> upperPositionLimit;
	Legato::NumberParam<double> lowerPositionLimitClearance;
	Legato::NumberParam<double> upperPositionLimitClearance;
	Legato::NumberParam<double> velocityLimit;
	Legato::NumberParam<double> accelerationLimit;
	
	//Homing
	Legato::OptionParam homingDirectionParameter;
	Legato::OptionParam signalApproachParameter;
	Legato::NumberParam<double> homingVelocityCoarse;
	Legato::NumberParam<double> homingVelocityFine;
	Legato::NumberParam<double> maxHomingDistanceCoarse;
	Legato::NumberParam<double> maxHomingDistanceFine;
	
	//Surveillance
	Legato::BoolParam enableSurveillanceParameter = Legato::BooleanParameter::createInstance(false, "Enable Surveillance", "EnableSurveillance");
	std::shared_ptr<MotionFeedbackInterface> surveillanceFeedbackModule;
	double surveillanceFeedbackUnitsPerAxisUnits;
	
	//parameter callbacks
	void updateConnectedModules();
	void updateMovementType();
	void updatePositionUnit();
	void updateControlMode();
	void updateLimitSignalType();
	void updateAxisConfiguration();
	
	
	
	
	//——— Connected Device Interfaces
	std::vector<std::shared_ptr<DeviceInterface>> connectedDeviceInterfaces;
	std::vector<std::shared_ptr<ActuatorInterface>> connectedActuatorInterfaces;
	std::vector<std::shared_ptr<MotionFeedbackInterface>> connectedFeedbackInteraces;
	std::vector<std::shared_ptr<GpioInterface>> connectedGpioInterfaces;
	std::shared_ptr<FeedbackMapping> positionFeedbackMapping = nullptr;
	std::shared_ptr<FeedbackMapping> velocityFeedbackMapping = nullptr;
	std::vector<std::shared_ptr<ActuatorMapping>> actuatorMappings;
	
	
	
	
	//——— Process Data
	Motion::Profile motionProfile;
	double profileTime_seconds;
	double profileTimeDelta_seconds;
	double enableRequestTime_seconds;
	bool b_isEnabling = false;
	///bool b_isDisabling = false;
	enum class InternalControlMode{
		MANUAL_VELOCITY_TARGET,				//command through the axis velocity slider
		MANUAL_POSITION_INTERPOLATION,		//command through the axis start move buttons
		HOMING_VELOCITY_TARGET,				//used by homing routine
		HOMING_POSITION_INTERPOLATION,		//used by homing routine
		EXTERNAL_POSITION_TARGET,			//used for external control through axis interface
		EXTERNAL_VELOCITY_TARGET,			//used for external control through axis interface
		NO_CONTROL							//when no control is necessary
	}internalControlMode = InternalControlMode::MANUAL_VELOCITY_TARGET;
	float internalVelocityTarget = 0.0;
	double positionFollowingError = 0.0;
	double velocityFollowingError = 0.0;
	double lowerPositionLimitWithoutClearance;
	double upperPositionLimitWithoutClearance;
		
	
	//——— Homing
	
	void startHoming();
	void stopHoming();
	void setHomingVelocityTarget(double velocity);
	void moveToHomingPositionTarget(double position);
	void overrideCurrentPosition(double newPosition);
	bool previousLowerLimitSignal = false;
	bool previousUpperLimitSignal = false;
	bool previousReferenceSignal = false;
	double homingOriginLowerEdgePosition = 0.0;
	double homingOriginUpperEdgePosition = 0.0;
	double homingOriginCenterPosition = 0.0;
	void homingControl();
	void homingRoutine_HomingOnCurrentPosition();
	void homingRoutine_HomeToLowerLimitSignal();
	void homingRoutine_HomeToLowerThenUpperLimitSignal();
	void homingRoutine_HomeToUpperThenLowerLimitSignal();
	void homingRoutine_HomingOnReferenceSignalCenter();
	void homingRoutine_HomingOnReferenceSignalEdge();
	
	enum class HomingStep{
		//general
		NOT_STARTED,
		RESETTING_POSITION_FEEDBACK,
		FINISHING,
		FINISHED,
		FAILED,
		//steps for searching low limit
		SEARCHING_LOW_LIMIT_COARSE,
		FOUND_LOW_LIMIT_COARSE,
		SEARCHING_LOW_LIMIT_FINE,
		FOUND_LOW_LIMIT,
		//stepts for searching high limit
		SEARCHING_HIGH_LIMIT_COARSE,
		FOUND_HIGH_LIMIT_COARSE,
		SEARCHING_HIGH_LIMIT_FINE,
		FOUND_HIGH_LIMIT,
		//steps for searching origin
		SEARCHING_ORIGIN_UPPER_EDGE_COARSE,
		FOUND_ORIGIN_UPPER_EDGE_COARSE,
		SEARCHING_ORIGIN_UPPER_EDGE_FINE,
		FOUND_ORIGIN_UPPER_EDGE,
		SEARCHING_ORIGIN_LOWER_EDGE_COARSE,
		FOUND_ORIGIN_LOWER_EDGE_COARSE,
		SEARCHING_ORIGIN_LOWER_EDGE_FINE,
		FOUND_ORIGIN_LOWER_EDGE,
		MOVING_TO_ORIGIN_CENTER
	}homingStep = HomingStep::NOT_STARTED;
	std::string getHomingStepString();
	
	
	//——— Ratio Utility
	void updateFeedbackRatioToMatchPosition();
	void onFeedbackRatioUpdate();
	double newPositionForFeedbackRatio = 0.0;
	bool b_shouldUpdateFeedbackRatio = false;
	
	
	//——— Manual Controls
	
	void setManualVelocityTarget(double velocity);
	void moveToManualPositionTargetWithTime(double position, double time, double acceleration);
	void moveToManualPositionTargetWithVelocity(double position, double velocity, double acceleration);
	float manualPositionEntry = 0.0;
	float manualVelocityEntry = 0.0;
	float manualAccelerationEntry = 0.0;
	float manualTimeEntry = 0.0;
	
};





//Feedback
class FeedbackMapping{
public:
	FeedbackMapping(std::shared_ptr<NodePin> interfacePin, std::shared_ptr<AxisNode> axisNode_) : axisNode(axisNode_) {
		feedbackInterface = interfacePin->getSharedPointer<MotionFeedbackInterface>();
		interfacePinID = interfacePin->getUniqueID();
		feedbackUnitsPerAxisUnit = Legato::NumberParameter<double>::createInstance(1.0, "Feedback units per axis unit", "UnitConversion");
		
		feedbackUnitsPerAxisUnit->addEditCallback([this](){ axisNode->updateAxisConfiguration(); });
	}
	std::shared_ptr<MotionFeedbackInterface> feedbackInterface;
	Legato::NumberParam<double> feedbackUnitsPerAxisUnit;
	int interfacePinID = 0;
	std::shared_ptr<AxisNode> axisNode;
};

//Actuators
class ActuatorMapping{
public:
	ActuatorMapping(std::shared_ptr<NodePin> actuatorPin, std::shared_ptr<AxisNode> axisNode_) : axisNode(axisNode_) {
		actuatorInterface = actuatorPin->getSharedPointer<ActuatorInterface>();
		interfacePinID = actuatorPin->getUniqueID();

		auto* defaultControlMode = &actuatorMode_None;
		if(actuatorInterface->supportsPositionControl()) defaultControlMode = &actuatorMode_Position;
		else if(actuatorInterface->supportsVelocityControl()) defaultControlMode = &actuatorMode_Velocity;
		else if(actuatorInterface->supportsForceControl()) defaultControlMode = &actuatorMode_Force;
		if(!actuatorInterface->supportsPositionControl()) actuatorMode_Position.disable();
		if(!actuatorInterface->supportsVelocityControl()) actuatorMode_Velocity.disable();
		if(!actuatorInterface->supportsForceControl()) actuatorMode_Force.disable();
		
		controlModeParameter = Legato::OptionParameter::createInstance(*defaultControlMode, {
																		&actuatorMode_None,
																		&actuatorMode_Position,
																		&actuatorMode_Velocity,
																		&actuatorMode_Force},
																	   "Control Mode", "ControlMode");
		
		controlModeParameter->addEditCallback([this](){
			controlMode = (ActuatorControlMode)controlModeParameter->getValue();
		});
		controlModeParameter->onEdit();
		actuatorUnitsPerAxisUnits = Legato::NumberParameter<double>::createInstance(1.0, "Actuator units per axis units", "UnitConversion");
		
		actuatorUnitsPerAxisUnits->addEditCallback([this](){ axisNode->updateAxisConfiguration(); });
		controlModeParameter->addEditCallback([this](){ axisNode->updateControlMode(); });
	}
	
	enum ActuatorControlMode{
		NO_CONTROL = 0,
		POSITION_CONTROL = 1,
		VELOCITY_CONTROL = 2,
		FORCE_CONTROL = 3
	}controlMode = ActuatorControlMode::NO_CONTROL;
	Legato::Option actuatorMode_None = 	Legato::Option(0, "Disabled", "Disabled");
	Legato::Option actuatorMode_Position = Legato::Option(1, "Position Control", "PositionControl");
	Legato::Option actuatorMode_Velocity = Legato::Option(2, "Velocity Control", "VelocityControl");
	Legato::Option actuatorMode_Force = 	Legato::Option(3, "Force Control", "ForceControl");
	Legato::OptionParam controlModeParameter;
	
	Legato::NumberParam<double> actuatorUnitsPerAxisUnits;
	
	double actuatorPositionOffset = 0.0;
	std::shared_ptr<ActuatorInterface> actuatorInterface;
	int interfacePinID = 0;
	std::shared_ptr<AxisNode> axisNode;
};
