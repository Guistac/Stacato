#include <pch.h>

#include "Gui.h"

#include "Motion/MotionCurve.h"

void sequencer() {

	static bool init = true;

	static MotionCurve::MotionConstraints constraints;
	static MotionCurve::CurvePoint startPoint;
	static MotionCurve::CurvePoint endPoint;
	static double velocity = 0.2;

	if (init) {
		startPoint.position = 0.0;
		startPoint.time = 0.0;
		startPoint.velocity = 0.0;
		startPoint.acceleration = 0.1;
		endPoint.position = 1.0;
		endPoint.time = 1.0;
		endPoint.velocity = 0.0;
		endPoint.acceleration = 0.1;
		constraints.maxAcceleration = 10000.0;
		constraints.maxVelocity = 100000.0;
		init = false;
	}

	float inputWidth = 300.0;
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("Vi", &startPoint.velocity, 0.01, 0.1);
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("Ai", &startPoint.acceleration, 0.01, 0.1);
	ImGui::Separator();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("Po", &endPoint.position, 0.01, 0.1);
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("Vo", &endPoint.velocity, 0.01, 0.1);
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("Ao", &endPoint.acceleration, 0.01, 0.1);
	ImGui::Separator();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("V", &velocity, 0.01, 0.1);


	//calculate total delta P of the curve (negative for negative moves)
	double totalDeltaPosition = endPoint.position - startPoint.position;

	//get stopping distance from initial velocity
	double rampInStoppingDeltaPosition = std::pow(startPoint.velocity, 2.0) / (2.0 * std::abs(startPoint.acceleration));
	if (startPoint.velocity < 0) rampInStoppingDeltaPosition *= -1.0;

	//get stopping distance from end velocity
	double rampOutStoppingDeltaPosition = std::pow(endPoint.velocity, 2.0) / (2.0 * std::abs(endPoint.acceleration));
	if (endPoint.velocity < 0) rampOutStoppingDeltaPosition *= -1.0;

	double totalRampStoppingDeltaPosition = rampInStoppingDeltaPosition + rampOutStoppingDeltaPosition;

	bool exceededSomething;
	if (totalDeltaPosition) exceededSomething = totalDeltaPosition < totalRampStoppingDeltaPosition;
	else exceededSomething = totalDeltaPosition > totalRampStoppingDeltaPosition;

	//calculate stopping position from the stopping deltas and start/end positions
	double rampInStoppingPosition = startPoint.position + rampInStoppingDeltaPosition;
	double rampOutStoppingPosition = endPoint.position - rampOutStoppingDeltaPosition;

	//calculate the delta between the stopping positions
	double rampStoppingDeltaPosition = rampOutStoppingPosition - rampInStoppingPosition;

	//use the delta to determine the sign of the constant velocity
	double constantVelocity = rampStoppingDeltaPosition > 0.0 ? std::abs(velocity) : -std::abs(velocity);

	//use the constant velocity to get the signs of in and out accelerations
	double rampInAcceleration = constantVelocity > startPoint.velocity ? std::abs(startPoint.acceleration) : -std::abs(startPoint.acceleration);
	double rampOutAcceleration = constantVelocity < endPoint.velocity ? std::abs(endPoint.acceleration) : -std::abs(endPoint.acceleration);

	//get the two delta p associated with the ramp in and ramp out phases
	double rampInDeltaPosition = (std::pow(constantVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);
	double rampOutDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(constantVelocity, 2.0)) / (2 * rampOutAcceleration);

	double rampInDeltaTime = (constantVelocity - startPoint.velocity) / rampInAcceleration;
	double rampOutDeltaTime = (endPoint.velocity - constantVelocity) / rampOutAcceleration;

	bool constantVelocityReachable;
	if (totalDeltaPosition > 0) constantVelocityReachable = rampInDeltaPosition + rampOutDeltaPosition < totalDeltaPosition;
	else constantVelocityReachable = rampInDeltaPosition + rampOutDeltaPosition > totalDeltaPosition;

	if ((rampStoppingDeltaPosition < 0 && totalDeltaPosition > 0) || (rampStoppingDeltaPosition > 0 && totalDeltaPosition < 0)) {
		constantVelocityReachable = !constantVelocityReachable;
	}

	if (!constantVelocityReachable) {

		//since we can't reach the constant velocity, we calculate a new ramp transition velocity
		//the simplest way to do this is by first calculating the position delta of the ramp transition
		double rampTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * totalDeltaPosition - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration - 2 * rampOutAcceleration);

		//we then can use this delta to calculate the transition velocity, taking care to set its sign equal to the initial constant velocity
		double rampTransitionVelocity = std::sqrt(std::pow(startPoint.velocity, 2.0) + 2.0 * rampInAcceleration * rampTransitionDeltaPosition);
		if (rampStoppingDeltaPosition < 0.0) rampTransitionVelocity *= -1.0;

		if (rampTransitionVelocity > velocity) {
			rampTransitionVelocity = velocity;
			rampInDeltaPosition = (std::pow(rampTransitionVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);
		}
		else if (rampTransitionVelocity < -velocity) {
			rampTransitionVelocity = -velocity;
			rampInDeltaPosition = (std::pow(rampTransitionVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);
		}

		//we also need to recalculate the rampOut delta, since we can't reach constant velocity
		rampOutDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(rampTransitionVelocity, 2.0)) / (2 * rampOutAcceleration);

		constantVelocity = rampTransitionVelocity;
		rampInDeltaPosition = rampTransitionDeltaPosition;

		rampInDeltaTime = (rampTransitionVelocity - startPoint.velocity) / rampInAcceleration;
		rampOutDeltaTime = (endPoint.velocity - rampTransitionVelocity) / rampOutAcceleration;
	}

	//end position of ramp-In is start + ramping delta
	double rampInEndPosition = startPoint.position + rampInDeltaPosition;

	//end time of ramp-In is startTime + rampIn delta time
	double rampInEndTime = startPoint.time + rampInDeltaTime;

	//ramp out start position is end position minus rampOut Delta
	double rampOutStartPosition = endPoint.position - rampOutDeltaPosition;

	//ramp out start time is rampIn end time + time of constant velocity coast
	double rampOutStartTime = rampInEndTime + (rampOutStartPosition - rampInEndPosition) / constantVelocity;

	//end of curve time is ramp out start time + ramp out delta time
	double curveEndTime = rampOutStartTime + rampOutDeltaTime;


	MotionCurve::CurveProfile profile;

	profile.rampInStartTime = startPoint.time;			//time of curve start
	profile.rampInStartPosition = startPoint.position;	//position of curve start
	profile.rampInStartVelocity = startPoint.velocity;	//velocity at curve start
	profile.rampInAcceleration = rampInAcceleration;	//acceleration of curve
	profile.rampInEndPosition = rampInEndPosition;		//position of curve after acceleration phase
	profile.rampInEndTime = rampInEndTime;			    //time of acceleration end
	profile.coastVelocity = constantVelocity;			//velocity of constant velocity phase
	profile.rampOutStartPosition = rampOutStartPosition;//position of deceleration start
	profile.rampOutStartTime = rampOutStartTime;		//time of deceleration start
	profile.rampOutAcceleration = rampOutAcceleration;	//deceleration of curve
	profile.rampOutEndTime = curveEndTime;			    //time of curve end
	profile.rampOutEndPosition = endPoint.position;		//position of curve end
	profile.rampOutEndVelocity = endPoint.velocity;		//velocity of curve end
	
	int pointCount = 200;
	double deltaT = (profile.rampOutEndTime - profile.rampInStartTime) / pointCount;
	std::vector<MotionCurve::CurvePoint> points;
	for (int i = 0; i <= pointCount; i++) {
		double T = profile.rampInStartTime + i * deltaT;
		points.push_back(MotionCurve::getCurvePointAtTime(T, profile));
	}

	if (ImPlot::BeginPlot("##curveTest", 0, 0, ImVec2(ImGui::GetContentRegionAvail().x, 800.0), ImPlotFlags_None)) {
	
		ImPlot::DragLineY("end", &endPoint.position);
	
		ImPlot::DragPoint("rampInEnd", &profile.rampInEndTime, &profile.rampInEndPosition);
		ImPlot::DragPoint("rampOutBegin", &profile.rampOutStartTime, &profile.rampOutStartPosition);
		ImPlot::DragPoint("actualCurveEnd", &profile.rampOutEndTime, &profile.rampOutEndPosition);

		ImPlot::PlotLine("position", &points.front().time, &points.front().position, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));
		ImPlot::PlotLine("velocity", &points.front().time, &points.front().velocity, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));
		//ImPlot::PlotLine("acceleration", &points.front().time, &points.front().acceleration, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));

		ImPlot::EndPlot();
	}

	ImGui::Text("totalDeltaPosition: %.3f", totalDeltaPosition);

	ImGui::Text("rampInStoppingDeltaPosition: %.3f", rampInStoppingDeltaPosition);
	ImGui::Text("rampOutStoppingDeltaPosition: %.3f", rampOutStoppingDeltaPosition);
	
	ImGui::Text("totalRampStoppingDeltaPosition: %.3f", totalRampStoppingDeltaPosition);
	ImGui::Text("exceededSomething: %i", exceededSomething);

	ImGui::Text("rampInStoppingPosition: %.3f", rampInStoppingPosition);
	ImGui::Text("rampOutStoppingPosition: %.3f", rampOutStoppingPosition);

	ImGui::Text("rampStoppingDeltaPosition: %.3f", rampStoppingDeltaPosition);

	ImGui::Text("constantVelocity: %.3f", constantVelocity);

	ImGui::Text("rampInAcceleration: %.3f", rampInAcceleration);
	ImGui::Text("rampOutAcceleration: %.3f", rampOutAcceleration);

	
	ImGui::Text("rampInDeltaPosition: %.3f", rampInDeltaPosition);
	ImGui::Text("rampOutDeltaPosition: %.3f", rampOutDeltaPosition);

	ImGui::Text("rampInDeltaTime: %.3f", rampInDeltaTime);
	ImGui::Text("rampOutDeltaTime: %.3f", rampOutDeltaTime);
	
	ImGui::Text("constantVelocityReachable: %i", constantVelocityReachable);

	ImGui::Separator();

	ImGui::Text("RampInEndPosition: %.3f", rampInEndPosition);
	ImGui::Text("rampInEndTime: %.3f", rampInEndTime);
	ImGui::Text("rampOutStartPosition: %.3f", rampOutStartPosition);
	ImGui::Text("rampOutStartTime: %.3f", rampOutStartTime);
	ImGui::Text("curveEndTime: %.3f", curveEndTime);
	ImGui::Text("curveEndPosition: %.3f", endPoint.position);
	
}