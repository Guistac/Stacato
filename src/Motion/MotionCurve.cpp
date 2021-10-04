#include <pch.h>
#include "MotionCurve.h"

namespace MotionCurve {

	CurveProfile getTimeConstrainedProfile(CurvePoint startPoint, CurvePoint endPoint, MotionConstraints constraints) {
		CurveProfile dummy;
		return dummy;
	}

    CurveProfile getVelocityContrainedProfile(CurvePoint startPoint, CurvePoint endPoint, double velocity, MotionConstraints constraints) {

        //get stopping distance from initial velocity
        double rampInStoppingDeltaPosition = std::pow(startPoint.velocity, 2.0) / (2.0 * std::abs(startPoint.acceleration));
        if (startPoint.velocity < 0) rampInStoppingDeltaPosition *= -1.0;

        //get stopping distance from end velocity
        double rampOutStoppingDeltaPosition = std::pow(endPoint.velocity, 2.0) / (2.0 * std::abs(endPoint.acceleration));
        if (endPoint.velocity < 0) rampOutStoppingDeltaPosition *= -1.0;

        //calculate stopping position from the stopping deltas and start/end positions
        double rampInStoppingPosition = startPoint.position + rampInStoppingDeltaPosition;
        double rampOutStoppingPosition = endPoint.position - rampOutStoppingDeltaPosition;

        //calculate the delta between the stopping positions
        double rampStoppingDeltaPosition = rampOutStoppingPosition - rampInStoppingPosition;

        //use the delta to determine the sign of the constant velocity
        double constantVelocity = rampStoppingDeltaPosition > 0.0 ? std::abs(velocity) : -std::abs(velocity);

        //use the constant velocity to get the signs of in and out accelerations
        double rampInAcceleration = constantVelocity > startPoint.velocity ? std::abs(startPoint.acceleration) : -std::abs(startPoint.acceleration);
        double rampOutAcceleration = constantVelocity < endPoint.velocity ? std::abs(endPoint.acceleration) : -std::abs(endPoint.acceleration);

        //calculate total delta P of the curve (negative for negative moves)
        double totalDeltaPosition = endPoint.position - startPoint.position;

        //get the two delta p associated with the ramp in and ramp out phases
        double rampInDeltaPosition = (std::pow(constantVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);
        double rampOutDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(constantVelocity, 2.0)) / (2 * rampOutAcceleration);

        bool constantVelocityReachable;
        if (totalDeltaPosition > 0) constantVelocityReachable = rampInDeltaPosition + rampOutDeltaPosition < totalDeltaPosition;
        else constantVelocityReachable = rampInDeltaPosition + rampOutDeltaPosition > totalDeltaPosition;

        if (!constantVelocityReachable) {
        
            //since we can't reach the constant velocity, we calculate a new ramp transition velocity
            //the simplest way to do this is by first calculating the position delta of the ramp transition
            double rampTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * totalDeltaPosition - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration - 2 * rampOutAcceleration);

            //we then can use this delta to calculate the transition velocity, taking care to set its sign equal to the initial constant velocity
            double rampTransitionVelocity = std::sqrt(std::pow(startPoint.velocity, 2.0) + 2.0 * rampInAcceleration * rampTransitionDeltaPosition);
            if (rampStoppingDeltaPosition < 0.0) rampTransitionVelocity *= -1.0;
            
            if (rampTransitionVelocity > velocity) {
                rampTransitionVelocity = velocity;
                rampInDeltaPosition = (std::pow(rampTransitionVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);
            }
            else if (rampTransitionVelocity < -velocity) {
                rampTransitionVelocity = -velocity;
                rampInDeltaPosition = (std::pow(rampTransitionVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);
            }

            //we also need to recalculate the rampOut delta, since we can't reach constant velocity
            rampOutDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(rampTransitionVelocity, 2.0)) / (2 * rampOutAcceleration);

            constantVelocity = rampTransitionVelocity;
            rampInDeltaPosition = rampTransitionDeltaPosition;
        }

        //end position of ramp-In is start + ramping delta
        double rampInEndPosition = startPoint.position + rampInDeltaPosition;

        //end time of ramp-In is startTime + rampIn delta time
        double rampInEndTime = startPoint.time + (constantVelocity - startPoint.velocity) / rampInAcceleration;

        //ramp out start position is end position minus rampOut Delta
        double rampOutStartPosition = endPoint.position - rampOutDeltaPosition;

        //ramp out start time is rampIn end time + time of constant velocity coast
        double rampOutStartTime = rampInEndTime + (rampOutStartPosition - rampInEndPosition) / constantVelocity;

        //end of curve time is ramp out start time + ramp out delta time
        double curveEndTime = rampOutStartTime + (endPoint.velocity - constantVelocity) / rampOutAcceleration;
      

        CurveProfile profile;

        //profile.requestedStartPoint;
        //profile.requestedEndPoint;

        profile.rampInStartTime = startPoint.time;			//time of curve start
        profile.rampInStartPosition = startPoint.position;	//position of curve start
        profile.rampInStartVelocity = startPoint.velocity;	//velocity at curve start
        profile.rampInAcceleration = rampInAcceleration;	//acceleration of curve
        profile.rampInEndPosition = rampInEndPosition;		//position of curve after acceleration phase
        profile.rampInEndTime = rampInEndTime;			    //time of acceleration end
        profile.coastVelocity = constantVelocity;			//velocity of constant velocity phase
        profile.rampOutStartPosition = rampOutStartPosition;//position of deceleration start
        profile.rampOutStartTime = rampOutStartTime;		//time of deceleration start
        profile.rampOutAcceleration = rampOutAcceleration;	//deceleration of curve
        profile.rampOutEndTime = curveEndTime;			    //time of curve end
        profile.rampOutEndPosition = endPoint.position;		//position of curve end
        profile.rampOutEndVelocity = endPoint.velocity;		//velocity of curve end

        return profile;
    }

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
                output.velocity = 0.0;
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
                output.velocity = 0.0;
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





