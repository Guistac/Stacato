#pragma once

#include "Motion/MotionTypes.h"

namespace Motion {

	enum class InterpolationType {
		STEP,
		LINEAR,
		TRAPEZOIDAL,
		BEZIER
	};

	enum class ValidationError {
		NO_VALIDATION_ERROR,
		CONTROL_POINT_POSITION_OUT_OF_RANGE,
		CONTROL_POINT_VELOCITY_LIMIT_EXCEEDED,
		CONTROL_POINT_INPUT_ACCELERATION_LIMIT_EXCEEDED,
		CONTROL_POINT_OUTPUT_ACCELERATION_LIMIT_EXCEEDED,
		CONTROL_POINT_INPUT_ACCELERATION_IS_ZERO,
		CONTROL_POINT_OUTPUT_ACCELERATION_IS_ZERO,
		INTERPOLATION_UNDEFINED,
		INTERPOLATION_VELOCITY_LIMIT_EXCEEDED,
		INTERPOLATION_POSITION_OUT_OF_RANGE,
		INTERPOLATION_INPUT_ACCELERATION_IS_ZERO,
		INTERPOLATION_OUTPUT_ACCELERATION_IS_ZERO
	};

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
		bool b_valid = false;
		ValidationError validationError = ValidationError::NO_VALIDATION_ERROR;
	};

	class Interpolation {
	public:

		InterpolationType type;
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

		//for linear and kinematic interpolation
		double interpolationVelocity = 0.0;

		//for kinematic interpolation
		double rampInEndPosition = 0.0;		//position of curve after acceleration phase
		double rampInEndTime = 0.0;			//time of acceleration end
		double rampOutStartPosition = 0.0;	//position of deceleration start
		double rampOutStartTime = 0.0;		//time of deceleration start

		bool isTimeInside(double time);
		CurvePoint getPointAtTime(double time);
		double getProgressAtTime(double time);

		void resetValues();

		void updateDisplayCurvePoints();
		std::vector<CurvePoint> displayPoints;
		std::vector<CurvePoint> displayInflectionPoints;

		//reports error when calculating the interpolation
		bool b_valid = false;
		ValidationError validationError = ValidationError::NO_VALIDATION_ERROR;
	};

	class Curve {
	public:
		std::vector<std::shared_ptr<ControlPoint>> points;
		InterpolationType interpolationType;
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

		bool b_valid = false;
	};



	namespace TrapezoidalInterpolation {
		//returns a profile using the position, velocity, acceleration and time data of the two specified points, while respecting the provided motion constraints
		bool getTimeConstrainedInterpolation(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, std::shared_ptr<Interpolation>& output);
		//return the fastest profile using the position, velocity and acceleration values of the two specified points, while respecting those motion constraints
		bool getFastestVelocityConstrainedInterpolation(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, double velocity, std::shared_ptr<Interpolation>& output);
		//get the best matching time based profile, not faster than the specified time
		bool getClosestTimeAndVelocityConstrainedInterpolation(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, double maxVelocity, std::shared_ptr<Interpolation>& output);
	}

	namespace LinearInterpolation {
		void getTimeConstrainedInterpolation(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, std::shared_ptr<Interpolation>& output);
		void getVelocityConstrainedInterpolation(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, double velocity, std::shared_ptr<Interpolation>& output);
	}

	namespace StepInterpolation {
		void getInterpolation(std::shared_ptr<ControlPoint>& startPoints, std::shared_ptr<ControlPoint>& endPoint, std::shared_ptr<Interpolation>& output);
	}

}

#define CurveValidationErrorTypeStrings \
	{Motion::ValidationError::NO_VALIDATION_ERROR,								"No Validation Error"},\
	{Motion::ValidationError::CONTROL_POINT_POSITION_OUT_OF_RANGE,				"Point Out of Range"},\
	{Motion::ValidationError::CONTROL_POINT_VELOCITY_LIMIT_EXCEEDED,			"Point Velocity Limit Exceeded"},\
	{Motion::ValidationError::CONTROL_POINT_INPUT_ACCELERATION_LIMIT_EXCEEDED,	"Point Input Acceleration Limit Exceeded"},\
	{Motion::ValidationError::CONTROL_POINT_OUTPUT_ACCELERATION_LIMIT_EXCEEDED,	"Point Output Acceleration Limit Exceeded"},\
	{Motion::ValidationError::CONTROL_POINT_INPUT_ACCELERATION_IS_ZERO,			"Point Input Acceleration Is Zero"},\
	{Motion::ValidationError::CONTROL_POINT_OUTPUT_ACCELERATION_IS_ZERO,		"Point Output Acceleration Is Zero"},\
	{Motion::ValidationError::INTERPOLATION_UNDEFINED,							"Interpolation Undefined"},\
	{Motion::ValidationError::INTERPOLATION_VELOCITY_LIMIT_EXCEEDED,			"Interpolation Velocity Limit Exceeded"},\
	{Motion::ValidationError::INTERPOLATION_POSITION_OUT_OF_RANGE,				"Interpolation Position Out of Range"},\
	{Motion::ValidationError::INTERPOLATION_INPUT_ACCELERATION_IS_ZERO,			"Interpolation Input Acceleration is Zero"},\
	{Motion::ValidationError::INTERPOLATION_OUTPUT_ACCELERATION_IS_ZERO,		"Interpolation Output Acceleration Is Zero"}\

DEFINE_ENUMERATOR(Motion::ValidationError, CurveValidationErrorTypeStrings)

#define InterpolationTypeStrings \
	{Motion::InterpolationType::STEP,			"Step", "Step"},\
	{Motion::InterpolationType::LINEAR,			"Linear", "Linear"},\
	{Motion::InterpolationType::TRAPEZOIDAL,	"Trapezoidal", "Trapezoidal"},\
	{Motion::InterpolationType::BEZIER,			"Bezier", "Bezier"}\

DEFINE_ENUMERATOR(Motion::InterpolationType, InterpolationTypeStrings)
