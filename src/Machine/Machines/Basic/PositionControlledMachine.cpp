#include <pch.h>

#include "PositionControlledMachine.h"

#include "Motion/Axis/PositionControlledAxis.h"
#include "Animation/Animatable.h"
#include "Animation/Animation.h"

#include "Animation/Manoeuvre.h"

#include <tinyxml2.h>

#include "Environnement/Environnement.h"

#include "Fieldbus/EtherCatFieldbus.h"

void PositionControlledMachine::initialize() {
	//inputs
	addNodePin(positionControlledAxisPin);
		
	//outputs
	positionPin->assignData(positionPinValue);
	addNodePin(positionPin);
	velocityPin->assignData(velocityPinValue);
	addNodePin(velocityPin);

	//machine parameters
	addAnimatable(positionParameter);
	
	auto thisMachine = std::dynamic_pointer_cast<PositionControlledMachine>(shared_from_this());
	controlWidget = std::make_shared<ControlWidget>(thisMachine, getName());
}

void PositionControlledMachine::onPinUpdate(std::shared_ptr<NodePin> pin){
	if(pin == positionControlledAxisPin) positionParameter->setUnit(getAxis()->getPositionUnit());
}

void PositionControlledMachine::onPinConnection(std::shared_ptr<NodePin> pin){
	if(pin == positionControlledAxisPin) positionParameter->setUnit(getAxis()->getPositionUnit());
}

void PositionControlledMachine::onPinDisconnection(std::shared_ptr<NodePin> pin){
	if(pin == positionControlledAxisPin) positionParameter->setUnit(Units::None::None);
}

void PositionControlledMachine::onAddToNodeGraph(){
	controlWidget->addToDictionnary();
}

void PositionControlledMachine::onRemoveFromNodeGraph(){
	controlWidget->removeFromDictionnary();
}

bool PositionControlledMachine::isHardwareReady() {
	if (!isAxisConnected()) return false;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	if (!axis->isReady()) return false;
	return true;
}

bool PositionControlledMachine::isMoving() {
	if (isAxisConnected()) return getAxis()->isMoving();
	return false;
}

void PositionControlledMachine::enableHardware() {
	if (isReady()) {
		std::thread machineEnabler([this]() {
			using namespace std::chrono;
			std::shared_ptr<PositionControlledAxis> axis = getAxis();
			axis->enable();
			time_point enableRequestTime = system_clock::now();
			while (duration(system_clock::now() - enableRequestTime) < milliseconds(100)) {
				std::this_thread::sleep_for(milliseconds(10));
				if (axis->isEnabled()) {
					b_enabled = true;
					onEnableHardware();
					break;
				}
			}
			});
		machineEnabler.detach();
	}
}

void PositionControlledMachine::disableHardware() {
	b_enabled = false;
	if (isAxisConnected()) getAxis()->disable();
	onDisableHardware();
}

void PositionControlledMachine::onEnableHardware() {
	Logger::info("Enabled Machine {}", getName());
}

void PositionControlledMachine::onDisableHardware() {
	setVelocityTarget(0.0);
	positionParameter->stopAnimationPlayback();
	Logger::info("Disabled Machine {}", getName());
}

bool PositionControlledMachine::isSimulationReady(){
	return isAxisConnected();
}

void PositionControlledMachine::onEnableSimulation() {
	motionProfile.resetInterpolation();
	motionProfile.setVelocity(0.0);
	motionProfile.setAcceleration(0.0);
	setVelocityTarget(0.0);
}

void PositionControlledMachine::onDisableSimulation() {
	positionParameter->stopAnimationPlayback();
	setVelocityTarget(0.0);
	motionProfile.setVelocity(0.0);
	motionProfile.setAcceleration(0.0);
}

