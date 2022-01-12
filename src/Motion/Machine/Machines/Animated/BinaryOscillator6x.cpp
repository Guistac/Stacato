#include <pch.h>

#include "BinaryOscillator6x.h"

#include <pch.h>

#include "Motion/Subdevice.h"
#include "Motion/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"

#include "Motion/Axis/PositionControlledAxis.h"
#include "Fieldbus/EtherCatFieldbus.h"

#include <tinyxml2.h>

void BinaryOscillator6x::assignIoData() {

	addIoData(gpioDevicePin);
	for (auto& outputPin : outputPins) addIoData(outputPin);

	addAnimatableParameter(oscillatorParameterGroup);
}

void BinaryOscillator6x::process() {

	if (b_enabled) {
		if (!isReady()) disable();
	}

	if (b_startOscillator && !b_oscillatorActive) {
		Logger::warn("START OSC");
		updateOscillatorParametersFromTracks();
		b_startOscillator = false;
		b_oscillatorActive = true;
		b_stopOscillator = false;
		//when starting the oscillator
		//turn all signals off
		//plan next switch on time starting from 0 delay
		for (int i = 0; i < 6; i++) {
			setOutput(i, false);
			currentStateLengths_seconds[i] = 0.0;
			nextStateChangeDelay_seconds[i] = Random::getRanged(0.0, maxOffTime_seconds);
		}
	}
	
	if (b_stopOscillator && b_oscillatorActive) {
		b_stopOscillator = false;
		b_oscillatorActive = false;
		for (int i = 0; i < 6; i++) {
			setOutput(i, false);
		}
		Logger::warn("Stop OSC");
	}

	double profileDeltaTime_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();

	if (b_oscillatorActive) {
		updateOscillatorParametersFromTracks();
		for (int i = 0; i < 6; i++) {
			//increment the length of the current signal state
			currentStateLengths_seconds[i] += profileDeltaTime_seconds;
			//if the length exceeded the planned length
			if (currentStateLengths_seconds[i] > nextStateChangeDelay_seconds[i]) {
				//flip the state of the output
				outputSignals[i] = !outputSignals[i];
				currentStateLengths_seconds[i] = 0.0;
				//plan the next state change
				if (outputSignals[i]) {
					//if the signal was turned on
					//get the length of the on time
					nextStateChangeDelay_seconds[i] = Random::getRanged(minOnTime_seconds, maxOnTime_seconds);
				}
				else {
					//if the signal was turned off
					//get the length of the off time
					nextStateChangeDelay_seconds[i] = Random::getRanged(minOffTime_seconds, maxOffTime_seconds);
				}
			}
			outputPins[i]->set(outputSignals[i]);
		}
	}
    
    if(!b_enabled){
        for(int i = 0; i < 6; i++){
            setOutput(i, false);
        }
    }
}

void BinaryOscillator6x::updateOscillatorParametersFromTracks() {
	AnimatableParameterValue value;
	if (minOffTimeParameter->hasParameterTrack()) {
		minOffTimeParameter->getActiveTrackParameterValue(value);
		minOffTime_seconds = value.realValue;
	}
	if (maxOffTimeParameter->hasParameterTrack()) {
		maxOffTimeParameter->getActiveTrackParameterValue(value);
		maxOffTime_seconds = value.realValue;
	}
	if (minOnTimeParameter->hasParameterTrack()) {
		minOnTimeParameter->getActiveTrackParameterValue(value);
		minOnTime_seconds = value.realValue;
	}
	if (maxOnTimeParameter->hasParameterTrack()) {
		maxOnTimeParameter->getActiveTrackParameterValue(value);
		maxOnTime_seconds = value.realValue;
	}
}

void BinaryOscillator6x::stopOscillatorParameterPlayback() {
	if (minOffTimeParameter->hasParameterTrack()) interruptParameterPlayback(minOffTimeParameter);
	if (maxOffTimeParameter->hasParameterTrack()) interruptParameterPlayback(maxOffTimeParameter);
	if (minOnTimeParameter->hasParameterTrack()) interruptParameterPlayback(minOnTimeParameter);
	if (maxOnTimeParameter->hasParameterTrack()) interruptParameterPlayback(maxOnTimeParameter);
	if (oscillatorParameterGroup->hasParameterTrack()) interruptParameterPlayback(oscillatorParameterGroup);
}

void BinaryOscillator6x::setOutput(int i, bool s) {
	outputSignals[i] = s;
	outputPins[i]->set(s);
}

void BinaryOscillator6x::manuallySetOutput(int i, bool s) {
	stopOscillator();
	setOutput(i, s);
}

//======================= STATE CONTROL ========================

