#include <pch.h>

/*
#include "AnimatableReal.h"

#include "Motion/Curve/Curve.h"
#include "Project/Editor/Parameter.h"
#include "Animation/Animation.h"

std::string AnimatableReal::getTargetValueString(){
    static char buffer[32];
    sprintf(buffer, "%.3f", targetValue->value);
    return std::string(buffer);
}

std::vector<std::string>& AnimatableReal::getCurveNames(){
	static std::vector<std::string> names = { "Value" };
	return names;
}

void AnimatableReal::fillControlPointDefaults(std::shared_ptr<Motion::ControlPoint> controlpoint){
	controlpoint->velocity = 0.0;
	controlpoint->inAcceleration = 0.0;
	controlpoint->outAcceleration = 0.0;
}

void AnimatableReal::updateActualValue(std::shared_ptr<AnimationValue> newActualValue){
    mutex.lock();
    actualValue = newActualValue->toReal();
    mutex.unlock();
}
void AnimatableReal::updateTargetValue(double time_seconds, double deltaTime_seconds){
    mutex.lock();
    if(hasAnimation() && getAnimation()->isPlaying()){
        auto target = getAnimationValue()->toReal();
        targetValue->value = target->value;
    }
    mutex.unlock();
}
void AnimatableReal::followActualValue(double time_seconds, double deltaTime_seconds){
    mutex.lock();
    targetValue->value = actualValue->value;
    mutex.unlock();
}

std::shared_ptr<AnimationValue> AnimatableReal::getActualValue(){
	auto ret = AnimationValue::makeReal();
	mutex.lock();
	ret->value = actualValue->value;
	mutex.unlock();
	return ret;
}
std::shared_ptr<AnimationValue> AnimatableReal::getTargetValue(){
	auto ret = AnimationValue::makeReal();
	mutex.lock();
	ret->value = targetValue->value;
	mutex.unlock();
	return ret;
}
	


bool AnimatableReal::generateTargetAnimation(std::shared_ptr<TargetAnimation> animation){
    
    double target = parameterValueToAnimationValue(animation->target)->toReal()->value;
    
    auto startPoint = std::make_shared<Motion::ControlPoint>();
    startPoint->time = 0.0;
    startPoint->position = targetValue->value;
    startPoint->velocity = 0.0;
    startPoint->outAcceleration = 0.0;
    startPoint->b_valid = true;
    
    auto endPoint = std::make_shared<Motion::ControlPoint>();
    endPoint->time = 0.0;
    endPoint->position = target;
    endPoint->velocity = 0.0;
    endPoint->inAcceleration = 0.0;
    endPoint->b_valid = true;
    
    std::shared_ptr<Motion::StepInterpolation> interpolation = Motion::StepInterpolation::getInterpolation(startPoint, endPoint);
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
bool AnimatableReal::validateAnimation(std::shared_ptr<Animation> animation){
	return true;
}

bool AnimatableReal::isReadyToMove() {
	return true;
}
bool AnimatableReal::isReadyToStartPlaybackFromValue(std::shared_ptr<AnimationValue> animationValue) {
	return true;
}

bool AnimatableReal::isInRapid(){
	return false;
}
float AnimatableReal::getRapidProgress(){
	return 0.0;
}
void AnimatableReal::cancelRapid(){}
std::shared_ptr<AnimationValue> AnimatableReal::getRapidTarget(){
	return AnimationValue::makeReal();
}

void AnimatableReal::stopMovement() {}


void AnimatableReal::onRapidToValue(std::shared_ptr<AnimationValue> animationValue) {
    mutex.lock();
    targetValue = animationValue->toReal();
    mutex.unlock();
}
void AnimatableReal::onPlaybackStart(std::shared_ptr<Animation> animation) {
    mutex.lock();
    currentAnimation = animation;
    mutex.unlock();
}
void AnimatableReal::onPlaybackPause() {
    mutex.lock();
    currentAnimation = nullptr;
    mutex.unlock();
}
void AnimatableReal::onPlaybackStop() {
    mutex.lock();
    currentAnimation = nullptr;
    mutex.unlock();
}
void AnimatableReal::onPlaybackEnd() {}




bool AnimatableReal::hasManualControls() {
	return false;
}
void AnimatableReal::onSetManualControlTarget(float x, float y, float z) {}



std::vector<InterpolationType>& AnimatableReal::getCompatibleInterpolationTypes() {
	static std::vector<InterpolationType> compatibleInterpolationTypes = { InterpolationType::STEP };
	return compatibleInterpolationTypes;
}
int AnimatableReal::getCurveCount() {
	return 1;
}
std::shared_ptr<Parameter> AnimatableReal::makeParameter() {
	return NumberParameter<double>::make(0.0, "Parameter", "Parameter", "%.3f", getUnit(), true, 0, 0, "", suffix.c_str());
}
void AnimatableReal::setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value) {
	std::static_pointer_cast<NumberParameter<double>>(parameter)->overwrite(value->toReal()->value);
}
void AnimatableReal::copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to) {
	std::static_pointer_cast<NumberParameter<double>>(to)->overwrite(std::static_pointer_cast<NumberParameter<double>>(from)->value);
}
std::shared_ptr<AnimationValue> AnimatableReal::parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter) {
	auto output = AnimationValue::makeReal();
	output->value = std::static_pointer_cast<NumberParameter<double>>(parameter)->value;
	return output;
}
bool AnimatableReal::isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2) {
	return value1->toReal()->value == value2->toReal()->value;
}
std::shared_ptr<AnimationValue> AnimatableReal::getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds) {
	auto output = AnimationValue::makeReal();
	Motion::Point point = animation->getCurves().front()->getPointAtTime(time_seconds);
	output->value = point.position;
	return output;
}
std::vector<double> AnimatableReal::getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value) {
	return std::vector<double>{value->toReal()->value};
}
bool AnimatableReal::isControlledManuallyOrByAnimation() {
	return false;
}

bool AnimatableReal::isMoving(){
	return false;
}
*/
