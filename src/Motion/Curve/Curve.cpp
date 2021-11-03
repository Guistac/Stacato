#include <pch.h>

#include "Curve.h"

namespace Motion {

	bool Interpolation::isTimeInside(double time) {
		return time >= inTime && time < outTime;
	}

	CurvePoint Interpolation::getPointAtTime(double time) {
		CurvePoint output;
		output.time = time;
		switch (type) {
			case InterpolationType::Type::STEP:
				if (time < inTime || time < outTime) output.position = inPoint->position;
				else output.position = outPoint->position;
				output.velocity = 0.0;
				output.acceleration = 0.0;
				break;
			case InterpolationType::Type::LINEAR:
				if (time < inTime) {
					output.position = inPoint->position;
					output.velocity = 0.0;
				}
				else if (time < outTime) {
					output.position = inPoint->position + (time - inPoint->time) * interpolationVelocity;
					output.velocity = interpolationVelocity;
				}
				else {
					output.position = outPoint->position;
					output.velocity = 0.0;
				}
				output.acceleration = 0.0;
				break;
			case InterpolationType::Type::BEZIER:
				output.position = inPoint->position;
				break;
			case InterpolationType::Type::TRAPEZOIDAL: {
				if (time < inTime) {
					output.position = inPosition;
					output.velocity = inVelocity;
					output.acceleration = 0.0;
				}
				else if (time < rampInEndTime) {
					double deltaT = time - inTime;
					output.position = inPosition + inVelocity * deltaT + inAcceleration * std::pow(deltaT, 2.0) / 2.0;
					output.velocity = inVelocity + inAcceleration * deltaT;
					output.acceleration = inAcceleration;
				}
				else if (time < rampOutStartTime) {
					double deltaT = time - rampInEndTime;
					output.position = rampInEndPosition + deltaT * interpolationVelocity;
					output.velocity = interpolationVelocity;
					output.acceleration = 0.0;
				}
				else if (time < outTime) {
					double deltaT = time - rampOutStartTime;
					output.position = rampOutStartPosition + interpolationVelocity * deltaT + outAcceleration * std::pow(deltaT, 2.0) / 2.0;
					output.velocity = interpolationVelocity + outAcceleration * deltaT;
					output.acceleration = outAcceleration;
				}
				else {
					output.position = outPosition;
					output.velocity = outVelocity;
					output.acceleration = 0.0;
				}
			}break;

		}
		return output;
	}

	double Interpolation::getProgressAtTime(double time) {
		double progress = (time - inTime) / (outTime - inTime);
		if (progress > 1.0) return 1.0;
		else if (progress < 0.0) return 0.0;
		return progress;
	}

	void Interpolation::resetValues() {
		inPoint = nullptr;
		outPoint = nullptr;
		inTime = 0.0;
		inPosition = 0.0;
		inVelocity = 0.0;
		inAcceleration = 0.0;
		outTime = 0.0;
		outPosition = 0.0;
		outVelocity = 0.0;
		outAcceleration = 0.0;
		isDefined = false;
		interpolationVelocity = 0.0;
		rampInEndPosition = 0.0;	
		rampInEndTime = 0.0;		
		rampOutStartPosition = 0.0;	
		rampOutStartTime = 0.0;		
	}

