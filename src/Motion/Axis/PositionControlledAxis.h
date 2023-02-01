#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Motion/MotionTypes.h"
#include "Utilities/CircularBuffer.h"
#include "Motion/Curve/Profile.h"

#include "Project/Editor/Parameter.h"

class Device;
namespace Motion { class Interpolation; }

class PositionControlledAxis : public Node {
public:

	DEFINE_NODE(PositionControlledAxis, "Position Controlled Axis", "PositionControlledAxis", Node::Type::AXIS, "")

	//============ PINS ==============
private:
	//Inputs
	std::shared_ptr<bool> lowLimitSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> highLimitSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> referenceSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> surveillanceValidInputSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> externalSurveillanceFaultResetSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> servoActuatorPin = std::make_shared<NodePin>(NodePin::DataType::SERVO_ACTUATOR,
																		  NodePin::Direction::NODE_INPUT_BIDIRECTIONAL,
																		  "Servo Actuator", "ServoActuator");
	std::shared_ptr<NodePin> gpioPin = std::make_shared<NodePin>(NodePin::DataType::GPIO,
																 NodePin::Direction::NODE_INPUT,
																 "Reference Device", "ReferenceDevice");
	std::shared_ptr<NodePin> lowLimitSignalPin = std::make_shared<NodePin>(lowLimitSignal,
																		   NodePin::Direction::NODE_INPUT,
																		   "Low Limit Signal", "LowLimitSignal");
	std::shared_ptr<NodePin> highLimitSignalPin = std::make_shared<NodePin>(highLimitSignal,
																			NodePin::Direction::NODE_INPUT,
																			"High Limit Signal", "HighLimitSignal");
	std::shared_ptr<NodePin> referenceSignalPin = std::make_shared<NodePin>(referenceSignal,
																			NodePin::Direction::NODE_INPUT,
																			"Reference Signal", "ReferenceSignal");
	std::shared_ptr<NodePin> surveillanceValidInputPin = std::make_shared<NodePin>(surveillanceValidInputSignal,
																				   NodePin::Direction::NODE_INPUT,
																				   "Surveillance Valid Input", "SurveillanceValidInput");
	std::shared_ptr<NodePin> surveillanceFeedbackDevicePin = std::make_shared<NodePin>(NodePin::DataType::POSITION_FEEDBACK,
																					   NodePin::Direction::NODE_INPUT_BIDIRECTIONAL,
																					   "Surveillance Feedback Device", "SurveillanceFeedbackDevice");
	std::shared_ptr<NodePin> externalSurveillanceFaultResetPin = std::make_shared<NodePin>(externalSurveillanceFaultResetSignal,
																						   NodePin::Direction::NODE_INPUT,
																						   "External Surveillance Fault Reset", "ExternalSurveillanceFaultReset");
	std::shared_ptr<NodePin> feedbackDevicePin = std::make_shared<NodePin>(NodePin::DataType::POSITION_FEEDBACK,
																		   NodePin::Direction::NODE_INPUT_BIDIRECTIONAL,
																		   "Feedback Device", "FeedbackDevice");
	
	
	//Outputs
	std::shared_ptr<double> actualPositionValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> actualVelocityValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> actualLoadValue = std::make_shared<double>(0.0);
	std::shared_ptr<bool> surveillanceValidOutputSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> surveillanceFaultResetSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> surveillanceValidOutputPin = std::make_shared<NodePin>(surveillanceValidOutputSignal,
																					NodePin::Direction::NODE_OUTPUT,
																					"Surveillance Valid Output", "SurveillanceValidOutput");
	std::shared_ptr<NodePin> surveillanceFaultResetPin = std::make_shared<NodePin>(surveillanceFaultResetSignal,
																				   NodePin::Direction::NODE_OUTPUT,
																				   "Surveillance Fault Reset", "SurveillanceFaultReset");
	std::shared_ptr<NodePin> axisPin = std::make_shared<NodePin>(NodePin::DataType::POSITION_CONTROLLED_AXIS,
																 NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL,
																 "Position Controlled Axis", "PositionControlledAxis");
	std::shared_ptr<NodePin> positionPin = std::make_shared<NodePin>(actualPositionValue,
																	 NodePin::Direction::NODE_OUTPUT,
																	 "Position", "Position");
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(actualVelocityValue,
																	 NodePin::Direction::NODE_OUTPUT,
																	 "Velocity", "Velocity");
	std::shared_ptr<NodePin> loadPin = std::make_shared<NodePin>(actualLoadValue,
																 NodePin::Direction::NODE_OUTPUT,
																 "Load", "Load");
	
	
public:
	DeviceState getState(){ return state; }
	std::string getStatusString();
	bool isEmergencyStopActive(){ return b_emergencyStopActive; }
	
private:
	DeviceState state = DeviceState::OFFLINE;
	bool b_emergencyStopActive = false;
	
	//============= CHECK CONNECTIONS ==============
private:
	
	bool isAxisPinConnected(){ return axisPin->isConnected(); }
	
	bool areAllPinsConnected();
	bool areAllDevicesReady();

