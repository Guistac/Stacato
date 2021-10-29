#pragma once

namespace Motion {

	struct Point;
	class Interpolation;
	struct InterpolationType;
	class Curve;

	struct Point {

		Point() {}
		Point(double t, double p, double a, double v) : time(t), position(p), velocity(v), acceleration(a) {}

		double time;
		double position;
		double velocity;
		double acceleration;

		double velocityIn;
		double velocityOut;
		double rampIn;
		double rampOut;
		std::shared_ptr<Interpolation> inInterpolation;
		std::shared_ptr<Interpolation> outInterpolation;
	};

	struct InterpolationType {
		enum class Type {
			NONE,
			LINEAR,
			BEZIER,
			KINEMATIC
		};
		Type type;
		const char displayName[64];
		const char saveName[64];
	};
	std::vector<InterpolationType>& getInterpolationTypes();
	InterpolationType* getInterpolationType(InterpolationType::Type t);
	InterpolationType* getInterpolationType(const char* saveName);

	class Interpolation {
	public:
		InterpolationType::Type type;
		std::shared_ptr<Point> inPoint;
		std::shared_ptr<Point> outPoint;

		double inTime = 0.0;
		double inPosition = 0.0;
		double inVelocity = 0.0;
		double inAcceleration = 0.0;

		double outTime = 0.0;
		double outPosition = 0.0;
		double outVelocity = 0.0;
		double outAcceleration = 0.0;

		//is possible
		bool isDefined = false;

		//for linear and kinematic interpolation
		double interpolationVelocity = 0.0;

		//for kinematic interpolation
		double rampInEndPosition = 0.0;		//position of curve after acceleration phase
		double rampInEndTime = 0.0;			//time of acceleration end
		double rampOutStartPosition = 0.0;	//position of deceleration start
		double rampOutStartTime = 0.0;		//time of deceleration start

		bool Interpolation::isTimeInside(double time);
		std::shared_ptr<Point> Interpolation::getPointAtTime(double time);
		double getProgressAtTime(double time);
	};

	class Curve {
	public:
		std::vector<std::shared_ptr<Interpolation>> interpolations;
		InterpolationType::Type interpolationType;

		void getPoints(std::vector<std::shared_ptr<Point>>& output);
		void addPoint(std::shared_ptr<Point> point);
		void removePoint(std::shared_ptr<Point> point);
		void refresh();

		std::shared_ptr<Point> getStart();
		std::shared_ptr<Point> getEnd();
		double getLength();
		bool isTimeInsideCurve(double time);
		std::shared_ptr<Point> getPointAtTime(double time);
	};




	//KINEMATIC POSITION INTERPOLATION

	//returns a profile using the position, velocity, acceleration and time data of the two specified points, while respecting the provided motion constraints
	bool getTimeConstrainedInterpolation(std::shared_ptr<Point>& startPoint, std::shared_ptr<Point>& endPoint, double maxVelocity, Interpolation& output);

	//return a profile using the position, velocity and acceleration values of the two specified points, using the specified velocity as a target while respecting the provided motion constraints
	bool getVelocityContrainedInterpolations(std::shared_ptr<Point>& startPoint, std::shared_ptr<Point>& endPoint, double velocity, std::vector<Interpolation>& output);

	//return a the fastest profile using the position, velocity and acceleration values of the two specified points, while respecting those motion constraints
	bool getFastestVelocityConstrainedInterpolation(std::shared_ptr<Point>& startPoint, std::shared_ptr<Point>& endPoint, double velocity, Interpolation& output);


}