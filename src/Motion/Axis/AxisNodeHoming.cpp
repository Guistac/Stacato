#include <pch.h>

#include "AxisNode.h"


bool AxisNode::overridePositionFeedback(double newValue){
	if(selectedPositionFeedbackMapping != nullptr && selectedPositionFeedbackMapping->isFeedbackConnected()){
		auto feedbackInterface = selectedPositionFeedbackMapping->getFeedbackInterface();
		feedbackInterface->overridePosition(0.0);
		overrideCurrentPosition(0.0);
		return true;
	}else{
		Logger::error("[{}] Could not reset position feedback, there is no position feedback device", getName());
		return false;
	}
}

bool AxisNode::isBusyOverridingPositionFeedback(){
	return selectedPositionFeedbackMapping != nullptr
	&& selectedPositionFeedbackMapping->isFeedbackConnected()
	&& selectedPositionFeedbackMapping->getFeedbackInterface()->isBusyOverridingPosition();
}

bool AxisNode::didPositionFeedbackOverrideSucceed(){
	return selectedPositionFeedbackMapping != nullptr
	&& selectedPositionFeedbackMapping->isFeedbackConnected()
	&& selectedPositionFeedbackMapping->getFeedbackInterface()->didPositionOverrideSucceed();
}



void AxisNode::homingControl(){
	switch(limitSignalType){
			
		case LimitSignalType::NONE:
			homingRoutine_HomingOnCurrentPosition();
			break;
			
		case LimitSignalType::SIGNAL_AT_LOWER_LIMIT:
			homingRoutine_HomeToLowerLimitSignal();
			break;
			
		case LimitSignalType::SIGNAL_AT_LOWER_AND_UPPER_LIMITS:
			switch(homingDirection){
				case NEGATIVE:
					homingRoutine_HomeToLowerThenUpperLimitSignal();
					break;
				case POSITIVE:
					homingRoutine_HomeToUpperThenLowerLimitSignal();
					break;
			}
			break;
			
		case LimitSignalType::SIGNAL_AT_ORIGIN:
			switch(signalApproachMethod){
				case SignalApproachMethod::FIND_SIGNAL_EDGE:
					switch (homingDirection) {
						case NEGATIVE:
							homingRoutine_HomingOnReferenceSignalEdgeNegative();
							break;
						case POSITIVE:
							homingRoutine_HomingOnReferenceSignalEdgePositive();
							break;
					}
					break;
				case SignalApproachMethod::FIND_SIGNAL_CENTER:
					homingRoutine_HomingOnReferenceSignalCenter();
					break;
			}
			break;
			
		case LimitSignalType::LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS:
			homingStep = HomingStep::FAILED;
			break;
	}
	
	if(homingStep == HomingStep::FAILED){
		axisInterface->processData.b_isHoming = false;
		axisInterface->processData.b_didHomingSucceed = false;
		setManualVelocityTarget(0.0);
		Logger::error("[{}] Homing Failed", getName());
	}else if(homingStep == HomingStep::FINISHED){
		axisInterface->processData.b_isHoming = false;
		axisInterface->processData.b_didHomingSucceed = true;
		setManualVelocityTarget(0.0);
		motionProfile.setPosition(0.0);
		Logger::info("[{}] Homing finished successfully", getName());
	}
	
}

void AxisNode::homingRoutine_HomingOnCurrentPosition(){
	switch(homingStep){
			
		case HomingStep::NOT_STARTED:
			homingStep = HomingStep::FOUND_LOW_LIMIT;
			setHomingVelocityTarget(0.0);
			break;
			
		case HomingStep::FOUND_LOW_LIMIT:
			if(motionProfile.getVelocity() == 0.0 && axisInterface->getVelocityActual() <= 0){
				if(selectedPositionFeedbackMapping && selectedPositionFeedbackMapping->isFeedbackConnected()){
					auto feedback = selectedPositionFeedbackMapping->getFeedbackInterface();
					feedback->overridePosition(0.0);
					overrideCurrentPosition(0.0);
					homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
				}else{
					Logger::error("[{}] Could not reset position feedback, there is no position feedback device", getName());
					homingStep = HomingStep::FAILED;
					break;
				}
			}
			break;
			
		case HomingStep::RESETTING_POSITION_FEEDBACK:
			overrideCurrentPosition(0.0);
			if(selectedPositionFeedbackMapping && selectedPositionFeedbackMapping->isFeedbackConnected()
			   && !selectedPositionFeedbackMapping->getFeedbackInterface()->isBusyOverridingPosition()){
				if(selectedPositionFeedbackMapping->getFeedbackInterface()->didPositionOverrideSucceed()){
					homingStep = HomingStep::FINISHING;
				}else{
					homingStep = HomingStep::FAILED;
				}
			}
			break;
			
			
		case HomingStep::FINISHING:
			homingStep = HomingStep::FINISHED;
			break;
			
		default:
			homingStep = HomingStep::FAILED;
			break;
	}
}

