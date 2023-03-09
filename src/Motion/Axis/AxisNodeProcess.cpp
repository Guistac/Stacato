#include <pch.h>

#include "AxisNode.h"

#include "Fieldbus/EtherCatFieldbus.h"



bool AxisNode::prepareProcess(){
	
	switch(axisControlMode){
		case ControlMode::POSITION_CONTROL:
			if(positionFeedbackMapping == nullptr){
				Logger::error("[{}] axis has control mode Position but has no position feedback device selected", getName());
				return false;
			}
			if(velocityFeedbackMapping == nullptr) {
				Logger::error("[{}] axis has control mode Position but has no velocity feedback device selected", getName());
				return false;
			}
			if(axisInterface->getLowerPositionLimit() >= axisInterface->getUpperPositionLimit()){
				Logger::error("[{}] axis position limits are not valid, lower limit is above or equal to upper limit.", getName());
				return false;
			}
			break;
		case ControlMode::VELOCITY_CONTROL:
			if(velocityFeedbackMapping == nullptr) {
				Logger::error("[{}] axis has control mode Velocity but has no velocity feedback device selected", getName());
				return false;
			}
			break;
		case ControlMode::NO_CONTROL:
		default:
			break;
	}
	
	
	
	
	
	
	return true;
}

void AxisNode::inputProcess(){
	
	//get the current time from the fieldbus namespace
	profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	profileTimeDelta_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
	
	//update axis state
	DeviceState lowestInterfaceState = DeviceState::ENABLED;
	for(auto interface : connectedDeviceInterfaces){
		if(interface->getState() < lowestInterfaceState){
			lowestInterfaceState = interface->getState();
		}
	}
	DeviceState previousAxisState = axisInterface->getState();
	axisInterface->state = lowestInterfaceState;
	
	//disable the axis if any connected interface is not enabled anymore
	if(previousAxisState == DeviceState::ENABLED && lowestInterfaceState != DeviceState::ENABLED){
		for(auto actuator : connectedActuatorInterfaces) actuator->disable();
		axisInterface->state = DeviceState::DISABLING;
		Logger::warn("[{}] Disabling axis, all devices were not enabled anymore", getName());
		for(auto actuator : connectedActuatorInterfaces){
			if(!actuator->isEnabled()) Logger::warn("[{}]    {} was not enabled", getName(), actuator->getName());
		}
		for(auto gpio : connectedGpioInterfaces){
			if(!gpio->isEnabled()) Logger::warn("[{}]    {} was not enabled", getName(), gpio->getName());
		}
		for(auto feedback : connectedFeedbackInteraces){
			if(!feedback->isEnabled()) Logger::warn("[{}]    {} was not enabled", getName(), feedback->getName());
		}
	}
	
	
	auto& processData = axisInterface->processData;
	
	//handle axis enable request
	if(processData.b_enable){
		processData.b_enable = false;
		if(axisInterface->getState() == DeviceState::READY){
			b_isEnabling = true;
			enableRequestTime_seconds = Timing::getProgramTime_seconds();
			for(auto actuator : connectedActuatorInterfaces) actuator->enable();
			Logger::info("[{}] Enabling Axis", getName());
		}
	}
	
	//handle enable process
	if(b_isEnabling){
		bool b_allEnabled = true;
		for(auto actuator : connectedActuatorInterfaces){
			if(!actuator->isEnabled()){
				b_allEnabled = false;
				break;
			}
		}
		if(b_allEnabled){
			b_isEnabling = false;
			Logger::info("[{}] Axis Enabled", getName());
		}
		else if(Timing::getProgramTime_seconds() - enableRequestTime_seconds > maxEnableTimeSeconds->value){
			for(auto actuator : connectedActuatorInterfaces) actuator->disable();
			b_isEnabling = false;
			Logger::warn("[{}] Could not enable axis :", getName());
			for(auto actuator : connectedActuatorInterfaces){
				if(!actuator->isEnabled()) Logger::warn("[{}]    {} did not enable on time", getName(), actuator->getName());
			}
		}
		else if(axisInterface->getState() < DeviceState::READY){
			for(auto actuator : connectedActuatorInterfaces) actuator->disable();
			b_isEnabling = false;
			Logger::warn("[{}] Could not enable axis :", getName());
			for(auto actuator : connectedActuatorInterfaces){
				if(!actuator->isReady()) Logger::warn("[{}]    {} was not read", getName(), actuator->getName());
			}
		}
	}
	
	//update axis position and position following error
	if(auto mapping = positionFeedbackMapping){
		auto feedback = mapping->feedbackInterface;
		processData.positionActual = feedback->getPosition() / mapping->feedbackUnitsPerAxisUnit->value;
		positionFollowingError = motionProfile.getPosition() - axisInterface->getPositionActual();
	}
	
	//update axis velocity and velocity following error
	if(auto mapping = velocityFeedbackMapping){
		auto feedback = mapping->feedbackInterface;
		processData.velocityActual = feedback->getVelocity() / mapping->feedbackUnitsPerAxisUnit->value;
		velocityFollowingError = motionProfile.getVelocity() - axisInterface->getVelocityActual();
	}
	processData.forceActual = 0.0;
	
	//update axis effort
	double effort = 0.0;
	int effortSampleCount = 0;
	for(auto actuator : connectedActuatorInterfaces){
		if(actuator->supportsEffortFeedback()){
			effortSampleCount++;
			effort += actuator->getEffort();
		}
	}
	processData.effortActual = effort / effortSampleCount;
	
	//update estop state
	bool b_estopActive = false;
	for(auto actuator : connectedActuatorInterfaces){
		if(actuator->isEmergencyStopActive()){
			b_estopActive = true;
			break;
		}
	}
	processData.b_isEmergencyStopActive = b_estopActive;
	
	
	
	
	//read and react to limit signals
	previousLowerLimitSignal = *lowerLimitSignal;
	previousUpperLimitSignal = *upperLimitSignal;
	previousReferenceSignal = *referenceSignal;
	switch(limitsignalType){
		case LimitSignalType::NONE:
			break;
		case LimitSignalType::SIGNAL_AT_LOWER_LIMIT:
			lowerLimitSignalPin->copyConnectedPinValue();
			if(axisInterface->isEnabled() && !axisInterface->isHoming()){
				if(*lowerLimitSignal && motionProfile.getVelocity() < 0.0) {
					Logger::warn("[{}] Triggered lower limit switch, disabling axis", getName());
					axisInterface->disable();
				}
			}
			break;
		case LimitSignalType::SIGNAL_AT_LOWER_AND_UPPER_LIMITS:
			lowerLimitSignalPin->copyConnectedPinValue();
			upperLimitSignalPin->copyConnectedPinValue();
			if(axisInterface->isEnabled() && !axisInterface->isHoming()){
				if(*lowerLimitSignal && motionProfile.getVelocity() < 0.0) {
					axisInterface->disable();
					Logger::warn("[{}] Triggered lower limit switch, disabling axis", getName());
				}
				if(*upperLimitSignal && motionProfile.getVelocity() > 0.0) {
					axisInterface->disable();
					Logger::warn("[{}] Triggered upper limit switch, disabling axis", getName());
				}
			}
			break;
		case LimitSignalType::SIGNAL_AT_ORIGIN:
			referenceSignalPin->copyConnectedPinValue();
			break;
		case LimitSignalType::LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS:
			lowerLimitSignalPin->copyConnectedPinValue();
			upperLimitSignalPin->copyConnectedPinValue();
			lowerSlowdownSignalPin->copyConnectedPinValue();
			upperSlowdownSignalPin->copyConnectedPinValue();
			break;
	}
	
	//update homing capability flag
	axisInterface->processData.b_canStartHoming = axisInterface->supportsHoming() && axisInterface->isEnabled() && motionProfile.getVelocity() == 0.0;
	
	//check if the following errors exceed thresholds
	if(axisInterface->isEnabled()){
		if(axisInterface->configuration.controlMode == AxisInterface::ControlMode::POSITION_CONTROL){
			if(std::abs(positionFollowingError) > std::abs(positionLoop_maxError->value)){
				Logger::warn("[{}] Position following error exceeded threshold, Disabling axis.", getName());
				axisInterface->disable();
			}
		}
		if((axisInterface->configuration.controlMode == AxisInterface::ControlMode::POSITION_CONTROL ||
		   axisInterface->configuration.controlMode == AxisInterface::ControlMode::VELOCITY_CONTROL) &&
		   velocityFeedbackMapping){
			if(std::abs(velocityFollowingError) > std::abs(velocityLoop_maxError->value)){
				Logger::warn("[{}] Velocity following error exceeded threshold, Disabling axis.", getName());
				axisInterface->disable();
			}
		}
	}
	
	
	//react to axis homing commands
	if(axisInterface->processData.b_startHoming){
		axisInterface->processData.b_startHoming = false;
		if(axisInterface->canStartHoming()){
			axisInterface->processData.b_isHoming = true;
			axisInterface->processData.b_didHomingSucceed = false;
			homingStep = HomingStep::NOT_STARTED;
		}
	}
	if(axisInterface->processData.b_stopHoming){
		axisInterface->processData.b_stopHoming = false;
		axisInterface->processData.b_isHoming = false;
		axisInterface->processData.b_didHomingSucceed = false;
		homingStep = HomingStep::NOT_STARTED;
		setManualVelocityTarget(0.0);
	}
	
	//decide on the internal control mode
	//if no condition matches, we are using manual controls
	if(previousAxisState != DeviceState::ENABLED == axisInterface->isEnabled()) internalControlMode = InternalControlMode::MANUAL_VELOCITY_TARGET;
	else if(!axisInterface->isEnabled()) internalControlMode = InternalControlMode::NO_CONTROL;
	else if(axisInterface->isHoming()) homingControl();
	else if(axisPin->isConnected()){
		switch(axisInterface->configuration.controlMode){
			case AxisInterface::ControlMode::VELOCITY_CONTROL:
				internalControlMode = InternalControlMode::EXTERNAL_VELOCITY_TARGET;
				break;
			case AxisInterface::ControlMode::POSITION_CONTROL:
				internalControlMode = InternalControlMode::EXTERNAL_POSITION_TARGET;
				break;
			case AxisInterface::ControlMode::NONE:
				internalControlMode = InternalControlMode::NO_CONTROL;
				break;
		}
	}
	
	//handle axis disable request
	if(processData.b_disable){
		processData.b_disable = false;
		b_isEnabling = false;
		for(auto actuator : connectedActuatorInterfaces) actuator->disable();
		axisInterface->state = DeviceState::DISABLING;
	}
	
	
}

