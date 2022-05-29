#include <pch.h>

#include "Animation.h"

#include "Machine/Machine.h"
#include "Animation/Animatable.h"
#include "Motion/Curve/Curve.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Environnement/Environnement.h"
#include "Animation/Manoeuvre.h"
#include "Plot/Plot.h"

#include "Project/Editor/Parameter.h"


std::shared_ptr<Animation> Animation::create(std::shared_ptr<Animatable> animatable, ManoeuvreType manoeuvreType){
	if(animatable->isComposite()) return std::make_shared<AnimationComposite>(animatable->toComposite(), manoeuvreType);
	switch(manoeuvreType){
		case ManoeuvreType::KEY:
			return std::make_shared<AnimationKey>(animatable);
		case ManoeuvreType::TARGET:
			return std::make_shared<TargetAnimation>(animatable);
		case ManoeuvreType::SEQUENCE:
			return std::make_shared<SequenceAnimation>(animatable);
	}
}

void Animation::subscribeToMachineParameter(){
	animatable->subscribeAnimation(shared_from_this());
}

void Animation::unsubscribeFromMachineParameter(){
	animatable->unsubscribeTrack(shared_from_this());
}


std::shared_ptr<Animation> Animation::copy(const std::shared_ptr<Animation> original){
	std::shared_ptr<Animation> copy;
	if(original->isComposite()) copy = original->toComposite()->copy();
	else{
		switch(original->getType()){
			case ManoeuvreType::KEY:	copy = original->toKey()->copy(); break;
			case ManoeuvreType::TARGET: copy = original->toTarget()->copy(); break;
			case ManoeuvreType::SEQUENCE: copy = original->toSequence()->copy(); break;
		}
	}
	//subscribe track to parameter changes
	copy->subscribeToMachineParameter();
	return copy;
}

std::shared_ptr<AnimationComposite> AnimationComposite::copy(){
	auto compositeCopy = std::make_shared<AnimationComposite>(getAnimatable()->toComposite());
	for(auto childParameterTrack : getChildren()){
		auto childCopy = Animation::copy(childParameterTrack);
		childCopy->setParentComposite(compositeCopy);
		compositeCopy->children.push_back(childCopy);
	}
	return compositeCopy;
}

std::shared_ptr<AnimationKey> AnimationKey::copy(){
	auto copy = std::make_shared<AnimationKey>(getAnimatable());
	getAnimatable()->copyParameterValue(target, copy->target);
	return copy;
}

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

std::shared_ptr<SequenceAnimation> SequenceAnimation::copy(){
	auto copy = std::make_shared<SequenceAnimation>(getAnimatable());
	getAnimatable()->copyParameterValue(target, copy->target);
	getAnimatable()->copyParameterValue(start, copy->start);
	copy->interpolationType->overwrite(interpolationType->value);
	copy->duration->overwrite(duration->value);
	copy->timeOffset->overwrite(timeOffset->value);
	copy->inAcceleration->overwrite(inAcceleration->value);
	copy->outAcceleration->overwrite(outAcceleration->value);
	//TODO: copy intermediate control points of all curves
	copy->updateAfterParameterEdit();
	return copy;
}


