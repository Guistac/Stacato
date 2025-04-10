#include "MecanumMachine.h"

#include <tinyxml2.h>
#include "Fieldbus/EtherCatFieldbus.h"
#include "Motion/Interfaces.h"


MecanumMachine::AxisMapping::AxisMapping(std::string name, std::string saveName){
	axisPin = std::make_shared<NodePin>(NodePin::DataType::AXIS_INTERFACE, NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, name.c_str(), saveName.c_str());
}
bool MecanumMachine::AxisMapping::save(tinyxml2::XMLElement* xml){
	tinyxml2::XMLElement* mappingXML = xml->InsertNewChildElement(axisPin->saveString);
	wheelPosition->save(mappingXML);
	wheelType->save(mappingXML);
	invertAxis->save(mappingXML);
	wheelDiameter->save(mappingXML);
	return true;
}
bool MecanumMachine::AxisMapping::load(tinyxml2::XMLElement* xml){
	if(tinyxml2::XMLElement* mappingXML = xml->FirstChildElement(axisPin->saveString)){
		wheelPosition->load(mappingXML);
		wheelType->load(mappingXML);
		invertAxis->load(mappingXML);
		wheelDiameter->load(mappingXML);
	}
	return true;
}

void MecanumMachine::updateFrictionVectors(){
	for(int i = 0; i < 4; i++){
		auto mapping = axisMappings[i];
		mapping->wheelCircumference = M_PI * mapping->wheelDiameter->value;
		float frictionVectorUnit = std::sin(M_PI_4) * mapping->wheelCircumference;
		if(mapping->wheelType->value == mapping->wheelType_A.getInt()){
			mapping->frictionVector = glm::vec2(frictionVectorUnit, frictionVectorUnit);
		}
		else if(mapping->wheelType->value == mapping->wheelType_B.getInt()){
			mapping->frictionVector = glm::vec2(-frictionVectorUnit, frictionVectorUnit);
		}
		if(mapping->invertAxis->value) {
			mapping->frictionVector *= glm::vec2(-1.0, -1.0);
		}
	}
}


void MecanumMachine::initialize() {
	
	widget = std::make_shared<MecanumWidget>(std::dynamic_pointer_cast<MecanumMachine>(shared_from_this()));
	
	axisMappings.resize(4);
	for(int i = 0; i < 4; i++){
		std::string name = "Axis " + std::to_string(i+1);
		switch(i){
			case 0: name = "FL"; break;
			case 1: name = "FR"; break;
			case 2: name = "BL"; break;
			case 3: name = "BR"; break;
			default: name = "??"; break;
		}
		std::string saveName = "Axis" + std::to_string(i+1);
		auto mapping = std::make_shared<AxisMapping>(name, saveName);
		addNodePin(mapping->axisPin);
		axisMappings[i] = mapping;
		mapping->wheelDiameter->overwrite(203.0);
		mapping->wheelDiameter->setFormat("%.1fmm");
		mapping->wheelPosition->setFormat("%.1fmm");
		mapping->wheelDiameter->addEditCallback([this](){ updateFrictionVectors(); });
		mapping->wheelType->addEditCallback([this](){ updateFrictionVectors(); });
		mapping->invertAxis->addEditCallback([this](){ updateFrictionVectors(); });
		switch(i){
			case 0:
				//FL
				mapping->invertAxis->overwrite(false);
				mapping->wheelType->overwrite(&mapping->wheelType_A);
				mapping->wheelPosition->overwrite(glm::vec2(-500.0, 500.0));
				break;
			case 1:
				//FR
				mapping->invertAxis->overwrite(true);
				mapping->wheelType->overwrite(&mapping->wheelType_B);
				mapping->wheelPosition->overwrite(glm::vec2(500.0, 500.0));
				break;
			case 2:
				//BL
				mapping->invertAxis->overwrite(false);
				mapping->wheelType->overwrite(&mapping->wheelType_B);
				mapping->wheelPosition->overwrite(glm::vec2(-500.0, -500.0));
				break;
			case 3:
				//BR
				mapping->invertAxis->overwrite(true);
				mapping->wheelType->overwrite(&mapping->wheelType_A);
				mapping->wheelPosition->overwrite(glm::vec2(500.0, -500.0));
				break;
			default:
				break;
		}
	}
	updateFrictionVectors();
	
	
	
	addNodePin(enablePin);
	addNodePin(speedModePin);
	addNodePin(moveModePin);
	addNodePin(brakeFeedbackPin);
	addNodePin(brakeOverrideFeedbackPin);
	addNodePin(joystickXpin);
	addNodePin(joystickYpin);
	addNodePin(joystickRpin);
	
	addNodePin(enabledFeedbackPin);
	addNodePin(errorLedPin);
	addNodePin(enabledLedPin);
	addNodePin(velocityModeFeedbackPin);
	addNodePin(moveModeFeedbackPin);
	addNodePin(brakeControlPin);
	
	//initialize this to be far in the future because else it will trigger when starting the fieldbus
	disableTime = -10'000'000'000;
}