void AxisNode::outputProcess(){
	
	//update the motion profile
	switch(internalControlMode){
		case InternalControlMode::MANUAL_VELOCITY_TARGET:
			if(axisInterface->configuration.controlMode == AxisInterface::ControlMode::POSITION_CONTROL){
				double acc = axisInterface->getAccelerationLimit();
				double minPos = axisInterface->getLowerPositionLimit();
				double maxPos = axisInterface->getUpperPositionLimit();
				motionProfile.matchVelocityAndRespectPositionLimits(profileTimeDelta_seconds, internalVelocityTarget, acc, minPos, maxPos, acc);
			}else{
				double acc = axisInterface->getAccelerationLimit();
				motionProfile.matchVelocity(profileTimeDelta_seconds, internalVelocityTarget, acc);
			}
			break;
		case InternalControlMode::MANUAL_POSITION_INTERPOLATION:
			motionProfile.updateInterpolation(profileTime_seconds);
			break;
		case InternalControlMode::HOMING_VELOCITY_TARGET:
			motionProfile.matchVelocity(profileTimeDelta_seconds, internalVelocityTarget, axisInterface->getAccelerationLimit());
			break;
		case InternalControlMode::HOMING_POSITION_INTERPOLATION:
			motionProfile.updateInterpolation(profileTime_seconds);
			break;
		case InternalControlMode::EXTERNAL_POSITION_TARGET:{
			double velLim = std::abs(axisInterface->getVelocityLimit());
			double accLim = std::abs(axisInterface->getAccelerationLimit());
			double lposLim = axisInterface->getLowerPositionLimit();
			double uposLim = axisInterface->getUpperPositionLimit();
			double posTar = axisInterface->processData.positionTarget;
			double velTar = std::clamp(axisInterface->processData.velocityTarget, -velLim, velLim);
			double accTar = std::clamp(axisInterface->processData.accelerationTarget, -accLim, accLim);
			motionProfile.matchPositionAndRespectPositionLimits(profileTimeDelta_seconds, posTar, velTar, accTar, accLim, velLim, lposLim, uposLim);
			}break;
		case InternalControlMode::EXTERNAL_VELOCITY_TARGET:{
			double velLim = std::abs(axisInterface->getVelocityLimit());
			double accLim = std::abs(axisInterface->getAccelerationLimit());
			double velTar = std::clamp(axisInterface->processData.velocityTarget, -velLim, velLim);
			double accTar = std::clamp(axisInterface->processData.accelerationTarget, -accLim, accLim);
			motionProfile.matchVelocity(profileTimeDelta_seconds, velTar, accTar);
			}break;
		case InternalControlMode::NO_CONTROL:
			motionProfile.setPosition(axisInterface->getPositionActual());
			motionProfile.setVelocity(axisInterface->getVelocityActual());
			motionProfile.setAcceleration(0.0);
			for(auto mapping : actuatorMappings){
				auto actuator = mapping->actuatorInterface;
				if(positionFeedbackMapping && positionFeedbackMapping->feedbackInterface == actuator){
					mapping->actuatorPositionOffset = 0.0;
				}
				mapping->actuatorPositionOffset = actuator->getPosition() - axisInterface->getPositionActual() * mapping->actuatorUnitsPerAxisUnits->value;
			}
			break;
	}
	
	//control loop update (depends on axis control mode)
	double velocityCommand;
	auto updatePositionControlLoop = [&](){
		if(motionProfile.getVelocity() == 0.0 && std::abs(positionFollowingError) < std::abs(positionLoop_minError->value))
			velocityCommand = 0.0;
		else
			velocityCommand = motionProfile.getVelocity() * positionLoop_velocityFeedForward->value + positionFollowingError * positionLoop_proportionalGain->value;
	};
	
	switch(internalControlMode){
		case InternalControlMode::MANUAL_VELOCITY_TARGET:
			switch(axisInterface->configuration.controlMode){
				case AxisInterface::ControlMode::POSITION_CONTROL:
					updatePositionControlLoop();
					break;
				case AxisInterface::ControlMode::VELOCITY_CONTROL:
					velocityCommand = motionProfile.getVelocity();
					break;
				case AxisInterface::ControlMode::NONE:
					velocityCommand = 0.0;
					break;
			}
			break;
		case InternalControlMode::MANUAL_POSITION_INTERPOLATION:
			updatePositionControlLoop();
			break;
		case InternalControlMode::HOMING_VELOCITY_TARGET:
			velocityCommand = motionProfile.getVelocity();
			break;
		case InternalControlMode::HOMING_POSITION_INTERPOLATION:
			updatePositionControlLoop();
			break;
		case InternalControlMode::EXTERNAL_POSITION_TARGET:
			updatePositionControlLoop();
			break;
		case InternalControlMode::EXTERNAL_VELOCITY_TARGET:
			velocityCommand = motionProfile.getVelocity();
			break;
		case InternalControlMode::NO_CONTROL:
			velocityCommand = 0.0;
			break;
	}
	
	//send commands to actuators
	for(auto mapping : actuatorMappings){
		auto actuator = mapping->actuatorInterface;
		switch(mapping->controlMode){
			case ActuatorMapping::POSITION_CONTROL:{
				double actuatorPosition = (motionProfile.getPosition() * mapping->actuatorUnitsPerAxisUnits->value) + mapping->actuatorPositionOffset;
				actuator->setPositionTarget(actuatorPosition);
				}break;
			case ActuatorMapping::VELOCITY_CONTROL:{
				double actuatorVelocity = velocityCommand * mapping->actuatorUnitsPerAxisUnits->value;
				actuator->setVelocityTarget(actuatorVelocity);
				}break;
			case ActuatorMapping::FORCE_CONTROL:
				//not implemented
				break;
			case ActuatorMapping::NO_CONTROL:
			default:
				break;
		}
	}
	
}

