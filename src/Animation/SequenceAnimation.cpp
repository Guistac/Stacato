#include "Animation.h"
#include "Machine/Machine.h"
#include "Animation/Manoeuvre.h"

SequenceAnimation::SequenceAnimation(std::shared_ptr<Animatable> animatable) : Animation(animatable){
	target = animatable->makeParameter();
	target->setName("End");
	target->setSaveString("End");
	start = animatable->makeParameter();
	start->setName("Start");
	start->setSaveString("Start");
	inAcceleration->denyNegatives();
	outAcceleration->denyNegatives();
	if(animatable->isNumber()){
		inAcceleration->setPrefix("In: ");
		inAcceleration->setSuffix("/s\xC2\xB2");
		outAcceleration->setPrefix("Out: ");
		outAcceleration->setSuffix("/s\xC2\xB2");
		Unit unit = animatable->toNumber()->getUnit();
		setUnit(unit);
	}
	
	if(!animatable->isNumber() || !animatable->toNumber()->isReal()){
		inAcceleration->setDisabled(true);
		outAcceleration->setDisabled(true);
	}
	
	InterpolationType defaultInterpolation = animatable->getCompatibleInterpolationTypes().front();
	interpolationType = std::make_shared<EnumeratorParameter<InterpolationType>>(defaultInterpolation, "Interpolation Type", "interpolationType");
	
	auto editCallback = [this](std::shared_ptr<Parameter> thisParameter){ updateAfterParameterEdit(); };
	interpolationType->setEditCallback(editCallback);
	target->setEditCallback(editCallback);
	start->setEditCallback(editCallback);
	duration->setEditCallback(editCallback);
	timeOffset->setEditCallback(editCallback);
	inAcceleration->setEditCallback(editCallback);
	outAcceleration->setEditCallback(editCallback);
	initializeCurves();
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

bool SequenceAnimation::onSave(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	interpolationType->save(xml);
	start->save(xml);
	target->save(xml);
	duration->save(xml);
	timeOffset->save(xml);
	inAcceleration->save(xml);
	outAcceleration->save(xml);
	
	XMLElement* curvesXML = xml->InsertNewChildElement("Curves");
	for(auto& curve : getCurves()){
		XMLElement* curveXML = curvesXML->InsertNewChildElement("Curve");
		for(auto& controlPoint : curve->getPoints()){
			XMLElement* controlPointXML = curveXML->InsertNewChildElement("ControlPoint");
			controlPointXML->SetAttribute("Position", controlPoint->position);
			controlPointXML->SetAttribute("Time", controlPoint->time);
			controlPointXML->SetAttribute("Velocity", controlPoint->velocity);
			controlPointXML->SetAttribute("InAcceleration", controlPoint->inAcceleration);
			controlPointXML->SetAttribute("OutAcceleration", controlPoint->outAcceleration);
		}
	}
	
	return true;
}

std::shared_ptr<SequenceAnimation> SequenceAnimation::load(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable){
	
	auto sequenceAnimation = std::make_shared<SequenceAnimation>(animatable);
	
	if(!sequenceAnimation->interpolationType->load(xml)){
		Logger::warn("could not load attribute Interpolation Type of Animation {}", animatable->getName());
	}
	if(!sequenceAnimation->start->load(xml)){
		Logger::warn("could not load attribute Start of Animation {}", animatable->getName());
		return nullptr;
	}
	if(!sequenceAnimation->target->load(xml)){
		Logger::warn("could not load attribute End of Animation {}", animatable->getName());
		return nullptr;
	}
	if(!sequenceAnimation->duration->load(xml)){
		Logger::warn("could not load attribute Duration of Animation {}", animatable->getName());
		return nullptr;
	}
	if(!sequenceAnimation->timeOffset->load(xml)){
		Logger::warn("could not load attribute Time Offset of Animation {}", animatable->getName());
		return nullptr;
	}
	if(!sequenceAnimation->inAcceleration->load(xml)){
		Logger::warn("could not load attribute inAcceleration of Animation {}", animatable->getName());
		return nullptr;
	}
	if(!sequenceAnimation->outAcceleration->load(xml)){
		Logger::warn("could not load attribute outAcceleration of Animation {}", animatable->getName());
		return nullptr;
	}

	
	//load curve points
	//generate curves
	
	
	int curveCount = animatable->getCurveCount();
	auto& curves = sequenceAnimation->getCurves();
		
	auto startValue = animatable->parameterValueToAnimationValue(sequenceAnimation->start);
	auto targetValue = animatable->parameterValueToAnimationValue(sequenceAnimation->target);
	std::vector<double> curveStartPositions = animatable->getCurvePositionsFromAnimationValue(startValue);
	std::vector<double> curveEndPositions = animatable->getCurvePositionsFromAnimationValue(targetValue);
	
	for(int i = 0; i < curveCount; i++){
		auto& curve = curves[i];
		auto& points = curve->getPoints();
		points.clear();
		curve->interpolationType = sequenceAnimation->interpolationType->value;
		
		auto startPoint = std::make_shared<Motion::ControlPoint>();
		auto targetPoint = std::make_shared<Motion::ControlPoint>();
		
		startPoint->position = curveStartPositions[i];
		startPoint->velocity = 0.0;
		startPoint->inAcceleration = sequenceAnimation->inAcceleration->value;
		startPoint->outAcceleration = sequenceAnimation->inAcceleration->value;
		startPoint->time = sequenceAnimation->timeOffset->value;
		
		targetPoint->position = curveEndPositions[i];
		targetPoint->velocity = 0.0;
		targetPoint->inAcceleration = sequenceAnimation->outAcceleration->value;
		targetPoint->outAcceleration = sequenceAnimation->outAcceleration->value;
		targetPoint->time = sequenceAnimation->timeOffset->value + sequenceAnimation->duration->value;
		
		points.push_back(startPoint);
		
		//push intermediary points here
		
		points.push_back(targetPoint);
		
		curve->refresh();
	}
	
	sequenceAnimation->setDuration(sequenceAnimation->timeOffset->value + sequenceAnimation->duration->value);

	return sequenceAnimation;
}


void SequenceAnimation::setUnit(Unit unit){
	if(getAnimatable()->isNumber()){
		target->toNumber()->setUnit(unit);
		start->toNumber()->setUnit(unit);
	}
}

bool SequenceAnimation::isSimple(){
	return getCurves().front()->getPoints().size() == 2;
}

bool SequenceAnimation::isComplex(){
	return getCurves().front()->getPoints().size() > 2;
}


void SequenceAnimation::getCurvePositionRange(double& min, double& max){
	double mi = DBL_MAX;
	double ma = DBL_MIN;
	for(auto& curve : getCurves()){
		for(auto& controlPoint : curve->getPoints()){
			mi = std::min(mi, controlPoint->position);
			ma = std::max(ma, controlPoint->position);
		}
	}
	min = mi;
	max = ma;
}



void SequenceAnimation::captureStart(){
	auto animatable = getAnimatable();
	animatable->setParameterValueFromAnimationValue(start, animatable->getActualValue());
}

void SequenceAnimation::captureTarget(){
	auto animatable = getAnimatable();
	animatable->setParameterValueFromAnimationValue(target, animatable->getActualValue());
}




bool SequenceAnimation::canRapidToStart(){
	return start->isValid() && canRapid();
}

bool SequenceAnimation::canRapidToTarget(){
	return target->isValid() && canRapid();
}

bool SequenceAnimation::canRapidToPlaybackPosition(){
	return isValid() && canRapid();
}




bool SequenceAnimation::isAtStart(){
	if(!start->isValid()) return false;
	auto animatable = getAnimatable();
	return animatable->isParameterValueEqual(animatable->parameterValueToAnimationValue(start), animatable->getTargetValue());
}

bool SequenceAnimation::isAtTarget(){
	if(!target->isValid()) return false;
	auto animatable = getAnimatable();
	return animatable->isParameterValueEqual(animatable->parameterValueToAnimationValue(target), animatable->getTargetValue());
}

bool SequenceAnimation::isAtPlaybackPosition(){
	if(!isValid()) return false;
	auto animatable = getAnimatable();
	return animatable->isParameterValueEqual(getValueAtPlaybackTime(), animatable->getTargetValue());
}




bool SequenceAnimation::onRapidToStart(){
	auto animatable = getAnimatable();
	auto targetValue = animatable->parameterValueToAnimationValue(start);
	animatable->rapidToValue(targetValue);
	return true;
}

bool SequenceAnimation::onRapidToTarget(){
	auto animatable = getAnimatable();
	auto targetValue = animatable->parameterValueToAnimationValue(target);
	animatable->rapidToValue(targetValue);
	return true;
}

bool SequenceAnimation::onRapidToPlaybackPosition(){
	auto animatable = getAnimatable();
	auto valueAtPlaybackTime = getValueAtPlaybackTime();
	animatable->rapidToValue(valueAtPlaybackTime);
	return true;
}




bool SequenceAnimation::canStartPlayback(){
	if(!isValid()) return false;
	if(getPlaybackPosition() >= getDuration()) return false;
	auto animatable = getAnimatable();
	if(!animatable->isReadyToMove()) return false;
	if(isPlaying()) return false;
	return animatable->isReadyToStartPlaybackFromValue(getValueAtPlaybackTime());
}

bool SequenceAnimation::canPausePlayback(){
	return isPlaying();
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
		auto& points = curve->getPoints();
		
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
		
		curve->refresh();
	}
	
	setDuration(timeOffset->value + duration->value);
	
	validate();
	
	requestCurveRefocus();
}