void MecanumMachine::onPinUpdate(std::shared_ptr<NodePin> pin){}
void MecanumMachine::onPinConnection(std::shared_ptr<NodePin> pin){
	for(int i = 0; i < 4; i++){
		auto mapping = axisMappings[i];
		if(mapping->axisPin->isConnected()) mapping->axis = mapping->axisPin->getConnectedPin()->getSharedPointer<AxisInterface>();
		else mapping->axis = nullptr;
	}
}
void MecanumMachine::onPinDisconnection(std::shared_ptr<NodePin> pin){}
bool MecanumMachine::isHardwareReady() { return state == DeviceState::READY || state == DeviceState::ENABLING || state == DeviceState::ENABLED; }
void MecanumMachine::enableHardware() { b_enableRequest = true; }
void MecanumMachine::disableHardware() { b_disableRequest = true; }

void MecanumMachine::inputProcess(){
	
	
	
	if(enablePin->isConnected()) enablePin->copyConnectedPinValue();
	else *enableControl = false;
	if(speedModePin->isConnected()) speedModePin->copyConnectedPinValue();
	else *speedModeControl = false;
	if(moveModePin->isConnected()) moveModePin->copyConnectedPinValue();
	else *moveModeControl = false;
	if(brakeFeedbackPin->isConnected()) brakeFeedbackPin->copyConnectedPinValue();
	else *brakeFeedback = false;
	if(brakeOverrideFeedbackPin->isConnected()) brakeOverrideFeedbackPin->copyConnectedPinValue();
	else *brakeOverrideFeedback = false;
 	if(joystickXpin->isConnected()) joystickXpin->copyConnectedPinValue();
	else *joystickXvalue = 0.0;
	if(joystickYpin->isConnected()) joystickYpin->copyConnectedPinValue();
	else *joystickYvalue = 0.0;
	if(joystickRpin->isConnected()) joystickRpin->copyConnectedPinValue();
	else *joystickRvalue = 0.0;
	

	b_brakesOpened = *brakeFeedback;
	b_brakeOverride = *brakeOverrideFeedback;
	
	bool b_previousMoveMode = b_absoluteMoveMode;
	if(b_localControl) {
		b_highSpeedMode = b_localControlSpeedMode;
		b_absoluteMoveMode = b_localControlMoveMode;
	}
	else{
		b_highSpeedMode = *speedModeControl;
		b_absoluteMoveMode = *moveModeControl;
	}
	if(!b_previousMoveMode && b_absoluteMoveMode){
		estimatedHeading_degrees = 0.0;
		estimatedPosition_absolute.x = 0.0;
		estimatedPosition_absolute.y = 0.0;
		positionHistoryMutex.lock();
		positionHistory.clear();
		positionHistoryMutex.unlock();
		Logger::info("[{}] Reset Heading", getName());
	}
	
	translationVelocityLimitCurrent = b_highSpeedMode ? linearVelocityLimit->value : linearVelocityLimit->value * lowSpeed_userAdjust / 100.0;
	rotationVelocityLimitCurrent = b_highSpeedMode ? angularVelocityLimit->value : angularVelocityLimit->value * lowSpeed_userAdjust / 100.0;
	translationAccelerationCurrent = linearAccelerationLimit->value * globalAcceleration_userAdjust / 100.0;
	rotationAccelerationCurrent = angularAccelerationLimit->value * globalAcceleration_userAdjust / 100.0;

	b_emergencyStopActive = areAllAxisOnline() && !areAllAxisEstopFree();
	
	if(isEnabled() && b_brakeOverride){
		Logger::warn("[{}] Machine disable because brakes were overridden", getName());
		disableTime = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
		disableAllAxis();
	}
	
	if(areAllAxisEnabled() && b_brakesOpened && !b_brakeOverride){
		if(state == DeviceState::ENABLING){
			state = DeviceState::ENABLED;
			enableTime = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
			Logger::info("[{}] Machine enabled", getName());
		}
		//
		if(b_disableRequest){
			disableAllAxis();
			disableTime = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
			state = DeviceState::DISABLING;
			Logger::debug("[{}] Disable Request", getName());
		}
		else state = DeviceState::ENABLED;
	}
	else if(areAllAxisReady() && !b_brakeOverride){
		if(state == DeviceState::DISABLING){
			Logger::info("[{}] Machine Disabled : user request", getName());
		}
		else if(state == DeviceState::ENABLED){
			Logger::warn("[{}] Machine disabled : one or more axis were disabled", getName());
			disableTime = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
		}
		//
		if(state != DeviceState::ENABLING && (*enableControl || b_enableRequest)){
			enableRequestTime = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
			enableAllAxis();
			state = DeviceState::ENABLING;
			Logger::debug("[{}] Enable Request", getName());
		}
		else if(state == DeviceState::ENABLING){
			if(EtherCatFieldbus::getCycleProgramTime_nanoseconds() - enableRequestTime > enableTimeout->value * 1'000'000'000){
				Logger::warn("[{}] Machine enable timeout", getName());
				if(!b_brakesOpened) Logger::debug("[{}] Brakes were not opened on time", getName());
				if(!areAllAxisEnabled()) Logger::debug("[{}] One or more axis were not enabled on time", getName());
				disableAllAxis();
				state = DeviceState::READY;
			}
			else state = DeviceState::ENABLING;
		}
		else state = DeviceState::READY;
	}
	else if(areAllAxisOnline()){
		if(state == DeviceState::ENABLED || state == DeviceState::ENABLING){
			if(b_emergencyStopActive) Logger::warn("[{}] Machine disable : Estop Triggered", getName());
			else Logger::warn("[{}] Machine disabled because one or more axis were not ready", getName());
			disableTime = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
		}
		//
		state = DeviceState::NOT_READY;
	}
	else{
		if(state == DeviceState::ENABLED || state == DeviceState::ENABLING){
			if(!areAllAxisOnline()){
				Logger::warn("[{}] Machine disabled because one or more axis were offline", getName());
				disableTime = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
			}
		}
		//
		state = DeviceState::OFFLINE;
	}
	b_enableRequest = false;
	b_disableRequest = false;
	
	bool b_closeBrakes = state != DeviceState::ENABLED && state != DeviceState::ENABLING && EtherCatFieldbus::getCycleProgramTime_nanoseconds() - disableTime > brakeApplyTime->value * 1'000'000'000;
	*brakeControl = !b_closeBrakes;
	
	if(b_emergencyStopActive) *errorLed = true;
	else if(!areAllAxisErrorFree()) *errorLed = Timing::getBlink(0.5);
	else if(state == DeviceState::NOT_READY) *errorLed = Timing::getBlink(.2);
	else *errorLed = false;
	
	if(isEnabled()) *enabledLed = true;
	else if(isReady()) *enabledLed = Timing::getBlink(0.5);
	else *enabledLed = false;
	
	*velocityModeFeedback = b_highSpeedMode;
	*moveModeFeedback = b_absoluteMoveMode;
	*enabledFeedback = state == DeviceState::ENABLED;
	
}

