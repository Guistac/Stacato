#include <pch.h>
#include "AnimatablePosition.h"

#include "Project/Editor/Parameter.h"
#include "Animation/Animation.h"
#include "Motion/MotionTypes.h"
#include "Machine/Machine.h"

std::vector<InterpolationType>& AnimatablePosition::getCompatibleInterpolationTypes(){
	static std::vector<InterpolationType> compatibleInterpolationTypes = { InterpolationType::TRAPEZOIDAL };
	return compatibleInterpolationTypes;
}

int AnimatablePosition::getCurveCount(){
	return 1;
}

std::shared_ptr<Parameter> AnimatablePosition::makeParameter(){
	return NumberParameter<double>::make(0.0, "DefaultName");
}

void AnimatablePosition::setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value){
	std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->overwrite(value->toPosition()->position);
}

void AnimatablePosition::copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to){
	std::dynamic_pointer_cast<NumberParameter<double>>(to)->overwrite(std::dynamic_pointer_cast<NumberParameter<double>>(from)->value);
}

std::shared_ptr<AnimationValue> AnimatablePosition::parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter){
	auto output = AnimationValue::makePosition();
	output->position = std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->value;
	return output;
}

bool AnimatablePosition::isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2){
	return value1->toPosition()->position == value2->toPosition()->position;
}

std::shared_ptr<AnimationValue> AnimatablePosition::getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds){
	auto output = AnimationValue::makePosition();
	Motion::Point point = animation->getCurves().front().getPointAtTime(time_seconds);
	output->position = point.position;
	output->velocity = point.velocity;
	output->acceleration = point.acceleration;
	return output;
}

std::vector<double> AnimatablePosition::getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value){
	return std::vector<double>{value->toPosition()->position};
}

