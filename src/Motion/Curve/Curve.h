#pragma once

#include "Motion/MotionTypes.h"
#include "Utilities/Units.h"

namespace Motion {

	class Interpolation;

	enum class ValidationError {
		NONE,
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

	struct Point{
		double time;
		double position;
		double velocity;
		double acceleration;
	};

	struct ControlPoint{
		double position;
		double velocity;
		double inAcceleration;
		double outAcceleration;
		double time;
		bool b_valid;
		ValidationError validationError;
		bool b_selected = false;
		int id;
		Unit unit = Units::None::None;
		Point toPoint(){
			return Point{.time = time, .position = position, .acceleration = outAcceleration, .velocity = velocity};
		}
		
		//TURN DISPLAY HACK
		double singleturndegrees = 0.f;
		int turnCount = 0;
		
		std::shared_ptr<Interpolation> inInterpolation;
		std::shared_ptr<Interpolation> outInterpolation;
		
		std::shared_ptr<ControlPoint> copy(){
			auto pointCopy = std::make_shared<ControlPoint>();
			pointCopy->position = position;
			pointCopy->velocity = velocity;
			pointCopy->inAcceleration = inAcceleration;
			pointCopy->outAcceleration = outAcceleration;
			pointCopy->time = time;
			pointCopy->b_valid = b_valid;
			pointCopy->validationError = validationError;
			return pointCopy;
		}
	};

	class TrapezoidalInterpolation;
	class LinearInterpolation;
	class StepInterpolation;

	class Interpolation : public std::enable_shared_from_this<Interpolation>{
	public:
		
		virtual InterpolationType getType() = 0;
		std::shared_ptr<TrapezoidalInterpolation> castToTrapezoidal(){ return std::static_pointer_cast<TrapezoidalInterpolation>(shared_from_this()); }
		std::shared_ptr<LinearInterpolation> castToLinear(){ return std::static_pointer_cast<LinearInterpolation>(shared_from_this()); }
		std::shared_ptr<StepInterpolation> castToStep() { return std::static_pointer_cast<StepInterpolation>(shared_from_this()); }
	
		double startPosition;
		double startTime;
		double endPosition;
		double endTime;
		
		bool b_valid = false;
		ValidationError validationError;

		bool containsTime(double time);
		float getProgressAtTime(double time);
		double getDuration(){ return endTime - startTime; }
		
		virtual Point getPointAtTime(double time) = 0;
		virtual double getNextIncrementTime(double previousPulseTime, double incrementsPerUnit) = 0;
		
		std::shared_ptr<ControlPoint> inPoint;
		std::shared_ptr<ControlPoint> outPoint;
		
		virtual void updateDisplayCurvePoints() = 0;
		std::vector<Point> displayPoints;
		std::vector<Point> displayInflectionPoints;
	};


	class TrapezoidalInterpolation : public Interpolation{
	public:
		
		virtual InterpolationType getType() override { return InterpolationType::TRAPEZOIDAL; }
		
		enum class Phase{
			NOT_STARTED,
			RAMP_IN,
			COAST,
			RAMP_OUT,
			FINISHED
		};
		
		double startVelocity;
		double startAcceleration;

		double coastStartTime;
		double coastStartPosition;
		double coastVelocity;
		double coastEndTime;
		double coastEndPosition;

		double endVelocity;
		double endAcceleration;
		
		virtual Point getPointAtTime(double time) override;
		virtual double getNextIncrementTime(double previousPulseTime, double incrementsPerUnit) override;
		virtual void updateDisplayCurvePoints() override;
		
		//returns a profile using the position, velocity, acceleration and time data of the two specified points, while respecting the provided motion constraints
		static std::shared_ptr<TrapezoidalInterpolation> getTimeConstrained(std::shared_ptr<ControlPoint>& startPoint,
																			std::shared_ptr<ControlPoint>& endPoint);
		