void MecanumMachine::outputProcess(){
	
	if(state == DeviceState::ENABLED){
		double dT = EtherCatFieldbus::getCycleTimeDelta_seconds();
		
		double targetVelocity_X;
		double targetVelocity_Y;
		double targetVelocity_R;
		
		if(b_localControl){
			targetVelocity_X = std::clamp(localControl_X, -1.0f, 1.0f) * translationVelocityLimitCurrent;
			targetVelocity_Y = std::clamp(localControl_Y, -1.0f, 1.0f) * translationVelocityLimitCurrent;
			targetVelocity_R = std::clamp(localControl_R, -1.0f, 1.0f) * rotationVelocityLimitCurrent;
		}
		else{
			targetVelocity_X = std::clamp(*joystickXvalue, -1.0, 1.0) * translationVelocityLimitCurrent;
			targetVelocity_Y = std::clamp(*joystickYvalue, -1.0, 1.0) * translationVelocityLimitCurrent;
			targetVelocity_R = std::clamp(*joystickRvalue, -1.0, 1.0) * rotationVelocityLimitCurrent;
		}
		
		//limit the translation vector magnitude to the max translation velocity
		//this effectively transforms the xy square of the left joystick into a circle
		double translationTargetVectorMagnitude = std::sqrt(std::pow(targetVelocity_X, 2.0) + std::pow(targetVelocity_Y, 2.0));
		if(translationTargetVectorMagnitude != 0.0){
			targetVelocity_X /= translationTargetVectorMagnitude;
			targetVelocity_Y /= translationTargetVectorMagnitude;
			translationTargetVectorMagnitude = std::clamp(translationTargetVectorMagnitude, 0.0, translationVelocityLimitCurrent);
			targetVelocity_X *= translationTargetVectorMagnitude;
			targetVelocity_Y *= translationTargetVectorMagnitude;
		}
		
		xProfile.matchVelocity(dT, targetVelocity_X, translationAccelerationCurrent);
		yProfile.matchVelocity(dT, targetVelocity_Y, translationAccelerationCurrent);
		rProfile.matchVelocity(dT, targetVelocity_R, rotationAccelerationCurrent);
	}
	else{
		xProfile.setVelocity(0.0);
		yProfile.setVelocity(0.0);
		rProfile.setVelocity(0.0);
	}
	
	
	//calculate odometry for the heading by integrating angular velocity
	double previousAngularVelocity = angularVelocity;
	angularVelocity = rProfile.getVelocity();
	double averageAngularVelocity = (previousAngularVelocity + angularVelocity) / 2.0;
	double angularPositionDelta = averageAngularVelocity * EtherCatFieldbus::getCycleTimeDelta_seconds();
	estimatedHeading_degrees += angularPositionDelta * headingCorrectFactor->value / 100.0;

	glm::vec2 translationVelocityRequest(xProfile.getVelocity(), yProfile.getVelocity());
	
	auto compensateForHeading = [](glm::vec2 translationVelocity, double headingDegrees) -> glm::vec2 {
		//————————————————————————————————————————————————————————————————————————————————————————————————————————
		//in compass mode we use the current heading to correct the angle of the translation vector
		//this way we can keep moving in a fixed direction
		
		//get info about current requested velocity
		//front is 0, right is PI/2, left is -PI/2
		//the value needs to be inverted to match the real movement
		double requestedVelocityHeading_radians = -atan2(translationVelocity.x, translationVelocity.y);
		double requestedVelocityMagnitude_mmPerS = sqrt(std::pow(translationVelocity.x, 2.0) + std::pow(translationVelocity.y, 2.0));

		//get the current heading in normalized radians
		double currentHeading_radians = M_PI * 2.0 * headingDegrees / 360.0;
		while(currentHeading_radians > M_PI) currentHeading_radians -= 2.0 * M_PI;
		while(currentHeading_radians < -M_PI) currentHeading_radians += 2.0 * M_PI;

		//we need to subtract the current heading angle from the requested velocity vector to keep moving in the same direction as the heading
		//at the same time subtract 90° to match the joystick angle
		double correctedVelocityHeading_radians = requestedVelocityHeading_radians + currentHeading_radians + (M_PI / 2.0);
		while(correctedVelocityHeading_radians < -M_PI) correctedVelocityHeading_radians += 2.0 * M_PI;
		while(correctedVelocityHeading_radians > M_PI) correctedVelocityHeading_radians -= 2.0 * M_PI;

		//get unit vector with the correct heading and set its magnitude to equal the original requested velocity
		glm::vec2 translationVelocity_HeadingAdjusted(cos(correctedVelocityHeading_radians), sin(correctedVelocityHeading_radians));
		translationVelocity_HeadingAdjusted.x *= requestedVelocityMagnitude_mmPerS;
		translationVelocity_HeadingAdjusted.y *= requestedVelocityMagnitude_mmPerS;

		return translationVelocity_HeadingAdjusted;
		//————————————————————————————————————————————————————————————————————————————————————————————————————————
	};
	
	glm::vec2 previousTranslationVelocity_absolute = translationVelocity_absolute;
	glm::vec2 previousTranslationVelocity_relative = translationVelocity_relative;
	
	if(b_absoluteMoveMode) {
		translationVelocity_relative = compensateForHeading(translationVelocityRequest, estimatedHeading_degrees);
		translationVelocity_absolute = translationVelocityRequest;
	}
	else {
		translationVelocity_relative = translationVelocityRequest;
		translationVelocity_absolute = compensateForHeading(translationVelocityRequest, -estimatedHeading_degrees);
	}
	
	glm::vec2 averageTranslationVelocity_absolute = (previousTranslationVelocity_absolute + translationVelocity_absolute) / 2.0;
	glm::vec2 translationPositionDelta_absolute = averageTranslationVelocity_absolute * EtherCatFieldbus::getCycleTimeDelta_seconds();
	estimatedPosition_absolute += translationPositionDelta_absolute;
	
	if(EtherCatFieldbus::getCycleProgramTime_nanoseconds() - lastPointTime > 10'0000'000){
		lastPointTime = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
		positionHistoryMutex.lock();
		positionHistory.push(estimatedPosition_absolute);
		positionHistoryMutex.unlock();
	}

	 //calculate target velocity for each wheel
	 for(int i = 0; i < 4; i++){
		 
		 glm::vec2 wheelFrictionVector = axisMappings[i]->frictionVector;
		 glm::vec2 wheelPosition = axisMappings[i]->wheelPosition->value;
		 
		 //————————————X & Y component velocity————————————
		 double wheelVelocity = translationVelocity_relative.x / wheelFrictionVector.x + translationVelocity_relative.y / wheelFrictionVector.y;

		 //———————Rotational Component velocity————————
		 float rotationRadius = std::sqrt(std::pow(wheelPosition.x, 2.0) + std::pow(wheelPosition.y, 2.0));
		 float rotationCirclePerimeter = 2.0 * M_PI * rotationRadius;
		 float rotationVectorMagnitude = rotationCirclePerimeter * angularVelocity / 360.0;

		 //vector perpendicular to radius of wheel position around rotation center vector
		 glm::vec2 wheelRotationVector;
		 wheelRotationVector.x = -wheelPosition.y;
		 wheelRotationVector.y = wheelPosition.x;

		 //normalize the perpendicular vector
		 double normalisationMagnitude = std::sqrt(std::pow(wheelRotationVector.x, 2.0) + std::pow(wheelRotationVector.y, 2.0));
		 if(normalisationMagnitude != 0.0){
			 wheelRotationVector.x /= normalisationMagnitude;
			 wheelRotationVector.y /= normalisationMagnitude;
			 //set the rotation vector magnitude to the rotation speed
			 wheelRotationVector.x *= rotationVectorMagnitude;
			 wheelRotationVector.y *= rotationVectorMagnitude;
		 }else{
			 wheelRotationVector.x = 0.0;
			 wheelRotationVector.y = 0.0;
		 }

		 //decompose the rotation vector and add to wheel velocity
		 //Testing: inverting because trigonometric positive is anticlockwise
		 wheelVelocity -= wheelRotationVector.x / wheelFrictionVector.x;
		 wheelVelocity -= wheelRotationVector.y / wheelFrictionVector.y;
		 
		 if(axisMappings[i]->axis) axisMappings[i]->axis->setVelocityTarget(wheelVelocity, 100000.0); //acceleration value does not matter here
	 }
	
}

