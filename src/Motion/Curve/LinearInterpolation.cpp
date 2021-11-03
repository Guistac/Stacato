#include <pch.h>

#include "Curve.h"

namespace Motion::LinearInterpolation {
	

	void getTimeConstrainedInterpolation(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, std::shared_ptr<Interpolation>& output) {
		const double pi = startPoint->position;
		const double ti = startPoint->time;
		const double po = endPoint->position;
		const double to = endPoint->time;
		const double dp = po - pi;
		const double dt = to - ti;
		const double v = dp / dt;

		output->inTime = ti;
		output->inPosition = pi;
		output->inVelocity = v;
		output->inAcceleration = 0.0;
		output->interpolationVelocity = v;
		output->outTime = to;
		output->outPosition = po;
		output->outVelocity = v;
		output->outAcceleration = 0.0;
		output->isDefined = true;

		startPoint->velocityOut = v;
		startPoint->rampOut = 0.0;
		endPoint->velocityIn = v;
		endPoint->rampIn = 0.0;
		startPoint->outInterpolation = output;
		endPoint->inInterpolation = output;
		output->inPoint = startPoint;
		output->outPoint = endPoint;

		output->type = InterpolationType::Type::LINEAR;
	}

	void getVelocityConstrainedInterpolation(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, double velocity, std::shared_ptr<Interpolation>& output) {
		const double pi = startPoint->position;
		const double ti = startPoint->time;
		const double po = endPoint->position;
		const double v = velocity;
		const double dp = po - pi;
		const double dt = std::abs(dp / v);
		const double to = ti + dt;

		output->inTime = ti;
		output->inPosition = pi;
		output->inVelocity = v;
		output->inAcceleration = 0.0;
		output->interpolationVelocity = v;
		output->outTime = to;
		output->outPosition = po;
		output->outVelocity = v;
		output->outAcceleration = 0.0;
		output->isDefined = true;

		endPoint->time = to;
		startPoint->velocityOut = v;
		startPoint->rampOut = 0.0;
		endPoint->velocityIn = v;
		endPoint->rampIn = 0.0;
		startPoint->outInterpolation = output;
		endPoint->inInterpolation = output;
		output->inPoint = startPoint;
		output->outPoint = endPoint;

		output->type = InterpolationType::Type::LINEAR;
	}


}
