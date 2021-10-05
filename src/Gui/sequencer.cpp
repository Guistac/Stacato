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
	
	//first condition evaluation, do we overshoot the target if we decelerate at the max acceleration ?
	double largerAcceleration = std::abs(startPoint.acceleration) > std::abs(endPoint.acceleration) ? std::abs(startPoint.acceleration) : std::abs(endPoint.acceleration);
	if (endPoint.velocity < startPoint.velocity) largerAcceleration *= -1.0;
	double overshootDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2.0 * largerAcceleration);


	bool overshootCondition = false;

	//determine the sign of accelerations and constant velocity
	double constantVelocity;
	if (totalDeltaPosition > 0) {
		if (totalDeltaPosition > overshootDeltaPosition) {
			constantVelocity = std::abs(velocity);
			ImGui::Text("positive move");
		}
		else {
			//instead of just flipping the constant velocity
			//we should check if juste flipping the rampin (or ramout?) acceleration is enough
			constantVelocity = -std::abs(velocity);
			overshootCondition = true;
			ImGui::Text("positive move and overshoot");
		}
	}
	else {
		if (totalDeltaPosition < overshootDeltaPosition) {
			constantVelocity = -std::abs(velocity);
			ImGui::Text("negative but no overshoot");
		}
		else {
			//same here, check with one inverted acceleration first
			constantVelocity = std::abs(velocity);
			overshootCondition = true;
			ImGui::Text("negative and overshoot");
		}
	}

	//use the constant velocity to get the signs of in and out accelerations
	double rampInAcceleration = startPoint.velocity < constantVelocity ? std::abs(startPoint.acceleration) : -std::abs(startPoint.acceleration);
	double rampOutAcceleration = endPoint.velocity > constantVelocity ? std::abs(endPoint.acceleration) : -std::abs(endPoint.acceleration);

	if (overshootCondition) {
		double rampInDeltaPosition = (std::pow(startPoint.velocity, 2.0) - std::pow(endPoint.velocity, 2.0) - 2.0 * rampInAcceleration * totalDeltaPosition) / (2.0 * rampInAcceleration - 2.0 * rampOutAcceleration);
		double rampTransitionVelocity = std::sqrt(std::pow(startPoint.velocity, 2.0) + 2.0 * startPoint.acceleration * rampInDeltaPosition);
		if (rampTransitionVelocity > velocity) rampTransitionVelocity = velocity;
		if (constantVelocity < 0) rampTransitionVelocity *= -1.0;
		constantVelocity = rampTransitionVelocity;
	}

	ImGui::Text("ConstantVelocity: %.3f", constantVelocity);

	ImGui::Text("Acceleration: %s  Deceleration: %s", rampInAcceleration > 0.0 ? "+" : "-", rampOutAcceleration > 0.0 ? "+" : "-");
	ImGui::Text("overshootDeltaPosition: %.3f  totalDeltaPosition: %.3f  overshot: %s", overshootDeltaPosition, totalDeltaPosition, overshootCondition ? "true" : "false");


	//get the two delta p associated with the ramp in and ramp out phases
	double rampInDeltaPosition = (std::pow(constantVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);
	double rampOutDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(constantVelocity, 2.0)) / (2 * rampOutAcceleration);

	double rampInDeltaTime = (constantVelocity - startPoint.velocity) / rampInAcceleration;
	double rampOutDeltaTime = (endPoint.velocity - constantVelocity) / rampOutAcceleration;

	double totalRampDeltaPosition = rampInDeltaPosition + rampOutDeltaPosition;

	ImGui::Text("RampInDeltaPosition: %.3f  RampOutDeltaPosition: %.3f", rampInDeltaPosition, rampOutDeltaPosition);
	ImGui::Text("totalRampDeltaPosition: %.3f", totalRampDeltaPosition);

	bool constantVelocityReachable = true;
	if (totalDeltaPosition > 0) constantVelocityReachable = totalRampDeltaPosition < totalDeltaPosition;
	else constantVelocityReachable = totalRampDeltaPosition > totalDeltaPosition;


	if (!constantVelocityReachable && !overshootCondition) {

		//since we can't reach the constant velocity, we calculate a new ramp transition velocity
		//the simplest way to do this is by first calculating the position delta of the ramp transition
		double rampTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * totalDeltaPosition - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration - 2 * rampOutAcceleration);

		//we then can use this delta to calculate the transition velocity, taking care to set its sign equal to the initial constant velocity
		double rampTransitionVelocity = std::sqrt(std::pow(startPoint.velocity, 2.0) + 2.0 * rampInAcceleration * rampTransitionDeltaPosition);
		if (constantVelocity < 0.0) rampTransitionVelocity *= -1.0;

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
	double velocityTangentLength = 1.0;
	std::vector<glm::vec2> rampInVelocity;
	rampInVelocity.push_back(glm::vec2(profile.rampInStartTime, profile.rampInStartPosition));
	rampInVelocity.push_back(glm::vec2(profile.rampInStartTime + velocityTangentLength, profile.rampInStartPosition + profile.rampInStartVelocity * velocityTangentLength));

	std::vector<glm::vec2> rampOutVelocity;
	rampOutVelocity.push_back(glm::vec2(profile.rampOutEndTime, profile.rampOutEndPosition));
	rampOutVelocity.push_back(glm::vec2(profile.rampOutEndTime - velocityTangentLength, profile.rampOutEndPosition - profile.rampOutEndVelocity * velocityTangentLength));

	if (ImPlot::BeginPlot("##curveTest", 0, 0, ImVec2(ImGui::GetContentRegionAvail().x, 800.0), ImPlotFlags_None)) {
	
		ImPlot::DragLineY("end", &endPoint.position);
	
		ImPlot::PlotLine("RampIn Velocity", &rampInVelocity.front().x, &rampInVelocity.front().y, 2, 0, sizeof(glm::vec2));
		ImPlot::PlotLine("RampOut Velocity", &rampOutVelocity.front().x, &rampOutVelocity.front().y, 2, 0, sizeof(glm::vec2));

		ImPlot::DragPoint("rampBegin", &profile.rampInStartTime, &profile.rampInStartPosition);
		ImPlot::DragPoint("rampInEnd", &profile.rampInEndTime, &profile.rampInEndPosition);
		ImPlot::DragPoint("rampOutBegin", &profile.rampOutStartTime, &profile.rampOutStartPosition);
		ImPlot::DragPoint("rampOutEnd", &profile.rampOutEndTime, &profile.rampOutEndPosition);

		ImPlot::PlotLine("position", &points.front().time, &points.front().position, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));
		ImPlot::PlotLine("velocity", &points.front().time, &points.front().velocity, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));
		//ImPlot::PlotLine("acceleration", &points.front().time, &points.front().acceleration, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));

		ImPlot::EndPlot();
	}

	//ImGui::Text("deltaPositionTransition: %.3f", deltaPositionTransition);

	ImGui::Text("constantVelocity: %.3f", constantVelocity);

	ImGui::Text("rampInAcceleration: %.3f", rampInAcceleration);
	ImGui::Text("rampOutAcceleration: %.3f", rampOutAcceleration);

	
	ImGui::Text("rampInDeltaPosition: %.3f", rampInDeltaPosition);
	ImGui::Text("rampOutDeltaPosition: %.3f", rampOutDeltaPosition);

	ImGui::Text("rampInDeltaTime: %.3f", rampInDeltaTime);
	ImGui::Text("rampOutDeltaTime: %.3f", rampOutDeltaTime);
	
	//ImGui::Text("constantVelocityReachable: %i", constantVelocityReachable);

	ImGui::Separator();

	ImGui::Text("RampInEndPosition: %.3f", rampInEndPosition);
	ImGui::Text("rampInEndTime: %.3f", rampInEndTime);
	ImGui::Text("rampOutStartPosition: %.3f", rampOutStartPosition);
	ImGui::Text("rampOutStartTime: %.3f", rampOutStartTime);
	ImGui::Text("curveEndTime: %.3f", curveEndTime);
	ImGui::Text("curveEndPosition: %.3f", endPoint.position);

	ImGui::Text("test");
	
}