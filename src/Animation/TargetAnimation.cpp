#include "Animation.h"
#include "Machine/Machine.h"

std::shared_ptr<TargetAnimation> TargetAnimation::copy(){
	auto copy = std::make_shared<TargetAnimation>(getAnimatable());
	getAnimatable()->copyParameterValue(target, copy->target);
	copy->interpolationType->overwrite(interpolationType->value);
	copy->timeConstraint->overwrite(timeConstraint->value);
	copy->velocityConstraint->overwrite(velocityConstraint->value);
	copy->inAcceleration->overwrite(inAcceleration->value);
	copy->outAcceleration->overwrite(outAcceleration->value);
	copy->timeOffset->overwrite(timeOffset->value);
	copy->constraintType->overwrite(constraintType->value);
	return copy;
}



bool TargetAnimation::isReadyToStartPlayback(){
	return isMachineEnabled();
}




void TargetAnimation::startPlayback(){
	//TODO:
	//generate a curve that starts at the current machine value and goes to the target
	//respect the interpolation type
	//respect the time offset
	//respect the velocity or time constraint
	//bool success = getParameter()->getMachine()->generateTargetParameterTrackCurves(shared_from_this()->castToTarget());
	//then play these curves
}


bool TargetAnimation::isAtTarget(){
	auto animatable = getAnimatable();
	auto actualValue = animatable->getActualValue();
	auto targetValue = animatable->parameterValueToAnimationValue(target);
	return animatable->isParameterValueEqual(actualValue, targetValue);
}

void TargetAnimation::rapidToTarget(){
	auto animatable = getAnimatable();
	animatable->stopAnimationPlayback();
	auto targetValue = animatable->parameterValueToAnimationValue(target);
	animatable->getMachine()->rapidAnimatableToValue(animatable, targetValue);
}

/*
class CaptureAnimatableValueCommand : public Command{
public:
	
	std::shared_ptr<Parameter> parameter;
	std::shared_ptr<Animatable> animatable;
	std::shared_ptr<AnimationValue> previousValue;
	std::shared_ptr<AnimationValue> capturedValue;
	
	CaptureAnimatableValueCommand(std::string& name, std::shared_ptr<Parameter> parameter_, std::shared_ptr<Animatable> animatable_)
	: Command(name) {
		parameter = parameter_;
		animatable = animatable_;
	};
	
	virtual void execute(){
		capturedValue = animatable->getCurrentValue();
		previousValue = animatable->parameterValueToAnimationValue(parameter);
		animatable->setParameterValueFromAnimationValue(parameter, capturedValue);
	}
	virtual void undo(){
		animatable->setParameterValueFromAnimationValue(parameter, previousValue);
	}
	virtual void redo(){
		animatable->setParameterValueFromAnimationValue(parameter, capturedValue);
	}
};

void AnimatedParameterTrack::captureCurrentValueAsTarget(){
	std::string name = "Capture " + std::string(getParameter()->getName()) + " Target";
	auto command = std::make_shared<CaptureCurrentMachineParameterValueCommand>(name, target, getAnimatableParameter());
	CommandHistory::pushAndExecute(command);
}

void SequenceParameterTrack::captureCurrentValueAsStart(){
	std::string name = "Capture " + std::string(getParameter()->getName()) + " Start";
	auto command = std::make_shared<CaptureCurrentMachineParameterValueCommand>(name, start, getAnimatableParameter());
	CommandHistory::pushAndExecute(command);
}
 */



bool TargetAnimation::onSave(tinyxml2::XMLElement* xml){
	//save type, constraint, target, ramps
	target->save(xml);
	constraintType->save(xml);
	timeConstraint->save(xml);
	velocityConstraint->save(xml);
	inAcceleration->save(xml);
	outAcceleration->save(xml);
	timeOffset->save(xml);
	return true;
}

std::shared_ptr<TargetAnimation> TargetAnimation::load(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable){
	auto targetAnimation = std::make_shared<TargetAnimation>(animatable);
	if(!targetAnimation->target->load(xml)){
		Logger::warn("could not load attribute target of animation {}", animatable->getName());
		return nullptr;
	}
	if(!targetAnimation->constraintType->load(xml)){
		Logger::warn("could not load attribute constraint Type of animation {}", animatable->getName());
		return nullptr;
	}
	if(!targetAnimation->timeConstraint->load(xml)){
		Logger::warn("could not load attribute time constraint of animation {}", animatable->getName());
		return nullptr;
	}
	if(!targetAnimation->velocityConstraint->load(xml)){
		Logger::warn("could not load attribute velocity constraint of animation {}", animatable->getName());
		return nullptr;
	}
	if(!targetAnimation->inAcceleration->load(xml)){
		Logger::warn("could not load attribute inAcceleration of animation {}", animatable->getName());
		return nullptr;
	}
	if(!targetAnimation->outAcceleration->load(xml)){
		Logger::warn("could not load attribute outAcceleration of animation {}", animatable->getName());
		return nullptr;
	}
	if(!targetAnimation->timeOffset->load(xml)){
		Logger::warn("could not load attribute timeOffset of animation {}", animatable->getName());
		return nullptr;
	}
	return targetAnimation;
}