bool BinaryOscillator6x::isHardwareReady() {
	//machine is ready if at least one gpio device is ready
	for (int i = 0; i < getGpioDeviceCount(); i++) {
		if (getGpioDevice(i)->isReady()) return true;
	}
	return false;
}

void BinaryOscillator6x::enableHardware() {
	if (isReady()) {
		b_enabled = true;
		stopOscillatorParameterPlayback();
		onEnableHardware();
	}
}

void BinaryOscillator6x::disableHardware() {
	b_stopOscillator = true;
	b_enabled = false;
	for (int i = 0; i < 6; i++) {
		setOutput(i, false);
	}
	stopOscillatorParameterPlayback();
	onDisableHardware();
}

bool BinaryOscillator6x::isMoving() {
	for (int i = 0; i < 6; i++) {
		if (outputSignals[i]) return true;
	}
	return false;
}

//======================== OSCILLATOR CONTROL ============================

void BinaryOscillator6x::startOscillator() {
	b_startOscillator = true;
}

void BinaryOscillator6x::stopOscillator() {
	b_stopOscillator = true;
}

bool BinaryOscillator6x::isOscillatorActive() {
	return b_oscillatorActive;
}



//=========================== AXES & DEVICES ==============================

int BinaryOscillator6x::getGpioDeviceCount() {
	if (gpioDevicePin->isConnected()) return gpioDevicePin->getConnectedPins().size();
	else return 0;
}

std::shared_ptr<GpioDevice> BinaryOscillator6x::getGpioDevice(int i) {
	return gpioDevicePin->getConnectedPins().at(i)->getGpioDevice();
}




//========================================================================
//============================ PLOT INTERFACE ============================
//========================================================================



void BinaryOscillator6x::rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	stopOscillator();
	for (int i = 0; i < 6; i++) {
		setOutput(i, false);
	}
}

float BinaryOscillator6x::getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter) {
	for (int i = 0; i < 6; i++) {
		if (outputSignals[i]) return 0.0;
	}
	return 1.0;
}


bool BinaryOscillator6x::isParameterReadyToStartPlaybackFromValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	for (int i = 0; i < 6; i++) {
		if (outputSignals[i]) return false;
	}
	return true;
}

void BinaryOscillator6x::onParameterPlaybackStart(std::shared_ptr<AnimatableParameter> parameter) {
	if (parameter == minOffTimeParameter) {}
	else if (parameter == maxOffTimeParameter) {}
	else if (parameter == minOnTimeParameter) {}
	else if (parameter == maxOnTimeParameter) {}
	else if (parameter == oscillatorParameterGroup) startOscillator();
}

void BinaryOscillator6x::onParameterPlaybackInterrupt(std::shared_ptr<AnimatableParameter> parameter) {
	if (parameter == minOffTimeParameter) stopOscillator();
	else if (parameter == maxOffTimeParameter) stopOscillator();
	else if (parameter == minOnTimeParameter) stopOscillator();
	else if (parameter == maxOnTimeParameter) stopOscillator();
	else if (parameter == oscillatorParameterGroup) stopOscillator();
}

void BinaryOscillator6x::onParameterPlaybackEnd(std::shared_ptr<AnimatableParameter> parameter) {
	if (parameter == minOffTimeParameter) stopOscillator();
	else if (parameter == maxOffTimeParameter) stopOscillator();
	else if (parameter == minOnTimeParameter) stopOscillator();
	else if (parameter == maxOnTimeParameter) stopOscillator();
	else if (parameter == oscillatorParameterGroup) stopOscillator();
}


void BinaryOscillator6x::getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	if (parameter == minOffTimeParameter) { value.realValue = minOffTime_seconds; }
	else if (parameter == maxOffTimeParameter) { value.realValue = maxOffTime_seconds; }
	else if (parameter == minOnTimeParameter) { value.realValue = minOnTime_seconds; }
	else if (parameter == maxOnTimeParameter) { value.realValue = maxOnTime_seconds; }
	else if (parameter == oscillatorParameterGroup) { value.realValue = 0.0; }
}

void BinaryOscillator6x::cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter) {
	for (int i = 0; i < 6; i++) {
		setOutput(i, false);
	}
}

