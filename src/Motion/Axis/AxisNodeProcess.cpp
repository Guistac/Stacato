#include <pch.h>

#include "AxisNode.h"

#include "Fieldbus/EtherCatFieldbus.h"



bool AxisNode::prepareProcess(){
	
	switch(controlMode){
		case ControlMode::POSITION_CONTROL:
			if(selectedPositionFeedbackMapping == nullptr || !selectedPositionFeedbackMapping->isFeedbackConnected()){
				Logger::error("[{}] axis has control mode Position but has no position feedback device selected", getName());
				return false;
			}
			if(selectedVelocityFeedbackMapping == nullptr || !selectedVelocityFeedbackMapping->isFeedbackConnected()) {
				Logger::error("[{}] axis has control mode Position but has no velocity feedback device selected", getName());
				return false;
			}
			if(axisInterface->getLowerPositionLimit() >= axisInterface->getUpperPositionLimit()){
				Logger::error("[{}] axis position limits are not valid, lower limit is above or equal to upper limit.", getName());
				return false;
			}
			break;
		case ControlMode::VELOCITY_CONTROL:
			if(selectedVelocityFeedbackMapping == nullptr || !selectedVelocityFeedbackMapping->isFeedbackConnected()) {
				Logger::error("[{}] axis has control mode Velocity but has no velocity feedback device selected", getName());
				return false;
			}
			break;
		case ControlMode::NO_CONTROL:
		default:
			break;
	}
	switch(limitSignalType){
		case LimitSignalType::NONE:
			break;
		case LimitSignalType::SIGNAL_AT_LOWER_LIMIT:
			if(!lowerLimitSignalPin->isConnected()){
				Logger::error("[{}] lower limit signal pin is not connected", getName());
				return false;
			}
			break;
		case LimitSignalType::SIGNAL_AT_LOWER_AND_UPPER_LIMITS:
			if(!lowerLimitSignalPin->isConnected()){
				Logger::error("[{}] lower limit signal pin is not connected", getName());
				return false;
			}
			if(!upperLimitSignalPin->isConnected()){
				Logger::error("[{}] upper limit signal pin is not connected", getName());
				return false;
			}
			break;
		case LimitSignalType::SIGNAL_AT_ORIGIN:
			if(!referenceSignalPin->isConnected()){
				Logger::error("[{}] reference signal pin is not connected", getName());
				return false;
			}
			break;
		case LimitSignalType::LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS:
			if(!lowerLimitSignalPin->isConnected() ||
			   !upperLimitSignalPin->isConnected() ||
			   !lowerSlowdownSignalPin->isConnected() ||
			   !upperSlowdownSignalPin->isConnected()){
				Logger::error("[{}] Not all limit and slowdown signal pins are connected", getName());
				return false;
			}
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
	bool b_readyStatePinInvalid = false;
	if(readyStateInputPin->isConnected()){
		readyStateInputPin->copyConnectedPinValue();
		if(!*readyStateInputSignal && lowestInterfaceState > DeviceState::NOT_READY) {
			lowestInterfaceState = DeviceState::NOT_READY;
			b_readyStatePinInvalid = true;
		}
	}
	DeviceState previousAxisState = axisInterface->getState();
	axisInterface->state = lowestInterfaceState;
	
	
	//handle axis safety
	bool previousSafetyResetSignal = *safetyResetInputSignal;
	if(safetyFaultInputPin->isConnected()) safetyFaultInputPin->copyConnectedPinValue();
	if(safetyResetInputPin->isConnected()) safetyResetInputPin->copyConnectedPinValue();
	if(!b_hasSafetyFault){
		*safetyFaultOutputSignal = true;
		if(safetyFaultInputPin->isConnected() && !*safetyFaultInputSignal){
			Logger::warn("{} Safety fault triggered by fault signal", getName());
			axisInterface->disable();
			b_hasSafetyFault = true;
		}
		if(velocitySafetyRule->b_enabled && !velocitySafetyRule->isRespected()){
			Logger::warn("{} Safety fault triggered by velocity deviation", getName());
			axisInterface->disable();
			b_hasSafetyFault = true;
		}
		if(positionSafetyRule->b_enabled && !positionSafetyRule->isRespected()){
			Logger::warn("{} Safety fault triggered by position deviation", getName());
			axisInterface->disable();
			b_hasSafetyFault = true;
		}
	}
	else{
		*safetyFaultOutputSignal = false;
		if(!previousSafetyResetSignal && *safetyResetInputSignal) b_safetyFaultClearRequest = true;
		if(b_safetyFaultClearRequest){
			b_safetyFaultClearRequest = false;
			b_isClearingSafetyFault = true;
			safetyFaultResetRequestTimeNanos = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
		}
	}
	if(b_isClearingSafetyFault){
		uint64_t timeSinceClearRequest = EtherCatFieldbus::getCycleProgramTime_nanoseconds() - safetyFaultResetRequestTimeNanos;
		uint64_t maxClearTime = safetyClearSignalLengthSeconds * 1000000000;
		uint64_t resetSignalOffsetTime = safetyResetSignalOffsetSeconds * 1000000000;
		*safetyFaultOutputSignal = true;
		if(timeSinceClearRequest >= resetSignalOffsetTime) *safetyResetOutputSignal = true;
		else *safetyResetOutputSignal = false;
		if(timeSinceClearRequest >= maxClearTime){
			b_hasSafetyFault = true;
			b_isClearingSafetyFault = false;
			*safetyResetOutputSignal = false;
			*safetyFaultOutputSignal = false;
			Logger::warn("{} Could not clear safety fault", getName());
		}
		if(*safetyFaultInputSignal){
			b_hasSafetyFault = false;
			b_isClearingSafetyFault = false;
			*safetyResetOutputSignal = false;
			Logger::info("{} Cleared safety fault", getName());
		}
	}
	
	
	//disable the axis if any connected interface is not enabled anymore
	if(previousAxisState == DeviceState::ENABLED && lowestInterfaceState != DeviceState::ENABLED){
		for(auto actuatorMapping : actuatorMappings) actuatorMapping->disable();
		axisInterface->state = DeviceState::DISABLING;
		Logger::warn("[{}] Disabling axis", getName());
		for(auto actuatorMapping : actuatorMappings){
			if(!actuatorMapping->isEnabled()) Logger::warn("[{}] {} was not enabled", getName(), actuatorMapping->getName());
		}
		for(auto gpio : connectedGpioInterfaces){
			if(!gpio->isEnabled()) Logger::warn("[{}] {} was not enabled", getName(), gpio->getName());
		}
		
		if(selectedPositionFeedbackMapping && selectedPositionFeedbackMapping->isFeedbackConnected() && !selectedPositionFeedbackMapping->isEnabled())
			Logger::warn("[{}] {} was not enabled", getName(), selectedPositionFeedbackMapping->getName());
		
		if(b_readyStatePinInvalid){
			Logger::warn("[{}] Ready Signal was not valid", getName());
		}
	}
	
	
	auto& processData = axisInterface->processData;
	
	//handle axis enable request
	if(processData.b_enable){
		processData.b_enable = false;
		if(axisInterface->getState() == DeviceState::READY){
			b_isEnabling = true;
			
			homingStep = HomingStep::NOT_STARTED;
			axisInterface->processData.b_isHoming = false;
			axisInterface->processData.b_didHomingSucceed = false;
			
			enableRequestTime_seconds = Timing::getProgramTime_seconds();
			for(auto actuatorMapping : actuatorMappings) {
				if(!actuatorMapping->isActuatorConnected()) continue;
				actuatorMapping->enable();
			}
			Logger::info("[{}] Enabling Axis", getName());
		}
	}
	if(b_isEnabling){
		axisInterface->state = DeviceState::ENABLING;
		bool b_allEnabled = true;
		for(auto actuatorMapping : actuatorMappings){
			if(!actuatorMapping->isActuatorConnected()) continue;
			if(!actuatorMapping->isEnabled()){
				b_allEnabled = false;
				break;
			}
		}
		if(b_allEnabled){
			b_isEnabling = false;
			homingStep = HomingStep::NOT_STARTED;
			Logger::info("[{}] Axis Enabled", getName());
		}
		else if(Timing::getProgramTime_seconds() - enableRequestTime_seconds > maxEnableTimeSeconds->value){
			for(auto actuatorMapping : actuatorMappings) {
				if(!actuatorMapping->isActuatorConnected()) continue;
				actuatorMapping->disable();
			}
			
			homingStep = HomingStep::NOT_STARTED;
			axisInterface->processData.b_isHoming = false;
			axisInterface->processData.b_didHomingSucceed = false;
			
			b_isEnabling = false;
			Logger::warn("[{}] Could not enable axis :", getName());
			for(auto actuatorMapping : actuatorMappings){
				if(!actuatorMapping->isActuatorConnected()) continue;
				if(!actuatorMapping->isEnabled()) Logger::warn("[{}]    {} did not enable on time", getName(), actuatorMapping->getName());
			}
		}
		else if(axisInterface->getState() < DeviceState::READY){
			for(auto actuatorMapping : actuatorMappings) {
				if(!actuatorMapping->isActuatorConnected()) continue;
				actuatorMapping->disable();
			}
			
			homingStep = HomingStep::NOT_STARTED;
			axisInterface->processData.b_isHoming = false;
			axisInterface->processData.b_didHomingSucceed = false;
			
			b_isEnabling = false;
			Logger::warn("[{}] Could not enable axis :", getName());
			for(auto actuatorMapping : actuatorMappings){
				if(!actuatorMapping->isActuatorConnected()) continue;
				if(!actuatorMapping->isReady()) Logger::warn("[{}]    {} was not read", getName(), actuatorMapping->getName());
			}
		}
	}
	
	
	
	
	//if it was requested, update the feedback ratio
	if(b_shouldUpdateFeedbackRatio){
		b_shouldUpdateFeedbackRatio = false;
		onFeedbackRatioUpdate();
	}
	
	
	//update axis position and position following error
	if(selectedPositionFeedbackMapping && selectedPositionFeedbackMapping->isFeedbackConnected()){
		auto feedbackDevice = selectedPositionFeedbackMapping->getFeedbackInterface();
		processData.positionActual = feedbackDevice->getPosition() / selectedPositionFeedbackMapping->deviceUnitsPerAxisUnits->value;
		positionFollowingError = motionProfile.getPosition() - axisInterface->getPositionActual();
	}
	
	//update axis velocity and velocity following error
	if(selectedVelocityFeedbackMapping && selectedVelocityFeedbackMapping->isFeedbackConnected()){
		auto feedbackDevice = selectedVelocityFeedbackMapping->getFeedbackInterface();
		processData.velocityActual = feedbackDevice->getVelocity() / selectedVelocityFeedbackMapping->deviceUnitsPerAxisUnits->value;
		velocityFollowingError = motionProfile.getVelocity() - axisInterface->getVelocityActual();
	}
	
	
	//update axis effort
	double effort = 0.0;
	int effortSampleCount = 0;
	for(auto actuatorMapping : actuatorMappings){
		if(!actuatorMapping->isActuatorConnected()) continue;
		auto actuatorInterface = actuatorMapping->getActuatorInterface();
		if(actuatorInterface->supportsEffortFeedback()){
			effortSampleCount++;
			effort += actuatorInterface->getEffort();
		}
	}
	processData.effortActual = effort / effortSampleCount;
	
	
	//update estop state
	bool b_estopActive = false;
	for(auto actuatorMapping : actuatorMappings){
		if(!actuatorMapping->isActuatorConnected()) continue;
		auto actuatorInterface = actuatorMapping->getActuatorInterface();
		if(actuatorInterface->isEmergencyStopActive()){
			b_estopActive = true;
			break;
		}
	}
	processData.b_isEmergencyStopActive = b_estopActive;
	
	
	if(useExternalLoadSensor_Param->value && loadSensorPin->isConnected()){
		loadSensorPin->copyConnectedPinValue();
		processData.forceActual = *loadSensorSignal * forceSensorMultiplier_Param->value + forceSensorOffset_Param->value;
	}else{
		processData.forceActual = 0.0; //implement load monitoring from actuators, maybe a force feedback mapping object ?
	}
	
	if(b_shouldTareForceSensor){
		b_shouldTareForceSensor = false;
		double newOffset = *loadSensorSignal * forceSensorMultiplier_Param->value;
		forceSensorOffset_Param->overwrite(-newOffset);
	}
	
	
	//react to force signal limit
	if(useExternalLoadSensor_Param->value && loadSensorPin->isConnected()){
		if(axisInterface->processData.forceActual > upperForceLimit->value){
			if(axisInterface->isEnabled()){
				Logger::warn("[{}] Axis Disabled : Upper Force Limit Exceeded", getName());
				axisInterface->disable();
			}
		}
		if(axisInterface->processData.forceActual < lowerForceLimit->value){
			if(axisInterface->isEnabled()){
				Logger::warn("[{}] Axis Disabled : Lower Force Limit Exceeded", getName());
				axisInterface->disable();
			}
		}
	}
	
	
	//read and react to limit signals
	previousLowerLimitSignal = *lowerLimitSignal;
	previousUpperLimitSignal = *upperLimitSignal;
	previousReferenceSignal = *referenceSignal;
	switch(limitSignalType){
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
			axisInterface->processData.b_isAtUpperLimit = *upperLimitSignal;
			axisInterface->processData.b_isAtLowerLimit = *lowerLimitSignal;
			if(controlMode == POSITION_CONTROL){
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
			axisInterface->processData.b_isAtUpperLimit = *upperLimitSignal;
			axisInterface->processData.b_isAtLowerLimit = *lowerLimitSignal;
			break;
	}
	
	
	
	//update homing capability flag
	axisInterface->processData.b_canStartHoming = axisInterface->supportsHoming() && axisInterface->isEnabled() && motionProfile.getVelocity() == 0.0;

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
	
	
	//check if the following errors exceed thresholds
	if(axisInterface->isEnabled()){
		if(axisInterface->configuration.controlMode == AxisInterface::ControlMode::POSITION_CONTROL && !axisInterface->isHoming()){
			if(std::abs(positionFollowingError) > std::abs(positionLoop_maxError->value)){
				uint64_t now = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
				if(!b_isOverPositionErrorTreshold){
					positionErrorStartTime_nanos = now;
					b_isOverPositionErrorTreshold = true;
				}
				Logger::trace("[{}] over position error treshold, error = {}", getName(), positionFollowingError);
				if(now - positionErrorStartTime_nanos >= positionLoop_errorTimeout_milliseconds->value * 1'000'000){
					Logger::warn("[{}] Position following error exceeded threshold, Disabling axis.", getName());
					Logger::warn("[{}] following error : {}", getName(), positionFollowingError);
					axisInterface->disable();
				}

			}else{
				b_isOverPositionErrorTreshold = false;
			}
		}
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
		for(auto actuatorMapping : actuatorMappings) {
			if(!actuatorMapping->isActuatorConnected()) continue;
			actuatorMapping->getActuatorInterface()->disable();
		}
		axisInterface->state = DeviceState::DISABLING;
		homingStep = HomingStep::NOT_STARTED;
		axisInterface->processData.b_isHoming = false;
		axisInterface->processData.b_didHomingSucceed = false;
	}
	
}

void AxisNode::outputProcess(){
	
	auto limitAndSlowdownVelocityControl = [this](double targetVelocity){
		double slowdownVelocity = std::abs(limitSlowdownVelocity->value);
		double acc = axisInterface->getAccelerationLimit();
		if(*lowerLimitSignal && (targetVelocity < 0.0 || motionProfile.getVelocity() < 0.0)){
			motionProfile.matchVelocity(profileTimeDelta_seconds, 0.0, acc);
		}
		else if(*upperLimitSignal && (targetVelocity > 0.0 || motionProfile.getVelocity() > 0.0)){
			motionProfile.matchVelocity(profileTimeDelta_seconds, 0.0, acc);
		}
		else if(*lowerSlowdownSignal && (targetVelocity < -slowdownVelocity || motionProfile.getVelocity() < -slowdownVelocity)){
			motionProfile.matchVelocity(profileTimeDelta_seconds, -slowdownVelocity, acc);
		}
		else if(*upperSlowdownSignal && (targetVelocity > slowdownVelocity || motionProfile.getVelocity() > slowdownVelocity)){
			motionProfile.matchVelocity(profileTimeDelta_seconds, slowdownVelocity, acc);
		}
		else{
			double vel = getFilteredVelocity(targetVelocity);
			motionProfile.matchVelocity(profileTimeDelta_seconds, vel, acc);
		}
	};
	
	//update the motion profile
	switch(internalControlMode){
		case InternalControlMode::MANUAL_VELOCITY_TARGET:
			if(axisInterface->configuration.controlMode == AxisInterface::ControlMode::POSITION_CONTROL){
				double vel = getFilteredVelocity(internalVelocityTarget);
				double acc = axisInterface->getAccelerationLimit();
				double minPos = axisInterface->getLowerPositionLimit();
				double maxPos = axisInterface->getUpperPositionLimit();
				motionProfile.matchVelocityAndRespectPositionLimits(profileTimeDelta_seconds, vel, acc, minPos, maxPos, acc);
			}else{
				
				double acc = axisInterface->getAccelerationLimit();
				double vel = getFilteredVelocity(internalVelocityTarget);
				
				switch(limitSignalType){
					case SIGNAL_AT_LOWER_AND_UPPER_LIMITS:
						if((axisInterface->isAtLowerLimit() && (internalVelocityTarget < 0 || motionProfile.getVelocity() < 0.0)) ||
							(axisInterface->isAtUpperLimit() && (internalVelocityTarget > 0 || motionProfile.getVelocity() > 0.0))){
							motionProfile.matchVelocity(profileTimeDelta_seconds, 0.0, acc);
							break;
						}
					case LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS:
						limitAndSlowdownVelocityControl(internalVelocityTarget);
						break;
					default:
						motionProfile.matchVelocity(profileTimeDelta_seconds, internalVelocityTarget, acc);
						break;
				}
				
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
			double accLim = std::abs(axisInterface->getAccelerationLimit());
			double accTar = std::clamp(axisInterface->processData.accelerationTarget, -accLim, accLim);
			double velTar = getFilteredVelocity(axisInterface->processData.velocityTarget);
			switch(limitSignalType){
				case SIGNAL_AT_LOWER_AND_UPPER_LIMITS:
					if((axisInterface->isAtLowerLimit() && (velTar < 0 || motionProfile.getVelocity() < 0)) ||
					   (axisInterface->isAtUpperLimit() && (velTar > 0 || motionProfile.getVelocity() > 0))){
						motionProfile.matchVelocity(profileTimeDelta_seconds, 0.0, accTar);
						break;
					}
				case LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS:
					limitAndSlowdownVelocityControl(velTar);
					break;
				default:
					motionProfile.matchVelocity(profileTimeDelta_seconds, velTar, accTar);
					break;
			}
			}break;
		case InternalControlMode::NO_CONTROL:
			motionProfile.setPosition(axisInterface->getPositionActual());
			motionProfile.setVelocity(axisInterface->getVelocityActual());
			motionProfile.setAcceleration(0.0);
			for(auto actuatorMapping : actuatorMappings){
				if(!actuatorMapping->isActuatorConnected()) continue;
				auto actuatorInterface = actuatorMapping->getActuatorInterface();
				if(selectedPositionFeedbackMapping && selectedPositionFeedbackMapping == actuatorMapping){
					actuatorMapping->actuatorPositionOffset = 0.0;
				}
				actuatorMapping->actuatorPositionOffset = actuatorInterface->getPosition() - axisInterface->getPositionActual() * actuatorMapping->deviceUnitsPerAxisUnits->value;
			}
			break;
	}
	
	//control loop update (depends on axis control mode)
	double velocityCommand;
	auto updatePositionControlLoop = [&](){
		if(b_isOverPositionErrorTreshold){
			velocityCommand = motionProfile.getVelocity();
		}
		else if(motionProfile.getVelocity() == 0.0 && std::abs(positionFollowingError) < std::abs(positionLoop_minError->value)){
			velocityCommand = 0.0;
		}
		else{
			velocityCommand = motionProfile.getVelocity() * positionLoop_velocityFeedForward->value * 0.01 + positionFollowingError * positionLoop_proportionalGain->value;
		}
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
	
	*brakeControlSignal = velocityCommand != 0.0;
	
	//send commands to actuators
	for(auto actuatorMapping : actuatorMappings){
		if(!actuatorMapping->isActuatorConnected()) continue;
		auto actuatorInterface = actuatorMapping->getActuatorInterface();
		switch(actuatorMapping->controlMode){
			case ActuatorMapping::POSITION_CONTROL:{
				double actuatorPosition = (motionProfile.getPosition() * actuatorMapping->deviceUnitsPerAxisUnits->value) + actuatorMapping->actuatorPositionOffset;
				actuatorInterface->setPositionTarget(actuatorPosition);
				}break;
			case ActuatorMapping::VELOCITY_CONTROL:{
				double actuatorVelocity = velocityCommand * actuatorMapping->deviceUnitsPerAxisUnits->value;
				actuatorInterface->setVelocityTarget(actuatorVelocity);
				}break;
			case ActuatorMapping::FORCE_CONTROL:
				//not implemented
				break;
			case ActuatorMapping::NO_CONTROL:
				break;
			default:
				break;
		}
	}
	
}

bool AxisNode::needsOutputProcess(){ return !axisPin->isConnected(); }

double AxisNode::getFilteredVelocity(double requestedVelocity){
	return std::clamp(requestedVelocity, -std::abs(velocityLimit->value), std::abs(velocityLimit->value));
}
	
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
	motionProfile.moveToPositionWithVelocity(profileTime_seconds, position, homingVelocityFine->value, axisInterface->getAccelerationLimit());
	internalControlMode = InternalControlMode::HOMING_POSITION_INTERPOLATION;
}

void AxisNode::overrideCurrentPosition(double newPosition){
	motionProfile.setPosition(newPosition);
	for(auto actuatorMapping : actuatorMappings){
		if(!actuatorMapping->isActuatorConnected()) continue;
		auto actuator = actuatorMapping->getActuatorInterface();
		if(actuatorMapping == selectedPositionFeedbackMapping) continue;
		actuatorMapping->actuatorPositionOffset = actuator->getPosition() - newPosition * actuatorMapping->deviceUnitsPerAxisUnits->value;
	}
}



 
void AxisNode::updateFeedbackRatioToMatchPosition(){
	b_shouldUpdateFeedbackRatio = true;
}

void AxisNode::onFeedbackRatioUpdate(){
	if(motionProfile.getVelocity() != 0.0)
		return Logger::error("[{}] Could not update feedback ratio while axis is moving", getName());
	if(selectedPositionFeedbackMapping == nullptr || !selectedPositionFeedbackMapping->isFeedbackConnected())
		return Logger::error("[{}] Could not update feedback ratio, there is no position feedback device", getName());
	if(newPositionForFeedbackRatio == 0.0)
		return Logger::error("[{}] Could not update feedback ratio, the target position is zero", getName());
	
	auto positionFeedbackInterface = selectedPositionFeedbackMapping->getFeedbackInterface();
	double newPositionFeedbackRatio = positionFeedbackInterface->getPosition() / newPositionForFeedbackRatio;
	double oldPositionFeedbackRatio = selectedPositionFeedbackMapping->deviceUnitsPerAxisUnits->value;
	selectedPositionFeedbackMapping->deviceUnitsPerAxisUnits->overwriteWithHistory(newPositionFeedbackRatio);
	
	if(selectedVelocityFeedbackMapping == selectedPositionFeedbackMapping)
		selectedVelocityFeedbackMapping->deviceUnitsPerAxisUnits->overwriteWithHistory(newPositionFeedbackRatio);
	else{
		double newVelocityFeedbackRatio = selectedVelocityFeedbackMapping->deviceUnitsPerAxisUnits->value * newPositionFeedbackRatio / oldPositionFeedbackRatio;
		selectedVelocityFeedbackMapping->deviceUnitsPerAxisUnits->overwriteWithHistory(newVelocityFeedbackRatio);
	}
	
	for(auto actuatorMapping : actuatorMappings){
		if(!actuatorMapping->isActuatorConnected()) continue;
		//if the actuator is the same as the position feedback interface, update its ratio like the feedback interface
		if(actuatorMapping == selectedPositionFeedbackMapping){
			actuatorMapping->deviceUnitsPerAxisUnits->overwriteWithHistory(newPositionFeedbackRatio);
		}
		//else adjust the ratio of other actuators by the same amount the feedback ratio was adjusted
		else{
			double newActuatorRatio = actuatorMapping->deviceUnitsPerAxisUnits->value * newPositionFeedbackRatio / oldPositionFeedbackRatio;
			actuatorMapping->deviceUnitsPerAxisUnits->overwriteWithHistory(newActuatorRatio);
		}
	}
	
	motionProfile.setPosition(newPositionForFeedbackRatio);
	Logger::info("[{}] Position feedback ratio override succeeded !", getName());
	
}
