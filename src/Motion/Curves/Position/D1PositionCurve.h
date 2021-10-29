#pragma once

namespace Motion::PositionCurve::D1 {

	struct Point {
		Point() : time(0.0), position(0.0), velocity(0.0), acceleration(0.0) {}
		Point(double t, double p, double a) : time(t), position(p), acceleration(a), velocity(0.0) {}
		Point(double t, double p, double a, double v) : time(t), position(p), acceleration(a), velocity(v) {}
		double time;
		double position;
		double velocity;
		double acceleration;
	};

	struct Interpolation {
		bool isDefined = false;

		double rampInStartTime = 0.0;		//time of curve start
		double rampInStartPosition = 0.0;	//position of curve start
		double rampInStartVelocity = 0.0;	//velocity at curve start
		double rampInAcceleration = 0.0;	//acceleration of curve

		double rampInEndPosition = 0.0;		//position of curve after acceleration phase
		double rampInEndTime = 0.0;			//time of acceleration end

		double coastVelocity = 0.0;			//velocity of constant velocity phase

		double rampOutStartPosition = 0.0;	//position of deceleration start
		double rampOutStartTime = 0.0;		//time of deceleration start

		double rampOutAcceleration = 0.0;	//deceleration of curve
		double rampOutEndTime = 0.0;		//time of curve end
		double rampOutEndPosition = 0.0;	//position of curve end
		double rampOutEndVelocity = 0.0;	//velocity of curve end
	};

	class Curve {
		std::vector<std::shared_ptr<Interpolation>> interpolations;
		std::vector<std::shared_ptr<Point>> getPoints();
		void addPoint(std::shared_ptr<Point> point);
		void removePoint(std::shared_ptr<Point> point);
		void refresh();
		std::shared_ptr<Point> getStart();
		std::shared_ptr<Point> getEnd();
		double getLength();
	};

	enum class CurvePhase {
		NOT_STARTED,
		RAMP_IN,
		COAST,
		RAMP_OUT,
		FINISHED
	};

	//returns a profile using the position, velocity, acceleration and time data of the two specified points, while respecting the provided motion constraints
	bool getTimeConstrainedInterpolation(const Point& startPoint, const Point& endPoint, double maxVelocity, Interpolation& output);
	//return a profile using the position, velocity and acceleration values of the two specified points, using the specified velocity as a target while respecting the provided motion constraints
	bool getVelocityContrainedInterpolations(const Point& startPoint, const Point& endPoint, double velocity, std::vector<Interpolation>& output);

	bool getFastestVelocityConstrainedInterpolation(const Point& startPoint, const Point& endPoint, double velocity, Interpolation& output);

	bool isInsideInterpolation(double time, const Interpolation& curveProfile);
	Point getInterpolationAtTime(double time, const Interpolation& curveProfile);
	CurvePhase getInterpolationPhaseAtTime(double time, const Interpolation& curveProfile);
	float getInterpolationProgress(double time, const Interpolation& curveProfile);

}