void AxisNode::homingRoutine_HomeToLowerLimitSignal(){
	switch(homingStep){
			
		case HomingStep::NOT_STARTED:
			setHomingVelocityTarget(-std::abs(homingVelocityCoarse->value));
			homingStep = HomingStep::SEARCHING_LOW_LIMIT_COARSE;
			break;
			
		case HomingStep::SEARCHING_LOW_LIMIT_COARSE:
			if(*lowerLimitSignal){
				setHomingVelocityTarget(0.0);
				homingStep = HomingStep::FOUND_LOW_LIMIT_COARSE;
			}
			break;
			
		case HomingStep::FOUND_LOW_LIMIT_COARSE:
			if(motionProfile.getVelocity() == 0.0 && axisInterface->getVelocityActual() >= 0.0){
				setHomingVelocityTarget(std::abs(homingVelocityFine->value));
				homingStep = HomingStep::SEARCHING_LOW_LIMIT_FINE;
			}
			break;
			
		case HomingStep::SEARCHING_LOW_LIMIT_FINE:
			if(previousLowerLimitSignal && !*lowerLimitSignal){
				setHomingVelocityTarget(0.0);
				homingStep = HomingStep::FOUND_LOW_LIMIT;
			}
			break;
			
		case HomingStep::FOUND_LOW_LIMIT:
			if(motionProfile.getVelocity() == 0.0 && axisInterface->getVelocityActual() <= 0){
				if(overridePositionFeedback(0.0)){
					homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
				}else{
					homingStep = HomingStep::FAILED;
				}
			}
			break;
			
		case HomingStep::RESETTING_POSITION_FEEDBACK:
			overrideCurrentPosition(0.0);
			if(!isBusyOverridingPositionFeedback()){
				if(didPositionFeedbackOverrideSucceed()){
					homingStep = HomingStep::FINISHING;
				}else{
					homingStep = HomingStep::FAILED;
				}
			}
			break;
		
		case HomingStep::FINISHING:
			homingStep = HomingStep::FINISHED;
			break;
			
		default:
			homingStep = HomingStep::FAILED;
			break;
	}
}