void PositionControlledMachine::inputProcess() {
	if (!isAxisConnected()) return;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	//Get Realtime values from axis (for position and velocity pins only)
	double actualPosition_machineUnits = axisPositionToMachinePosition(axis->getActualPosition());
	double actualVelocity_machineUnits = axisVelocityToMachineVelocity(axis->getActualVelocity());
	*positionPinValue = actualPosition_machineUnits;
	*velocityPinValue = actualVelocity_machineUnits;

	//Handle Axis state changes
	if (isEnabled() && !axis->isEnabled()) disable();
	
	//Abort the process if the axis is not enabled or is homing (in which case the axis has full control of motion)
	if (!isEnabled() || isHoming()) {
		//we still need to copy the current axis motion values to the machines motion profile
		//so they they are correct when we start the machine or gain back control after homing is finished
		motionProfile.setPosition(actualPosition_machineUnits);
		motionProfile.setVelocity(actualVelocity_machineUnits);
		motionProfile.setAcceleration(0.0);
		return;
	}
}


void PositionControlledMachine::outputProcess(){
	if (!isAxisConnected()) return;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	
	profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	profileDeltaTime_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
		
	//Update Motion Profile
	switch(controlMode){
			
		case ControlMode::PARAMETER_TRACK:{
			auto value = positionParameter->getAnimationValue()->toPosition();
			
			motionProfile.matchPositionAndRespectPositionLimits(profileDeltaTime_seconds,
																value->position,
																value->velocity,
																value->acceleration,
																rapidAcceleration_machineUnitsPerSecond,
																rapidVelocity_machineUnitsPerSecond,
																getLowPositionLimit(),
																getHighPositionLimit());
			}break;
			
		case ControlMode::VELOCITY_TARGET:{
			motionProfile.matchVelocityAndRespectPositionLimits(profileDeltaTime_seconds,
																manualVelocityTarget_machineUnitsPerSecond,
																rapidAcceleration_machineUnitsPerSecond,
																getLowPositionLimit(),
																getHighPositionLimit(),
																axis->getAccelerationLimit());
			}break;
			
		case ControlMode::POSITION_TARGET:{
			motionProfile.updateInterpolation(profileTime_seconds);
			if(motionProfile.isInterpolationFinished(profileTime_seconds)) setVelocityTarget(0.0);
		}break;
	}
	
	//Send motion values to axis profile
	axis->setMotionCommand(machinePositionToAxisPosition(motionProfile.getPosition()), machineVelocityToAxisVelocity(motionProfile.getVelocity()));
}

void PositionControlledMachine::simulateInputProcess() {
	if (!isAxisConnected()) return;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	//update position and velocity pins
	*positionPinValue = motionProfile.getPosition();
	*velocityPinValue = motionProfile.getVelocity();

	//Update Time
	double profileTime_seconds = Environnement::getTime_seconds();
	double profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();

	//Update Motion Profile
	switch(controlMode){
			
		case ControlMode::PARAMETER_TRACK:{
			auto value = positionParameter->getAnimationValue()->toPosition();
			motionProfile.matchPositionAndRespectPositionLimits(profileDeltaTime_seconds,
																value->position,
																value->velocity,
																value->acceleration,
																rapidAcceleration_machineUnitsPerSecond,
																rapidVelocity_machineUnitsPerSecond,
																getLowPositionLimit(),
																getHighPositionLimit());
			}break;
		case ControlMode::VELOCITY_TARGET:{
			double lowLimit_machineUnits = getLowPositionLimit();
			double highLimit_machineUnits = getHighPositionLimit();
			double accelerationLimit = axis->getAccelerationLimit();
			motionProfile.matchVelocityAndRespectPositionLimits(profileDeltaTime_seconds,
																  manualVelocityTarget_machineUnitsPerSecond,
																  rapidAcceleration_machineUnitsPerSecond,
																  lowLimit_machineUnits,
																  highLimit_machineUnits,
																  accelerationLimit);
		}break;
			
		case ControlMode::POSITION_TARGET:{
			motionProfile.updateInterpolation(profileTime_seconds);
			if(motionProfile.isInterpolationFinished(profileTime_seconds)) setVelocityTarget(0.0);
		}break;
			
	}
	
}

void PositionControlledMachine::simulateOutputProcess(){}

bool PositionControlledMachine::canStartHoming(){
	return isEnabled() && !Environnement::isSimulating();
}

