#include <pch.h>

#include "Curve.h"

namespace Motion {

	bool Interpolation::isTimeInside(double time) {
		return time >= inPoint->time && time < outPoint->time;
	}

	std::shared_ptr<Point> Interpolation::getPointAtTime(double time) {
		std::shared_ptr<Point> output = std::make_shared<Point>();
		output->time = time;
		switch (type) {
			case InterpolationType::Type::STEP:
				output->position = inPoint->position;
				break;
			case InterpolationType::Type::LINEAR:
				output->position = inPoint->position + (time - inPoint->time) * interpolationVelocity;
				break;
			case InterpolationType::Type::BEZIER:
				output->position = inPoint->position;
				break;
			case InterpolationType::Type::TRAPEZOIDAL: {
				if (time < inTime) {
					output->position = inPosition;
					output->velocity = inVelocity;
					output->acceleration = 0.0;
				}
				else if (time < rampInEndTime) {
					double deltaT = time - inTime;
					output->position = inPosition + inVelocity * deltaT + inAcceleration * std::pow(deltaT, 2.0) / 2.0;
					output->velocity = inVelocity + inAcceleration * deltaT;
					output->acceleration = inAcceleration;
				}
				else if (time < rampOutStartTime) {
					double deltaT = time - rampInEndTime;
					output->position = rampInEndPosition + deltaT * interpolationVelocity;
					output->velocity = interpolationVelocity;
					output->acceleration = 0.0;
				}
				else if (time < outTime) {
					double deltaT = time - rampOutStartTime;
					output->position = rampOutStartPosition + interpolationVelocity * deltaT + outAcceleration * std::pow(deltaT, 2.0) / 2.0;
					output->velocity = interpolationVelocity + outAcceleration * deltaT;
					output->acceleration = outAcceleration;
				}
				else {
					output->position = outPosition;
					output->velocity = outVelocity;
					output->acceleration = 0.0;
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

	void Curve::getPoints(std::vector<std::shared_ptr<Point>>& output) {
		output.reserve(interpolations.size());
		for (auto& interpolation : interpolations) {
			output.push_back(interpolation->inPoint);
		}
		output.push_back(interpolations.back()->outPoint);
	}

	void Curve::addPoint(std::shared_ptr<Point> point) {
		for (auto& interpolation : interpolations) {
			if (interpolation->isTimeInside(point->time)) {



			}
		}
	}

	void Curve::removePoint(std::shared_ptr<Point> point) {
	
	
	}

	void Curve::refresh() {
	
		//reorder points based on their timeand recalculate interpolation values
	
	}

	std::shared_ptr<Point> Curve::getStart() {
		return interpolations.front()->inPoint;
	}

	std::shared_ptr<Point> Curve::getEnd() {
		return interpolations.back()->outPoint;
	}

	double Curve::getLength() {
		return getEnd()->time - getStart()->time;
	}

	bool Curve::isTimeInsideCurve(double time) {
		return time >= getStart()->time && time < getEnd()->time;
	}

	std::shared_ptr<Point> Curve::getPointAtTime(double time) {
		if (time < getStart()->time) {
			std::shared_ptr<Point> output = std::make_shared<Point>();
			output->position = getStart()->position;
			output->time = time;
			return output;
		}
		else if (time >= getStart()->time) {
			std::shared_ptr<Point> output = std::make_shared<Point>();
			output->position = getEnd()->position;
			output->time = time;
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

	std::vector<SequenceType::Type> Curve::getCompatibleSequenceTypes() {
		std::vector<SequenceType::Type> output;
		switch (interpolationType) {
			case InterpolationType::Type::STEP:
				output.push_back(SequenceType::Type::STEP_MOVE);
				output.push_back(SequenceType::Type::ANIMATED_MOVE);
				break;
			case InterpolationType::Type::LINEAR:
				output.push_back(SequenceType::Type::TIMED_MOVE);
				output.push_back(SequenceType::Type::ANIMATED_MOVE);
				break;
			case InterpolationType::Type::BEZIER:
				output.push_back(SequenceType::Type::TIMED_MOVE);
				output.push_back(SequenceType::Type::ANIMATED_MOVE);
				break;
			case InterpolationType::Type::TRAPEZOIDAL:
				output.push_back(SequenceType::Type::TIMED_MOVE);
				output.push_back(SequenceType::Type::VELOCITY_MOVE);
				output.push_back(SequenceType::Type::ANIMATED_MOVE);
				break;
		}
		return output;
	}

};