void AxisNode::homingRoutine_HomeToLowerThenUpperLimitSignal(){
	
	switch(homingStep){
			
		case HomingStep::NOT_STARTED:
			setHomingVelocityTarget(-std::abs(homingVelocityCoarse->value));
			homingStep = HomingStep::SEARCHING_LOW_LIMIT_COARSE;
			break;
			
		case HomingStep::SEARCHING_LOW_LIMIT_COARSE:
			if(*lowerLimitSignal){
				setHomingVelocityTarget(0.0);
				homingStep = HomingStep::FOUND_LOW_LIMIT_COARSE;
			}
			break;
			
		case HomingStep::FOUND_LOW_LIMIT_COARSE:
			if(motionProfile.getVelocity() == 0.0 && axisInterface->getVelocityActual() >= 0.0){
				setHomingVelocityTarget(std::abs(homingVelocityFine->value));
				homingStep = HomingStep::SEARCHING_LOW_LIMIT_FINE;
			}
			break;
			
		case HomingStep::SEARCHING_LOW_LIMIT_FINE:
			if(previousLowerLimitSignal && !*lowerLimitSignal){
				setHomingVelocityTarget(0.0);
				homingStep = HomingStep::FOUND_LOW_LIMIT;
			}
			break;
			
		case HomingStep::FOUND_LOW_LIMIT:
			if(motionProfile.getVelocity() == 0.0 && axisInterface->getVelocityActual() <= 0){
				if(overridePositionFeedback(0.0)){
					homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
				}else{
					homingStep = HomingStep::FAILED;
				}
			}
			break;
			
		case HomingStep::RESETTING_POSITION_FEEDBACK:
			overrideCurrentPosition(0.0);
			if(!isBusyOverridingPositionFeedback()){
				if(didPositionFeedbackOverrideSucceed()){
					setHomingVelocityTarget(std::abs(homingVelocityCoarse->value));
					homingStep = HomingStep::SEARCHING_HIGH_LIMIT_COARSE;
				}else{
					homingStep = HomingStep::FAILED;
				}
			}
			break;
			
		case HomingStep::SEARCHING_HIGH_LIMIT_COARSE:
			if(*upperLimitSignal){
				setHomingVelocityTarget(0.0);
				homingStep = HomingStep::FOUND_HIGH_LIMIT_COARSE;
			}
			break;
			
		case HomingStep::FOUND_HIGH_LIMIT_COARSE:
			if(motionProfile.getVelocity() == 0.0 && axisInterface->getVelocityActual() <= 0.0){
				setHomingVelocityTarget(-std::abs(homingVelocityFine->value));
				homingStep = HomingStep::SEARCHING_HIGH_LIMIT_FINE;
			}
			break;
			
		case HomingStep::SEARCHING_HIGH_LIMIT_FINE:
			if(previousUpperLimitSignal && !*upperLimitSignal){
				setHomingVelocityTarget(0.0);
				homingStep = HomingStep::FOUND_HIGH_LIMIT;
			}
			break;
			
		case HomingStep::FOUND_HIGH_LIMIT:
			if(motionProfile.getVelocity() == 0.0 && axisInterface->getVelocityActual() >= 0){
				upperPositionLimit->overwrite(axisInterface->getPositionActual());
				enableUpperPositionLimit->overwrite(true);
				homingStep = HomingStep::FINISHED;
			}
			break;
			
		default: break;
	}
}

void AxisNode::homingRoutine_HomeToUpperThenLowerLimitSignal(){
	homingStep = HomingStep::FAILED;
	switch(homingStep){
			
		case HomingStep::NOT_STARTED:
			
		case HomingStep::SEARCHING_HIGH_LIMIT_COARSE:
			
		case HomingStep::SEARCHING_HIGH_LIMIT_FINE:
			
		case HomingStep::FOUND_HIGH_LIMIT:
			
		case HomingStep::SEARCHING_LOW_LIMIT_COARSE:
			
		case HomingStep::SEARCHING_LOW_LIMIT_FINE:
			
		case HomingStep::FOUND_LOW_LIMIT:
			
		case HomingStep::RESETTING_POSITION_FEEDBACK:
			
		default: break;
	}
}