bool PositionControlledMachine::isHoming(){
	return isAxisConnected() && getAxis()->isHoming();
}
void PositionControlledMachine::startHoming(){
	positionParameter->stopAnimationPlayback();
	getAxis()->startHoming();
}
void PositionControlledMachine::stopHoming(){
	getAxis()->cancelHoming();
}
bool PositionControlledMachine::didHomingSucceed(){
	return isAxisConnected() && getAxis()->didHomingSucceed();
}
bool PositionControlledMachine::didHomingFail(){
	return isAxisConnected() && getAxis()->didHomingFail();
}
float PositionControlledMachine::getHomingProgress(){
	return getAxis()->getHomingProgress();
}
const char* PositionControlledMachine::getHomingStateString(){
	if(!isAxisConnected()) return "No axis is connected to machine";
	return Enumerator::getDisplayString(getAxis()->getHomingStep());
}


bool PositionControlledMachine::isAxisConnected() {
	return positionControlledAxisPin->isConnected();
}

std::shared_ptr<PositionControlledAxis> PositionControlledMachine::getAxis() {
	return positionControlledAxisPin->getConnectedPins().front()->getSharedPointer<PositionControlledAxis>();
}



//===================== MANUAL CONTROLS =========================

void PositionControlledMachine::setVelocityTarget(double velocityTarget_machineUnitsPerSecond) {
	positionParameter->stopAnimationPlayback();
	
	manualVelocityTarget_machineUnitsPerSecond = velocityTarget_machineUnitsPerSecond;
	if(controlMode == ControlMode::POSITION_TARGET) motionProfile.resetInterpolation();
	controlMode = ControlMode::VELOCITY_TARGET;
}

void PositionControlledMachine::moveToPosition(double target_machineUnits) {
	positionParameter->stopAnimationPlayback();
	
	double lowLimit_machineUnits = getLowPositionLimit();
	double highLimit_machineUnits = getHighPositionLimit();
	target_machineUnits = std::min(target_machineUnits, highLimit_machineUnits);
	target_machineUnits = std::max(target_machineUnits, lowLimit_machineUnits);
	
	double startTime;
	if(isSimulating()) startTime = Environnement::getTime_seconds();
	else startTime = EtherCatFieldbus::getCycleProgramTime_seconds();
	
	bool success = motionProfile.moveToPositionWithVelocity(startTime,
															target_machineUnits,
															rapidVelocity_machineUnitsPerSecond,
															rapidAcceleration_machineUnitsPerSecond);
	
	double target = motionProfile.getInterpolationTarget();
	double left = motionProfile.getRemainingInterpolationTime(startTime);
	
	if(success) controlMode = ControlMode::POSITION_TARGET;
	else setVelocityTarget(0.0);
}






void PositionControlledMachine::rapidAnimatableToValue(std::shared_ptr<Animatable> animatable, std::shared_ptr<AnimationValue> value) {
	if (animatable == positionParameter) {
		positionParameter->stopAnimationPlayback();
		moveToPosition(value->toPosition()->position);
	}
}

float PositionControlledMachine::getAnimatableRapidProgress(std::shared_ptr<Animatable> animatable) {
	if (animatable == positionParameter) {
		float progress = motionProfile.getInterpolationProgress(Environnement::getTime_seconds());
		return progress;
	}
	return 0.0;
}

void PositionControlledMachine::cancelAnimatableRapid(std::shared_ptr<Animatable> animatable) {
	if (animatable == positionParameter) {
		setVelocityTarget(0.0);
	}
}

bool PositionControlledMachine::isAnimatableReadyToStartPlaybackFromValue(std::shared_ptr<Animatable> animatable, std::shared_ptr<AnimationValue> value) {
	if (animatable == positionParameter) {
		return motionProfile.getPosition() == value->toPosition()->position && motionProfile.getVelocity() == 0.0;
	}
	return false;
}

void PositionControlledMachine::onAnimationPlaybackStart(std::shared_ptr<Animatable> animatable) {
	if (animatable == positionParameter) {
		controlMode = ControlMode::PARAMETER_TRACK;
	}
}

void PositionControlledMachine::onAnimationPlaybackInterrupt(std::shared_ptr<Animatable> animatable) {
	//here we just set the velocity target to 0 regardless of where we are at in the manoeuvre
	if (animatable == positionParameter) {
		setVelocityTarget(0.0);
	}
}

