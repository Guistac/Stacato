#include "Animation.h"
#include "Machine/Machine.h"

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
		for(auto& controlPoint : curve.getPoints()){
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
		auto& points = curve.getPoints();
		points.clear();
		curve.interpolationType = sequenceAnimation->interpolationType->value;
		
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
		
		curve.refresh();
	}
	
	sequenceAnimation->setDuration(sequenceAnimation->timeOffset->value + sequenceAnimation->duration->value);

	return sequenceAnimation;
}
