#include <pch.h>

/*
#include "Axis.h"

#include "Motion/Interfaces.h"

namespace Motion{

void Axis::homingControl(){
	
	switch (positionReferenceSignal) {
			
		case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
			
			
			switch (homingStep) {
				case HomingStep::NOT_STARTED:
					homingStep = HomingStep::SEARCHING_LOW_LIMIT_COARSE;
					Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
					break;
				case HomingStep::SEARCHING_LOW_LIMIT_COARSE:
					setVelocityTarget(-homingVelocityCoarse);
					//we don't check the signal rising edge but only the high condition
					//this way it triggers if we are already at the limit signal when homing was started
					if (*lowLimitSignal){
						homingStep = HomingStep::FOUND_LOW_LIMIT_COARSE;
						setVelocityTarget(0.0);
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
					}
					break;
				case HomingStep::FOUND_LOW_LIMIT_COARSE:
					if (!isMoving()) {
						setVelocityTarget(homingVelocityFine);
						homingStep = HomingStep::SEARCHING_LOW_LIMIT_FINE;
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
					}
					break;
				case HomingStep::SEARCHING_LOW_LIMIT_FINE:
					//here we must check the falling edge
					//since we may have overshot the range of the limit signal
					if (previousLowLimitSignal && !*lowLimitSignal) {
						homingStep = HomingStep::FOUND_LOW_LIMIT_FINE;
						setVelocityTarget(0.0);
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
					}
					break;
				case HomingStep::FOUND_LOW_LIMIT_FINE:
					if (!isMoving()) {
						auto servoActuator = getServoActuatorDevice();
						servoActuator->overridePosition(0.0);;
						homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						Logger::info("Homing Axis {} : Waiting For Encoder Position Override", getName());
					}
					break;
				case HomingStep::RESETTING_POSITION_FEEDBACK:
					if(!getServoActuatorDevice()->isBusyOverridingPosition()){
						motionProfile.setPosition(servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getPosition()));
						onHomingSuccess();
					}
					break;
				default: break;
			}
			break;
			
			
		case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			
			
			if (homingDirection == HomingDirection::NEGATIVE) {
				switch (homingStep) {
					case HomingStep::NOT_STARTED:
						homingStep = HomingStep::SEARCHING_LOW_LIMIT_COARSE;
						setVelocityTarget(-homingVelocityCoarse);
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						break;
					case HomingStep::SEARCHING_LOW_LIMIT_COARSE:
						//we don't check the signal rising edge but only the high condition
						//this way it triggers if we are already at the limit signal when homing was started
						if (*highLimitSignal) {
							homingError = HomingError::TRIGGERED_WRONG_LIMIT_SIGNAL;
							onHomingError();
						}
						else if (*lowLimitSignal) {
							homingStep = HomingStep::FOUND_LOW_LIMIT_COARSE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_LOW_LIMIT_COARSE:
						if (!isMoving()) {
							homingStep = HomingStep::SEARCHING_LOW_LIMIT_FINE;
							setVelocityTarget(homingVelocityFine);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SEARCHING_LOW_LIMIT_FINE:
						//here we have to check the falling edge since we might have overshot the signal
						if (previousLowLimitSignal && !*lowLimitSignal) {
							homingStep = HomingStep::FOUND_LOW_LIMIT_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_LOW_LIMIT_FINE:
						if (!isMoving()) {
							getServoActuatorDevice()->overridePosition(0.0);
							homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
							Logger::info("Homing Axis {} : Overriding Position Feedback", getName());
						}
						break;
					case HomingStep::RESETTING_POSITION_FEEDBACK:
						//if the servo actuator can hard reset its encoder, check if we are done resetting
						if(!getServoActuatorDevice()->isBusyOverridingPosition()) {
							motionProfile.setPosition(servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getPosition()));
							homingStep = HomingStep::SEARCHING_HIGH_LIMIT_COARSE;
							setVelocityTarget(homingVelocityCoarse);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SEARCHING_HIGH_LIMIT_COARSE:
						//don't check the rising edge
						if (*lowLimitSignal) {
							homingError = HomingError::TRIGGERED_WRONG_LIMIT_SIGNAL;
							onHomingError();
						}
						else if (*highLimitSignal){
							setVelocityTarget(0.0);
							homingStep = HomingStep::FOUND_HIGH_LIMIT_COARSE;
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_HIGH_LIMIT_COARSE:
						if (!isMoving()) {
							setVelocityTarget(-homingVelocityFine);
							homingStep = HomingStep::SEARCHING_HIGH_LIMIT_FINE;
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SEARCHING_HIGH_LIMIT_FINE:
						//check the falling edge since we might have overshot the sensor
						if (previousHighLimitSignal && !*highLimitSignal) {
							homingStep = HomingStep::FOUND_HIGH_LIMIT_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_HIGH_LIMIT_FINE:
						if (!isMoving()) {
							homingStep = HomingStep::SETTING_HIGH_LIMIT;
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SETTING_HIGH_LIMIT:
						setCurrentPositionAsPositiveLimit();
						onHomingSuccess();
						break;
					default:
						break;
				}
			}
			else if (homingDirection == HomingDirection::POSITIVE) {
				switch (homingStep) {
					case HomingStep::NOT_STARTED:
						homingStep = HomingStep::SEARCHING_HIGH_LIMIT_COARSE;
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						break;
					case HomingStep::SEARCHING_HIGH_LIMIT_COARSE:
						setVelocityTarget(homingVelocityCoarse);
						if (*lowLimitSignal) {
							homingError = HomingError::TRIGGERED_WRONG_LIMIT_SIGNAL;
							onHomingError();
						}
						//dont check the rising edge since we might already have triggered the sensor at homing start
						else if (*highLimitSignal) {
							homingStep = HomingStep::FOUND_HIGH_LIMIT_COARSE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_HIGH_LIMIT_COARSE:
						if (!isMoving()) {
							homingStep = HomingStep::SEARCHING_HIGH_LIMIT_FINE;
							setVelocityTarget(-homingVelocityFine);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SEARCHING_HIGH_LIMIT_FINE:
						//check the falling edge since we might have overshot the signal
						if (previousHighLimitSignal && !*highLimitSignal) {
							homingStep = HomingStep::FOUND_HIGH_LIMIT_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_HIGH_LIMIT_FINE:
						if (!isMoving()) {
							homingStep = HomingStep::SETTING_HIGH_LIMIT;
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SETTING_HIGH_LIMIT:
						setCurrentPosition(0.0); //set a zero reference now, we will use it later
						homingStep = HomingStep::SEARCHING_LOW_LIMIT_COARSE;
						setVelocityTarget(-homingVelocityCoarse);
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						break;
					case HomingStep::SEARCHING_LOW_LIMIT_COARSE:
						//don't check the rising edge
						if (*highLimitSignal) {
							homingError = HomingError::TRIGGERED_WRONG_LIMIT_SIGNAL;
							onHomingError();
						}
						else if (*lowLimitSignal) {
							homingStep = HomingStep::FOUND_LOW_LIMIT_COARSE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_LOW_LIMIT_COARSE:
						if (!isMoving()) {
							homingStep = HomingStep::SEARCHING_LOW_LIMIT_FINE;
							setVelocityTarget(homingVelocityFine);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SEARCHING_LOW_LIMIT_FINE:
						if (previousLowLimitSignal && !*lowLimitSignal){
							homingStep = HomingStep::FOUND_LOW_LIMIT_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_LOW_LIMIT_FINE:
						if (!isMoving()) {
							highPositionLimit = std::abs(*actualPositionValue);
							getServoActuatorDevice()->overridePosition(0.0);
							Logger::info("Homing Axis {} : Hard Reset Position Feedback", getName());
							homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::RESETTING_POSITION_FEEDBACK:
						if(!getServoActuatorDevice()->isBusyOverridingPosition()){
							motionProfile.setPosition(servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getPosition()));
							onHomingSuccess();
						}
						break;
					default:
						break;
				}
			}
			break;
			
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			
			
			if (homingDirection == HomingDirection::POSITIVE) {
				switch (homingStep) {
					case HomingStep::NOT_STARTED:
						homingStep = HomingStep::SEARCHING_REFERENCE_FROM_BELOW_COARSE;
						setVelocityTarget(homingVelocityCoarse);
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_COARSE:
						//Don't check rising edge since the signal may have been triggered already when homing was started
						if (*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_BELOW_COARSE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_BELOW_COARSE:
						if (!isMoving()) {
							homingStep = HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE;
							setVelocityTarget(-homingVelocityFine);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE:
						//check for falling edge since the signal may have been overshot
						if (previousReferenceSignal && !*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE:
						if (!isMoving()) {
							
							switch(signalApproach->value){
								case SignalApproach::FIND_SIGNAL_EDGE:
								{
									auto servoActuator = getServoActuatorDevice();
									servoActuator->overridePosition(0.0);
									motionProfile.setPosition(0.0);
									homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
									Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
									Logger::info("Homing Axis {} : Waiting For Encoder Hard Reset", getName());
									homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
								}
									break;
									
								case SignalApproach::FIND_SIGNAL_CENTER:
									setCurrentPosition(0.0);
									homingStep = HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE;
									setVelocityTarget(homingVelocityFine);
									Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
									break;
							}
							
						}
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE:
						//check for falling edge since the signal might have been overshot
						if (previousReferenceSignal && !*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE:
						if (!isMoving()) {
							//TODO: need to return to center and hard reset encoder if possible
							setCurrentPosition(*actualPositionValue / 2.0);
							onHomingSuccess();
						}
						break;
					default:
						break;
				}
			}
			else if (homingDirection == HomingDirection::NEGATIVE) {
				switch (homingStep) {
					case HomingStep::NOT_STARTED:
						homingStep = HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_COARSE;
						setVelocityTarget(-homingVelocityCoarse);
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_COARSE:
						//dont check the rising edge since we might already have triggered the signal at homing start
						if (*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_ABOVE_COARSE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_ABOVE_COARSE:
						if (!isMoving()) {
							homingStep = HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE;
							setVelocityTarget(homingVelocityFine);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE:
						//check the falling edge since we might have overshot the signal
						if (previousReferenceSignal && !*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE:
						if (!isMoving()) {
							setCurrentPosition(0.0);
							
							
							switch(signalApproach->value){
								case SignalApproach::FIND_SIGNAL_EDGE:
									
								{
									
									
									
									if(useFeedbackDevice_Param->value){
										auto feedbackDevice = getFeedbackDevice();
										
										feedbackDevice->overridePosition(0.0);
										motionProfile.setPosition(0.0);
										homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
										Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
										Logger::info("Homing Axis {} : Waiting For hard Reset of {}", getName(), feedbackDevice->getName());
										
									}else{
										auto servoActuator = getServoActuatorDevice();
										//if the servo actuator can hard reset its encoder, do it and wait for the procedure to finish
										servoActuator->overridePosition(0.0);
										motionProfile.setPosition(0.0);
										homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
										Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
										Logger::info("Homing Axis {} : Waiting For Encoder Hard Reset", getName());
										
										homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
									}
									
									
									
									
								}
									
									break;
								case SignalApproach::FIND_SIGNAL_CENTER:
									homingStep = HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE;
									setVelocityTarget(-homingVelocityFine);
									Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
									break;
							}
							
						}
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE:
						//check the falling edge since we might have overshot the signal
						if (previousReferenceSignal && !*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE:
						setVelocityTarget(0.0);
						if(!isMoving()){
							moveToPositionWithVelocity(*actualPositionValue / 2.0, homingVelocityCoarse);
							homingStep = HomingStep::MOVING_TO_REFERENCE_MIDDLE;
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::MOVING_TO_REFERENCE_MIDDLE:
						if(motionProfile.getInterpolationProgress(profileTime_seconds) >= 1.0){
							auto servoActuator = getServoActuatorDevice();
							//if the servo actuator can hard reset its encoder, do it and wait for the procedure to finish
							
							servoActuator->overridePosition(0.0);
							motionProfile.setPosition(0.0);
							homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
							Logger::info("Homing Axis {} : Waiting For Encoder Hard Reset", getName());
							
							
							homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
						}
						break;
					case HomingStep::RESETTING_POSITION_FEEDBACK:
						motionProfile.setPosition(0.0);
						
						
						
						if(useFeedbackDevice_Param->value){
							auto feedbackDevice = getFeedbackDevice();
							
							if(!feedbackDevice->isBusyOverridingPosition()){
								double feedbackPosition = feedbackUnitsToAxisUnits(feedbackDevice->getPosition());
								motionProfile.setPosition(feedbackPosition);
								onHomingSuccess();
							}
							
						}else{
							
							if(!getServoActuatorDevice()->isBusyOverridingPosition()) {
								motionProfile.setPosition(servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getPosition()));
								onHomingSuccess();
							}
						}
						
						
						break;
					default:
						break;
				}
			}
			break;
			
		default:
			homingError = HomingError::HOMING_NOT_SUPORTED;
			onHomingError(); //homing should not be started for modes that don't support homing
			break;
	}
	
}

}

 */