bool AnimatablePosition::generateTargetAnimation(std::shared_ptr<TargetAnimation> animation){
	
	auto target = parameterValueToAnimationValue(animation->target)->toPosition();
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
		interpolation = Motion::TrapezoidalInterpolation::getTimeConstrainedOrSlower(startPoint, endPoint, velocityLimit);
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

bool AnimatablePosition::validateAnimation(std::shared_ptr<Animation> animation){
	
	using namespace Motion;
	
	bool b_animationValid = true;
	
	auto validatePosition = [&](std::shared_ptr<BaseNumberParameter> animatablePosition){
		if(!animatablePosition->validateRange(lowerPositionLimit, upperPositionLimit)){
			std::string error = std::string(animatablePosition->getName()) + " is out of range. (" + std::to_string(lowerPositionLimit) + " - " + std::to_string(upperPositionLimit) + ")";
			animation->appendValidationErrorString(error);
			b_animationValid = false;
		}
	};
	
	auto validateVelocity = [&](std::shared_ptr<NumberParameter<double>> velocityParameter){
		if(!velocityParameter->validateRange(0.0, std::abs(velocityLimit), false, true)){
			std::string error = std::string(velocityParameter->getName()) + " ";
			if(velocityParameter->getReal() == 0.0) error += "is Zero";
			else error += "is out of range. (max: " + std::to_string(std::abs(velocityLimit)) + ")";
			animation->appendValidationErrorString(error);
			b_animationValid = false;
		}
	};
	
	auto validateAcceleration = [&](std::shared_ptr<NumberParameter<double>> accelerationParameter){
		if(!accelerationParameter->validateRange(0.0, std::abs(accelerationLimit), false, true)){
			std::string error = std::string(accelerationParameter->getName()) + " ";
			if(accelerationParameter->getReal() == 0.0) error += "is Zero";
			else error += "is out of range. (max: " + std::to_string(std::abs(accelerationLimit)) + ")";
			animation->appendValidationErrorString(error);
			b_animationValid = false;
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
			if (controlPoint->position < lowerPositionLimit || controlPoint->position > upperPositionLimit)
				controlPoint->validationError = ValidationError::CONTROL_POINT_POSITION_OUT_OF_RANGE;
			else if (std::abs(controlPoint->velocity) > velocityLimit)
				controlPoint->validationError = ValidationError::CONTROL_POINT_VELOCITY_LIMIT_EXCEEDED;
			else if (std::abs(controlPoint->inAcceleration) > accelerationLimit)
				controlPoint->validationError = ValidationError::CONTROL_POINT_INPUT_ACCELERATION_LIMIT_EXCEEDED;
			else if (std::abs(controlPoint->outAcceleration) > accelerationLimit)
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
			
			if(interpolation->getType() != InterpolationType::TRAPEZOIDAL){
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
			if (std::abs(trapezoidalInteroplation->coastVelocity) > velocityLimit) {
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
				if (point.position > upperPositionLimit || point.position < lowerPositionLimit) {
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
			b_animationValid = false;
		}
	}
	
	//we return the result of the validation
	return b_animationValid;
}











bool AnimatablePosition::isReadyToMove(){
	const std::lock_guard<std::mutex> lock(mutex);
	return !getMachine()->isEnabled() || getMachine()->isHoming();
}

bool AnimatablePosition::isReadyToStartPlaybackFromValue(std::shared_ptr<AnimationValue> animationValue){
	const std::lock_guard<std::mutex> lock(mutex);
	return animationValue->toPosition()->position == getActualPosition();
}

bool AnimatablePosition::isInRapid(){
	const std::lock_guard<std::mutex> lock(mutex);
	return motionProfile.hasInterpolationTarget();
}

float AnimatablePosition::getRapidProgress(){
	const std::lock_guard<std::mutex> lock(mutex);
	return motionProfile.getInterpolationProgress(profileTime_seconds);
}

void AnimatablePosition::cancelRapid(){
	setVelocityTarget(0.0);
}

void AnimatablePosition::onRapidToValue(std::shared_ptr<AnimationValue> animationValue){
	moveToPositionWithVelocity(animationValue->toPosition()->position, rapidVelocity);
}

void AnimatablePosition::onSetManualControlTarget(float x, float y, float z){
	setVelocityTarget(velocityLimit * x);
}









void AnimatablePosition::setVelocityTarget(double velocityTarget){
	const std::lock_guard<std::mutex> lock(mutex);
	velocitySetpoint = std::clamp(velocityTarget, -velocityLimit, velocityLimit);
	controlMode = VELOCITY_SETPOINT;
	motionProfile.resetInterpolation();
}

void AnimatablePosition::moveToPositionWithVelocity(double targetPosition, double targetVelocity){
	const std::lock_guard<std::mutex> lock(mutex);
	targetPosition = std::clamp(targetPosition, lowerPositionLimit, upperPositionLimit);
	motionProfile.moveToPositionWithVelocity(profileTime_seconds, targetPosition, targetVelocity, rapidAcceleration);
	controlMode = POSITION_TARGET;
}

void AnimatablePosition::moveToPositionInTime(double targetPosition, double targetTime){
	const std::lock_guard<std::mutex> lock(mutex);
	targetPosition = std::clamp(targetPosition, lowerPositionLimit, upperPositionLimit);
	motionProfile.moveToPositionInTime(profileTime_seconds, targetPosition, targetTime, rapidAcceleration, velocityLimit);
	controlMode = POSITION_TARGET;
}







bool AnimatablePosition::hasPositionSetpoint(){
	const std::lock_guard<std::mutex> lock(mutex);
	return controlMode == POSITION_SETPOINT || controlMode == POSITION_TARGET || hasAnimation();
}
double AnimatablePosition::getPositionSetpoint(){
	const std::lock_guard<std::mutex> lock(mutex);
	double target;
	if(controlMode == POSITION_TARGET) target = motionProfile.getInterpolationTarget();
	else target = getTargetValue()->toPosition()->position;
	return target;
}
double AnimatablePosition::getPositionSetpointNormalized(){
	return (getPositionSetpoint() - lowerPositionLimit) / (upperPositionLimit - lowerPositionLimit);
}


bool AnimatablePosition::hasVelocitySetpoint(){
	const std::lock_guard<std::mutex> lock(mutex);
	return isReadyToMove();
}
double AnimatablePosition::getVelocitySetpoint(){
	const std::lock_guard<std::mutex> lock(mutex);
	return getTargetValue()->toPosition()->velocity;
}
double AnimatablePosition::getVelocitySetpointNormalized(){
	return getVelocitySetpoint() / velocityLimit;
}


double AnimatablePosition::getActualPosition(){
	return getActualValue()->toPosition()->position;
}
double AnimatablePosition::getActualPositionNormalized(){
	return (getActualPosition() - lowerPositionLimit) / (upperPositionLimit - lowerPositionLimit);
}


double AnimatablePosition::getActualVelocity(){
	return getActualValue()->toPosition()->velocity;
}
double AnimatablePosition::getActualVelocityNormalized(){
	return getActualVelocity() / velocityLimit;
}

double AnimatablePosition::getActualAcceleration(){
	return getActualValue()->toPosition()->acceleration;
}
double AnimatablePosition::getActualAccelerationNormalized(){
	return getActualAcceleration() / accelerationLimit;
}











void AnimatablePosition::updateActualValue(std::shared_ptr<AnimationValue> newActualValue){
	const std::lock_guard<std::mutex> lock(mutex);
	actualValue = newActualValue->toPosition();
}

std::shared_ptr<AnimationValue> AnimatablePosition::getActualValue(){
	const std::lock_guard<std::mutex> lock(mutex);
	return actualValue;
}

void AnimatablePosition::updateDisabled(){
	const std::lock_guard<std::mutex> lock(mutex);
	auto newTargetValue = AnimationValue::makePosition();
	newTargetValue->position = actualValue->position;
	newTargetValue->velocity = actualValue->velocity;
	newTargetValue->acceleration = actualValue->acceleration;
	targetValue = newTargetValue;
}

void AnimatablePosition::updateTargetValue(double time_seconds, double deltaT_seconds){
	const std::lock_guard<std::mutex> lock(mutex);
	profileTime_seconds = time_seconds;
	
	if(hasAnimation()){
		auto target = getAnimationValue()->toPosition();
		motionProfile.matchPositionAndRespectPositionLimits(deltaT_seconds,
															target->position,
															target->velocity,
															target->acceleration,
															accelerationLimit,
															velocityLimit,
															lowerPositionLimit,
															upperPositionLimit);
	}else{
		switch(controlMode){
			case VELOCITY_SETPOINT:
				motionProfile.matchVelocityAndRespectPositionLimits(deltaT_seconds,
																	velocitySetpoint,
																	rapidAcceleration,
																	lowerPositionLimit,
																	upperPositionLimit,
																	rapidAcceleration);
				break;
			case POSITION_SETPOINT:
				motionProfile.matchPositionAndRespectPositionLimits(deltaT_seconds,
																	positionSetpoint,
																	0.0,
																	0.0,
																	accelerationLimit,
																	velocityLimit,
																	lowerPositionLimit,
																	upperPositionLimit);
				break;
			case POSITION_TARGET:
				motionProfile.updateInterpolation(profileTime_seconds);
				if(motionProfile.isInterpolationFinished(profileTime_seconds)) {
					controlMode = VELOCITY_SETPOINT;
					velocitySetpoint = 0.0;
				}
				break;
		}
	}
		
	//generate an output target value to be read by the machine
	auto newTargetValue = AnimationValue::makePosition();
	newTargetValue->position = motionProfile.getPosition();
	newTargetValue->velocity = motionProfile.getVelocity();
	newTargetValue->acceleration = motionProfile.getAcceleration();
	targetValue = newTargetValue;
}

std::shared_ptr<AnimationValue> AnimatablePosition::getTargetValue(){
	const std::lock_guard<std::mutex> lock(mutex);
	return targetValue;
}
