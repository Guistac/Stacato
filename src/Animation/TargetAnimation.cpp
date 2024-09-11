#include "Animation.h"
#include "Machine/Machine.h"
#include "Animation/Manoeuvre.h"


TargetAnimation::TargetAnimation(std::shared_ptr<Animatable> animatable) : Animation(animatable){
	target = animatable->makeParameter();
	target->setSaveString("Target");
	target->setName("Target");
	velocityConstraint->denyNegatives();
	inAcceleration->denyNegatives();
	outAcceleration->denyNegatives();
	if(animatable->isNumber()){
		velocityConstraint->setPrefix("Velocity: ");
		velocityConstraint->setSuffix("/s");
		inAcceleration->setPrefix("In: ");
		inAcceleration->setSuffix("/s\xC2\xB2");
		outAcceleration->setPrefix("Out: ");
		outAcceleration->setSuffix("/s\xC2\xB2");
		Unit unit = animatable->toNumber()->getUnit();
		setUnit(unit);
	}
	if(!animatable->isNumber() || !animatable->toNumber()->isReal()){
		constraintType->overwrite(Constraint::TIME);
		constraintType->setDisabled(true);
		velocityConstraint->setDisabled(true);
		inAcceleration->setDisabled(true);
		outAcceleration->setDisabled(true);
	}
	
	InterpolationType defaultInterpolation = animatable->getCompatibleInterpolationTypes().front();
	interpolationType = std::make_shared<EnumeratorParameter<InterpolationType>>(defaultInterpolation, "Interpolation Type", "interpolationType");
	
	auto editCallback = [this](std::shared_ptr<Parameter> thisParameter){ validate(); };
	target->setEditCallback(editCallback);
	interpolationType->setEditCallback(editCallback);
	velocityConstraint->setEditCallback(editCallback);
	inAcceleration->setEditCallback(editCallback);
	outAcceleration->setEditCallback(editCallback);
	constraintType->setEditCallback(editCallback);
	timeConstraint->setEditCallback(editCallback);
	
	getCurves().resize(animatable->getCurveCount());
	int curveCount = animatable->getCurveCount();
	auto& curveNames = animatable->getCurveNames();
	for(int i = 0; i < curveCount; i++) getCurves()[i] = std::make_shared<Motion::Curve>(curveNames[i]);
	
}


std::shared_ptr<TargetAnimation> TargetAnimation::copy(){
	auto copy = std::make_shared<TargetAnimation>(getAnimatable());
	getAnimatable()->copyParameterValue(target, copy->target);
	copy->interpolationType->overwrite(interpolationType->value);
	copy->timeConstraint->overwrite(timeConstraint->value);
	copy->velocityConstraint->overwrite(velocityConstraint->value);
	copy->inAcceleration->overwrite(inAcceleration->value);
	copy->outAcceleration->overwrite(outAcceleration->value);
	copy->constraintType->overwrite(constraintType->value);
	return copy;
}

bool TargetAnimation::onSave(tinyxml2::XMLElement* xml){
	//save type, constraint, target, ramps
	target->save(xml);
	constraintType->save(xml);
	timeConstraint->save(xml);
	velocityConstraint->save(xml);
	inAcceleration->save(xml);
	outAcceleration->save(xml);
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
	return targetAnimation;
}


bool TargetAnimation::areCurvesGenerated(){
	for(auto& curve : getCurves()){
		if(curve->getPoints().empty() || curve->getInterpolations().empty()) return false;
	}
	return true;
}

void TargetAnimation::clearCurves(){
	for(auto& curve : getCurves()){
		curve->getPoints().clear();
		curve->getInterpolations().clear();
	}
}


void TargetAnimation::setUnit(Unit unit){
	if(getAnimatable()->isNumber()){
		target->toNumber()->setUnit(unit);
		velocityConstraint->setUnit(unit);
		inAcceleration->setUnit(unit);
		outAcceleration->setUnit(unit);
	}
}


void TargetAnimation::getCurvePositionRange(double& min, double& max){
	double mi = DBL_MAX;
	double ma = DBL_MIN;
	if(areCurvesGenerated()){
		for(auto& curve : getCurves()){
			for(auto& controlPoint : curve->getPoints()){
				mi = std::min(mi, controlPoint->position);
				ma = std::max(ma, controlPoint->position);
			}
		}
	}else{
		auto animatable = getAnimatable();
		auto targetValue = animatable->parameterValueToAnimationValue(target);
		auto curvePositions = animatable->getCurvePositionsFromAnimationValue(targetValue);
		for(double value : curvePositions){
			mi = std::min(mi, value);
			ma = std::max(ma, value);
		}
	}
	min = mi;
	max = ma;
}


bool TargetAnimation::isAtTarget(){
	if(!target->isValid()) return false;
	auto animatable = getAnimatable();
	if(animatable->getType() == AnimatableType::VELOCITY) {
		//for velocity animatables, the end target is always 0 velocity
		return animatable->getCurvePositionsFromAnimationValue(animatable->getTargetValue()).front() == 0.0;
	}
	bool equal = animatable->isParameterValueEqual(animatable->parameterValueToAnimationValue(target), animatable->getTargetValue());
	return equal;
}

bool TargetAnimation::canRapidToTarget(){
	return target->isValid() && canRapid();
}

bool TargetAnimation::onRapidToTarget(){
	auto animatable = getAnimatable();
	auto targetValue = animatable->parameterValueToAnimationValue(target);
	animatable->rapidToValue(targetValue);
	return true;
}


bool TargetAnimation::canStartPlayback(){
	if(!isValid()) return false;
	auto animatable = getAnimatable();
	if(!animatable->isReadyToMove()) return false;
	auto animatableTargetValue = animatable->getTargetValue();
	auto targetValue = animatable->parameterValueToAnimationValue(target);
	bool b_alreadyAtTarget = animatable->isParameterValueEqual(animatableTargetValue, targetValue);
	return !b_alreadyAtTarget;
	return true;
}

bool TargetAnimation::onStartPlayback(){
	clearCurves();
	if(!canStartPlayback()) return false;
	auto animatable = getAnimatable();
	auto thisTargetAnimation = shared_from_this()->toTarget();
	setPlaybackPosition(0.0);
	return animatable->generateTargetAnimation(thisTargetAnimation);
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
