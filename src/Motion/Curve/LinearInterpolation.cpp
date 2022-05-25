#include <pch.h>

#include "Curve.h"

namespace Motion{

	std::shared_ptr<LinearInterpolation> LinearInterpolation::getTimeConstrained(std::shared_ptr<ControlPoint> startPoint,
																				 std::shared_ptr<ControlPoint> endPoint){
		
		
		const double pi = startPoint->position;
		const double ti = startPoint->time;
		const double po = endPoint->position;
		const double to = endPoint->time;
		const double dp = po - pi;
		const double dt = to - ti;
		const double v = dp / dt;
		
		std::shared_ptr<LinearInterpolation> output = std::make_shared<LinearInterpolation>();

		output->startTime = ti;
		output->startPosition = pi;
		output->endTime = to;
		output->endPosition = po;
		output->interpolationVelocity = v;
		
		output->inPoint = startPoint;
		output->outPoint = endPoint;
		
		output->b_valid = true;
		output->validationError = ValidationError::NONE;
		
		return output;
	}

	std::shared_ptr<LinearInterpolation> LinearInterpolation::getVelocityConstrained(std::shared_ptr<ControlPoint> startPoint,
																					 std::shared_ptr<ControlPoint> endPoint,
																					 double velocity){
		
		const double pi = startPoint->position;
		const double ti = startPoint->time;
		const double po = endPoint->position;
		const double v = velocity;
		const double dp = po - pi;
		const double dt = std::abs(dp / v);
		const double to = ti + dt;
		
		std::shared_ptr<LinearInterpolation> output = std::make_shared<LinearInterpolation>();
		
		output->startTime = ti;
		output->startPosition = pi;
		output->endTime = to;
		output->endPosition = po;
		output->interpolationVelocity = v;
		
		output->inPoint = startPoint;
		output->outPoint = endPoint;
		
		output->b_valid = true;
		output->validationError = ValidationError::NONE;

		return output;
	}


	Point LinearInterpolation::getPointAtTime(double time){
		if(time < startTime) return Point{
			.time = time,
			.position = startPosition,
			.velocity = 0.0,
			.acceleration = 0.0
		};
		else if(time > endTime) return Point{
			.time = time,
			.position = endPosition,
			.velocity = 0.0,
			.acceleration = 0.0
		};
		else return Point{
			.time = time,
			.position = startPosition + (time - startTime) * interpolationVelocity,
			.velocity = interpolationVelocity,
			.acceleration = 0.0
		};
	}

	void LinearInterpolation::updateDisplayCurvePoints(){
		displayPoints.clear();
		displayPoints.reserve(2);
		displayPoints.push_back(Point{.time = startTime, .position = startPosition, .velocity = interpolationVelocity, .acceleration = 0.0});
		displayPoints.push_back(Point{.time = endTime, .position = endPosition, .velocity = interpolationVelocity, .acceleration = 0.0});
		displayInflectionPoints.clear();
	}

	double LinearInterpolation::getNextIncrementTime(double previousPulseTime, double incrementsPerUnit){
		//TODO: implement for future use
		return DBL_MAX;
	}

};
