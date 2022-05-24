#include <pch.h>

#include "ParameterTrack.h"

#include "Machine/Machine.h"
#include "Machine/AnimatableParameter.h"
#include "Motion/Curve/Curve.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Environnement/Environnement.h"
#include "Motion/Manoeuvre/Manoeuvre.h"
#include "Plot/Plot.h"

#include "Project/Editor/Parameter.h"


std::shared_ptr<ParameterTrack> ParameterTrack::create(std::shared_ptr<MachineParameter> parameter, ManoeuvreType manoeuvreType){
	if(parameter->isGroup()) return std::make_shared<ParameterTrackGroup>(parameter->castToGroup(), manoeuvreType);
	else{
		auto animatableParameter = parameter->castToAnimatable();
		switch(manoeuvreType){
			case ManoeuvreType::KEY:
				return std::make_shared<KeyParameterTrack>(animatableParameter);
			case ManoeuvreType::TARGET:
				return std::make_shared<TargetParameterTrack>(animatableParameter);
			case ManoeuvreType::SEQUENCE:
				return std::make_shared<SequenceParameterTrack>(animatableParameter);
		}
	}
}

void ParameterTrack::subscribeToMachineParameter(){
	getParameter()->subscribeTrack(shared_from_this());
}

void ParameterTrack::unsubscribeFromMachineParameter(){
	getParameter()->unsubscribeTrack(shared_from_this());
}


std::shared_ptr<ParameterTrack> ParameterTrack::copy(const std::shared_ptr<ParameterTrack> original){
	std::shared_ptr<ParameterTrack> copy;
	if(original->isGroup()) copy = original->castToGroup()->copy();
	else{
		auto animatedOriginal = original->castToAnimated();
		switch(animatedOriginal->getType()){
			case ManoeuvreType::KEY:	copy = original->castToKey()->copy(); break;
			case ManoeuvreType::TARGET: copy = original->castToTarget()->copy(); break;
			case ManoeuvreType::SEQUENCE: copy = original->castToSequence()->copy(); break;
		}
	}
	//subscribe track to parameter changes
	copy->subscribeToMachineParameter();
	return copy;
}

std::shared_ptr<ParameterTrackGroup> ParameterTrackGroup::copy(){
	auto groupCopy = std::make_shared<ParameterTrackGroup>(getParameter()->castToGroup());
	for(auto childParameterTrack : getChildren()){
		auto childCopy = ParameterTrack::copy(childParameterTrack);
		childCopy->setParent(groupCopy);
		groupCopy->children.push_back(childCopy);
	}
	return groupCopy;
}

std::shared_ptr<KeyParameterTrack> KeyParameterTrack::copy(){
	auto copy = std::make_shared<KeyParameterTrack>(getAnimatableParameter());
	getAnimatableParameter()->copyParameterValue(target, copy->target);
	return copy;
}

std::shared_ptr<TargetParameterTrack> TargetParameterTrack::copy(){
	auto copy = std::make_shared<TargetParameterTrack>(getAnimatableParameter());
	getAnimatableParameter()->copyParameterValue(target, copy->target);
	copy->interpolationType->overwrite(interpolationType->value);
	copy->timeConstraint->overwrite(timeConstraint->value);
	copy->velocityConstraint->overwrite(velocityConstraint->value);
	copy->inAcceleration->overwrite(inAcceleration->value);
	copy->outAcceleration->overwrite(outAcceleration->value);
	copy->timeOffset->overwrite(timeOffset->value);
	copy->constraintType->overwrite(constraintType->value);
	return copy;
}

std::shared_ptr<SequenceParameterTrack> SequenceParameterTrack::copy(){
	auto copy = std::make_shared<SequenceParameterTrack>(getAnimatableParameter());
	getAnimatableParameter()->copyParameterValue(target, copy->target);
	getAnimatableParameter()->copyParameterValue(start, copy->start);
	copy->interpolationType->overwrite(interpolationType->value);
	copy->duration->overwrite(duration->value);
	copy->timeOffset->overwrite(timeOffset->value);
	//TODO: copy curves
	return copy;
}


void ParameterTrack::validate(){
	validationErrorString = "";
	b_valid = getParameter()->getMachine()->validateParameterTrack(shared_from_this());
	if(hasManoeuvre()) manoeuvre->updateTrackSummary();
}



class CaptureCurrentMachineParameterValueCommand : public Command{
public:
	
	std::shared_ptr<Parameter> parameter;
	std::shared_ptr<AnimatableParameter> animatableParameter;
	std::shared_ptr<AnimatableParameterValue> previousValue;
	std::shared_ptr<AnimatableParameterValue> capturedValue;
	
