#pragma once

#include "Motion/MotionTypes.h"

struct InterpolationType {
	enum class Type {
		STEP,
		LINEAR,
		TRAPEZOIDAL,
		BEZIER
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};

std::vector<InterpolationType>& getInterpolationTypes();
InterpolationType* getInterpolationType(InterpolationType::Type t);
InterpolationType* getInterpolationType(const char* saveName);

namespace Motion {

	struct ControlPoint;
	class Interpolation;
	class Curve;

	struct CurvePoint {
		CurvePoint() {}
		CurvePoint(double t, double p, double a, double v) : time(t), position(p), velocity(v), acceleration(a) {}
		double time;
		double position;
		double velocity;
		double acceleration;
	};

	struct ControlPoint : public CurvePoint {

		ControlPoint() {}
		ControlPoint(double t, double p, double a, double v) : CurvePoint(t,p,a,v) {}

		double velocityIn;
		double velocityOut;
		double rampIn;
		double rampOut;
		std::shared_ptr<Interpolation> inInterpolation;
		std::shared_ptr<Interpolation> outInterpolation;

		char name[64] = "";
	};

	class Interpolation {
	public:

		InterpolationType::Type type;
		std::shared_ptr<ControlPoint> inPoint;
		std::shared_ptr<ControlPoint> outPoint;

		double inTime = 0.0;
		double inPosition = 0.0;
		double inVelocity = 0.0;
		double inAcceleration = 0.0;

		double outTime = 0.0;
		double outPosition = 0.0;
		double outVelocity = 0.0;
		double outAcceleration = 0.0;

		//reports error when calculating the interpolation
		bool isDefined = false;

		//for linear and kinematic interpolation
		double interpolationVelocity = 0.0;

		//for kinematic interpolation
		double rampInEndPosition = 0.0;		//position of curve after acceleration phase
		double rampInEndTime = 0.0;			//time of acceleration end
		double rampOutStartPosition = 0.0;	//position of deceleration start
		double rampOutStartTime = 0.0;		//time of deceleration start

		bool Interpolation::isTimeInside(double time);
		CurvePoint getPointAtTime(double time);
		double getProgressAtTime(double time);

		void resetValues();

		void updateDisplayCurvePoints();
		std::vector<CurvePoint> displayPoints;
	};

	class Curve {
	public:
		std::vector<std::shared_ptr<ControlPoint>> points;
		InterpolationType::Type interpolationType;
		std::vector<std::shared_ptr<Interpolation>> interpolations;

		std::vector<std::shared_ptr<ControlPoint>>& getPoints();
		void addPoint(std::shared_ptr<ControlPoint> point);
		void removePoint(std::shared_ptr<ControlPoint> point);
		void removeAllPoints();
		void refresh();

		std::shared_ptr<ControlPoint> getStart();
		std::shared_ptr<ControlPoint> getEnd();
		double getLength();
		bool isTimeInsideCurve(double time);
		CurvePoint getPointAtTime(double time);

		void updateDisplayCurvePoints();

		char name[64] = "";
	};



	namespace TrapezoidalInterpolation {
		//returns a profile using the position, velocity, acceleration and time data of the two specified points, while respecting the provided motion constraints
		bool getTimeConstrainedInterpolation(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, double maxVelocity, std::shared_ptr<Interpolation>& output);
		//return a the fastest profile using the position, velocity and acceleration values of the two specified points, while respecting those motion constraints
		bool getFastestVelocityConstrainedInterpolation(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, double velocity, std::shared_ptr<Interpolation>& output);
	}

	namespace LinearInterpolation {
		void getTimeConstrainedInterpolation(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, std::shared_ptr<Interpolation>& output);
		void getVelocityConstrainedInterpolation(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, double velocity, std::shared_ptr<Interpolation>& output);
	}

	namespace StepInterpolation {
		void getInterpolation(std::shared_ptr<ControlPoint>& startPoints, std::shared_ptr<ControlPoint>& endPoint, std::shared_ptr<Interpolation>& output);
	}

}