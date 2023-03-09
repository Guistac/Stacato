#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Motion/Interfaces.h"
#include "Project/Editor/Parameter.h"
#include "Motion/Curve/Profile.h"

class FeedbackMapping;
class ActuatorMapping;

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
	void axisInterfaceTab();
	
private:
	
	friend class ActuatorMapping;
	friend class FeedbackMapping;
	
	//——— Parameters
	enum MovementType{
		LINEAR = 0,
		ANGULAR = 1
	}movementType = LINEAR;
	OptionParameter::Option option_movementType_Linear = 	OptionParameter::Option(MovementType::LINEAR, "Linear Motion", "LinearMotion");
	OptionParameter::Option option_movementType_Angular = 	OptionParameter::Option(MovementType::ANGULAR, "Angular Motion", "AngularMotion");
	OptionParam movementTypeParameter;
	 
	OptionParameter::Option option_positionUnit_Millimeter = 	OptionParameter::Option(0, "Millimeter", "Millimeter");
	OptionParameter::Option option_positionUnit_Centimeter = 	OptionParameter::Option(1, "Centimeter", "Centimeter");
	OptionParameter::Option option_positionUnit_Meter = 		OptionParameter::Option(2, "Meter", "Meter");
	OptionParameter::Option option_positionUnit_Degree = 		OptionParameter::Option(3, "Degree", "Degree");
	OptionParameter::Option option_positionUnit_Radian = 		OptionParameter::Option(4, "Radian", "Radian");
	OptionParameter::Option option_positionUnit_Revolution = 	OptionParameter::Option(5, "Revolution", "Revolution");
	OptionParam positionUnitParameter;
	
	enum ControlMode{
		POSITION_CONTROL = 0,
		VELOCITY_CONTROL = 1,
		NO_CONTROL = 2
	}controlMode = NO_CONTROL;
	OptionParameter::Option option_controlMode_Position =	OptionParameter::Option(ControlMode::POSITION_CONTROL, "Position Control", "PositionControl");
	OptionParameter::Option option_controlMode_Velocity =	OptionParameter::Option(ControlMode::VELOCITY_CONTROL, "Velocity Control", "VelocityControl");
	OptionParameter::Option option_controlMode_None =		OptionParameter::Option(ControlMode::NO_CONTROL, "No Control", "No Control");
	OptionParam controlModeParameter;
	
	enum LimitSignalType{
		NONE = 0,
		SIGNAL_AT_LOWER_LIMIT = 1,
		SIGNAL_AT_LOWER_AND_UPPER_LIMITS = 2,
		SIGNAL_AT_ORIGIN = 3,
		LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS = 4
	}limitSignalType = NONE;
	OptionParameter::Option option_LimitSignalType_NoLimitSignal = OptionParameter::Option(LimitSignalType::NONE,
																						   "No limit signal", "NoLimitSignal");
	OptionParameter::Option option_LimitSignalType_SignalAtLowerLimit = OptionParameter::Option(LimitSignalType::SIGNAL_AT_LOWER_LIMIT,
																								"Signal at lower limit", "SignalAtLowerLimit");
	OptionParameter::Option option_LimitSignalType_SignalAtLowerAndUpperLimits = OptionParameter::Option(LimitSignalType::SIGNAL_AT_LOWER_AND_UPPER_LIMITS,
																										 "Signal at lower and upper limit", "SignalAtLowerAndUpperLimit");
	OptionParameter::Option option_LimitSignalType_SignalAtOrigin = OptionParameter::Option(LimitSignalType::SIGNAL_AT_ORIGIN,
																							"Signal at origin", "SignalAtOrigin");
	OptionParameter::Option option_LimitSignalType_LimitAndSlowdownAtLowerAndUpperLimits =
		OptionParameter::Option(LimitSignalType::LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS,
								"Limit and slowdown signals", "LimitAndSlowdownSignals");
	OptionParam limitSignalTypeParameter;
	
	enum HomingDirection{
		NEGATIVE = 0,
		POSITIVE = 1
	}homingDirection = NEGATIVE;
	OptionParameter::Option option_HomingDirection_Negative = OptionParameter::Option(HomingDirection::NEGATIVE, "Negative", "Negative");
	OptionParameter::Option option_HomingDirection_Positive = OptionParameter::Option(HomingDirection::POSITIVE, "Positive", "Positive");
	
	enum SignalApproachMethod{
		FIND_SIGNAL_EDGE = 0,
		FIND_SIGNAL_CENTER = 1
	}signalApproachMethod = FIND_SIGNAL_EDGE;
	OptionParameter::Option option_SignalApproachMethod_FindSignalEdge = 	OptionParameter::Option(SignalApproachMethod::FIND_SIGNAL_EDGE,
																									"Find signal edge", "FindSignalEdge");
	OptionParameter::Option option_SignalApproachMethod_FindSignalCenter = 	OptionParameter::Option(SignalApproachMethod::FIND_SIGNAL_CENTER,
																									"Find signal center", "FindSignalCenter");
	
	//General
	NumberParam<double> maxEnableTimeSeconds;
	
	//Position Control
	NumberParam<double> positionLoop_velocityFeedForward;
	NumberParam<double> positionLoop_proportionalGain;
	NumberParam<double> positionLoop_maxError;
	NumberParam<double> positionLoop_minError;
	
	//Velocity Control
	NumberParam<double> limitSlowdownVelocity;
	NumberParam<double> velocityLoop_maxError;
	
	//Limits
	double actuatorVelocityLimit;
	double actuatorAccelerationLimit;
	double feedbackLowerPositionLimit;
	double feedbackUpperPositionLimit;
	BoolParam enableLowerPositionLimit;
	BoolParam enableUpperPositionLimit;
	BoolParam limitPositionToFeedbackWorkingRange;
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
		feedbackUnitsPerAxisUnit = NumberParameter<double>::make(1.0, "Feedback units per axis unit", "UnitConversion");
		
		feedbackUnitsPerAxisUnit->addEditCallback([this](){ axisNode->updateAxisConfiguration(); });
	}
	std::shared_ptr<MotionFeedbackInterface> feedbackInterface;
	NumberParam<double> feedbackUnitsPerAxisUnit;
	int interfacePinID = 0;
	std::shared_ptr<AxisNode> axisNode;
};