	CaptureCurrentMachineParameterValueCommand(std::string& name, std::shared_ptr<Parameter> parameter_, std::shared_ptr<AnimatableParameter> animatableParameter_)
	: Command(name) {
		parameter = parameter_;
		animatableParameter = animatableParameter_;
	};
	
	virtual void execute(){
		capturedValue = animatableParameter->getActualMachineValue();
		previousValue = animatableParameter->getParameterValue(parameter);
		animatableParameter->setParameterValue(parameter, capturedValue);
	}
	virtual void undo(){
		animatableParameter->setParameterValue(parameter, previousValue);
	}
	virtual void redo(){
		animatableParameter->setParameterValue(parameter, capturedValue);
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





void PlayableParameterTrack::updatePlaybackStatus(){
	if(playbackPosition_seconds >= duration_seconds){
		auto parameter = getParameter();
		parameter->getMachine()->onParameterPlaybackEnd(parameter);
		//TODO: don't call this if the playback is already finished
	}
}

std::shared_ptr<AnimatableParameterValue> PlayableParameterTrack::getParameterValueAtPlaybackTime(){
	return getAnimatableParameter()->getParameterValueAtCurveTime(castToPlayable(), playbackPosition_seconds);
}


bool ParameterTrack::isMachineEnabled(){ return parameter->getMachine()->isEnabled(); }




bool AnimatedParameterTrack::isAtTarget(){
	auto animatable = getAnimatableParameter();
	return animatable->isParameterValueEqual(animatable->getParameterValue(target), animatable->getActualMachineValue());
}

bool AnimatedParameterTrack::isInRapid(){ return false; }

float AnimatedParameterTrack::getRapidProgress(){
	auto animatable = getAnimatableParameter();
	return animatable->getMachine()->getParameterRapidProgress(animatable);
}

void AnimatedParameterTrack::rapidToTarget(){
	auto animatable = getAnimatableParameter();
	animatable->getMachine()->rapidParameterToValue(animatable, animatable->getParameterValue(target));
}

void AnimatedParameterTrack::stop(){
	auto animatable = getAnimatableParameter();
	animatable->getMachine()->cancelParameterRapid(animatable);
	
	animatable->getMachine()->onParameterPlaybackInterrupt(animatable);
	
}

void ParameterTrack::startPlayback(){
	auto parameter = getParameter();
	parameter->getMachine()->startParameterPlayback(shared_from_this());
}








bool TargetParameterTrack::isAtPlaybackPosition(){
	auto animatable = getAnimatableParameter();
	return animatable->isParameterValueEqual(getParameterValueAtPlaybackTime(), animatable->getActualMachineValue());
}

bool TargetParameterTrack::isReadyToStartPlayback(){
	auto animatable = getAnimatableParameter();
	animatable->getMachine()->isParameterReadyToStartPlaybackFromValue(animatable, getParameterValueAtPlaybackTime());
}



bool SequenceParameterTrack::isAtStart(){
	auto animatable = getAnimatableParameter();
	return animatable->isParameterValueEqual(animatable->getParameterValue(start), animatable->getActualMachineValue());
}

void SequenceParameterTrack::rapidToStart(){
	auto animatable = getAnimatableParameter();
	animatable->getMachine()->rapidParameterToValue(animatable, animatable->getParameterValue(start));
}

void SequenceParameterTrack::rapidToPlaybackPosition(){
	auto animatable = getAnimatableParameter();
	auto valueAtPlaybackTime = getParameterValueAtPlaybackTime();
	animatable->getMachine()->rapidParameterToValue(animatable, valueAtPlaybackTime);
}

bool SequenceParameterTrack::isAtPlaybackPosition(){
	auto animatable = getAnimatableParameter();
	return animatable->isParameterValueEqual(getParameterValueAtPlaybackTime(), animatable->getActualMachineValue());
}

bool SequenceParameterTrack::isReadyToStartPlayback(){
	auto animatable = getAnimatableParameter();
	return animatable->getMachine()->isParameterReadyToStartPlaybackFromValue(animatable, getParameterValueAtPlaybackTime());
}


void SequenceParameterTrack::updateAfterParameterEdit(){
	int curveCount = getAnimatableParameter()->getCurveCount();
	auto& curves = getCurves();
	
	auto animatable = getAnimatableParameter();
	auto startValue = animatable->getParameterValue(start);
	auto targetValue = animatable->getParameterValue(target);
	std::vector<double> curveStartPositions = animatable->getCurvePositionsFromParameterValue(startValue);
	std::vector<double> curveEndPositions = animatable->getCurvePositionsFromParameterValue(targetValue);
	
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
	
	duration_seconds = timeOffset->value + duration->value;
	
	validate();
}

void SequenceParameterTrack::updateAfterCurveEdit(){
	auto& curves = getCurves();
	for(auto& curve : curves) curve.refresh();
	validate();
}
