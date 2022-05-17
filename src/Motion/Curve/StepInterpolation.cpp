#include <pch.h>

#include "Curve.h"

namespace Motion{
	std::shared_ptr<StepInterpolation> StepInterpolation::getInterpolation(std::shared_ptr<ControlPoint> startPoint,
																		   std::shared_ptr<ControlPoint> endPoint){
		
		std::shared_ptr<StepInterpolation> output = std::make_shared<StepInterpolation>();
		
		output->startTime = startPoint->time;
		output->startPosition = startPoint->position;
		
		output->endTime = endPoint->time;
		output->endPosition = endPoint->position;
		
		output->inPoint = startPoint;
		output->outPoint = endPoint;
		
		output->b_valid = true;
		output->validationError = ValidationError::NONE;
		
		return output;

	}

	Point StepInterpolation::getPointAtTime(double time){
		return Point{
			.time = time,
			.position = startPosition,
			.velocity = 0.0,
			.acceleration = 0.0
		};
	}

	void StepInterpolation::updateDisplayCurvePoints(){
		displayPoints.clear();
		displayPoints.reserve(3);
		displayPoints.push_back(Point{.time = startTime, .position = startPosition});
		displayPoints.push_back(Point{.time = endTime, .position = startPosition});
		displayPoints.push_back(Point{.time = endTime, .position = endPosition});
		displayInflectionPoints.clear();
		displayInflectionPoints.push_back(Point{.time = endTime, .position = startPosition});
	}

	double StepInterpolation::getNextIncrementTime(double previousPulseTime, double incrementsPerUnit){
		//TODO: implement this for future use
		return DBL_MAX;
	}


};