void PositionControlledMachine::onAnimationPlaybackEnd(std::shared_ptr<Animatable> animatable) {
	//here we have to make sure that the last position of the manoeuvre stays in the motion profile on the next loop
	//to make sure of this we manually set the profile velocity to 0.0, and the target velocity to 0.0 to make sure nothing moves after the manoeuvre is done playing
	if (animatable == positionParameter) {
		motionProfile.setVelocity(0.0);
		setVelocityTarget(0.0);
	}
}

std::shared_ptr<AnimationValue> PositionControlledMachine::getActualAnimatableValue(std::shared_ptr<Animatable> animatable) {
	if (animatable == positionParameter) {
		auto output = AnimationValue::makePosition();
		output->position = motionProfile.getPosition();
		output->velocity = motionProfile.getVelocity();
		output->acceleration = motionProfile.getAcceleration();
		return output;
	}
	return nullptr;
}

void PositionControlledMachine::fillAnimationDefaults(std::shared_ptr<Animation> animation){
	if(animation->getAnimatable() != positionParameter) return;
	switch(animation->getType()){
		case ManoeuvreType::KEY:
			animation->toKey()->captureTarget();
			break;
		case ManoeuvreType::TARGET:
			animation->toTarget()->captureTarget();
			animation->toTarget()->inAcceleration->overwrite(rapidAcceleration_machineUnitsPerSecond);
			animation->toTarget()->outAcceleration->overwrite(rapidAcceleration_machineUnitsPerSecond);
			animation->toTarget()->velocityConstraint->overwrite(rapidVelocity_machineUnitsPerSecond);
			animation->toTarget()->timeConstraint->overwrite(0.0);
			animation->toTarget()->constraintType->overwrite(TargetAnimation::Constraint::TIME);
			break;
		case ManoeuvreType::SEQUENCE:
			animation->toSequence()->captureStart();
			animation->toSequence()->captureTarget();
			animation->toSequence()->duration->overwrite(0.0);
			animation->toSequence()->inAcceleration->overwrite(rapidAcceleration_machineUnitsPerSecond);
			animation->toSequence()->outAcceleration->overwrite(rapidAcceleration_machineUnitsPerSecond);
			break;
	}
}