	bool needsReferenceDevice();
	bool isReferenceDeviceConnected(){ return gpioPin->isConnected(); }
	std::shared_ptr<GpioModule> getReferenceDevice() { return gpioPin->getConnectedPin()->getSharedPointer<GpioModule>(); }

	void updateAxisState();
	void reactToReferenceSignals();
	
	bool isServoActuatorDeviceConnected(){ return servoActuatorPin->isConnected(); }
	std::shared_ptr<ActuatorModule> getServoActuatorDevice() { return servoActuatorPin->getConnectedPin()->getSharedPointer<ActuatorModule>(); }

	bool needsSurveillanceFeedbackDevice(){ return b_isSurveilled->value; }
	bool isSurveillanceFeedbackDeviceConnected(){ return surveillanceFeedbackDevicePin->isConnected(); }
	std::shared_ptr<MotionFeedbackModule> getSurveillanceFeedbackDevice(){ return surveillanceFeedbackDevicePin->getConnectedPin()->getSharedPointer<MotionFeedbackModule>(); }
	
	//——————— ONEGIN
	bool isFeedbackDeviceConnected(){ return feedbackDevicePin->isConnected(); }
	std::shared_ptr<MotionFeedbackModule> getFeedbackDevice(){ return feedbackDevicePin->getConnectedPin()->getSharedPointer<MotionFeedbackModule>(); }
	BoolParam useFeedbackDevice_Param = BooleanParameter::make(false, "Use Feedback Device", "UseFeedbackDevice");
	NumberParam<double> feedbackUnitsPerAxisUnits_Param = NumberParameter<double>::make(0.0, "Feedback units per axis unit", "FeedbackUnitsPerAxisUnit", "%.5f");
	double feedbackUnitsToAxisUnits(double feedbackValue) { return feedbackValue / feedbackUnitsPerAxisUnits_Param->value; }
	double axisUnitsToFeedbackUnits(double axisValue) { return axisValue * feedbackUnitsPerAxisUnits_Param->value; }
	double actuatorToFeedbackPositionOffset = 0.0;
	
public:
	void getDevices(std::vector<std::shared_ptr<Device>>& output);
	
	//========== PROCESSING =============
public:
	virtual void inputProcess() override;
	void setMotionCommand(double position, double velocity, double acceleration);
	virtual void outputProcess() override;
	virtual bool needsOutputProcess() override { return !isAxisPinConnected(); }
	
	//==================== PARAMETERS ====================
private:
	//units
	MovementType movementType = MovementType::ROTARY;
	Unit positionUnit = Units::AngularDistance::Degree;
	double servoActuatorUnitsPerAxisUnits = 0.0;

	//Reference Signals and Homing
	PositionReferenceSignal positionReferenceSignal = PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT;
	HomingDirection homingDirection = HomingDirection::NEGATIVE;
	double homingVelocityCoarse = 0.0;
	double homingVelocityFine = 0.0;
	
	EnumParam<SignalApproach> signalApproach = EnumeratorParameter<SignalApproach>::make(SignalApproach::FIND_SIGNAL_EDGE,
																						 "Homing Signal Approach",
																						 "HomingSignalApproach");
	
	
	//kinematic limits
	double velocityLimit = 0.0;
	double accelerationLimit = 0.0;
	double manualAcceleration = 0.0;
	
	//position limits
	bool limitToFeedbackWorkingRange = true;
	double lowPositionLimit = 0.0;
	double lowLimitClearance = 0.0;
	bool b_enableLowLimit = true;
	double highPositionLimit = 0.0;
	double highLimitClearance = 0.0;
	bool b_enableHighLimit = true;
	
	void sanitizeParameters();
	
	void setMovementType(MovementType t);
	void setPositionUnit(Unit u);
	void setPositionReferenceSignalType(PositionReferenceSignal type);
	
	double servoActuatorUnitsToAxisUnits(double actuatorValue) { return actuatorValue / servoActuatorUnitsPerAxisUnits; }
	double axisUnitsToServoActuatorUnits(double axisValue) { return axisValue * servoActuatorUnitsPerAxisUnits; }
	
public:
	Unit getPositionUnit(){ return positionUnit; }
	MovementType getMovementType(){ return movementType; }
	double getVelocityLimit() { return velocityLimit; }
	double getAccelerationLimit() { return accelerationLimit; }
	double getLowPositionLimit();
	double getHighPositionLimit();
	
	//========== STATE ===========
public:
	bool isReadyToEnable();
	bool isMoving();

	void enable();
	void disable();
	
private:
	void onEnable();
	void onDisable();
	
	//========= MOTION
private:
	Motion::Profile motionProfile;
	ControlMode controlMode = ControlMode::VELOCITY_TARGET;
	double profileTime_seconds = 0.0;
	double profileTimeDelta_seconds = 0.0;
	
	//Manual Controls
	void setVelocityTarget(double velocity);
	void fastStop();
	void moveToPositionWithVelocity(double position, double velocity);
	void moveToPositionInTime(double position, double movementTime);
	float manualVelocityTarget = 0.0;
	
