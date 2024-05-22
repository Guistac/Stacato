#include "Animation.h"
#include "Machine/Machine.h"
#include "Animation/Manoeuvre.h"
#include "Animation/Animatables/AnimatablePosition.h"

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
	
	auto& copyCurves = copy->getCurves();
	auto& originalCurves = getCurves();
	
	copyCurves.clear();
	for(auto& curve : getCurves()){
		copyCurves.push_back(curve->copy());
	}
	
	copy->updateAfterParameterEdit();
	return copy;
}

bool SequenceAnimation::onSave(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	xml->SetAttribute("isLoop", b_isLoop);
	interpolationType->save(xml);
	start->save(xml);
	target->save(xml);
	duration->save(xml);
	timeOffset->save(xml);
	inAcceleration->save(xml);
	outAcceleration->save(xml);
	
	XMLElement* curvesXML = xml->InsertNewChildElement("Curves");
	int curveCount = getAnimatable()->getCurveCount();
	auto& curveNames = getAnimatable()->getCurveNames();
	
	for(int c = 0; c < curveCount; c++){
		XMLElement* curveXML = curvesXML->InsertNewChildElement("Curve");
		curveXML->SetAttribute("Name", curveNames[c].c_str());
		auto curve = getCurves()[c];
		auto& controlPoints = curve->getPoints();
		for(int i = 1; i < controlPoints.size() - 1; i++){
			auto controlPoint = controlPoints[i];
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
	
	using namespace tinyxml2;
	
	auto sequenceAnimation = std::make_shared<SequenceAnimation>(animatable);
	
	xml->QueryBoolAttribute("isLoop", &sequenceAnimation->b_isLoop);
	if(!sequenceAnimation->interpolationType->load(xml)) return nullptr;
	if(!sequenceAnimation->start->load(xml)) return nullptr;
	if(!sequenceAnimation->target->load(xml)) return nullptr;
	if(!sequenceAnimation->duration->load(xml)) return nullptr;
	if(!sequenceAnimation->timeOffset->load(xml)) return nullptr;
	if(!sequenceAnimation->inAcceleration->load(xml)) return nullptr;
	if(!sequenceAnimation->outAcceleration->load(xml)) return nullptr;
	
	int curveCount = animatable->getCurveCount();
	auto& curves = sequenceAnimation->getCurves();
		
	auto startValue = animatable->parameterValueToAnimationValue(sequenceAnimation->start);
	auto targetValue = animatable->parameterValueToAnimationValue(sequenceAnimation->target);
	std::vector<double> curveStartPositions = animatable->getCurvePositionsFromAnimationValue(startValue);
	std::vector<double> curveEndPositions = animatable->getCurvePositionsFromAnimationValue(targetValue);
	
	for(int i = 0; i < curveCount; i++){
		auto& curve = curves[i];
		curve->getPoints().clear();
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
		
		curve->addPoint(startPoint);
		curve->addPoint(targetPoint);
	}
	
	XMLElement* curvesXML;
	if(!loadXMLElement("Curves", xml, curvesXML)) return nullptr;
	auto& curveNames = sequenceAnimation->getAnimatable()->getCurveNames();
	
	XMLElement* curveXML = curvesXML->FirstChildElement("Curve");
	while(curveXML){
        
		const char* curveName;
		if(curveXML->QueryStringAttribute("Name", &curveName) != XML_SUCCESS) {
			Logger::warn("could not load curve name attribute");
			return nullptr;
		}
		
		std::shared_ptr<Motion::Curve> curve = nullptr;
		for(int c = 0; c < curveCount; c++){
			if(strcmp(curveName, curveNames[c].c_str()) == 0){
				curve = curves[c];
				break;
			}
		}
		if(curve == nullptr) {
			Logger::warn("Could not identify curve");
			return nullptr;
		}
		
		XMLElement* controlPointXML = curveXML->FirstChildElement("ControlPoint");
		while(controlPointXML){
			
			auto controlPoint = std::make_shared<Motion::ControlPoint>();
			
			if(controlPointXML->QueryDoubleAttribute("Position", &controlPoint->position) != XML_SUCCESS){
				Logger::warn("could not load controlpoint position");
				return nullptr;
			}
			if(controlPointXML->QueryDoubleAttribute("Time", &controlPoint->time) != XML_SUCCESS){
				Logger::warn("could not load controlpoint time");
				return nullptr;
			}
			if(controlPointXML->QueryDoubleAttribute("Velocity", &controlPoint->velocity) != XML_SUCCESS){
				Logger::warn("could not load controlpoint velocity");
				return nullptr;
			}
			if(controlPointXML->QueryDoubleAttribute("InAcceleration", &controlPoint->inAcceleration) != XML_SUCCESS){
				Logger::warn("could not load controlpoint in Acceleration");
				return nullptr;
			}
			if(controlPointXML->QueryDoubleAttribute("OutAcceleration", &controlPoint->outAcceleration) != XML_SUCCESS){
				Logger::warn("could not load controlpoint out Acceleration");
				return nullptr;
			}

			curve->addPoint(controlPoint);
			
			controlPointXML = controlPointXML->NextSiblingElement("ControlPoint");
		}
		
		
		
		curveXML = curveXML->NextSiblingElement("Curve");
	}
	
	
	for(auto curve : curves) curve->refresh();
	
	sequenceAnimation->setDuration(sequenceAnimation->timeOffset->value + sequenceAnimation->duration->value);

	sequenceAnimation->updateTheoreticalShortestDuration();
	
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
	animatable->setParameterValueFromAnimationValue(start, animatable->getTargetValue());
	updateAfterParameterEdit();
}

void SequenceAnimation::captureTarget(){
	auto animatable = getAnimatable();
	animatable->setParameterValueFromAnimationValue(target, animatable->getTargetValue());
	updateAfterParameterEdit();
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
	
	//make start and end point the same
	if(b_isLoop) animatable->copyParameterValue(start, target);
	
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
	
	updateTheoreticalShortestDuration();
	
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
	
	Unit animatableUnit = Units::None::None;
	if(animatable->isNumber()) animatableUnit = animatable->toNumber()->getUnit();
	
	for(int i = 0; i < curveCount; i++){
		auto& curve = curves[i];
		
		curve = std::make_shared<Motion::Curve>(curveNames[i]);
		curve->unit = animatableUnit;
		
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
		startPoint->unit = animatableUnit;
		
		targetPoint->position = curveEndPositions[i];
		targetPoint->velocity = 0.0;
		targetPoint->inAcceleration = outAcceleration->value;
		targetPoint->outAcceleration = outAcceleration->value;
		targetPoint->time = timeOffset->value + duration->value;
		targetPoint->unit = animatableUnit;
		
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
	newControlPoint->b_valid = true;
	
	getAnimatable()->fillControlPointDefaults(newControlPoint);
	
	targetCurve->addPoint(newControlPoint);
	getManoeuvre()->selectControlPoint(newControlPoint);
}

void SequenceAnimation::updateTheoreticalShortestDuration(){

	auto animatable = getAnimatable();
	
	if(animatable->getType() == AnimatableType::POSITION){
		
		auto startValue = animatable->parameterValueToAnimationValue(start);
		auto targetValue = animatable->parameterValueToAnimationValue(target);
		std::vector<double> curveStartPositions = animatable->getCurvePositionsFromAnimationValue(startValue);
		std::vector<double> curveEndPositions = animatable->getCurvePositionsFromAnimationValue(targetValue);
		
		auto startPoint = std::make_shared<Motion::ControlPoint>();
		startPoint->time = 0.0;
		startPoint->position = curveStartPositions[0];
		startPoint->velocity = 0.0;
		startPoint->inAcceleration = inAcceleration->value;
		startPoint->outAcceleration = inAcceleration->value;
		auto endPoint = std::make_shared<Motion::ControlPoint>();
		endPoint->time = 0.0;
		endPoint->position = curveEndPositions[0];
		endPoint->velocity = 0.0;
		endPoint->inAcceleration = outAcceleration->value;
		endPoint->outAcceleration = outAcceleration->value;
		
		auto animatablePosition = animatable->toPosition();
		if(auto fastestInterpolation = Motion::TrapezoidalInterpolation::getTimeConstrainedOrSlower(startPoint, endPoint, animatablePosition->velocityLimit)){
			theoreticalShortestDuration = fastestInterpolation->endTime + 0.01;
		}else{
			theoreticalShortestDuration = 0.0;
		}
	}
	
}


void SequenceAnimation::changeGlobalSpeed(double factor){
	if(factor <= 0.0) return;
	for(auto curve : getCurves()){
		for(auto point : curve->getPoints()){
			point->time *= factor;
			point->velocity /= factor;
			point->inAcceleration /= factor;
			point->outAcceleration /= factor;
		}
	}
	setDuration(getDuration()*factor);
	duration->overwrite(getDuration());
	updateAfterCurveEdit();
}

void SequenceAnimation::subtractTime(double from, double amount){
	if(from < 0.0) return;
	//if(amount <= 0.0) return;
	//if(amount > from) return;
	
	for(auto curve : getCurves()){
		for(auto point : curve->getPoints()){
			if(point->time < from) continue;
			if(point == curve->getPoints().back()) continue;
			point->time -= amount;
		}
	}
	
	updateAfterCurveEdit();
	
}
