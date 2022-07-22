#pragma once

#include "Motion/Curve/Curve.h"

#define square(x) (x) * (x)

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
	void matchVelocityAndRespectPositionLimits(double deltaT_seconds,
											   double targetVelocity,
											   double fixedAcceleration,
											   double lowPositionLimit,
											   double highPositionLimit,
											   double maxAcceleration){
		//check braking position at max deceleration to see if a fast stop is needed
		double brakingPosition = getBrakingPosition(deltaT_seconds, maxAcceleration);
		if((position < lowPositionLimit && velocity < 0.0) || (position > highPositionLimit && velocity > 0.0)){
			stop(deltaT_seconds, maxAcceleration);
			return;
		}
		else if(velocity < 0.0 && brakingPosition <= lowPositionLimit){
			double distanceToLimit = std::abs(lowPositionLimit - position);
			double exactStoppingAcceleration = square(velocity) / (2.0 * distanceToLimit);
			stop(deltaT_seconds, exactStoppingAcceleration);
			return;
		}else if(velocity > 0.0 && brakingPosition >= highPositionLimit){
			double distanceToLimit = std::abs(highPositionLimit - position);
			double exactStoppingAcceleration = square(velocity) / (2.0 * distanceToLimit);
			stop(deltaT_seconds, exactStoppingAcceleration);
			return;
		}else if((position <= lowPositionLimit && targetVelocity < 0.0) || (position >= highPositionLimit && targetVelocity > 0.0)){
			stop(deltaT_seconds, maxAcceleration);
			return;
		}
		
		//basic linear interpolation for target velocity
		matchVelocity(deltaT_seconds, targetVelocity, fixedAcceleration);
	}
	
	