bool MecanumMachine::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	enableTimeout->save(xml);
	brakeApplyTime->save(xml);
	headingCorrectFactor->save(xml);
	linearVelocityLimit->save(xml);
	angularVelocityLimit->save(xml);
	linearAccelerationLimit->save(xml);
	angularAccelerationLimit->save(xml);
	for(int i = 0; i < 4; i++) axisMappings[i]->save(xml);
	
	xml->SetAttribute("UserLowSpeedScaling", lowSpeed_userAdjust);
	xml->SetAttribute("UserAccelerationScaling", globalAcceleration_userAdjust);
	xml->SetAttribute("WidgetID", widget->uniqueID);
	
	return true;
}

bool MecanumMachine::loadMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	enableTimeout->load(xml);
	brakeApplyTime->load(xml);
	headingCorrectFactor->load(xml);
	linearVelocityLimit->load(xml);
	angularVelocityLimit->load(xml);
	linearAccelerationLimit->load(xml);
	angularAccelerationLimit->load(xml);
	for(int i = 0; i < 4; i++) axisMappings[i]->load(xml);
	xml->QueryFloatAttribute("UserLowSpeedScaling", &lowSpeed_userAdjust);
	xml->QueryFloatAttribute("UserAccelerationScaling", &globalAcceleration_userAdjust);
	
	updateFrictionVectors();
	
	xml->QueryAttribute("WidgetID", &widget->uniqueID);
	
	return true;
}


