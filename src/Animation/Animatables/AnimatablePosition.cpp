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
	auto parameter = NumberParameter<double>::make(0.0, "DefaultName");
	if(getUnit() == Units::AngularDistance::Degree) {
		parameter->b_useTurnAndDegrees = true;
		parameter->setFormat("%.1f");
	}
	return parameter;
}

void AnimatablePosition::setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value){
	std::static_pointer_cast<NumberParameter<double>>(parameter)->overwrite(value->toPosition()->position);
}

void AnimatablePosition::copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to){
	std::static_pointer_cast<NumberParameter<double>>(to)->overwrite(std::static_pointer_cast<NumberParameter<double>>(from)->value);
}

std::shared_ptr<AnimationValue> AnimatablePosition::parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter){
	auto output = AnimationValue::makePosition();
	output->position = std::static_pointer_cast<NumberParameter<double>>(parameter)->value;
	return output;
}

bool AnimatablePosition::isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2){
	return value1->toPosition()->position == value2->toPosition()->position;
}

std::shared_ptr<AnimationValue> AnimatablePosition::getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds){
	auto output = AnimationValue::makePosition();
	Motion::Point point = animation->getCurves().front()->getPointAtTime(time_seconds);
	output->position = point.position;
	output->velocity = point.velocity;
	output->acceleration = point.acceleration;
	return output;
}

std::vector<double> AnimatablePosition::getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value){
	return std::vector<double>{value->toPosition()->position};
}

std::vector<std::string> curveNames_animatablePosition = {
	"Position"
};

std::vector<std::string>& AnimatablePosition::getCurveNames(){
	return curveNames_animatablePosition;
}