void AxisNode::homingRoutine_HomingOnReferenceSignalCenter(){
	switch(homingStep){
			
		case HomingStep::NOT_STARTED:
			switch(homingDirection){
				case NEGATIVE:
					setHomingVelocityTarget(-std::abs(homingVelocityCoarse->value));
					homingStep = HomingStep::SEARCHING_ORIGIN_UPPER_EDGE_COARSE;
					break;
				case POSITIVE:
					setHomingVelocityTarget(std::abs(homingVelocityCoarse->value));
					homingStep = HomingStep::SEARCHING_ORIGIN_LOWER_EDGE_COARSE;
					break;
			}
			break;
			
			
			
			
		case HomingStep::SEARCHING_ORIGIN_UPPER_EDGE_COARSE:
			if(*referenceSignal){
				setHomingVelocityTarget(0.0);
				homingStep = HomingStep::FOUND_ORIGIN_UPPER_EDGE_COARSE;
			}
			break;
			
		case HomingStep::FOUND_ORIGIN_UPPER_EDGE_COARSE:
			if(motionProfile.getVelocity() == 0.0 && axisInterface->getVelocityActual() >= 0.0){
				setHomingVelocityTarget(std::abs(homingVelocityFine->value));
				homingStep = HomingStep::SEARCHING_ORIGIN_UPPER_EDGE_FINE;
			}
			break;
			
		case HomingStep::SEARCHING_ORIGIN_UPPER_EDGE_FINE:
			if(!*referenceSignal && previousReferenceSignal){
				setHomingVelocityTarget(0.0);
				homingStep = HomingStep::FOUND_ORIGIN_UPPER_EDGE;
			}
			break;
			
			
			
			
		case HomingStep::SEARCHING_ORIGIN_LOWER_EDGE_COARSE:
			if(*referenceSignal){
				setHomingVelocityTarget(0.0);
				homingStep = HomingStep::FOUND_ORIGIN_LOWER_EDGE_COARSE;
			}
			break;
			
		case HomingStep::FOUND_ORIGIN_LOWER_EDGE_COARSE:
			if(motionProfile.getVelocity() == 0.0 && axisInterface->getVelocityActual() <= 0.0){
				setHomingVelocityTarget(-std::abs(homingVelocityFine->value));
				homingStep = HomingStep::SEARCHING_ORIGIN_LOWER_EDGE_FINE;
			}
			break;
			
		case HomingStep::SEARCHING_ORIGIN_LOWER_EDGE_FINE:
			if(!*referenceSignal && previousReferenceSignal){
				setHomingVelocityTarget(0.0);
				homingStep = HomingStep::FOUND_ORIGIN_LOWER_EDGE;
			}
			break;
			
			
			
			
		case HomingStep::FOUND_ORIGIN_UPPER_EDGE:
			if(motionProfile.getVelocity() == 0.0 && axisInterface->getVelocityActual() <= 0){
				//mark down upper edge position
				homingOriginUpperEdgePosition = axisInterface->getPositionActual();
				switch(homingDirection){
					case NEGATIVE:
						setHomingVelocityTarget(-std::abs(homingVelocityFine->value));
						homingStep = HomingStep::SEARCHING_ORIGIN_LOWER_EDGE_FINE;
						break;
					case POSITIVE:
						//compute center, then go to center and zero
						moveToHomingPositionTarget(homingOriginLowerEdgePosition + (homingOriginUpperEdgePosition - homingOriginLowerEdgePosition) * 0.5);
						homingStep = HomingStep::MOVING_TO_ORIGIN_CENTER;
						break;
				}
			}
			break;
			
		case HomingStep::FOUND_ORIGIN_LOWER_EDGE:
			if(motionProfile.getVelocity() == 0.0 && axisInterface->getVelocityActual() >= 0){
				//mark down lower edge position
				homingOriginLowerEdgePosition = axisInterface->getPositionActual();
				switch(homingDirection){
					case NEGATIVE:
						//compute center, then go to center and zero
						moveToHomingPositionTarget(homingOriginLowerEdgePosition + (homingOriginUpperEdgePosition - homingOriginLowerEdgePosition) * 0.5);
						homingStep = HomingStep::MOVING_TO_ORIGIN_CENTER;
						break;
					case POSITIVE:
						setHomingVelocityTarget(std::abs(homingVelocityFine->value));
						homingStep = HomingStep::SEARCHING_ORIGIN_UPPER_EDGE_FINE;
						break;
				}
			}
			break;
			
			
			
		case HomingStep::MOVING_TO_ORIGIN_CENTER:
			if(motionProfile.getInterpolationTarget() == motionProfile.getPosition() && motionProfile.getVelocity() == 0.0){
				setHomingVelocityTarget(0.0);
				auto feedback = selectedPositionFeedbackMapping->getFeedbackInterface();
				feedback->overridePosition(0.0);
				overrideCurrentPosition(0.0);
				homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
			}
			break;
			
		case HomingStep::RESETTING_POSITION_FEEDBACK:
			overrideCurrentPosition(0.0);
			if(selectedPositionFeedbackMapping->getFeedbackInterface()->didPositionOverrideSucceed()){
				homingStep = HomingStep::FINISHING;
			}
			break;
			
		case HomingStep::FINISHING:
			homingStep = HomingStep::FINISHED;
			break;
			
		default: break;
	}
}