void MecanumMachine::enableAllAxis(){
	for(auto mapping : axisMappings) {
		if(mapping->axis) mapping->axis->enable();
	}
}

void MecanumMachine::disableAllAxis(){
	for(auto mapping : axisMappings) {
		if(mapping->axis) mapping->axis->disable();
	}
}

bool MecanumMachine::areAllAxisEnabled(){
	for(auto mapping : axisMappings) {
		if(mapping->axis == nullptr) return false;
		else if (!mapping->axis->isEnabled()) return false;
	}
	return true;
}

bool MecanumMachine::areAllAxisReady(){
	for(auto mapping : axisMappings) {
		if(mapping->axis == nullptr) return false;
		else if(!mapping->axis->isReady()) return false;
	}
	return true;
}
bool MecanumMachine::areAllAxisOnline(){
	for(auto mapping : axisMappings) {
		if(mapping->axis == nullptr) return false;
		else if(!mapping->axis->isOnline()) return false;
	}
	return true;
}
bool MecanumMachine::areAllAxisErrorFree(){
	for(auto mapping : axisMappings) {
		if(mapping->axis == nullptr) return false;
		else if (mapping->axis->hasFault()) return false;
	}
	return true;
}

bool MecanumMachine::areAllAxisEstopFree(){
	for(auto mapping : axisMappings) {
		if(mapping->axis == nullptr) return false;
		else if (mapping->axis->isEmergencyStopActive()) return false;
	}
	return true;
}




bool MecanumMachine::isMoving() {}
void MecanumMachine::onEnableHardware() {}
void MecanumMachine::onDisableHardware() {}
bool MecanumMachine::isSimulationReady(){return true;}
void MecanumMachine::onEnableSimulation() {}
void MecanumMachine::onDisableSimulation() {}
std::string MecanumMachine::getStatusString(){return "";}
void MecanumMachine::simulateInputProcess() {}
void MecanumMachine::simulateOutputProcess(){}
void MecanumMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {}
void MecanumMachine::fillAnimationDefaults(std::shared_ptr<Animation> anim){}
