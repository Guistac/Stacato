#pragma once

namespace MotionCurve {

	struct CurvePoint {
		CurvePoint() {}
		CurvePoint(double t, double p, double a) : time(t), position(p), acceleration(a), velocity(0.0) {}
		CurvePoint(double t, double p, double a, double v) : time(t), position(p), acceleration(a), velocity(v) {}
		double time;
		double position;
		double velocity;
		double acceleration;
	};

	struct MotionConstraints {
		MotionConstraints() {}
		MotionConstraints(double maxV, double maxA) : maxAcceleration(maxA), maxVelocity(maxV), minPosition(FLT_MIN), maxPosition(FLT_MAX) {}
		MotionConstraints(double maxV, double maxA, double minPos, double maxPos) : maxAcceleration(maxA), maxVelocity(maxV), minPosition(FLT_MIN), maxPosition(FLT_MAX) {}
		double maxVelocity;
		double maxAcceleration;
		double maxPosition;
		double minPosition;
	};

	struct CurveProfile {

		CurvePoint requestedStartPoint;
		CurvePoint requestedEndPoint;

		double rampInStartTime;			//time of curve start
		double rampInStartPosition;		//position of curve start
		double rampInStartVelocity;		//velocity at curve start

		double rampInAcceleration;		//acceleration of curve

		double rampInEndPosition;		//position of curve after acceleration phase
		double rampInEndTime;			//time of acceleration end

		double coastVelocity;			//velocity of constant velocity phase

		double rampOutStartPosition;	//position of deceleration start
		double rampOutStartTime;		//time of deceleration start

		double rampOutAcceleration;		//deceleration of curve

		double rampOutEndTime;			//time of curve end
		double rampOutEndPosition;		//position of curve end
		double rampOutEndVelocity;		//velocity of curve end
	};

	enum class CurvePhase {
		NOT_STARTED,
		RAMP_IN,
		COAST,
		RAMP_OUT,
		FINISHED
	};

	//returns a profile using the position, velocity, acceleration and time data of the two specified points, while respecting the provided motion constraints
	CurveProfile getTimeConstrainedProfile(CurvePoint startPoint, CurvePoint endPoint, MotionConstraints contraints);
	//return a profile using the position, velocity and acceleration values of the two specified points, using the specified velocity as a target while respecting the provided motion constraints
	CurveProfile getVelocityContrainedProfile(CurvePoint startPoint, CurvePoint endPoint, double velocity, MotionConstraints constraints);

	bool isInsideCurveTime(double time, CurveProfile& curvePoints);

	CurvePoint getCurvePointAtTime(double time, CurveProfile& curveProfile);

	CurvePhase getCurvePhaseAtTime(double time, CurveProfile& curveProfile);

}