void Animation::validate(){
	validationErrorString = "";
	b_valid = getAnimatable()->getMachine()->validateAnimation(shared_from_this());
	if(hasManoeuvre()) manoeuvre->updateTrackSummary();
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





void Animation::updatePlaybackStatus(){
	if(playbackPosition_seconds >= duration_seconds){
		auto animatable = getAnimatable();
		if(animatable->hasAnimation()) animatable->getMachine()->endAnimationPlayback(animatable);
	}
}

std::shared_ptr<AnimationValue> Animation::getValueAtPlaybackTime(){
	return getAnimatable()->getValueAtAnimationTime(shared_from_this(), playbackPosition_seconds);
}


bool Animation::isMachineEnabled(){ return animatable->getMachine()->isEnabled(); }




bool Animation::isAtTarget(){
	//auto animatable = getAnimatableParameter();
	//return animatable->isParameterValueEqual(animatable->getParameterValue(target), animatable->getActualMachineValue());
}

bool Animation::isInRapid(){ return false; }

float Animation::getRapidProgress(){
	auto animatable = getAnimatable();
	float prog = animatable->getMachine()->getAnimatableRapidProgress(animatable);
	return prog;
}

void Animation::rapidToTarget(){
	/*
	auto animatable = getAnimatable();
	animatable->stopParameterPlayback();
	auto targetValue = animatable->getParameterValue(target);
	animatable->getMachine()->rapidParameterToValue(animatable, targetValue);
	 */
}

void Animation::startPlayback(){
	if(!isReadyToStartPlayback()) return;
	auto animatable = getAnimatable();
	animatable->getMachine()->startAnimationPlayback(shared_from_this());
}

void Animation::stop(){
	auto animatable = getAnimatable();
	if(animatable->hasAnimation()) animatable->getMachine()->endAnimationPlayback(animatable);
	else animatable->getMachine()->cancelAnimatableRapid(animatable);
	setPlaybackPosition(0.0);
	if(hasManoeuvre()) getManoeuvre()->onTrackPlaybackStop();
}

void Animation::interrupt(){
	auto animatable = getAnimatable();
	animatable->getMachine()->interruptAnimationPlayback(animatable);
	animatable->getMachine()->cancelAnimatableRapid(animatable);
}


bool Animation::isPlaying(){
	return getAnimatable()->getAnimation() == shared_from_this();
}















bool TargetAnimation::isAtPlaybackPosition(){
	auto animatable = getAnimatable();
	return animatable->isParameterValueEqual(getValueAtPlaybackTime(), animatable->getActualValue());
}

bool TargetAnimation::isReadyToStartPlayback(){
	return getAnimatable()->getMachine()->isEnabled();
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









bool SequenceAnimation::isAtStart(){
	auto animatable = getAnimatable();
	return animatable->isParameterValueEqual(animatable->parameterValueToAnimationValue(start), animatable->getActualValue());
}

void SequenceAnimation::rapidToStart(){
	auto animatable = getAnimatable();
	animatable->stopAnimationPlayback();
	animatable->getMachine()->rapidAnimatableToValue(animatable, animatable->parameterValueToAnimationValue(start));
}

void SequenceAnimation::rapidToPlaybackPosition(){
	auto animatable = getAnimatable();
	animatable->stopAnimationPlayback();
	auto valueAtPlaybackTime = getValueAtPlaybackTime();
	animatable->getMachine()->rapidAnimatableToValue(animatable, valueAtPlaybackTime);
}

bool SequenceAnimation::isAtPlaybackPosition(){
	auto animatable = getAnimatable();
	return animatable->isParameterValueEqual(getValueAtPlaybackTime(), animatable->getActualValue());
}

bool SequenceAnimation::isReadyToStartPlayback(){
	auto animatable = getAnimatable();
	if(!animatable->getMachine()->isEnabled()) return false;
	return animatable->getMachine()->isAnimatableReadyToStartPlaybackFromValue(animatable, getValueAtPlaybackTime());
}


void SequenceAnimation::updateAfterParameterEdit(){
	int curveCount = getAnimatable()->getCurveCount();
	auto& curves = getCurves();
	
	auto animatable = getAnimatable();
	auto startValue = animatable->parameterValueToAnimationValue(start);
	auto targetValue = animatable->parameterValueToAnimationValue(target);
	std::vector<double> curveStartPositions = animatable->getCurvePositionsFromAnimationValue(startValue);
	std::vector<double> curveEndPositions = animatable->getCurvePositionsFromAnimationValue(targetValue);
	
	for(int i = 0; i < curveCount; i++){
		auto& curve = curves[i];
		auto& points = curve.getPoints();
		
		std::shared_ptr<Motion::ControlPoint> startPoint = points.front();
		std::shared_ptr<Motion::ControlPoint> targetPoint = points.back();
				
		startPoint->position = curveStartPositions[i];
		startPoint->velocity = 0.0;
		startPoint->inAcceleration = inAcceleration->value;
		startPoint->outAcceleration = inAcceleration->value;
		startPoint->time = timeOffset->value;
		
		targetPoint->position = curveEndPositions[i];
		targetPoint->velocity = 0.0;
		targetPoint->inAcceleration = outAcceleration->value;
		targetPoint->outAcceleration = outAcceleration->value;
		targetPoint->time = timeOffset->value + duration->value;
		
		curve.refresh();
	}
	
	setDuration(timeOffset->value + duration->value);
	
	validate();
}

void SequenceAnimation::updateAfterCurveEdit(){
	auto& curves = getCurves();
	for(auto& curve : curves) curve.refresh();
	validate();
}

void SequenceAnimation::initializeCurves(){
	auto animatable = getAnimatable();
	int curveCount = animatable->getCurveCount();
	auto& curves = getCurves();
	
	auto startValue = animatable->parameterValueToAnimationValue(start);
	auto targetValue = animatable->parameterValueToAnimationValue(target);
	std::vector<double> curveStartPositions = animatable->getCurvePositionsFromAnimationValue(startValue);
	std::vector<double> curveEndPositions = animatable->getCurvePositionsFromAnimationValue(targetValue);
	
	for(int i = 0; i < curveCount; i++){
		auto& curve = curves[i];
		auto& points = curve.getPoints();
		curve.interpolationType = interpolationType->value;
		
		auto startPoint = std::make_shared<Motion::ControlPoint>();
		auto targetPoint = std::make_shared<Motion::ControlPoint>();
		
		startPoint->position = curveStartPositions[i];
		startPoint->velocity = 0.0;
		startPoint->inAcceleration = inAcceleration->value;
		startPoint->outAcceleration = inAcceleration->value;
		startPoint->time = timeOffset->value;
		
		targetPoint->position = curveEndPositions[i];
		targetPoint->velocity = 0.0;
		targetPoint->inAcceleration = outAcceleration->value;
		targetPoint->outAcceleration = outAcceleration->value;
		targetPoint->time = timeOffset->value + duration->value;
		
		points.push_back(startPoint);
		points.push_back(targetPoint);
		
		curve.refresh();
	}
	
	setDuration(timeOffset->value + duration->value);
	
}