	//actuator update
	void sendActuatorCommands();

	//======= MOTION INTERFACE
public:
	double getProfileVelocity() { return motionProfile.getVelocity(); }
	double getProfilePosition() { return motionProfile.getPosition(); }
	double getActualVelocity() { return *actualVelocityValue; }
	double getActualPosition() { return *actualPositionValue; }
	double getActualFollowingError();
	float getActualFollowingErrorNormalized();
	double getFollowingErrorLimit();
	float getActualVelocityNormalized() { return *actualVelocityValue / velocityLimit; }
	float getActualPositionNormalized() {
		double low = getLowPositionLimit();
		double high = getHighPositionLimit();
		return (*actualPositionValue - low) / (high - low);
	}
	double getActualEffort(){ return *actualLoadValue; }
	
	//============== AXIS SETUP
private:
	void setCurrentPosition(double distanceFromAxisOrigin);
	void setCurrentPositionAsNegativeLimit();
	void setCurrentPositionAsPositiveLimit();
	void scaleFeedbackToMatchPosition(double position_axisUnits);
	
public:
	bool isHomeable();
	void startHoming();
	void cancelHoming();
	bool isHoming();
	bool didHomingSucceed();
	bool didHomingFail();
	float getHomingProgress();
	HomingStep getHomingStep();
	HomingStep homingStep = HomingStep::NOT_STARTED;
	HomingError homingError = HomingError::NONE;
	
private:
	bool b_isHoming = false;
	void homingControl();
	void onHomingSuccess();
	void onHomingError();

	//============ POSITION REFERENCES AND LIMITS ==============
private:
	//limit and reference signals
	void updateReferenceSignals();
	bool previousLowLimitSignal = false;
	bool previousHighLimitSignal = false;
	bool previousReferenceSignal = false;

	//feedback position limits
	double getLowPositionLimitWithoutClearance();
	double getHighPositionLimitWithoutClearance();
	double getLowFeedbackPositionLimit();
	double getHighFeedbackPositionLimit();
	double getRange();
		
	//================ SURVEILLANCE ==================
	
	bool b_hasSurveillanceError = true;
	bool b_isClearingSurveillanceError = false;
	
	std::shared_ptr<BooleanParameter> b_isSurveilled = BooleanParameter::make(false,
																			  "Axis is Surveilled",
																			  "AxisIsSurveilled");
	std::shared_ptr<NumberParameter<double>> surveillancefeedbackUnitsPerAxisUnits = NumberParameter<double>::make(0.0,
																												   "Surveillance Feedback Units Per Axis Units",
																												   "SurveillanceFeedbackUnitsPerAxisUnits",
																												   "%.6f");
	std::shared_ptr<NumberParameter<double>> maxVelocityDeviation = NumberParameter<double>::make(0.0,
																								  "Max Surveillance Velocity Deviation",
																								  "MaxSurveillanceVelocityDeviation",
																								  "%.3f");
	std::shared_ptr<NumberParameter<double>> maxSurveillanceErrorClearTime = NumberParameter<double>::make(0.0,
																										   "Max Surveillance Error Clear Time",
																										   "MaxSurveillanceErrorClearTime",
																										   "%.3f",
																										   Units::Time::Second);

	double surveillanceUnitsToAxisUnits(double surveillanceValue) { return surveillanceValue / surveillancefeedbackUnitsPerAxisUnits->value; }
	double axisUnitsToSurveillanceUnits(double axisValue) { return axisValue * surveillancefeedbackUnitsPerAxisUnits->value; }
	
	double surveillanceVelocity = 0.0;
	double surveillanceVelocityError = 0.0;
	
	bool isSurveilled(){ return b_isSurveilled->value; }
	void updateSurveillance();
	
	void clearSurveillanceFault();
	void triggerSurveillanceFault();
	
	void setSurveillance(bool isSurveilled);
	
	//============= METRICS ============

	void updateMetrics();
	const size_t historyLength = 2048;
	CircularBuffer positionHistory = CircularBuffer(historyLength);
	CircularBuffer actualPositionHistory = CircularBuffer(historyLength);
	CircularBuffer positionErrorHistory = CircularBuffer(historyLength);
	CircularBuffer velocityHistory = CircularBuffer(historyLength);
	CircularBuffer accelerationHistory = CircularBuffer(historyLength);
	CircularBuffer loadHistory = CircularBuffer(historyLength);
	
	//============ LOADING & SAVING ============
public:
	virtual bool load(tinyxml2::XMLElement* xml) override;
	virtual bool save(tinyxml2::XMLElement* xml) override;

	//============== GUI ================

	virtual void nodeSpecificGui() override;
	virtual void controlsGui();
	virtual void settingsGui();
	virtual void devicesGui();
	virtual void metricsGui();
	float manualVelocityTargetDisplay = 0.0;
	double interpolationPositionTarget = 0.0;
	double interpolationVelocityTarget = 0.0;
	double interpolationTimeTarget = 0.0;
	double axisScalingPosition = 0.0;

};
