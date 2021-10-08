#include <pch.h>
#include "MotionCurve.h"

namespace MotionCurve {

	CurveProfile getTimeConstrainedProfile(CurvePoint startPoint, CurvePoint endPoint, MotionConstraints constraints) {
		CurveProfile dummy;
		return dummy;
	}

	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================

    CurveProfile getVelocityContrainedProfile(const CurvePoint& startPoint, const CurvePoint& endPoint, double velocity, const MotionConstraints& constraints) {

		//Algorithm for procuding a motion curve of the 2nd degree:
		//this algorithm takes the following parameters:
		//- start position
		//- start time
		//- end position
		//- start velocity (signed)
		//- end velocity (signed)
		//- start acceleration (unsigned)
		//- end acceleration (unsigned)
		//- target movement velocity (unsigned)
		//the algorithm produces a profile for the motion curve which can be used
		//to get position values at each instant of the curve
		//the procudure of this algorithm is to first define the shape of the motion curve
		//we do this by calculating these variables:
		//- the sign of both acceleration values
		//- the sign and value of the transition velocity
		//- the presence or absence of a constant velocity coast phase
		//using these variables we will compute four points which will define the motion curve:
		//- the start time and position of the first ramp
		//- the end time and position of the first ramp
		//- the start time and position of the second ramp
		//- the end time and position of the second ramp
		//along with the signed acceleration values and coast phase velocity value the curve is fully defined at each instant

		auto clamp = [](double in, double a, double b) -> double {
			if (a < b) {
				if (in < a) return a;
				else if (in > b) return b;
			}
			else {
				if (in < b) return b;
				else if (in > a) return a;
			}
			return in;
		};

		//these are the variables used to calculate the ramp
		//don't constrain start position, and velocity since those may already be physically present
		const double rampStartPosition = startPoint.position;
		const double rampStartTime = startPoint.time;
		const double rampInVelocity = startPoint.velocity;
		const double rampInAccelerationAbs = clamp(std::abs(startPoint.acceleration), 0.0, constraints.maxAcceleration);
		const double rampEndPosition = clamp(endPoint.position, constraints.minPosition, constraints.maxPosition);
		const double rampOutVelocity = clamp(endPoint.velocity, -constraints.maxVelocity, constraints.maxVelocity);
		const double rampOutAccelerationAbs = clamp(std::abs(endPoint.acceleration), 0.0, constraints.maxAcceleration);
		const double requestedVelocity = clamp(std::abs(velocity), 0.0, constraints.maxVelocity);

		const double totalDeltaPosition = rampEndPosition - rampStartPosition;
		const double totalDeltaVelocity = rampOutVelocity - rampInVelocity;

		//these are the values necessary to compute ramp profile points
		double transitionVelocity;
		double rampInAcceleration;
		double rampOutAcceleration;
		bool requestedVelocityReacheable;

		//get 1.0 for positive values and -1.0 for negative values
		auto getSignMultiplier = [](double a) -> double {
			if (a > 0.0) return 1.0;
			return -1.0;
		};

		//compare the rampIn and rampOut velocities with the transition velocity to set the signs if the rampIn and rampOut acceleration values
		auto updateAccelerationSigns = [&]() {
			if (rampInVelocity < transitionVelocity) rampInAcceleration = std::abs(rampInAcceleration);
			else rampInAcceleration = -std::abs(rampInAcceleration);
			if (transitionVelocity < rampOutVelocity) rampOutAcceleration = std::abs(rampOutAcceleration);
			else rampOutAcceleration = -std::abs(rampOutAcceleration);
		};

		//is the target too close to be reached using only the highest of the two acceleration values ?
		auto isTargetOvershotWithLargerAcceleration = [&]()->bool {
			//get the absolute value of the larger acceleration
			double largerAcceleration;
			if (std::abs(rampInAcceleration) > std::abs(rampOutAcceleration)) largerAcceleration = std::abs(rampInAcceleration);
			else largerAcceleration = std::abs(rampOutAcceleration);
			//sign the acceleration based on the total velocity delta
			largerAcceleration *= getSignMultiplier(totalDeltaVelocity);
			double overshootDeltaPosition = (std::pow(rampOutVelocity, 2.0) - std::pow(rampInVelocity, 2.0)) / (2.0 * largerAcceleration);
			if (totalDeltaPosition > 0.0 && totalDeltaPosition < overshootDeltaPosition) return true;
			else if (totalDeltaPosition < 0.0 && totalDeltaPosition > overshootDeltaPosition) return true;
			else return false;
		};

		//is the target too close to be reached using only the lowest of the two acceleration values ?
		auto isTargetOvershotWithSmallerAcceleration = [&]()->bool {
			//get the absolute value of the larger acceleration
			double smallerAcceleration;
			if (std::abs(rampInAcceleration) < std::abs(rampOutAcceleration)) smallerAcceleration = std::abs(rampInAcceleration);
			else smallerAcceleration = std::abs(rampOutAcceleration);
			//sign the acceleration based on the total velocity delta
			smallerAcceleration *= getSignMultiplier(totalDeltaVelocity);
			double overshootDeltaPosition = (std::pow(rampOutVelocity, 2.0) - std::pow(rampInVelocity, 2.0)) / (2.0 * smallerAcceleration);
			if (totalDeltaPosition > 0.0 && totalDeltaPosition < overshootDeltaPosition) return true;
			else if (totalDeltaPosition < 0.0 && totalDeltaPosition > overshootDeltaPosition) return true;
			else return false;
		};

		//is the target close enough for the movement to reach the requested velocity ?
		auto isTransitionVelocityReachable = [&]()->bool {
			double rampInDeltaPosition = (std::pow(transitionVelocity, 2.0) - std::pow(rampInVelocity, 2.0)) / (2 * rampInAcceleration);
			double rampOutDeltaPosition = (std::pow(rampOutVelocity, 2.0) - std::pow(transitionVelocity, 2.0)) / (2 * rampOutAcceleration);
			double totalRampDeltaPosition = rampInDeltaPosition + rampOutDeltaPosition;
			if (transitionVelocity > 0.0) return totalRampDeltaPosition < totalDeltaPosition;
			else return totalRampDeltaPosition > totalDeltaPosition;
		};

		//get the velocity at the transition point between two tangent velocity ramps
		auto getAbsRampTangentVelocity = [&]()->double {
			double rampInTangentDeltaPosition = (std::pow(rampOutVelocity, 2.0) - std::pow(rampInVelocity, 2.0) - 2.0 * rampOutAcceleration * totalDeltaPosition) / (2.0 * (rampInAcceleration - rampOutAcceleration));
			double rampTangentVelocity = std::sqrt(std::pow(rampInVelocity, 2.0) + 2.0 * rampInAcceleration * rampInTangentDeltaPosition);
			if (isnan(rampTangentVelocity) || rampTangentVelocity < 0.0) rampTangentVelocity = std::numeric_limits<double>::infinity();
			//this function potentially returns garbage values, if the values are out of range they need to be handled
			return rampTangentVelocity;
		};

		//make a move that briefly reverses direction to reach the target
		auto makeInvertedMovement = [&]() {
			//first invert the transition velocity and adjust acceleration signs
			transitionVelocity *= -1.0;
			updateAccelerationSigns();
			//after inverting the direction, check if the inverted direction is reachable
			if (isTransitionVelocityReachable()) {
				//if the inverted direction is reachable we have all we need to know about the movement
				requestedVelocityReacheable = true;
			}
			else {
				//else get the fastest velocity for a ramp to ramp transition to the target
				transitionVelocity = getAbsRampTangentVelocity() * getSignMultiplier(transitionVelocity);
				requestedVelocityReacheable = std::abs(transitionVelocity) >= requestedVelocity;
				//make sure the velocity doesn't exceed the requested velocity
				if (transitionVelocity < -requestedVelocity) transitionVelocity = -requestedVelocity;
				else if (transitionVelocity > requestedVelocity) transitionVelocity = requestedVelocity;
				//get new acceleration signs in case they need to change
				updateAccelerationSigns();
			}
		};

		//set the default transition velocity sign based on the movement direction
		transitionVelocity = requestedVelocity * getSignMultiplier(totalDeltaPosition);
		//initialize the default acceleration values
		rampInAcceleration = rampInAccelerationAbs;
		rampOutAcceleration = rampOutAccelerationAbs;
		//get default acceleration signs based on the requested transition velocity
		updateAccelerationSigns();
		//by default, we assume these signs are correct and a basic move with acceleration to transition velocity followed by deceleration is possible
		//however, several cases need different parameters and the following condition checking will separate and handle them
		//some moves will not be able to reach the requested velocity, other moves will invert the acceleration signs or even the transition velocity sign

		//========== Curve type identification logic ==========

		if (isTargetOvershotWithLargerAcceleration() && !isTransitionVelocityReachable()) {
			//here the target is too close to be reached using only the max acceleration value
			//we need invert the transition velocity and get new acceleration signs
			//we effectively backtrack to come back to the target at the correct end point position and velocity
			makeInvertedMovement();
		}
		else if (!isTransitionVelocityReachable()) {
			//here the target can be reached, but is too close for the requested velocity to be reached
			if (isTargetOvershotWithSmallerAcceleration() && !isTargetOvershotWithLargerAcceleration()) {
				//in this special subcase, the target is not reachable using only the smaller of the two acceleration values
				//it can still be reached by using a blend of the two accelerations, and without inverting the direction of motion
				//but we need to get new acceleration signs without knowing the transition velocity
				//the velocity in that segment will be somewhere between the input and outputvelocity
				//we can use the average of the two to find out acceleration signs and get a real transition velocity value later
				double previousTransitionVelocity = transitionVelocity;
				transitionVelocity = (rampInVelocity + rampOutVelocity) / 2.0;
				updateAccelerationSigns();
				transitionVelocity = previousTransitionVelocity;
			}	
			//find the fastest possible transition velocity
			double absRampTangentVelocity = getAbsRampTangentVelocity();
			if (absRampTangentVelocity < requestedVelocity) {
				//here we can get a transition velocity that is below the requested one
				//in this case we keep the same movement direction, but without reaching the requested velocity
				transitionVelocity = absRampTangentVelocity * getSignMultiplier(transitionVelocity);
				updateAccelerationSigns();
				requestedVelocityReacheable = false;
			}
			else {
				//in this case, the fastest transition velocity is higher than the requested one
				//here we absolutely cannot reach the target by moving in the same direction as the total delta
				//so we flip the velocity and do the calculation again
				makeInvertedMovement();
			}
		}
		else {
			//this is the basic case for regular moves
			//no additional logic is required
			requestedVelocityReacheable = true;
		}

		//using the final transition velocity value and signed acceleration values
		//we can get time and position deltas as well as absolute position and time points for the ramps
		double rampInDeltaTime = (transitionVelocity - rampInVelocity) / rampInAcceleration;
		double rampOutDeltaTime = (rampOutVelocity - transitionVelocity) / rampOutAcceleration;
		double rampInDeltaPosition = rampInVelocity * rampInDeltaTime + rampInAcceleration * std::pow(rampInDeltaTime, 2.0) / 2.0;
		double rampOutDeltaPosition = transitionVelocity * rampOutDeltaTime + rampOutAcceleration * std::pow(rampOutDeltaTime, 2.0) / 2.0;
		double rampInEndPosition = rampStartPosition + rampInDeltaPosition;
		double rampInEndTime = rampStartTime + rampInDeltaTime;

		//the calculation of rampOut start point is different depending on if we have a constant velocity phase
		double rampOutStartPosition;
		double rampOutStartTime;
		if (requestedVelocityReacheable) {
			//with constant velocity phase
			rampOutStartPosition = rampEndPosition - rampOutDeltaPosition;
			rampOutStartTime = rampInEndTime + (rampOutStartPosition - rampInEndPosition) / transitionVelocity;
		}
		else {
			//without constant velocity phase
			rampOutStartPosition = rampInEndPosition;
			rampOutStartTime = rampInEndTime;
		}

		//finally we can get the absolute end time of the curve
		double curveEndTime = curveEndTime = rampOutStartTime + rampOutDeltaTime;

		//We can construct the curve profile object with all points we calculated
		MotionCurve::CurveProfile profile;
		profile.rampInStartTime = startPoint.time;			//time of curve start
		profile.rampInStartPosition = startPoint.position;	//position of curve start
		profile.rampInStartVelocity = startPoint.velocity;	//velocity at curve start
		profile.rampInAcceleration = rampInAcceleration;	//acceleration of curve
		profile.rampInEndPosition = rampInEndPosition;		//position of curve after acceleration phase
		profile.rampInEndTime = rampInEndTime;			    //time of acceleration end
		profile.coastVelocity = transitionVelocity;			//velocity of constant velocity phase
		profile.rampOutStartPosition = rampOutStartPosition;//position of deceleration start
		profile.rampOutStartTime = rampOutStartTime;		//time of deceleration start
		profile.rampOutAcceleration = rampOutAcceleration;	//deceleration of curve
		profile.rampOutEndTime = curveEndTime;			    //time of curve end
		profile.rampOutEndPosition = endPoint.position;		//position of curve end
		profile.rampOutEndVelocity = endPoint.velocity;		//velocity of curve end

		return profile;
    }

	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================