//Actuators
class ActuatorMapping{
public:
	ActuatorMapping(std::shared_ptr<NodePin> actuatorPin, std::shared_ptr<AxisNode> axisNode_) : axisNode(axisNode_) {
		actuatorInterface = actuatorPin->getSharedPointer<ActuatorInterface>();
		interfacePinID = actuatorPin->getUniqueID();
		actuatorMode_None = 		OptionParameter::Option(0, "Disabled", "Disabled");
		actuatorMode_Position = 	OptionParameter::Option(1, "Position Control", "PositionControl");
		actuatorMode_Velocity = 	OptionParameter::Option(2, "Velocity Control", "VelocityControl");
		actuatorMode_Force = 		OptionParameter::Option(3, "Force Control", "ForceControl");
		actuatorModeOptions = {
			&actuatorMode_None,
			&actuatorMode_Position,
			&actuatorMode_Velocity,
			&actuatorMode_Force
		};
		auto* defaultControlMode = &actuatorMode_None;
		if(actuatorInterface->supportsPositionControl()) defaultControlMode = &actuatorMode_Position;
		else if(actuatorInterface->supportsVelocityControl()) defaultControlMode = &actuatorMode_Velocity;
		else if(actuatorInterface->supportsForceControl()) defaultControlMode = &actuatorMode_Force;
		
		if(!actuatorInterface->supportsPositionControl()) actuatorMode_Position.disable();
		if(!actuatorInterface->supportsVelocityControl()) actuatorMode_Velocity.disable();
		if(!actuatorInterface->supportsForceControl()) actuatorMode_Force.disable();
		
		controlModeParameter = OptionParameter::make(*defaultControlMode, actuatorModeOptions, "Control Mode", "ControlMode");
		controlModeParameter->addEditCallback([this](){
			switch(controlModeParameter->value){
					case ActuatorControlMode::POSITION_CONTROL:
					case ActuatorControlMode::VELOCITY_CONTROL:
					case ActuatorControlMode::FORCE_CONTROL:
						controlMode = (ActuatorControlMode)controlModeParameter->value;
						break;
					case ActuatorControlMode::NO_CONTROL:
					default:
						controlMode = ActuatorControlMode::NO_CONTROL;
						break;
			}
		});
		controlModeParameter->onEdit();
		actuatorUnitsPerAxisUnits = NumberParameter<double>::make(1.0, "Actuator units per axis units", "UnitConversion");
		
		actuatorUnitsPerAxisUnits->addEditCallback([this](){ axisNode->updateAxisConfiguration(); });
		controlModeParameter->addEditCallback([this](){ axisNode->updateControlMode(); });
	}
	
	OptionParameter::Option actuatorMode_None;
	OptionParameter::Option actuatorMode_Position;
	OptionParameter::Option actuatorMode_Velocity;
	OptionParameter::Option actuatorMode_Force;
	std::vector<OptionParameter::Option*> actuatorModeOptions;
	enum ActuatorControlMode{
		NO_CONTROL = 0,
		POSITION_CONTROL = 1,
		VELOCITY_CONTROL = 2,
		FORCE_CONTROL = 3
	}controlMode = ActuatorControlMode::NO_CONTROL;
	
	OptionParam controlModeParameter;
	NumberParam<double> actuatorUnitsPerAxisUnits;
	
	double actuatorPositionOffset = 0.0;
	std::shared_ptr<ActuatorInterface> actuatorInterface;
	int interfacePinID = 0;
	std::shared_ptr<AxisNode> axisNode;
};