#define square(x) (x) * (x)
	
	
	struct CurveEquation{
		
		CurveEquation(double po_, double vo_, double ac_) : po(po_), vo(vo_), ac(ac_){}
		
		double po, vo, ac;
		
		bool intersectsAfter0(CurveEquation& other){
			double root = square(vo - other.vo) - 2.0 * (ac - other.ac) * (po - other.po);
			if(root < 0.0) return false;
			double intersectionTime1 = (other.vo - vo + sqrt(root)) / (ac - other.ac);
			if(intersectionTime1 >= 0.0) return true;
			double intersectionTime2 = (other.vo - vo - sqrt(root)) / (ac - other.ac);
			if(intersectionTime2 >= 0.0) return true;
			return false;
		}
		
		CurveEquation getEquationAtNewT0(double t0){
			double positionAtT0 = po + vo * t0 + ac * square(t0) / 2.0;
			double velocityAtT0 = vo + t0 * ac;
			return CurveEquation(positionAtT0, velocityAtT0, ac);
		}
		
		double getAccelerationToMatchCurveTangentially(CurveEquation& other){
			return square(vo - other.vo) / (2.0 * (po - other.po)) + other.ac;
		}
	};
	
	
	
	//Match a moving or stopped target position
	bool matchPosition(double deltaT, double targetPosition, double targetVelocity, double targetAcceleration, double fixedAcceleration, double maxVelocity){
		
		//we have two target curve approach strategies:
		//-try to match the target position and velocity simultaneously
		//-try to match the target position, velocity and acceleration simultaneously
		
		//the first strategy works best when the target acceleration is smaller than the catchup acceleration
		//in case the target acceleration is higher, we can never reach the target, and the algorithm will produce inefficient paths for acceleration segments
		//here we should use the other algorithm and ignore the targets acceleration
		
		//the second algorithm produces less optimal results since it can't predict the change in velocity of the target curve,
		//but it produces better results in case the targets acceleration is higher than the max catchup acceleration
		
		//both algorithms work the same, they only differ in their use of the target acceleration value
		//for the velocity/position only algorithm, the target acceleration value is 0.0
		
		//first we get the position error at the previous point (-deltaT)
		//since we try to match the curve as fast as possible, we always need to slow down to match the target tangentially
		//we use the sign of this error to get the sign of our catchup acceleration value
		
		if(std::abs(fixedAcceleration) <= std::abs(targetAcceleration)) targetAcceleration = 0.0;
		
		CurveEquation targetCurve(targetPosition, targetVelocity, targetAcceleration); 		//t0 is 0.0
		CurveEquation targetCurveAtPreviousTime = targetCurve.getEquationAtNewT0(-deltaT);	//t0 is -deltaT
		
		double pError = targetCurveAtPreviousTime.po - this->position;
		double vError = targetVelocity - this->velocity;
		
		double maxDeltaV = std::abs(deltaT * fixedAcceleration);
		if(std::abs(vError) < maxDeltaV){
			double vMax = this->velocity + maxDeltaV;
			double vMin = this->velocity - maxDeltaV;
			double pMax = this->position + deltaT * (this->velocity + vMax) / 2.0;
			double pMin = this->position + deltaT * (this->velocity + vMin) / 2.0;
			if(targetPosition > pMin && targetPosition < pMax){
				this->position = targetPosition;
				this->velocity = targetVelocity;
				this->acceleration = targetAcceleration;
				return true;
			}
		}
		
		
		double positionMatchingDeceleration = pError > 0.0 ? -std::abs(fixedAcceleration) : std::abs(fixedAcceleration);
		CurveEquation previousCurve(this->position, this->velocity, positionMatchingDeceleration); 	//t0 is -deltaT
		bool b_previousIntersects = previousCurve.intersectsAfter0(targetCurveAtPreviousTime);
		
		//if the previous curve does not intersect the target curve, we can keep accelerating to the target
		//else we need to decelerate
		double currentAcceleration;
		if(!b_previousIntersects) currentAcceleration = -positionMatchingDeceleration;
		else currentAcceleration = positionMatchingDeceleration;
		
		//calculate the current motion point
		double deltaV = deltaT * currentAcceleration;
		double currentVelocity = this->velocity + deltaV;
		currentVelocity = std::min(currentVelocity, std::abs(maxVelocity));
		currentVelocity = std::max(currentVelocity, -std::abs(maxVelocity));
		double deltaP = deltaT * (this->velocity + currentVelocity) / 2.0;
		double currentPosition = this->position + deltaP;
		
		//if the previous point could not decelerate to avoid intersecting the target,
		//we can't plan a correct trajectory and should just start decelerating to match the target
		if(b_previousIntersects){
			this->position = currentPosition;
			this->velocity = currentVelocity;
			this->acceleration = currentAcceleration;
			return false;
		}
		
		CurveEquation currentCurve(currentPosition, currentVelocity, positionMatchingDeceleration);
		bool b_currentIntersects = currentCurve.intersectsAfter0(targetCurve);
		
		//if after accelerating towards the target we still don't intersect it
		//just keep moving towards it
		if(!b_currentIntersects){
			this->position = currentPosition;
			this->velocity = currentVelocity;
			this->acceleration = -positionMatchingDeceleration;
			return false;
		}
		
		//if the current curve intersects the target and the previous did not
		//this means we can start decelerating to match the target curve
		//we need to find an acceleration value which will join the curve tangentially
		
		double positionAndVelocityMatchingAcceleration = previousCurve.getAccelerationToMatchCurveTangentially(targetCurveAtPreviousTime);
		deltaV = deltaT * positionAndVelocityMatchingAcceleration;
		currentVelocity = this->velocity + deltaV;
		currentVelocity = std::min(currentVelocity, std::abs(maxVelocity));
		currentVelocity = std::max(currentVelocity, -std::abs(maxVelocity));
		deltaP = deltaT * (this->velocity + currentVelocity) / 2.0;
		currentPosition = this->position + deltaP;
		
		this->position = currentPosition;
		this->velocity = currentVelocity;
		this->acceleration = positionAndVelocityMatchingAcceleration;
		return false;
		
	}
	
	bool matchPositionAndRespectPositionLimits(double deltaT,
											   double targetPosition,
											   double targetVelocity,
											   double targetAcceleration,
											   double fixedAcceleration,
											   double maxVelocity,
											   double lowerPositionLimit,
											   double upperPositionLimit){
		
		//check braking position at max deceleration to see if a fast stop is needed
		double brakingPosition = getBrakingPosition(deltaT * 2.0, fixedAcceleration);
		if((velocity < 0.0 && brakingPosition < lowerPositionLimit) || (velocity > 0.0 && brakingPosition > upperPositionLimit)){
			//if yes, stop at full deceleration and don't update the profile any more
			stop(deltaT, fixedAcceleration);
			return false;
		}
		
		//if the current braking position is not over limit
		//we might still exceed limit with the normal profile routine
		//take a backup in case we need to revert to those values
		double positionBackup = position;
		double velocityBackup = velocity;
		double accelerationBackup = acceleration;
		
		matchPosition(deltaT, targetPosition, targetVelocity, targetAcceleration, fixedAcceleration, maxVelocity);
		
		//if we would exceed limits, just stop the profile and ignore target velocity
		if((position <= lowerPositionLimit && velocity < 0.0) || (position >= upperPositionLimit && velocity > 0.0)){
			position = positionBackup;
			velocity = velocityBackup;
			acceleration = accelerationBackup;
		}
		
		return targetPosition == position && targetVelocity == velocity;
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
	
	
	//========== Motion Interpolation ==========
	
	bool moveToPositionInTime(double startTime, double targetPosition, double targetTime, double targetAcceleration, double velocityLimit){
		auto startPoint = std::make_shared<Motion::ControlPoint>();
		startPoint->time = startTime;
		startPoint->position = position;
		startPoint->velocity = velocity;
		startPoint->outAcceleration = targetAcceleration;
		
		auto endPoint = std::make_shared<Motion::ControlPoint>();
		endPoint->time = startTime + targetTime;
		endPoint->position = targetPosition;
		endPoint->velocity = 0.0;
		endPoint->inAcceleration = targetAcceleration;
		
		targetInterpolation = Motion::TrapezoidalInterpolation::getTimeConstrainedOrSlower(startPoint, endPoint, velocityLimit);
		return targetInterpolation != nullptr;
	}
	
	bool moveToPositionWithVelocity(double startTime, double targetPosition, double targetVelocity, double targetAcceleration){
		auto startPoint = std::make_shared<Motion::ControlPoint>();
		startPoint->time = startTime;
		startPoint->position = position;
		startPoint->velocity = velocity;
		startPoint->outAcceleration = targetAcceleration;
		
		auto endPoint = std::make_shared<Motion::ControlPoint>();
		endPoint->time = 0.0;
		endPoint->position = targetPosition;
		endPoint->velocity = 0.0;
		endPoint->inAcceleration = targetAcceleration;
		
		targetInterpolation = Motion::TrapezoidalInterpolation::getVelocityConstrained(startPoint, endPoint, targetVelocity);
		return targetInterpolation != nullptr;
	}
	
	/*
	void updateInterpolation(double time){
		Motion::Point point = targetInterpolation->getPointAtTime(time);
		position = point.position;
		velocity = point.velocity;
		acceleration = point.acceleration;
	}
	*/
	
	Motion::Point getInterpolationPoint(double time){
		return targetInterpolation->getPointAtTime(time);
	}
	
	double getInterpolationProgress(double time){
		if(targetInterpolation) return targetInterpolation->getProgressAtTime(time);
		return 1.0;
	}
	
	bool hasInterpolationTarget(){
		return targetInterpolation != nullptr;
	}
	
	bool isInterpolationFinished(double time){
		return targetInterpolation->getProgressAtTime(time) >= 1.0;
	}
	
	double getRemainingInterpolationTime(double time){
		return std::max(targetInterpolation->endTime - time, 0.0);
	}
	
	double getInterpolationTarget(){
		return targetInterpolation->endPosition;
	}
	
	void resetInterpolation(){
		targetInterpolation = nullptr;
	}
	
private:
	
	double position = 0.0;
	double velocity = 0.0;
	double acceleration = 0.0;
	
	std::shared_ptr<Motion::TrapezoidalInterpolation> targetInterpolation;
	
};


}