	void Interpolation::updateDisplayCurvePoints() {
		displayPoints.clear();
		switch (type) {
			case InterpolationType::Type::STEP:
				displayPoints.reserve(3);
				displayPoints.push_back(CurvePoint(inTime, inPosition, 0.0, 0.0));
				displayPoints.push_back(CurvePoint(outTime, inPosition, 0.0, 0.0));
				displayPoints.push_back(CurvePoint(outTime, outPosition, 0.0, 0.0));
				break;
			case InterpolationType::Type::LINEAR:
				displayPoints.reserve(2);
				displayPoints.push_back(CurvePoint(inTime, inPosition, 0.0, 0.0));
				displayPoints.push_back(CurvePoint(outTime, outPosition, 0.0, 0.0));
				break;
			case InterpolationType::Type::BEZIER:
				displayPoints.reserve(16);
				break;
			case InterpolationType::Type::TRAPEZOIDAL:
				displayPoints.reserve(32);
				double rampInLength_seconds = rampInEndTime - inTime;
				for (int i = 0; i < 16; i++) {
					double time_seconds = inTime + rampInLength_seconds * i / (16 - 1);
					displayPoints.push_back(getPointAtTime(time_seconds));
				}
				double rampOutLength_seconds = outTime - rampOutStartTime;
				for (int i = 0; i < 16; i++) {
					double time_seconds = rampOutStartTime + rampOutLength_seconds * i / (16 - 1);
					displayPoints.push_back(getPointAtTime(time_seconds));
				}
				break;
		}
	}

	std::vector<std::shared_ptr<ControlPoint>>& Curve::getPoints() {
		return points;
	}

	void Curve::addPoint(std::shared_ptr<ControlPoint> point) {
		points.push_back(point);
	}

	void Curve::removePoint(std::shared_ptr<ControlPoint> point) {
		for (int i = 0; i < points.size(); i++) {
			if (points[i] == point) {
				points.erase(points.begin() + i);
				break;
			}
		}
	}

	void Curve::removeAllPoints() {
		points.clear();
		interpolations.clear();
	}

	void Curve::refresh() {
		
		//remove all previous interpolations
		interpolations.clear();
		
		auto sortfunction = [](std::shared_ptr<Motion::ControlPoint>& a, std::shared_ptr<Motion::ControlPoint>& b) -> bool { return a->time < b->time; };
		std::sort(points.begin(), points.end(), sortfunction);

		for (int i = 0; i < points.size() - 1; i++) {
			std::shared_ptr<Motion::ControlPoint>& inPoint = points[i];
			std::shared_ptr<Motion::ControlPoint>& outPoint = points[i + 1];
			std::shared_ptr<Motion::Interpolation> interpolation = std::make_shared<Motion::Interpolation>();
			switch (interpolationType) {
				case InterpolationType::Type::STEP:
					Motion::StepInterpolation::getInterpolation(inPoint, outPoint, interpolation);
					break;
				case InterpolationType::Type::LINEAR:
					Motion::LinearInterpolation::getTimeConstrainedInterpolation(inPoint, outPoint, interpolation);
					break;
				case InterpolationType::Type::BEZIER:
					//not supported yet:
					break;
				case InterpolationType::Type::TRAPEZOIDAL:
					Motion::TrapezoidalInterpolation::getTimeConstrainedInterpolation(inPoint, outPoint, 100, interpolation);
					break;
			}
			interpolations.push_back(interpolation);
		}
		updateDisplayCurvePoints();
	}

	std::shared_ptr<ControlPoint> Curve::getStart() {
		return points.front();
	}

	std::shared_ptr<ControlPoint> Curve::getEnd() {
		return points.back();
	}

	double Curve::getLength() {
		return getEnd()->time - getStart()->time;
	}

	bool Curve::isTimeInsideCurve(double time) {
		return time >= getStart()->time && time < getEnd()->time;
	}

	CurvePoint Curve::getPointAtTime(double time) {
		if (time < getStart()->time) {
			CurvePoint output;
			output.position = getStart()->position;
			output.time = time;
			output.velocity = 0.0;
			output.acceleration = 0.0;
			return output;
		}
		else if (time >= getEnd()->time) {
			CurvePoint output;
			output.position = getEnd()->position;
			output.time = time;
			output.velocity = 0.0;
			output.acceleration = 0.0;
			return output;
		}
		else {
			for (auto& interpolation : interpolations) {
				if (interpolation->isTimeInside(time)) {
					return interpolation->getPointAtTime(time);
				}
			}
		}
	}


	void Curve::updateDisplayCurvePoints() {
		for (auto& interpolation : interpolations) {
			interpolation->updateDisplayCurvePoints();
		}
	}

};