void SequenceAnimation::updateAfterCurveEdit(){
	auto& curves = getCurves();
	for(auto& curve : curves) curve->refresh();
	validate();
}

void SequenceAnimation::initializeCurves(){
	auto animatable = getAnimatable();
	int curveCount = animatable->getCurveCount();
	auto& curveNames = animatable->getCurveNames();
	auto& curves = getCurves();
	curves.resize(curveCount);
	
	auto startValue = animatable->parameterValueToAnimationValue(start);
	auto targetValue = animatable->parameterValueToAnimationValue(target);
	std::vector<double> curveStartPositions = animatable->getCurvePositionsFromAnimationValue(startValue);
	std::vector<double> curveEndPositions = animatable->getCurvePositionsFromAnimationValue(targetValue);
	
	for(int i = 0; i < curveCount; i++){
		auto& curve = curves[i];
		
		curve = std::make_shared<Motion::Curve>(curveNames[i]);
		
		
		auto& points = curve->getPoints();
		curve->interpolationType = interpolationType->value;
		
		auto startPoint = std::make_shared<Motion::ControlPoint>();
		auto targetPoint = std::make_shared<Motion::ControlPoint>();
		startPoint->b_valid = true;
		targetPoint->b_valid = true;
		
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
		
		curve->refresh();
	}
	
	setDuration(timeOffset->value + duration->value);
	
}

void SequenceAnimation::addCurvePoint(std::shared_ptr<Motion::Curve> targetCurve, float time, float position){
	//check if the animation containts the target curve
	bool b_hasCurve = false;
	for(auto curve : getCurves()){
		if(curve == targetCurve) {
			b_hasCurve = true;
			break;
		}
	}
	if(!b_hasCurve) return;
	
	auto newControlPoint = std::make_shared<Motion::ControlPoint>();
	newControlPoint->position = position;
	newControlPoint->time = time;
	newControlPoint->velocity = 0.0;
	newControlPoint->inAcceleration = 0.0;
	newControlPoint->outAcceleration = 0.0;
	
	targetCurve->addPoint(newControlPoint);
	getManoeuvre()->selectControlPoint(newControlPoint);
}
