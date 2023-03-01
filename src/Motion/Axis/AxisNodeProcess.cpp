#include <pch.h>

#include "AxisNode.h"

#include "Fieldbus/EtherCatFieldbus.h"



bool AxisNode::prepareProcess(){
	return true;
}

void AxisNode::inputProcess(){
	
	//update axis state
	DeviceState lowestInterfaceState = DeviceState::ENABLED;
	for(auto interface : connectedDeviceInterfaces){
		if(interface->getState() < lowestInterfaceState){
			lowestInterfaceState = interface->getState();
		}
	}
	if(axisInterface->getState() == DeviceState::ENABLED && lowestInterfaceState != DeviceState::ENABLED){
		for(auto actuator : connectedActuatorInterfaces) actuator->disable();
	}
	axisInterface->state = lowestInterfaceState;
	
	auto& processData = axisInterface->processData;
	
	if(processData.b_enable){
		processData.b_enable = false;
		if(axisInterface->getState() == DeviceState::READY){
			b_isEnabling = true;
			enableRequestTime_seconds = Timing::getProgramTime_seconds();
			for(auto actuator : connectedActuatorInterfaces) actuator->enable();
		}
	}
	
	if(processData.b_disable){
		processData.b_disable = false;
		b_isEnabling = false;
		for(auto actuator : connectedActuatorInterfaces) actuator->disable();
	}
	
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
			Logger::info("Axis Enabled");
		}
		else if(Timing::getProgramTime_seconds() - enableRequestTime_seconds > maxEnableTimeSeconds->value){
			for(auto actuator : connectedActuatorInterfaces) actuator->disable();
			b_isEnabling = false;
		}
		else if(axisInterface->getState() < DeviceState::READY){
			for(auto actuator : connectedActuatorInterfaces) actuator->disable();
			b_isEnabling = false;
		}
	}
	
	if(auto mapping = positionFeedbackMapping){
		auto feedback = mapping->feedbackInterface;
		processData.positionActual = feedback->getPosition() / mapping->feedbackUnitsPerAxisUnit->value;
		positionFollowingError = motionProfile.getPosition() - feedback->getPosition();
	}
	if(auto mapping = velocityFeedbackMapping){
		auto feedback = mapping->feedbackInterface;
		processData.velocityActual = feedback->getVelocity() / mapping->feedbackUnitsPerAxisUnit->value;
		velocityFollowingError = motionProfile.getVelocity() - feedback->getVelocity();
	}
	
	//Logger::error("IN: pos {}", processData.positionActual);
	
	//processData.forceActual = 0.0;
	
	double effort = 0.0;
	int effortSampleCount = 0;
	for(auto actuator : connectedActuatorInterfaces){
		if(actuator->supportsEffortFeedback()){
			effortSampleCount++;
			effort += actuator->getEffort();
		}
	}
	processData.effortActual = effort / effortSampleCount;
	
	bool b_estopActive = false;
	for(auto actuator : connectedActuatorInterfaces){
		if(actuator->isEmergencyStopActive()){
			b_estopActive = true;
			break;
		}
	}
	processData.b_isEmergencyStopActive = b_estopActive;
	
	//read limit signals
	/*
	lowerLimitSignalPin->copyConnectedPinValue();
	upperLimitSignalPin->copyConnectedPinValue();
	lowerSlowdownSignalPin->copyConnectedPinValue();
	upperSlowdownSignalPin->copyConnectedPinValue();
	referenceSignalPin->copyConnectedPinValue();
	*/
}

void AxisNode::outputProcess(){
	
	double profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	double profileTimeDelta_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
	
	if(axisInterface->isEnabled()){
		if(axisInterface->isHoming()){
			//homing routine
		}
		else if(axisPin->isConnected()){
			switch(axisInterface->configuration.controlMode){
				case AxisInterface::ControlMode::VELOCITY_CONTROL:{
					double velLim = std::abs(axisInterface->getVelocityLimit());
					double accLim = std::abs(axisInterface->getAccelerationLimit());
					double velTar = std::clamp(axisInterface->processData.velocityTarget, -velLim, velLim);
					double accTar = std::clamp(axisInterface->processData.accelerationTarget, -accLim, accLim);
					motionProfile.matchVelocity(profileTimeDelta_seconds, velTar, accTar);
				}break;
				case AxisInterface::ControlMode::POSITION_CONTROL:{
					double velLim = std::abs(axisInterface->getVelocityLimit());
					double accLim = std::abs(axisInterface->getAccelerationLimit());
					double lposLim = axisInterface->getLowerPositionLimit();
					double uposLim = axisInterface->getUpperPositionLimit();
					double posTar = axisInterface->processData.positionTarget;
					double velTar = std::clamp(axisInterface->processData.velocityTarget, -velLim, velLim);
					double accTar = std::clamp(axisInterface->processData.accelerationTarget, -accLim, accLim);
					motionProfile.matchPositionAndRespectPositionLimits(profileTimeDelta_seconds, posTar, velTar, accTar, accLim, velLim, lposLim, uposLim);
				}break;
				case AxisInterface::ControlMode::NONE:
					motionProfile.setPosition(axisInterface->getPositionActual());
					motionProfile.setVelocity(axisInterface->getVelocityActual());
					motionProfile.setAcceleration(0.0);
					break;
			}
		}
		else{
			switch(internalControlMode){
				case InternalControlMode::POSITION_TARGET:
					break;
				case InternalControlMode::VELOCITY_TARGET:
					motionProfile.matchVelocity(profileTimeDelta_seconds, manualVelocityTarget, manualVelocityAcceleration);
					break;
			}
		}
	}else{
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
		
		
		
	}
	
	
	//control loop update (depends on axis control mode)
	double positionError = motionProfile.getPosition() - axisInterface->getPositionActual();
	double velocityCommand = motionProfile.getVelocity() * positionLoop_velocityFeedForward->value + positionError * positionLoop_proportionalGain->value;
	
	//send commands to actuators
	for(auto mapping : actuatorMappings){
		auto actuator = mapping->actuatorInterface;
		switch(mapping->controlMode){
			case ActuatorMapping::POSITION_CONTROL:
				{
				double actuatorPosition = (motionProfile.getPosition() * mapping->actuatorUnitsPerAxisUnits->value) + mapping->actuatorPositionOffset;
				actuator->setPositionTarget(actuatorPosition);
				}
				break;
			case ActuatorMapping::VELOCITY_CONTROL:
				{
				double actuatorVelocity = velocityCommand * mapping->actuatorUnitsPerAxisUnits->value;
				actuator->setVelocityTarget(actuatorVelocity);
				}
				break;
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