bool AxisNode::needsOutputProcess(){ return !axisPin->isConnected(); }



void AxisNode::setManualVelocityTarget(double velocity){
	internalControlMode = InternalControlMode::MANUAL_VELOCITY_TARGET;
	double velLim = std::abs(axisInterface->getVelocityLimit());
	internalVelocityTarget = std::clamp(velocity, -velLim, velLim);
}

void AxisNode::moveToManualPositionTargetWithTime(double position, double time, double acceleration){
	if(acceleration == 0.0) return;
	if(axisInterface->getVelocityLimit() == 0.0) return;
	double pos = std::clamp(position, axisInterface->getLowerPositionLimit(), axisInterface->getUpperPositionLimit());
	double acc = std::clamp(acceleration, 0.0, axisInterface->getAccelerationLimit());
	Logger::info("moving to {}", pos);
	motionProfile.moveToPositionInTime(profileTime_seconds, pos, time, acc, axisInterface->getVelocityLimit());
	internalControlMode = InternalControlMode::MANUAL_POSITION_INTERPOLATION;
}

void AxisNode::moveToManualPositionTargetWithVelocity(double position, double velocity, double acceleration){
	if(acceleration == 0.0) return;
	if(velocity <= 0.0) return;
	double pos = std::clamp(position, axisInterface->getLowerPositionLimit(), axisInterface->getUpperPositionLimit());
	double vel = std::clamp(velocity, 0.0, std::abs(axisInterface->getVelocityLimit()));
	double acc = std::clamp(acceleration, 0.0, std::abs(axisInterface->getAccelerationLimit()));
	Logger::info("moving to {}", pos);
	motionProfile.moveToPositionWithVelocity(profileTime_seconds, pos, vel, acc);
	internalControlMode = InternalControlMode::MANUAL_POSITION_INTERPOLATION;
}

void AxisNode::setHomingVelocityTarget(double velocity){
	internalControlMode = InternalControlMode::HOMING_VELOCITY_TARGET;
	double velLim = std::abs(axisInterface->getVelocityLimit());
	internalVelocityTarget = std::clamp(velocity, -velLim, velLim);
}

void AxisNode::moveToHomingPositionTarget(double position){
	motionProfile.moveToPositionInTime(profileTime_seconds, position, 0.0, axisInterface->getAccelerationLimit(), axisInterface->getVelocityLimit());
	internalControlMode = InternalControlMode::HOMING_POSITION_INTERPOLATION;
}

void AxisNode::overrideCurrentPosition(double newPosition){
	motionProfile.setPosition(newPosition);
	for(auto actuatorMapping : actuatorMappings){
		auto actuator = actuatorMapping->actuatorInterface;
		if(actuator == positionFeedbackMapping->feedbackInterface) continue;
		actuatorMapping->actuatorPositionOffset = actuator->getPosition();
	}
}