//validate curves, interpolations and points
bool BinaryOscillator6x::validateParameterTrack(const std::shared_ptr<ParameterTrack> parameterTrack) {
	bool trackValid = true;
	std::shared_ptr<AnimatableParameter> parameter = parameterTrack->parameter;

	if (parameter == minOffTimeParameter || parameter == maxOffTimeParameter || parameter == minOnTimeParameter || parameter == maxOnTimeParameter) { 
		for (auto& curve : parameterTrack->curves) {
			bool b_curveValid = true;
			for (auto& point : curve->points) {
				if (point->position >= 1.0 && point->position <= maxTime_seconds) {
					point->b_valid = true;
					point->validationError = Motion::ValidationError::Error::NO_VALIDATION_ERROR;
				}
				else {
					point->b_valid = false;
					point->validationError = Motion::ValidationError::Error::CONTROL_POINT_POSITION_OUT_OF_RANGE;
					b_curveValid = false;
				}
			}
			for (auto& interpolation : curve->interpolations) {
				bool interpolationValid = true;
				for (auto& point : interpolation->displayPoints) {
					if (point.position < 1.0 && point.position > maxTime_seconds) {
						interpolationValid = false;
						interpolation->validationError = Motion::ValidationError::Error::INTERPOLATION_POSITION_OUT_OF_RANGE;
						b_curveValid = false;
						break;
					}
				}
			}
			if (b_curveValid) curve->b_valid = true;
			else {
				curve->b_valid = false;
				trackValid = false;
			}
		}
	}
	else if (parameter == oscillatorParameterGroup) { 
		trackValid = true;
	}
	parameterTrack->b_valid = trackValid;
	return trackValid;
}

bool BinaryOscillator6x::getCurveLimitsAtTime(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& parameterCurves, double time, const std::shared_ptr<Motion::Curve> queriedCurve, double& lowLimit, double& highLimit) {
	if (parameter == minOffTimeParameter) { lowLimit = 1.0; highLimit = maxTime_seconds; }
	else if (parameter == maxOffTimeParameter) { lowLimit = 1.0; highLimit = maxTime_seconds; }
	else if (parameter == minOnTimeParameter) { lowLimit = 1.0; highLimit = maxTime_seconds; }
	else if (parameter == maxOnTimeParameter) { lowLimit = 1.0; highLimit = maxTime_seconds; }
	else if (parameter == oscillatorParameterGroup) { lowLimit = 1.0; highLimit = maxTime_seconds; }
	return true;
}

void BinaryOscillator6x::getTimedParameterCurveTo(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::ControlPoint>> targetPoints, double time, double rampIn, const std::vector<std::shared_ptr<Motion::Curve>>& outputCurves) {
	/*
	if (parameter == frequencyParameter ||
		parameter == minAmplitudeParameter ||
		parameter == maxAmplitudeParameter ||
		parameter == phaseOffsetParameter ||
		parameter == oscillatorParameterGroup) {

		//move to start or end

	}
	else if (parameter == axis1PositionParameter) {

	}
	else if (parameter == axis2PositionParameter) {

	}
	else if (parameter == axis3PositionParameter) {

	}
	*/
}

void BinaryOscillator6x::onEnableHardware() {
}

void BinaryOscillator6x::onDisableHardware() {
}


void BinaryOscillator6x::simulateProcess() {
	//TODO: Simulate Oscillator
}

bool BinaryOscillator6x::isSimulationReady(){
	return false;
}

void BinaryOscillator6x::onEnableSimulation() {
}

void BinaryOscillator6x::onDisableSimulation() {
}






void BinaryOscillator6x::getDevices(std::vector<std::shared_ptr<Device>>& output) {}

bool BinaryOscillator6x::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* defaultOscillatorSettingsXML = xml->InsertNewChildElement("DefaultOscillatorSettings");
	defaultOscillatorSettingsXML->SetAttribute("MinOffTime", minOffTime_seconds);
	defaultOscillatorSettingsXML->SetAttribute("MaxOffTime", maxOffTime_seconds);
	defaultOscillatorSettingsXML->SetAttribute("MinOnTime", minOnTime_seconds);
	defaultOscillatorSettingsXML->SetAttribute("MaxOnTime", maxOnTime_seconds);
	return true;
}

bool BinaryOscillator6x::loadMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* defaultOscillatorSettingsXML = xml->FirstChildElement("DefaultOscillatorSettings");
	if (defaultOscillatorSettingsXML == nullptr) return Logger::warn("Could not find default oscillator settings attribute");
	if (defaultOscillatorSettingsXML->QueryFloatAttribute("MinOffTime", &minOffTime_seconds) != XML_SUCCESS) return Logger::warn("Could not find min off time Attribute");
	if(defaultOscillatorSettingsXML->QueryFloatAttribute("MaxOffTime", &maxOffTime_seconds) != XML_SUCCESS) return Logger::warn("Could not find max off time Attribute");
	if(defaultOscillatorSettingsXML->QueryFloatAttribute("MinOnTime", &minOnTime_seconds) != XML_SUCCESS) return Logger::warn("Could not find min on time Attribute");
	if(defaultOscillatorSettingsXML->QueryFloatAttribute("MaxOnTime", &maxOnTime_seconds) != XML_SUCCESS) return Logger::warn("Could not find max on time Attribute");
	return true;
}
