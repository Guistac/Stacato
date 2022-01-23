#pragma once

#include "Motion/Curve/Curve.h"

namespace Motion{

//profile generator for 2nd degree motion profiles with constant acceleration
//TODO: start work on profile generator of 3rd degree with constant jerk

class Profile{
public:
	
	
	//used to initialize the profile
	void setPosition(double _position) { position = _position; }
	void setVelocity(double _velocity) { velocity = _velocity; }
	void setAcceleration(double _acceleration) { acceleration = _acceleration; }
	
	
	double getPosition() { return position; }
	double getVelocity(){ return velocity; }
	double getAcceleration() { return acceleration; }
	
	
	//Match a fixed target velocity using linear velocity interpolation
	void matchVelocity(double deltaT_seconds, double targetVelocity, double fixedAcceleration){
		double previousVelocity = velocity;
		if(velocity != targetVelocity){
			double deltaV = deltaT_seconds * std::abs(fixedAcceleration);
			if(velocity < targetVelocity) {
				//accelerate but don't overshoot target velocity
				velocity = std::min(velocity + deltaV, targetVelocity);
				acceleration = std::abs(fixedAcceleration);
			}
			else {
				//accelerate but don't overshoot target velocity
				velocity = std::max(velocity - deltaV, targetVelocity);
				acceleration = -std::abs(fixedAcceleration);
			}
		}else acceleration = 0.0;
		double averageVelocity = (previousVelocity + velocity) / 2.0;
		double deltaP = averageVelocity * deltaT_seconds;
		position += deltaP;
	}
	
	
	//Match a fixed target velocity using linear velocity interpolation but don't go over specified position limits, brake using a specified max acceleration if necessary
	void matchVelocityAndRespectPositionLimits(double deltaT_seconds, double targetVelocity, double fixedAcceleration, double lowPositionLimit, double highPositionLimit, double maxAcceleration){
		//check braking position at max deceleration to see if a fast stop is needed
		double brakingPosition = getBrakingPosition(deltaT_seconds, maxAcceleration);
		if((velocity < 0.0 && brakingPosition <= lowPositionLimit) || (velocity > 0.0 && brakingPosition >= highPositionLimit)){
			//if yes, stop at full deceleration and don't update the profile any more
			matchVelocity(deltaT_seconds, 0.0, maxAcceleration);
			return;
		}
		
		//if the current braking position is not over limit
		//we might still exceed limit with the normal profile routine
		//take a backup in case we need to revert to those values
		double positionBackup = position;
		double velocityBackup = velocity;
		double accelerationBackup = acceleration;
		
		//basic linear interpolation for target velocity
		matchVelocity(deltaT_seconds, targetVelocity, fixedAcceleration);
		
		//if we would exceed limits, just stop the profile and ignore target velocity
		if((position <= lowPositionLimit && velocity < 0.0) || (position >= highPositionLimit && velocity > 0.0)){
			position = positionBackup;
			velocity = velocityBackup;
			acceleration = accelerationBackup;
		}
	}
	
	
	//come to a stop at a given deceleration value
	void stop(double deltaT_seconds, double deceleration){
		matchVelocity(deltaT_seconds, 0.0, deceleration);
	}
	
	
	//is the profile moving
	bool isMoving(){
		return velocity != 0.0;
	}
	
	
	//get braking position at current velocity and max acceleration (projected one deltaT period in the future for safety)
	double getBrakingPosition(double deltaT_seconds, double brakingDeceleration){
		double decelerationSigned = std::abs(brakingDeceleration);
		if (velocity < 0.0) decelerationSigned *= -1.0;
		double decelerationPositionDelta = std::pow(velocity, 2.0) / (2.0 * decelerationSigned);
		double brakingPosition_positionUnits = position + decelerationPositionDelta;
		//do the future projection here
		//effectively as if we started braking on the next cycle instead of the current one
		return brakingPosition_positionUnits + velocity * deltaT_seconds;
	}

	
	//Match a fixed target position
	void matchFixedPosition(double deltaT_seconds, double targetPosition, double fixedAcceleration, double maxVelocity){
		//TODO: implement this
	}
	
	
	//Match a moving target position
	void matchMovingPosition(double deltaT_seconds, double targetPosition, double targetVelocity, double targetAcceleration, double fixedAcceleration, double maxVelocity){
		//TODO: implement this
	}
	
	
	//========== Motion Interpolation ==========
	
	bool moveToPositionInTime(double startTime, double targetPosition, double targetTime, double targetAcceleration, double velocityLimit){
		auto startPoint = std::make_shared<Motion::ControlPoint>(startTime,
																 position,
																 targetAcceleration,
																 velocity);
		auto endPoint = std::make_shared<Motion::ControlPoint>(startTime + targetTime,
															   targetPosition,
															   targetAcceleration,
															   0.0);
		return Motion::TrapezoidalInterpolation::getClosestTimeAndVelocityConstrainedInterpolation(startPoint,
																								   endPoint,
																								   velocityLimit,
																								   targetInterpolation);
	}
	
	bool moveToPositionWithVelocity(double startTime, double targetPosition, double targetVelocity, double targetAcceleration){
		auto startPoint = std::make_shared<Motion::ControlPoint>(startTime,
																 position,
																 targetAcceleration,
																 velocity);
		auto endPoint = std::make_shared<Motion::ControlPoint>(0.0,
															   targetPosition,
															   targetAcceleration,
															   0.0);
		return Motion::TrapezoidalInterpolation::getFastestVelocityConstrainedInterpolation(startPoint,
																							endPoint,
																							targetVelocity,
																							targetInterpolation);
	}
	
	void updateInterpolation(double time){
		Motion::CurvePoint point = targetInterpolation->getPointAtTime(time);
		position = point.position;
		velocity = point.velocity;
		acceleration = point.acceleration;
	}
	
	double getInterpolationProgress(double time){
		return targetInterpolation->getProgressAtTime(time);
	}
	
	bool isInterpolationFinished(double time){
		return targetInterpolation->getProgressAtTime(time) >= 1.0;
	}
	
	double getRemainingInterpolationTime(double time){
		return std::max(targetInterpolation->outTime - time, 0.0);
	}
	
private:
	
	double position = 0.0;
	double velocity = 0.0;
	double acceleration = 0.0;
	
	std::shared_ptr<Motion::Interpolation> targetInterpolation = std::make_shared<Motion::Interpolation>();
	
};


}
