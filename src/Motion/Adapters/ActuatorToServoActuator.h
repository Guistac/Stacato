#include "NodeGraph/Node.h"

#include "NodeGraph/Node.h"
#include "Motion/MotionTypes.h"
#include "Motion/SubDevice.h"
#include "Motion/Profile/Profile.h"
#include "Motion/Curve/Curve.h"

#include "Utilities/CircularBuffer.h"

class ActuatorToServoActuator : public Node{
public:
	
	DEFINE_NODE(ActuatorToServoActuator, "Actuator To Servo Actuator", "ActuatorToServoActuator", Node::Type::PROCESSOR, "Adapters")
	
	//output data
	std::shared_ptr<ServoActuatorDevice> servoActuator = std::make_shared<ServoActuatorDevice>("Servo Actuator", PositionUnit::REVOLUTION, PositionFeedbackType::ABSOLUTE);
	std::shared_ptr<NodePin> servoActuatorPin = std::make_shared<NodePin>(servoActuator, NodePin::Direction::NODE_OUTPUT, "Servo Actuator");
	
	//input data
	std::shared_ptr<NodePin> positionFeedbackPin = std::make_shared<NodePin>(NodePin::DataType::POSITIONFEEDBACK, NodePin::Direction::NODE_INPUT, "Position Feedback");
	std::shared_ptr<NodePin> actuatorPin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR, NodePin::Direction::NODE_INPUT, "Actuator");
	
	//pin checking
	bool isPositionFeedbackConnected(){ return positionFeedbackPin->isConnected(); }
	std::shared_ptr<PositionFeedbackDevice> getPositionFeedbackDevice(){ return positionFeedbackPin->getConnectedPin()->getSharedPointer<PositionFeedbackDevice>(); }
		
	bool isActuatorConnected(){ return actuatorPin->isConnected(); }
	std::shared_ptr<ActuatorDevice> getActuatorDevice(){ return actuatorPin->getConnectedPin()->getSharedPointer<ActuatorDevice>(); }
	
	bool isServoActuatorPinConnected(){ return servoActuatorPin->isConnected(); }
	
	bool areAllPinsConnected();
	
	//processing
	virtual void process();
	virtual void updatePin(std::shared_ptr<NodePin> pin);
	void onDisable();
	void onEnable();
	
	//gui stuff
	virtual void nodeSpecificGui();
	void controlGui();
	void settingsGui();
	float manualVelocityDisplay = 0.0;
	float targetPositionDisplay = 0.0;
	float targetVelocityDisplay = 0.0;
	float targetTimeDisplay = 0.0;
	
private:
	
	//parameters
	double positionFeedbackUnitsPerActuatorUnit = 0.0;
	
	//control parameters
	double velocityLoopProportionalGain = 0.0;
	double velocityLoopIntegralGain = 0.0;
	double positionLoopProportionalGain = 0.0;
	double maxPositionFollowingError = 0.0;
	
	PositionUnit getPositionUnit(){ return getActuatorDevice()->getPositionUnit(); }
	
	void sanitizeParameters();
	
public:
	
	double feedbackUnitsToActuatorUnits(double feedbackValue){ return feedbackValue / positionFeedbackUnitsPerActuatorUnit; }
	double actuatorUnitsToFeedbackUnits(double actuatorValue){ return actuatorValue * positionFeedbackUnitsPerActuatorUnit; }
	
private:
	
	Motion::Profile motionProfile;
	ControlMode controlMode = ControlMode::VELOCITY_TARGET;
	double profileTime_seconds = 0.0;
	double profileTimeDelta_seconds = 0.0;
	
	double manualAcceleration = 0.0;
	double manualVelocityTarget = 0.0;
	void setVelocityTarget(double targetVelocity);
	void fastStop();
	void moveToPositionInTime(double targetPosition, double targetTime);
	void movetoPositionWithVelocity(double targetPosition, double targetVelocity);
	
	void controlLoop();
	
	void enable();
	void disable();
	
	const size_t historyLength = 2048;
	CircularBuffer positionTargetHistory = CircularBuffer(historyLength);
	CircularBuffer positionHistory = CircularBuffer(historyLength);
	CircularBuffer velocityTargetHistory = CircularBuffer(historyLength);
	CircularBuffer velocityHistory = CircularBuffer(historyLength);
	CircularBuffer positionErrorHistory = CircularBuffer(historyLength);
	CircularBuffer velocityErrorHistory = CircularBuffer(historyLength);
	
	double targetPosition = 0.0;
	double targetVelocity = 0.0;
	double realPosition = 0.0;
	double realVelocity = 0.0;
	double positionError = 0.0;
	double velocityError = 0.0;
	double outputVelocity;
	
public:
	

	//saving & loading
	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool save(tinyxml2::XMLElement* xml);
};