void AxisNode::homingRoutine_HomingOnReferenceSignalEdgeNegative(){
	switch(homingStep){
			
		case HomingStep::NOT_STARTED:
			setHomingVelocityTarget(-std::abs(homingVelocityCoarse->value));
			homingStep = HomingStep::SEARCHING_ORIGIN_UPPER_EDGE_COARSE;
			break;
			
		case HomingStep::SEARCHING_ORIGIN_UPPER_EDGE_COARSE:
			if(*lowerLimitSignal){
				setHomingVelocityTarget(0.0);
				homingStep = HomingStep::FOUND_ORIGIN_UPPER_EDGE_COARSE;
			}
			break;
			
		case HomingStep::FOUND_ORIGIN_UPPER_EDGE_COARSE:
			if(motionProfile.getVelocity() == 0.0 && axisInterface->getVelocityActual() >= 0.0){
				setHomingVelocityTarget(std::abs(homingVelocityFine->value));
				homingStep = HomingStep::SEARCHING_ORIGIN_UPPER_EDGE_FINE;
			}
			break;
			
		case HomingStep::SEARCHING_ORIGIN_UPPER_EDGE_FINE:
			if(previousLowerLimitSignal && !*lowerLimitSignal){
				setHomingVelocityTarget(0.0);
				homingStep = HomingStep::FOUND_ORIGIN_UPPER_EDGE;
			}
			break;
			
		case HomingStep::FOUND_ORIGIN_UPPER_EDGE:
			if(motionProfile.getVelocity() == 0.0 && axisInterface->getVelocityActual() <= 0){
				if(overridePositionFeedback(0.0)){
					homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
				}else{
					homingStep = HomingStep::FAILED;
				}
			}
			break;
			
		case HomingStep::RESETTING_POSITION_FEEDBACK:
			overrideCurrentPosition(0.0);
			if(!isBusyOverridingPositionFeedback()){
				if(didPositionFeedbackOverrideSucceed()){
					homingStep = HomingStep::FINISHING;
				}else{
					homingStep = HomingStep::FAILED;
				}
			}
			break;
		
		case HomingStep::FINISHING:
			homingStep = HomingStep::FINISHED;
			break;
			
		default:
			homingStep = HomingStep::FAILED;
			break;
	}
	
}

void AxisNode::homingRoutine_HomingOnReferenceSignalEdgePositive(){
	
	switch(homingStep){
			
		case HomingStep::NOT_STARTED:
			setHomingVelocityTarget(std::abs(homingVelocityCoarse->value));
			homingStep = HomingStep::SEARCHING_ORIGIN_LOWER_EDGE_COARSE;
			break;
			
		case HomingStep::SEARCHING_ORIGIN_LOWER_EDGE_COARSE:
			if(*lowerLimitSignal){
				setHomingVelocityTarget(0.0);
				homingStep = HomingStep::FOUND_ORIGIN_LOWER_EDGE_COARSE;
			}
			break;
			
		case HomingStep::FOUND_ORIGIN_LOWER_EDGE_COARSE:
			if(motionProfile.getVelocity() == 0.0 && axisInterface->getVelocityActual() >= 0.0){
				setHomingVelocityTarget(-std::abs(homingVelocityFine->value));
				homingStep = HomingStep::SEARCHING_ORIGIN_LOWER_EDGE_FINE;
			}
			break;
			
		case HomingStep::SEARCHING_ORIGIN_LOWER_EDGE_FINE:
			if(previousLowerLimitSignal && !*lowerLimitSignal){
				setHomingVelocityTarget(0.0);
				homingStep = HomingStep::FOUND_ORIGIN_LOWER_EDGE;
			}
			break;
			
		case HomingStep::FOUND_ORIGIN_LOWER_EDGE:
			if(motionProfile.getVelocity() == 0.0 && axisInterface->getVelocityActual() <= 0){
				if(overridePositionFeedback(0.0)){
					homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
				}else{
					homingStep = HomingStep::FAILED;
				}
			}
			break;
			
		case HomingStep::RESETTING_POSITION_FEEDBACK:
			overrideCurrentPosition(0.0);
			if(!isBusyOverridingPositionFeedback()){
				if(didPositionFeedbackOverrideSucceed()){
					homingStep = HomingStep::FINISHING;
				}else{
					homingStep = HomingStep::FAILED;
				}
			}
			break;
		
		case HomingStep::FINISHING:
			homingStep = HomingStep::FINISHED;
			break;
			
		default:
			homingStep = HomingStep::FAILED;
			break;
	}
	
}
