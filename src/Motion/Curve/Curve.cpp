#include <pch.h>

#include "Curve.h"

namespace Motion {



	bool Interpolation::containsTime(double time) {
		return time >= startTime && time < endTime;
	}

	float Interpolation::getProgressAtTime(double time){
		double progress = (time - startTime) / (endTime - startTime);
		progress = std::min(progress, 1.0);
		progress = std::max(progress, 0.0);
		return progress;
	}


	void Curve::addPoint(std::shared_ptr<ControlPoint> point) {
		idCounter++;
		point->id = idCounter;
		controlPoints.push_back(point);
		point->unit = unit;
		refresh();
	}

	void Curve::removePoint(std::shared_ptr<ControlPoint> point) {
		for (int i = 0; i < controlPoints.size() - 1; i++) {
			if (controlPoints[i] == point) {
				controlPoints.erase(controlPoints.begin() + i);
				break;
			}
		}
	}

	std::shared_ptr<ControlPoint> Curve::getStart() { return controlPoints.front(); }
	
	std::shared_ptr<ControlPoint> Curve::getEnd() { return controlPoints.back(); }

	double Curve::getLength() { return getEnd()->time - getStart()->time; }

	bool Curve::containsTime(double time) { return time >= getStart()->time && time < getEnd()->time; }

	Point Curve::getPointAtTime(double time) {
		for (auto& interpolation : interpolations) {
			if (interpolation->containsTime(time)) {
				return interpolation->getPointAtTime(time);
			}
		}
		if (time < getStart()->time) {
			return Point{
				.position = getStart()->position,
				.time = time,
				.velocity = 0.0,
				.acceleration = 0.0
			};
		}
		else {
			return Point{
				.position = getEnd()->position,
				.time = time,
				.velocity = 0.0,
				.acceleration = 0.0
			};
		}
	}

	void Curve::updateDisplayCurvePoints(){
		for (auto& interpolation : interpolations) interpolation->updateDisplayCurvePoints();
	}

	void Curve::refresh() {
		
		//remove all previous interpolations
		interpolations.clear();
		
		//sort all control points in chronological order
		std::sort(controlPoints.begin(),
				  controlPoints.end(),
				  [](std::shared_ptr<Motion::ControlPoint>& a, std::shared_ptr<Motion::ControlPoint>& b) -> bool { return a->time < b->time; });
		
		for(auto controlPoint : controlPoints){
			//TURN DISPLAY HACK
			controlPoint->turnCount = std::floor(controlPoint->position / 360.0);
			controlPoint->singleturndegrees = controlPoint->position - controlPoint->turnCount * 360.0;
		}

		//build and append interpolations between all control points
		for (int i = 0; i < controlPoints.size() - 1; i++) {
			std::shared_ptr<Motion::ControlPoint>& inPoint = controlPoints[i];
			std::shared_ptr<Motion::ControlPoint>& outPoint = controlPoints[i + 1];
			std::shared_ptr<Motion::Interpolation> interpolation;
			switch (interpolationType) {
				case InterpolationType::STEP:
					interpolation = Motion::StepInterpolation::getInterpolation(inPoint, outPoint);
					break;
				case InterpolationType::LINEAR:
					interpolation = Motion::LinearInterpolation::getTimeConstrained(inPoint, outPoint);
					break;
				case InterpolationType::BEZIER: //not supported yet:
					interpolation = Motion::LinearInterpolation::getTimeConstrained(inPoint, outPoint);
					break;
				case InterpolationType::TRAPEZOIDAL:
					interpolation = TrapezoidalInterpolation::getTimeConstrained(inPoint, outPoint);
					break;
			}
			interpolations.push_back(interpolation);
		}
		
		//update point previsualisation data
		updateDisplayCurvePoints();
	}

};
