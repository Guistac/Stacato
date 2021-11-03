#include <pch.h>

#include "Curve.h"

namespace Motion::StepInterpolation {

	void getInterpolation(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, std::shared_ptr<Interpolation>& output) {
	
		output->inTime = startPoint->time;
		output->inPosition = startPoint->position;
		output->inVelocity = 0.0;
		output->inAcceleration = 0.0;
		output->interpolationVelocity = 0.0;
		output->outTime = endPoint->time;
		output->outPosition = endPoint->position;
		output->outVelocity = 0.0;
		output->outAcceleration = 0.0;
		output->isDefined = true;

		startPoint->velocityOut = 0.0;
		startPoint->rampOut = 0.0;
		endPoint->velocityIn = 0.0;
		endPoint->rampIn = 0.0;
		startPoint->outInterpolation = output;
		endPoint->inInterpolation = output;
		output->inPoint = startPoint;
		output->outPoint = endPoint;

		output->type = InterpolationType::Type::STEP;

	}

}