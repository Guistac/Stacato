#include "NodeGraph/Node.h"

#include "NodeGraph/Node.h"
#include "Motion/MotionTypes.h"
#include "Motion/SubDevice.h"
#include "Motion/Profile/Profile.h"
#include "Motion/Curve/Curve.h"

class GpioServoActuator : public Node{
public:
	
	DEFINE_NODE(GpioServoActuator, "Gpio Servo Actuator", "GpioServoActuator", Node::Type::PROCESSOR, "Adapters")
	
	//SERVO ACTUATOR
	std::shared_ptr<ServoActuatorDevice> servoActuator = std::make_shared<ServoActuatorDevice>("Servo Actuator", PositionUnit::REVOLUTION, PositionFeedbackType::ABSOLUTE);
	
	//output data
	std::shared_ptr<bool> enableSignal = std::make_shared<bool>(false);
	std::shared_ptr<double> controlSignal = std::make_shared<double>(0.0);
	
	std::shared_ptr<NodePin> enablePin = std::make_shared<NodePin>(enableSignal, NodePin::Direction::NODE_OUTPUT, "Enable");
	std::shared_ptr<NodePin> controlSignalPin = std::make_shared<NodePin>(controlSignal, NodePin::Direction::NODE_OUTPUT, "Control Signal");
	std::shared_ptr<NodePin> servoActuatorPin = std::make_shared<NodePin>(servoActuator, NodePin::Direction::NODE_OUTPUT, "Servo Actuator");
	
	//input data
	std::shared_ptr<bool> readySignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> brakeSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> emergencyStopSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> positionFeedbackPin = std::make_shared<NodePin>(NodePin::DataType::POSITIONFEEDBACK, NodePin::Direction::NODE_INPUT, "Position Feedback");
	std::shared_ptr<NodePin> gpioDevicePin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_INPUT, "GPIO Device");
	std::shared_ptr<NodePin> readyPin = std::make_shared<NodePin>(readySignal, NodePin::Direction::NODE_INPUT, "Ready");
	std::shared_ptr<NodePin> brakePin = std::make_shared<NodePin>(brakeSignal, NodePin::Direction::NODE_INPUT, "Brake");
	std::shared_ptr<NodePin> emergencyStopPin = std::make_shared<NodePin>(emergencyStopSignal, NodePin::Direction::NODE_INPUT, "Emergency Stop");
	
	//pin checking
	bool isPositionFeedbackConnected(){ return positionFeedbackPin->isConnected(); }
	std::shared_ptr<PositionFeedbackDevice> getPositionFeedbackDevice(){ return positionFeedbackPin->getConnectedPin()->getSharedPointer<PositionFeedbackDevice>(); }
	
	bool isGpioDeviceConnected(){ return gpioDevicePin->isConnected(); }
	std::shared_ptr<GpioDevice> getGpioDevice(){ return gpioDevicePin->getConnectedPin()->getSharedPointer<GpioDevice>(); }
	
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
	double controlSignalRange = 10.0;
	bool b_controlSignalIsCenteredOnZero = true;
	double controlSignalUnitsPerActuatorVelocityUnit = 1.0;
	bool b_invertControlSignal = false;
	double positionFeedbackUnitsPerActuatorUnit = 0.0;
	
	double proportionalGain = 0.0;
	double derivativeGain = 0.0;
	double maxFollowingError = 0.0;
	
	void sanitizeParameters();
	
public:
	double getControlSignalLowLimit();
	double getControlSignalHighLimit();
	double getControlSignalZero();
	double controlSignalToActuatorVelocity(double signal);
	double actuatorVelocityToControlSignal(double velocity);
	double getControlSignalLimitVelocity();
	double feedbackUnitsToActuatorUnits(double feedbackValue);
	
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
	
public:
	

	//saving & loading
	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool save(tinyxml2::XMLElement* xml);
};