	bool isInsideCurveTime(double time, CurveProfile& curvePoints) {
		return false;
	}

	CurvePoint getCurvePointAtTime(double time, CurveProfile& curveProfile) {
        CurvePoint output;
        output.time = time;
        double deltaT;
        switch (getCurvePhaseAtTime(time, curveProfile)) {
            case CurvePhase::NOT_STARTED:
                output.position = curveProfile.rampInStartPosition;
                output.velocity = curveProfile.rampInStartVelocity;
                output.acceleration = 0.0;
                break;
            case CurvePhase::RAMP_IN:
                deltaT = time - curveProfile.rampInStartTime;
                output.position = curveProfile.rampInStartPosition + curveProfile.rampInStartVelocity * deltaT + curveProfile.rampInAcceleration * std::pow(deltaT, 2.0) / 2.0;
                output.velocity = curveProfile.rampInStartVelocity + curveProfile.rampInAcceleration * deltaT;
                output.acceleration = curveProfile.rampInAcceleration;
                break;
            case CurvePhase::COAST:
                deltaT = time - curveProfile.rampInEndTime;
                output.position = curveProfile.rampInEndPosition + deltaT * curveProfile.coastVelocity;
                output.velocity = curveProfile.coastVelocity;
                output.acceleration = 0.0;
                break;
            case CurvePhase::RAMP_OUT:
                deltaT = time - curveProfile.rampOutStartTime;
                output.position = curveProfile.rampOutStartPosition + curveProfile.coastVelocity * deltaT + curveProfile.rampOutAcceleration * std::pow(deltaT, 2.0) / 2.0;
                output.velocity = curveProfile.coastVelocity + curveProfile.rampOutAcceleration * deltaT;
                output.acceleration = curveProfile.rampOutAcceleration;
                break;
            case CurvePhase::FINISHED:
                output.position = curveProfile.rampOutEndPosition;
                output.velocity = curveProfile.rampOutEndVelocity;
                output.acceleration = 0.0;
                break;
        }
		return output;
	}

	CurvePhase getCurvePhaseAtTime(double time, CurveProfile& curveProfile) {
        if (time < curveProfile.rampInStartTime) return CurvePhase::NOT_STARTED;
        else if (time < curveProfile.rampInEndTime) return CurvePhase::RAMP_IN;
        else if (time < curveProfile.rampOutStartTime) return CurvePhase::COAST;
        else if (time < curveProfile.rampOutEndTime) return CurvePhase::RAMP_OUT;
        else return CurvePhase::FINISHED;
	}

}





