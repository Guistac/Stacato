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
	if(hasManoeuvre()) manoeuvre->updateValidation();
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




/*

//================================== EDITING ==================================


void ParameterTrack::refreshAfterCurveEdit() {
	//copy the settings from the start and endpoints to the track settings

	switch (parameter->dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER:
			origin.boolean = startPoints.front()->position > 0.5 ? true : false;
			target.boolean = endPoints.front()->position > 0.5 ? true : false;
			break;
		case ParameterDataType::INTEGER_PARAMETER:
			origin.integer = std::round(startPoints.front()->position);
			target.integer = std::round(endPoints.front()->position);
			break;
		case ParameterDataType::STATE_PARAMETER: {
			int originInteger = std::round(startPoints.front()->position);
			std::clamp(originInteger, 0, (int)parameter->getStateValues().size() - 1);
			origin.state = &parameter->getStateValues().at(originInteger);
			int targetInteger = std::round(endPoints.front()->position);
			std::clamp(targetInteger, 0, (int)parameter->getStateValues().size() - 1);
			target.state = &parameter->getStateValues().at(targetInteger);
			}break;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::POSITION_3D:
			origin.vector3.x = startPoints[0]->position;
			origin.vector3.y = startPoints[1]->position;
			origin.vector3.z = startPoints[2]->position;
			target.vector3.x = endPoints[0]->position;
			target.vector3.y = endPoints[1]->position;
			target.vector3.z = endPoints[2]->position;
			break;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::POSITION_2D:
			origin.vector2.x = startPoints[0]->position;
			origin.vector2.y = startPoints[1]->position;
			target.vector2.x = endPoints[0]->position;
			target.vector2.y = endPoints[1]->position;
			break;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::POSITION:
			origin.real = startPoints[0]->position;
			target.real = endPoints[0]->position;
			break;
		case ParameterDataType::PARAMETER_GROUP:
			break;
	}

	bool offsetChanged = false;
	//detect if the startpoint was moved (offset time changed)
	for (int i = 0; i < getCurveCount(); i++) {
		if (startPoints[i]->time != timeOffset) {
			timeOffset = startPoints[i]->time;
			if (timeOffset < 0) timeOffset = 0;
			offsetChanged = true;
			break;
		}
	}

	switch (interpolationType) {
		case Motion::InterpolationType::STEP:
			timeOffset = 0.0;
			break;
		default:
			break;
	}

	if (!offsetChanged) {
		//detect if the endpoint was moved (movement time constraint changed)
		for (int i = 0; i < getCurveCount(); i++) {
			double thisCurveTimeConstraint = endPoints[i]->time - startPoints[i]->time;
			if (thisCurveTimeConstraint != movementTime) {
				movementTime = thisCurveTimeConstraint;
				if (movementTime < 0) movementTime = 0;
				break;
			}
		}
	}

	refreshAfterParameterEdit();
}


//========================================== PLAYBACK ====================================================



void ParameterTrack::rapidToStart() {
	if (parameter->machine->isEnabled()) {
		parameter->machine->rapidParameterToValue(parameter, origin);
	}
}

void ParameterTrack::rapidToEnd() {
	if (parameter->machine->isEnabled()) {
		parameter->machine->rapidParameterToValue(parameter, target);
	}
}

void ParameterTrack::rapidToPlaybackPosition() {
	if (parameter->machine->isEnabled()) {
		AnimatableParameterValue parameterValueAtPlaybackPosition;
		getParameterValueAtPlaybackTime(parameterValueAtPlaybackPosition);
		parameter->machine->rapidParameterToValue(parameter, parameterValueAtPlaybackPosition);
	}
}

void ParameterTrack::cancelRapid() {
	parameter->machine->cancelParameterRapid(parameter);
}


float ParameterTrack::getRapidProgress() {
	return parameter->machine->getParameterRapidProgress(parameter);
}


bool ParameterTrack::isPrimedToStart() {
	return parameter->machine->isParameterReadyToStartPlaybackFromValue(parameter, origin);
}

bool ParameterTrack::isPrimedToEnd() {
	return parameter->machine->isParameterReadyToStartPlaybackFromValue(parameter, target);
}

bool ParameterTrack::isPrimedToPlaybackPosition() {
	AnimatableParameterValue parameterValueAtPlaybackPosition;
	getParameterValueAtPlaybackTime(parameterValueAtPlaybackPosition);
	return parameter->machine->isParameterReadyToStartPlaybackFromValue(parameter, parameterValueAtPlaybackPosition);
}


double ParameterTrack::getLength_seconds() {
	double longestCurve = 0.0;
	for (auto& curve : curves) {
		double time;
		if (curve->points.size() <= 1) time = 0;
		else if (curve->getEnd()->time > longestCurve) longestCurve = curve->getEnd()->time;
	}
	return longestCurve;
}



*/