void AnimatablePosition::fillControlPointDefaults(std::shared_ptr<Motion::ControlPoint> controlpoint){
	controlpoint->inAcceleration = accelerationLimit;
	controlpoint->outAcceleration = accelerationLimit;
	controlpoint->velocity = 0.0;
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
	auto& points = curve->getPoints();
	auto& interpolations = curve->getInterpolations();
	
	
	
	points.clear();
	points.push_back(interpolation->inPoint);
	points.push_back(interpolation->outPoint);
	
	interpolations.clear();
	interpolation->updateDisplayCurvePoints();
	interpolations.push_back(interpolation);
	
	curve->b_valid = true;
	
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
		for (auto& controlPoint : curve->getPoints()) {
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
		for (auto& interpolation : curve->getInterpolations()) {
			
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
		curve->b_valid = b_curveValid;
		if(!b_curveValid){
			animation->appendValidationErrorString("Curve could not be validated : check the Curve editor for details.");
			b_animationValid = false;
		}
	}
	
	//we return the result of the validation
	return b_animationValid;
}





bool AnimatablePosition::isControlledManuallyOrByAnimation(){
	if(hasAnimation()) return true;
		
	if(isInRapid()) return true;
	
	if(controlMode == VELOCITY_SETPOINT &&
	   (velocitySetpoint != 0.0 || motionProfile.getVelocity() != 0.0)) return true;
	
	if(controlMode == POSITION_SETPOINT &&
	   motionProfile.getPosition() != positionSetpoint &&
	   motionProfile.getVelocity() != 0.0 &&
	   motionProfile.getAcceleration() != 0.0) return true;
	
	return false;
}





bool AnimatablePosition::isReadyToMove(){
	mutex.lock();
	bool b = getMachine()->isEnabled() && state == Animatable::State::READY;
	mutex.unlock();
	return b;
}

bool AnimatablePosition::isReadyToStartPlaybackFromValue(std::shared_ptr<AnimationValue> animationValue){
	mutex.lock();
	bool b = animationValue->toPosition()->position == targetValue->position;
	mutex.unlock();
	return b;
}

bool AnimatablePosition::isInRapid(){
	mutex.lock();
	bool b = motionProfile.hasInterpolationTarget();
	mutex.unlock();
	return b;
}

float AnimatablePosition::getRapidProgress(){
	mutex.lock();
	float r = motionProfile.getInterpolationProgress(profileTime_seconds);
	mutex.unlock();
	return r;
}

void AnimatablePosition::cancelRapid(){
	mutex.lock();
	velocitySetpoint = 0.0;
	controlMode = VELOCITY_SETPOINT;
	motionProfile.resetInterpolation();
	mutex.unlock();
}

std::shared_ptr<AnimationValue> AnimatablePosition::getRapidTarget(){
	auto rapidTarget = AnimationValue::makePosition();
	rapidTarget->position = motionProfile.getInterpolationTarget();
	return rapidTarget;
}

void AnimatablePosition::onRapidToValue(std::shared_ptr<AnimationValue> animationValue){
	moveToPositionWithVelocity(animationValue->toPosition()->position, velocityLimit);
}

void AnimatablePosition::onSetManualControlTarget(float x, float y, float z){
	
	float manualCommand;
	switch(direction){
		case Direction::HORIZONTAL:
			manualCommand = x;
			break;
		case Direction::VERTICAL:
			manualCommand = y;
			break;
	}
	
	//we don't invert the slider display value
	velocitySliderDisplayValue = manualCommand;
	if(b_invertManualControls) manualCommand *= -1.0;
	
	//if we are in the middle of an animation and the requested velocity is 0, skip the command
	if(manualCommand == 0.0 && hasAnimation()) return;
	setManualVelocityTarget(velocityLimit * manualCommand);
}

void AnimatablePosition::onPlaybackStart(std::shared_ptr<Animation> animation){
	mutex.lock();
	currentAnimation = animation;
	mutex.unlock();
}
void AnimatablePosition::onPlaybackPause(){
	//mutex.lock();
	currentAnimation = nullptr;
	//mutex.unlock();
}
void AnimatablePosition::onPlaybackStop(){
	mutex.lock();
	currentAnimation = nullptr;
	setVelocityTarget(0.0);
	mutex.unlock();
}
void AnimatablePosition::onPlaybackEnd(){}


bool AnimatablePosition::isMoving(){
	mutex.lock();
	bool b_moving = motionProfile.getVelocity() != 0.0;
	mutex.unlock();
	return b_moving;
}







void AnimatablePosition::setVelocityTarget(double velocityTarget){
	velocitySetpoint = std::clamp(velocityTarget, -std::abs(velocityLimit), std::abs(velocityLimit));
	positionSetpoint = 0.0;
	accelerationSetpoint = 0.0;
	controlMode = VELOCITY_SETPOINT;
	motionProfile.resetInterpolation();
}

void AnimatablePosition::moveToPositionWithVelocity(double targetPosition, double targetVelocity){
	targetPosition = std::clamp(targetPosition, lowerPositionLimit, upperPositionLimit);
	targetVelocity = std::clamp(targetVelocity, 0.0, velocityLimit);
	if(motionProfile.moveToPositionWithVelocity(profileTime_seconds, targetPosition, targetVelocity, accelerationLimit)) controlMode = POSITION_SETPOINT;
}

void AnimatablePosition::moveToPositionInTime(double targetPosition, double targetTime){
	targetPosition = std::clamp(targetPosition, lowerPositionLimit, upperPositionLimit);
	if(motionProfile.moveToPositionInTime(profileTime_seconds, targetPosition, targetTime, accelerationLimit, velocityLimit)) controlMode = POSITION_SETPOINT;
}




void AnimatablePosition::setManualVelocityTarget(double velocityTarget){
	stopAnimation();
	mutex.lock();
	setVelocityTarget(velocityTarget);
	mutex.unlock();
}

void AnimatablePosition::setManualPositionTargetWithVelocity(double targetPosition, double targetVelocity){
	mutex.lock();
	stopAnimation();
	moveToPositionWithVelocity(targetPosition, targetVelocity);
	mutex.unlock();
}

void AnimatablePosition::setManualPositionTargetWithTime(double targetPosition, double targetTime){
	mutex.lock();
	stopAnimation();
	moveToPositionInTime(targetPosition, targetTime);
	mutex.unlock();
}

void AnimatablePosition::forcePositionTarget(double position, double velocity, double acceleration){
	positionSetpoint = position;
	velocitySetpoint = velocity;
	accelerationSetpoint = acceleration;
	controlMode = FORCED_POSITION_SETPOINT;
}

void AnimatablePosition::forceVelocityTarget(double velocity, double acceleration){
	positionSetpoint = 0.0;
	velocitySetpoint = velocity;
	accelerationSetpoint = acceleration;
	controlMode = FORCED_VELOCITY_SETPOINT;
}

void AnimatablePosition::overridePositionTarget(double positionTarget){
	positionSetpoint = positionTarget;
	controlMode = POSITION_SETPOINT;
	motionProfile.setPosition(positionTarget);
}



bool AnimatablePosition::hasPositionSetpoint(){
	mutex.lock();
	bool r = controlMode == POSITION_SETPOINT || hasAnimation();
	mutex.unlock();
	return r;
}
double AnimatablePosition::getPositionSetpoint(){
	mutex.lock();
	bool r = getTargetValue()->toPosition()->position;
	mutex.lock();
	return r;
}
double AnimatablePosition::getPositionSetpointNormalized(){
	return (getPositionSetpoint() - lowerPositionLimit) / (upperPositionLimit - lowerPositionLimit);
}


bool AnimatablePosition::hasVelocitySetpoint(){
	mutex.lock();
	bool r = isReadyToMove();
	mutex.unlock();
	return r;
}
double AnimatablePosition::getVelocitySetpoint(){
	mutex.lock();
	bool r = getTargetValue()->toPosition()->velocity;
	mutex.unlock();
	return r;
}
double AnimatablePosition::getVelocitySetpointNormalized(){
	return getVelocitySetpoint() / velocityLimit;
}


bool AnimatablePosition::hasAccelerationSetpoint(){
	mutex.lock();
	bool r = isReadyToMove();
	mutex.unlock();
	return r;
}

double AnimatablePosition::getAccelerationSetpoint(){
	mutex.lock();
	double r = getTargetValue()->toPosition()->acceleration;
	mutex.unlock();
	return r;
	
}

double AnimatablePosition::getAccelerationSetpointNormalized(){
	mutex.lock();
	double r = getAccelerationSetpoint() / accelerationLimit;
	mutex.unlock();
	return r;
}



double AnimatablePosition::getActualPosition(){
	return getActualValue()->toPosition()->position;
}
double AnimatablePosition::getActualPositionNormalized(){
	return (getActualPosition() - lowerPositionLimit) / (upperPositionLimit - lowerPositionLimit);
}

double AnimatablePosition::normalizePosition(double position){
	return (position - lowerPositionLimit) / (upperPositionLimit - lowerPositionLimit);
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







double AnimatablePosition::getBrakingPosition(){
	return motionProfile.getBrakingPosition(Environnement::getDeltaTime_seconds(), accelerationLimit);
}



void AnimatablePosition::updateActualValue(std::shared_ptr<AnimationValue> newActualValue){
	mutex.lock();
	actualValue = newActualValue->toPosition();
	mutex.unlock();
}

std::shared_ptr<AnimationValue> AnimatablePosition::getActualValue(){
	auto ret = AnimationValue::makePosition();
	mutex.lock();
	ret->position = actualValue->position;
	ret->velocity = actualValue->velocity;
	ret->acceleration = actualValue->acceleration;
	mutex.unlock();
	return ret;
}

void AnimatablePosition::copyMotionProfilerValueToTargetValue(){
	auto newTargetValue = AnimationValue::makePosition();
	newTargetValue->position = motionProfile.getPosition();
	newTargetValue->velocity = motionProfile.getVelocity();
	newTargetValue->acceleration = motionProfile.getAcceleration();
	targetValue = newTargetValue;
}

void AnimatablePosition::followActualValue(double time_seconds, double deltaTime_seconds){
	mutex.lock();
	profileTime_seconds = time_seconds;
	deltaTime_seconds = deltaTime_seconds;
	controlMode = POSITION_SETPOINT;
	positionSetpoint = actualValue->position;
	motionProfile.setPosition(actualValue->position);
	motionProfile.setVelocity(actualValue->velocity);
	motionProfile.setAcceleration(actualValue->acceleration);
	copyMotionProfilerValueToTargetValue();
	mutex.unlock();
}

void AnimatablePosition::getConstraintPositionLimits(double& min, double& max){
	double position = motionProfile.getPosition();
	min = lowerPositionLimit;
	max = upperPositionLimit;
	for(auto& constraint : getConstraints()){
		if(!constraint->isEnabled()) continue;
		auto keepout = std::static_pointer_cast<AnimatablePosition_KeepoutConstraint>(constraint);
		if(position > keepout->keepOutMinPosition && position < keepout->keepOutMaxPosition){
			double middle = (keepout->keepOutMinPosition + keepout->keepOutMaxPosition) / 2.0;
			if(position > middle){
				min = keepout->keepOutMaxPosition;
			}else{
				max = keepout->keepOutMinPosition;
			}
		}
		else if(position < keepout->keepOutMinPosition && keepout->keepOutMinPosition < max) max = keepout->keepOutMinPosition;
		else if(position > keepout->keepOutMaxPosition && keepout->keepOutMaxPosition > min) min = keepout->keepOutMaxPosition;
	}
}

void AnimatablePosition::updateTargetValue(double time_seconds, double deltaT_seconds){
	mutex.lock();
	
	profileTime_seconds = time_seconds;
	
	double minPosition, maxPosition;
	getConstraintPositionLimits(minPosition, maxPosition);
	
	//update the setpoints
	if(hasAnimation() && getAnimation()->isPlaying()){
		auto target = getAnimationValue()->toPosition();
		positionSetpoint = target->position;
		velocitySetpoint = target->velocity;
		accelerationSetpoint = target->acceleration;
		controlMode = POSITION_SETPOINT;
	}
	else if(motionProfile.hasInterpolationTarget()){
		auto interpolationPoint = motionProfile.getInterpolationPoint(profileTime_seconds);
		positionSetpoint = interpolationPoint.position;
		velocitySetpoint = interpolationPoint.velocity;
		accelerationSetpoint = interpolationPoint.acceleration; //EXPERIMENTAL
		controlMode = POSITION_SETPOINT;
		if(motionProfile.isInterpolationFinished(profileTime_seconds)) {
			velocitySetpoint = 0.0;
			motionProfile.resetInterpolation();
		}
	}else if(controlMode == POSITION_SETPOINT){
		velocitySetpoint = 0.0;
		accelerationSetpoint = 0.0;
	}
	//else control mode is forced and setpoint is adjuste externally
	
	//handle motion interrupt if the setpoint goes outside limits
	if(controlMode == POSITION_SETPOINT || controlMode == FORCED_POSITION_SETPOINT){
		if((positionSetpoint < minPosition && velocitySetpoint < 0.0) || (positionSetpoint > maxPosition && velocitySetpoint > 0.0)){
			if(hasAnimation()) getAnimation()->pausePlayback();
			stopMovement();
			Logger::info("{} movement aborted, position set point was outside limits", getName());
		}
	}
	
	//update the motion profiler
	switch(controlMode){
		case FORCED_VELOCITY_SETPOINT:
		case VELOCITY_SETPOINT:
			motionProfile.matchVelocityAndRespectPositionLimits(deltaT_seconds,
																velocitySetpoint,
																accelerationLimit,
																minPosition,
																maxPosition,
																accelerationLimit);
			break;
		case FORCED_POSITION_SETPOINT:
		case POSITION_SETPOINT:
			motionProfile.matchPositionAndRespectPositionLimits(deltaT_seconds,
																positionSetpoint,
																velocitySetpoint,
																accelerationSetpoint,
																accelerationLimit,
																velocityLimit,
																minPosition,
																maxPosition);
			break;
	}
	
		
	//generate an output target value to be read by the machine
	copyMotionProfilerValueToTargetValue();
	
	mutex.unlock();
}


void AnimatablePosition::simulateTargetValue(double time_seconds, double deltaT_seconds) {
    mutex.lock();
    
    profileTime_seconds = time_seconds;
    
    double minPosition, maxPosition;
    getConstraintPositionLimits(minPosition, maxPosition);
    
    //update the setpoints
    if(hasAnimation()){
        auto target = getAnimationValue()->toPosition();
        motionProfile.setPosition(target->position);
        motionProfile.setVelocity(target->velocity);
        motionProfile.setAcceleration(target->acceleration);
        copyMotionProfilerValueToTargetValue();
        mutex.unlock();
        return;
    }
    else if(motionProfile.hasInterpolationTarget()){
        auto interpolationPoint = motionProfile.getInterpolationPoint(profileTime_seconds);
        positionSetpoint = interpolationPoint.position;
        velocitySetpoint = interpolationPoint.velocity;
        controlMode = POSITION_SETPOINT;
        if(motionProfile.isInterpolationFinished(profileTime_seconds)) {
            velocitySetpoint = 0.0;
            motionProfile.resetInterpolation();
        }
    }else if(controlMode == POSITION_SETPOINT){
        velocitySetpoint = 0.0;
        accelerationSetpoint = 0.0;
    }
    //else control mode is forced and setpoint is adjuste externally
    
    //handle motion interrupt if the setpoint goes outside limits
    if(controlMode == POSITION_SETPOINT || controlMode == FORCED_POSITION_SETPOINT){
        if((positionSetpoint < minPosition && velocitySetpoint < 0.0) || (positionSetpoint > maxPosition && velocitySetpoint > 0.0)){
            if(hasAnimation()) getAnimation()->pausePlayback();
            stopMovement();
            Logger::info("{} movement aborted, position set point was outside limits", getName());
        }
    }
    
    //update the motion profiler
    switch(controlMode){
        case FORCED_VELOCITY_SETPOINT:
        case VELOCITY_SETPOINT:
            motionProfile.matchVelocityAndRespectPositionLimits(deltaT_seconds,
                                                                velocitySetpoint,
                                                                accelerationLimit,
                                                                minPosition,
                                                                maxPosition,
                                                                accelerationLimit);
            break;
        case FORCED_POSITION_SETPOINT:
        case POSITION_SETPOINT:
            motionProfile.matchPositionAndRespectPositionLimits(deltaT_seconds,
                                                                positionSetpoint,
                                                                velocitySetpoint,
                                                                accelerationSetpoint,
                                                                accelerationLimit,
                                                                velocityLimit,
                                                                minPosition,
                                                                maxPosition);
            break;
    }
    
        
    //generate an output target value to be read by the machine
    copyMotionProfilerValueToTargetValue();
    mutex.unlock();
}


std::shared_ptr<AnimationValue> AnimatablePosition::getTargetValue(){
	auto r = AnimationValue::makePosition();
	mutex.lock();
	r->position = targetValue->position;
	r->velocity = targetValue->velocity;
	r->acceleration = targetValue->acceleration;
	mutex.unlock();
	return r;
}
