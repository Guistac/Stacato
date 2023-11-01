#include "AxisMapping.h"

bool AxisMapping::save(tinyxml2::XMLElement* parent){
	parent->SetAttribute("AxisPinSaveName", axisPin->saveString);
	parent->SetAttribute("InvertDirection", b_invertDirection);
	parent->SetAttribute("PositionOffset", userPositionOffset);
	parent->SetAttribute("UserLowerPositionLimit", userLowerPositionLimit);
	parent->SetAttribute("UserUpperPositionLimit", userUpperPositionLimit);
	parent->SetAttribute("MinimumLoad", minimumLoad_Kilograms);
	return true;
}

bool AxisMapping::load(tinyxml2::XMLElement* parent){
	const char* pinSaveString;
	parent->QueryAttribute("AxisPinSaveName", &pinSaveString);
	strcpy(axisPin->saveString, pinSaveString);
	parent->QueryAttribute("InvertDirection", &b_invertDirection);
	parent->QueryAttribute("PositionOffset", &userPositionOffset);
	parent->QueryAttribute("UserLowerPositionLimit", &userLowerPositionLimit);
	parent->QueryAttribute("UserUpperPositionLimit", &userUpperPositionLimit);
	parent->QueryAttribute("MinimumLoad", &minimumLoad_Kilograms);
	return true;
}

bool AxisMapping::isAxisConnected(){
	return axisPin->isConnected();
}

std::shared_ptr<AxisInterface> AxisMapping::getAxis(){
	if(!axisPin->isConnected()) return nullptr;
	return axisPin->getConnectedPin()->getSharedPointer<AxisInterface>();
}

void AxisMapping::enableAxis(){
	if(axisPin->isConnected()) getAxis()->enable();
}
void AxisMapping::disableAxis(){
	if(axisPin->isConnected()) getAxis()->disable();
}

void AxisMapping::updateAnimatableParameters(){
	if(!isAxisConnected()) {
		animatablePosition->setUnit(Units::None::None);
		animatablePosition->lowerPositionLimit = 0.0;
		animatablePosition->upperPositionLimit = 0.0;
		animatablePosition->velocityLimit = 0.0;
		animatablePosition->accelerationLimit = 0.0;
		return;
	}
	auto axis = getAxis();
	
	axisLowerPositionLimit = axisPositionToMachinePosition(axis->getLowerPositionLimit());
	axisUpperPositionLimit = axisPositionToMachinePosition(axis->getUpperPositionLimit());
	if(b_invertDirection) std::swap(axisLowerPositionLimit, axisUpperPositionLimit);
	
	setUserLowerLimit(userLowerPositionLimit);
	setUserUpperLimit(userUpperPositionLimit);
	
	animatablePosition->setUnit(axis->getPositionUnit());
	animatablePosition->lowerPositionLimit = userLowerPositionLimit;
	animatablePosition->upperPositionLimit = userUpperPositionLimit;
	animatablePosition->velocityLimit = std::abs(axis->getVelocityLimit());
	animatablePosition->accelerationLimit = std::abs(axis->getAccelerationLimit());
}

void AxisMapping::updateRealAnimatableValues(){
	//how does this work with an axis that is homing ? values should be zero ??
	if(!isAxisConnected()) return;
	auto axis = getAxis();
	
	if(b_userZeroUpdateRequest){
		b_userZeroUpdateRequest = false;
		userPositionOffset = axis->getPositionActual();
		animatablePosition->overridePositionTarget(0.0);
		axisLowerPositionLimit = axisPositionToMachinePosition(axis->getLowerPositionLimit());
		axisUpperPositionLimit = axisPositionToMachinePosition(axis->getUpperPositionLimit());
		if(b_invertDirection) std::swap(axisLowerPositionLimit, axisUpperPositionLimit);
		setUserLowerLimit(userLowerPositionLimit);
		setUserUpperLimit(userUpperPositionLimit);
	}
	
	auto actualPosition = AnimationValue::makePosition();
	actualPosition->position = axisPositionToMachinePosition(axis->getPositionActual());
	actualPosition->velocity = axisVelocityToMachineVelocity(axis->getVelocityActual());
	actualPosition->acceleration = axisAccelerationToMachineAcceleration(0.0);
	animatablePosition->updateActualValue(actualPosition);
}

void AxisMapping::updateAxisCommand(double profileTime_seconds, double profileDeltaTime_seconds){
	if(!isAxisConnected()) animatablePosition->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
	
	auto axis = getAxis();
	
	if(!axis->isEnabled() || axis->isHoming()){
		//if the axis is not enabled or is homing, the animatable doesn't do anything
		animatablePosition->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
	}
	else{
		animatablePosition->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
		auto target = animatablePosition->getTargetValue()->toPosition();
		double axisPositionTarget = machinePositionToAxisPosition(target->position);
		double axisVelocityTarget = machineVelocityToAxisVelocity(target->velocity);
		double axisAccelerationTarget = machineAccelerationToAxisAcceleration(target->acceleration);
		axis->setPositionTarget(axisPositionTarget, axisVelocityTarget, axisAccelerationTarget);
	}
	
}


void AxisMapping::setUserLowerLimit(double lowerLimit){
	userLowerPositionLimit = std::clamp(lowerLimit, axisLowerPositionLimit, userUpperPositionLimit);
	animatablePosition->lowerPositionLimit = userLowerPositionLimit;
}

void AxisMapping::setUserUpperLimit(double upperLimit){
	userUpperPositionLimit = std::clamp(upperLimit, userLowerPositionLimit, axisUpperPositionLimit);
	animatablePosition->upperPositionLimit = userUpperPositionLimit;
}

void AxisMapping::captureLowerUserLimit(){
	userLowerPositionLimit = animatablePosition->getActualPosition();
	setUserLowerLimit(userLowerPositionLimit);
}

void AxisMapping::captureUpperUserLimit(){
	userUpperPositionLimit = animatablePosition->getActualPosition();
	setUserUpperLimit(userUpperPositionLimit);
}

void AxisMapping::resetLowerUserLimit(){
	setUserLowerLimit(axisLowerPositionLimit);
}

void AxisMapping::resetUpperUserLimit(){
	setUserUpperLimit(axisUpperPositionLimit);
}

void AxisMapping::captureUserZero(){
	requestedUserZeroOffset = getAxis()->getPositionActual();
	b_userZeroUpdateRequest = true;
}

void AxisMapping::resetUserZero(){
	requestedUserZeroOffset = 0.0;
	b_userZeroUpdateRequest = true;
}


