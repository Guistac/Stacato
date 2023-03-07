#include <pch.h>

#include "AxisNode.h"


void AxisNode::homingControl(){
	switch(limitsignalType){
			
		case LimitSignalType::NONE:
			//setHomingVelocityTarget(0.0);
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
					homingRoutine_HomingOnReferenceSignalEdge();
					break;
				case SignalApproachMethod::FIND_SIGNAL_CENTER:
					homingRoutine_HomingOnReferenceSignalCenter();
					break;
			}
			break;
			
		case LimitSignalType::LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS:
			//setHomingVelocityTarget(0.0);
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
		Logger::info("[{}] Homing finished successfully", getName());
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
			if(motionProfile.getVelocity() == 0.0){
				if(positionFeedbackMapping){
					auto feedback = positionFeedbackMapping->feedbackInterface;
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
			if(positionFeedbackMapping->feedbackInterface->didPositionOverrideSucceed()){
				homingStep = HomingStep::FINISHED;
			}
			break;
			
		default:
			homingStep = HomingStep::FAILED;
			break;
	}
}

void AxisNode::homingRoutine_HomeToLowerThenUpperLimitSignal(){
	switch(homingStep){
			
		case HomingStep::NOT_STARTED:
			
		case HomingStep::SEARCHING_LOW_LIMIT_COARSE:
			
		case HomingStep::SEARCHING_LOW_LIMIT_FINE:
			
		case HomingStep::FOUND_LOW_LIMIT:
			
		case HomingStep::RESETTING_POSITION_FEEDBACK:
			
		case HomingStep::SEARCHING_HIGH_LIMIT_COARSE:
			
		case HomingStep::SEARCHING_HIGH_LIMIT_FINE:
			
		case HomingStep::FOUND_HIGH_LIMIT:
			
		default: break;
	}
}

void AxisNode::homingRoutine_HomeToUpperThenLowerLimitSignal(){
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
					break;
				case POSITIVE:
					break;
			}
			break;
			
		case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_COARSE:
			
		case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE:
			
		case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_COARSE:
			
		case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE:
			
		case HomingStep::FOUND_REFERENCE_FROM_ABOVE:
			switch(homingDirection){
				case NEGATIVE:
					break;
				case POSITIVE:
					break;
			}
			break;
			
		case HomingStep::FOUND_REFERENCE_FROM_BELOW:
			switch(homingDirection){
				case NEGATIVE:
					break;
				case POSITIVE:
					break;
			}
			break;
			
		case HomingStep::MOVING_TO_REFERENCE_MIDDLE:
			
		case HomingStep::RESETTING_POSITION_FEEDBACK:
			
		default: break;
	}
}

void AxisNode::homingRoutine_HomingOnReferenceSignalEdge(){
	switch(homingStep){
			
		case HomingStep::NOT_STARTED:
			switch(homingDirection){
				case NEGATIVE:
					break;
				case POSITIVE:
					break;
			}
			break;
			
		case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_COARSE:
			
		case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE:
			
		case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_COARSE:
			
		case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE:
			
		case HomingStep::FOUND_REFERENCE_FROM_ABOVE:
			
		case HomingStep::FOUND_REFERENCE_FROM_BELOW:
			
		case HomingStep::MOVING_TO_REFERENCE_MIDDLE:
			
		case HomingStep::RESETTING_POSITION_FEEDBACK:
			
		default: break;
			
	}
}