bool PositionControlledMachine::validateAnimation(const std::shared_ptr<Animation> animation) {
	using namespace Motion;
	
	if (animation->getAnimatable() != positionParameter) {
		animation->appendValidationErrorString("Unknown Parameter");
		return false;
	}
	else if (!isAxisConnected()) {
		animation->appendValidationErrorString("No Axis Connected To Machine");
		return false;
	}
	
	bool b_trackValid = true;
	
	double lowLimit_machineUnits = getLowPositionLimit();
	double highLimit_machineUnits = getHighPositionLimit();
	double velocityLimit_machineUnits = getAxis()->getVelocityLimit();
	double accelerationLimit_machineUnits = getAxis()->getAccelerationLimit();
	
	auto validatePosition = [&](std::shared_ptr<BaseNumberParameter> positionParameter){
		if(!positionParameter->validateRange(lowLimit_machineUnits, highLimit_machineUnits)){
			std::string error = std::string(positionParameter->getName()) + " is out of range. (" + std::to_string(lowLimit_machineUnits) + " - " + std::to_string(highLimit_machineUnits) + ")";
			animation->appendValidationErrorString(error);
			b_trackValid = false;
		}
	};
	
	auto validateVelocity = [&](std::shared_ptr<NumberParameter<double>> velocityParameter){
		if(!velocityParameter->validateRange(0.0, std::abs(velocityLimit_machineUnits), false, true)){
			std::string error = std::string(velocityParameter->getName()) + " ";
			if(velocityParameter->getReal() == 0.0) error += "is Zero";
			else error += "is out of range. (max: " + std::to_string(std::abs(velocityLimit_machineUnits)) + ")";
			animation->appendValidationErrorString(error);
			b_trackValid = false;
		}
	};
	
	auto validateAcceleration = [&](std::shared_ptr<NumberParameter<double>> accelerationParameter){
		if(!accelerationParameter->validateRange(0.0, std::abs(accelerationLimit_machineUnits), false, true)){
			std::string error = std::string(accelerationParameter->getName()) + " ";
			if(accelerationParameter->getReal() == 0.0) error += "is Zero";
			else error += "is out of range. (max: " + std::to_string(std::abs(accelerationLimit_machineUnits)) + ")";
			animation->appendValidationErrorString(error);
			b_trackValid = false;
		}
	};
	
	
	if(animation->getType() == ManoeuvreType::KEY){
		auto key = animation->toKey();
		validatePosition(key->target->toNumber());
		
	}else if(animation->getType() == ManoeuvreType::TARGET){
		auto target = animation->toTarget();
		validatePosition(target->target->toNumber());
		if(target->getConstraintType() == TargetAnimation::Constraint::VELOCITY) validateVelocity(target->velocityConstraint);
		else target->velocityConstraint->setValid(true);
		target->timeConstraint->setValid(true);
		validateAcceleration(target->inAcceleration);
		validateAcceleration(target->outAcceleration);
		
	}else if(animation->getType() == ManoeuvreType::SEQUENCE){
		auto sequence = animation->toSequence();
		validatePosition(sequence->start->toNumber());
		validatePosition(sequence->target->toNumber());
		validateAcceleration(sequence->inAcceleration);
		validateAcceleration(sequence->outAcceleration);
		sequence->duration->setValid(true);
		
		auto& curves = sequence->getCurves();
		if(curves.size() != 1) {
			Logger::warn("Parameter Track has wrong curve count. Has {}, expected 1", curves.size());
			animation->appendValidationErrorString("Critical: ParameterTrack has wrong curve count.");
			return false;
		}
		
		auto& curve = curves.front();
		bool b_curveValid = true;
		
		//validate all control points
		for (auto& controlPoint : curve.getPoints()) {
			//check all validation conditions and find validaiton error state
			if (controlPoint->position < lowLimit_machineUnits || controlPoint->position > highLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_POSITION_OUT_OF_RANGE;
			else if (std::abs(controlPoint->velocity) > velocityLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_VELOCITY_LIMIT_EXCEEDED;
			else if (std::abs(controlPoint->inAcceleration) > accelerationLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_INPUT_ACCELERATION_LIMIT_EXCEEDED;
			else if (std::abs(controlPoint->outAcceleration) > accelerationLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_OUTPUT_ACCELERATION_LIMIT_EXCEEDED;
			else if (controlPoint->inAcceleration == 0.0)
				controlPoint->validationError = ValidationError::CONTROL_POINT_INPUT_ACCELERATION_IS_ZERO;
			else if (controlPoint->outAcceleration == 0.0)
				controlPoint->validationError = ValidationError::CONTROL_POINT_OUTPUT_ACCELERATION_IS_ZERO;
			else controlPoint->validationError = ValidationError::NONE; //All Checks Passed: No Validation Error !
			//set valid flag for point, if invalid, set flag for whole curve
			if(controlPoint->validationError == Motion::ValidationError::NONE) controlPoint->b_valid = true;
			else {
				controlPoint->b_valid = false;
				b_curveValid = false;
			}
		}

		//validate all interpolations of the curve
		for (auto& interpolation : curve.getInterpolations()) {
			
			if(interpolation->getType() != Motion::Interpolation::Type::TRAPEZOIDAL){
				Logger::critical("Sequence Track Curve Interpolation is wrong type. Is {}, expected Trapezoidal", Enumerator::getDisplayString(interpolation->getType()));
				animation->appendValidationErrorString("Critical: ParameterTrack Interpolation has wrong type.");
				return false;
			}
			
			//if the interpolation is already marked invalid an validation error type was already set by the interpolation engine
			//in this case we don't overwrite the validation error value
			if (!interpolation->b_valid) {
				b_curveValid = false;
				if(sequence->isSimple() && interpolation->validationError == Motion::ValidationError::INTERPOLATION_UNDEFINED){
					sequence->duration->setValid(false);
					animation->appendValidationErrorString("Requested movement duration is too short");
				}
				continue;
			}
			
			auto trapezoidalInteroplation = interpolation->castToTrapezoidal();
			
			//check if the velocity of the interpolation exceeds the limit
			if (std::abs(trapezoidalInteroplation->coastVelocity) > velocityLimit_machineUnits) {
				interpolation->validationError = ValidationError::INTERPOLATION_VELOCITY_LIMIT_EXCEEDED;
				interpolation->b_valid = false;
				if(sequence->isSimple()) {
					sequence->duration->setValid(false);
					animation->appendValidationErrorString("Requested movement duration is too short");
				}
				b_curveValid = false;
				continue;
			}
			//if all interpolation checks passed, we check all interpolation preview points for their range
			for (auto& point : interpolation->displayPoints) {
				if (point.position > highLimit_machineUnits || point.position < lowLimit_machineUnits) {
					interpolation->validationError = ValidationError::INTERPOLATION_POSITION_OUT_OF_RANGE;
					interpolation->b_valid = false;
					animation->appendValidationErrorString("Interpolation is out of range: check the curve editor for details.");
					b_curveValid = false;
					break;
				}
			}
		}
		
		//after performing all checks, we assign the curve validation flag
		//the curve itself doesn't have a validation error value
		curve.b_valid = b_curveValid;
		if(!b_curveValid){
			animation->appendValidationErrorString("Curve could not be validated : check the Curve editor for details.");
			b_trackValid = false;
		}
	}
	
	//we return the result of the validation
	return b_trackValid;
}

bool PositionControlledMachine::generateTargetAnimation(std::shared_ptr<TargetAnimation> animation){
	
	if(animation->getAnimatable() != positionParameter) return false;
	
	auto target = positionParameter->parameterValueToAnimationValue(animation->target)->toPosition();
	double inAcceleration = animation->inAcceleration->value;
	double outAcceleration = animation->outAcceleration->value;
	TargetAnimation::Constraint constraint = animation->getConstraintType();
	
	if(motionProfile.getPosition() == target->position && motionProfile.getVelocity() == 0.0) return false;
	
	auto startPoint = std::make_shared<Motion::ControlPoint>();
	startPoint->time = 0.0;
	startPoint->position = motionProfile.getPosition();
	startPoint->velocity = motionProfile.getVelocity();
	startPoint->outAcceleration = inAcceleration;
	startPoint->b_valid = true;
	
	auto endPoint = std::make_shared<Motion::ControlPoint>();
	endPoint->position = target->position;
	endPoint->velocity = 0.0;
	endPoint->inAcceleration = outAcceleration;
	endPoint->b_valid = true;
	
	std::shared_ptr<Motion::TrapezoidalInterpolation> interpolation;
	
	if(constraint == TargetAnimation::Constraint::TIME){
		endPoint->time = animation->timeConstraint->value;
		interpolation = Motion::TrapezoidalInterpolation::getTimeConstrainedOrSlower(startPoint, endPoint, getAxis()->getVelocityLimit());
	}else if(constraint == TargetAnimation::Constraint::VELOCITY){
		double velocityConstraint = animation->velocityConstraint->value;
		interpolation = Motion::TrapezoidalInterpolation::getVelocityConstrained(startPoint, endPoint, velocityConstraint);
	}
	
	if(!interpolation->b_valid) return false;
	
	auto& curve = animation->getCurves().front();
	auto& points = curve.getPoints();
	auto& interpolations = curve.getInterpolations();
	
	points.clear();
	points.push_back(startPoint);
	points.push_back(endPoint);
	
	interpolations.clear();
	interpolation->updateDisplayCurvePoints();
	interpolations.push_back(interpolation);
	
	curve.b_valid = true;
	animation->setDuration(interpolation->getDuration());
	animation->setPlaybackPosition(0.0);
	
	return true;
	
}









void PositionControlledMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isAxisConnected()) getAxis()->getDevices(output);
}



bool PositionControlledMachine::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* rapidsXML = xml->InsertNewChildElement("Rapids");
	rapidsXML->SetAttribute("Velocity_machineUnitsPerSecond", rapidVelocity_machineUnitsPerSecond);
	rapidsXML->SetAttribute("Acceleration_machineUnitsPerSecondSquared", rapidAcceleration_machineUnitsPerSecond);
	
	XMLElement* machineZeroXML = xml->InsertNewChildElement("MachineZero");
	machineZeroXML->SetAttribute("MachineZero_AxisUnits", machineZero_axisUnits);
	machineZeroXML->SetAttribute("InvertAxisDirection", b_invertDirection);
	
	XMLElement* widgetXML = xml->InsertNewChildElement("ControWidget");
	widgetXML->SetAttribute("UniqueID", controlWidget->uniqueID);
	
	return true;
}


bool PositionControlledMachine::loadMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* rapidsXML = xml->FirstChildElement("Rapids");
	if (rapidsXML == nullptr) return Logger::warn("Could not find Rapids attribute");
	if (rapidsXML->QueryDoubleAttribute("Velocity_machineUnitsPerSecond", &rapidVelocity_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could find rapid velocity attribute");
	if (rapidsXML->QueryDoubleAttribute("Acceleration_machineUnitsPerSecondSquared", &rapidAcceleration_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could find rapid acceleration attribute");
	
	XMLElement* machineZeroXML = xml->FirstChildElement("MachineZero");
	if(machineZeroXML == nullptr) return Logger::warn("Could not find Machine Zero Attribute");
	if(machineZeroXML->QueryDoubleAttribute("MachineZero_AxisUnits", &machineZero_axisUnits) != XML_SUCCESS) return Logger::warn("Could not find machine zero value attribute");
	if(machineZeroXML->QueryBoolAttribute("InvertAxisDirection", &b_invertDirection) != XML_SUCCESS) return Logger::warn("Could not find invert axis direction attribute");
	
	XMLElement* widgetXML = xml->FirstChildElement("ControWidget");
	if(widgetXML == nullptr) return Logger::warn("Could not find Control Widget Attribute");
	if(widgetXML->QueryIntAttribute("UniqueID", &controlWidget->uniqueID) != XML_SUCCESS) return Logger::warn("Could not find machine control widget uid attribute");
	 
	return true;
}




void PositionControlledMachine::captureMachineZero(){
	machineZero_axisUnits = motionProfile.getPosition();
}




double PositionControlledMachine::axisPositionToMachinePosition(double axisPosition){
	if(b_invertDirection) return -1.0f * (axisPosition - machineZero_axisUnits);
	return axisPosition - machineZero_axisUnits;
}

double PositionControlledMachine::axisVelocityToMachineVelocity(double axisVelocity){
	if(b_invertDirection) return axisVelocity * -1.0;
	return axisVelocity;
}

double PositionControlledMachine::machinePositionToAxisPosition(double machinePosition){
	if(b_invertDirection) return (-1.0f * machinePosition) + machineZero_axisUnits;
	return machinePosition + machineZero_axisUnits;
}

double PositionControlledMachine::machineVelocityToAxisVelocity(double machineVelocity){
	if(b_invertDirection) return machineVelocity * -1.0;
	return machineVelocity;
}


double PositionControlledMachine::getLowPositionLimit(){
	//if the machine motion is inverted relative to the axis motion, we also need to invert the limits
	if(b_invertDirection) return axisPositionToMachinePosition(getAxis()->getHighPositionLimit());
	return axisPositionToMachinePosition(getAxis()->getLowPositionLimit());
}

double PositionControlledMachine::getHighPositionLimit(){
	//if the machine motion is inverted relative to the axis motion, we also need to invert the limits
	if(b_invertDirection) return axisPositionToMachinePosition(getAxis()->getLowPositionLimit());
	return axisPositionToMachinePosition(getAxis()->getHighPositionLimit());
}

double PositionControlledMachine::getPositionNormalized(){
	double lowLimit = getLowPositionLimit();
	double highLimit = getHighPositionLimit();
	double position;
	if(isSimulating()) position = motionProfile.getPosition();
	else position = axisPositionToMachinePosition(getAxis()->getActualPosition());
	return (position - lowLimit) / (highLimit - lowLimit);
}

double PositionControlledMachine::getVelocityNormalized(){
	double velocityLimit = getAxis()->getVelocityLimit();
	double velocity;
	if(isSimulating()) velocity = motionProfile.getVelocity();
	else velocity = axisVelocityToMachineVelocity(getAxis()->getActualVelocity());
	return velocity / velocityLimit;
}


bool PositionControlledMachine::hasManualPositionTarget(){
	return controlMode == ControlMode::POSITION_TARGET;
}

double PositionControlledMachine::getManualPositionTarget(){
	return motionProfile.getInterpolationTarget();
}