		//return the fastest profile using the position, velocity and acceleration values of the two specified points, while respecting those motion constraints
		static std::shared_ptr<TrapezoidalInterpolation> getVelocityConstrained(std::shared_ptr<ControlPoint>& startPoint,
																				std::shared_ptr<ControlPoint>& endPoint,
																				double velocity);
		
		//get the best matching time based profile, not faster than the specified time
		static std::shared_ptr<TrapezoidalInterpolation> getTimeConstrainedOrSlower(std::shared_ptr<ControlPoint>& startPoint,
																					std::shared_ptr<ControlPoint>& endPoint,
																					double maxVelocity);
		
	private:
		Phase getPhaseAtTime(double time);
		static Phase getNextPhase(Phase phase);
		Point getPointAtPhaseTime(double time, Phase phase);
		enum class Solution{
			NONE,
			SINGLE,
			DOUBLE
		};
		Solution getTimeAtPosition(Phase phase, double position, double& time_a, double& time_b);
	};



	class LinearInterpolation : public Interpolation{
	public:
		
		virtual InterpolationType getType() override { return InterpolationType::LINEAR; }
		
		double interpolationVelocity;
		
		virtual Point getPointAtTime(double time) override;
		virtual double getNextIncrementTime(double previousPulseTime, double incrementsPerUnit) override;
		virtual void updateDisplayCurvePoints() override;
		
		static std::shared_ptr<LinearInterpolation> getTimeConstrained(std::shared_ptr<ControlPoint> startPoint,
																	   std::shared_ptr<ControlPoint> endPoint);
		static std::shared_ptr<LinearInterpolation> getVelocityConstrained(std::shared_ptr<ControlPoint> startPoint,
																		   std::shared_ptr<ControlPoint> endPoint,
																		   double velocity);
	};

	class StepInterpolation : public Interpolation{
	public:
		
		virtual InterpolationType getType() override { return InterpolationType::STEP; }
		
		virtual Point getPointAtTime(double time) override;
		virtual double getNextIncrementTime(double previousPulseTime, double incrementsPerUnit) override;
		virtual void updateDisplayCurvePoints() override;
		
		static std::shared_ptr<StepInterpolation> getInterpolation(std::shared_ptr<ControlPoint> startPoint,
																   std::shared_ptr<ControlPoint> endPoint);
	};


	class Curve{
	public:
		
		Curve(std::string name_) : name(name_){}
		
		std::shared_ptr<Curve> copy(){
			auto curveCopy = std::make_shared<Motion::Curve>(name);
			curveCopy->interpolationType = interpolationType;
			curveCopy->b_valid = b_valid;
			curveCopy->b_visibleInEditor = b_visibleInEditor;
			for(auto controlPoint : controlPoints){
				curveCopy->addPoint(controlPoint->copy());
			}
			return curveCopy;
		}
		
		std::vector<std::shared_ptr<ControlPoint>>& getPoints(){ return controlPoints; }
		void addPoint(std::shared_ptr<ControlPoint> point);
		void removePoint(std::shared_ptr<ControlPoint> point);
		void refresh();
		
		std::vector<std::shared_ptr<Interpolation>>& getInterpolations(){ return interpolations; }
		
		void updateDisplayCurvePoints();
		
		std::shared_ptr<ControlPoint> getStart();
		std::shared_ptr<ControlPoint> getEnd();
		double getLength();
		bool containsTime(double time);
		Point getPointAtTime(double time);
		
		bool b_valid;
		bool b_visibleInEditor = true;
		
		std::string getName(){ return name; }
		
		int idCounter = 0;
		
		InterpolationType interpolationType;
		
		Unit unit = Units::None::None;
		
	private:
		std::vector<std::shared_ptr<ControlPoint>> controlPoints;
		std::vector<std::shared_ptr<Interpolation>> interpolations;
		std::string name;
	};

}

#define CurveValidationErrorTypeStrings \
	{Motion::ValidationError::NONE,												"No Validation Error"},\
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
