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

	
	
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================


	//calculate total delta P of the curve (negative for negative moves)
	double totalDeltaPosition = endPoint.position - startPoint.position;
	
	//assume the coast phase velocity sign is the same as the total deltaV (this may change depending on later checks)
	double constantVelocity;
	if (totalDeltaPosition > 0) constantVelocity = std::abs(velocity);
	else constantVelocity = -std::abs(velocity);

	//===== first condition evaluation =====
	//do we overshoot the target when using only the larger of the two acceleration values ?
	//if yes we might need to change signs of acceleration and velocities to reach the target at the right position and velocity

	//get the larger absolute value of the two accelerations (in the edge case, we would only use the larger accleration to reach the target)
	double largerAcceleration;
	if (std::abs(startPoint.acceleration > std::abs(endPoint.acceleration))) largerAcceleration = std::abs(startPoint.acceleration);
	else largerAcceleration = std::abs(endPoint.acceleration);
	//get the sign of the larger acceleration by comparing the rampIn and rampOut velocities (the accelerations needs to transition between the velocities)
	if (endPoint.velocity < startPoint.velocity) largerAcceleration *= -1.0;
	double overshootDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2.0 * largerAcceleration);

	//figure out if the overshoot condition is triggered
	//if yes, we need to invert the sign of the coast phase velocity to reach the target position at target velocity
	bool overshootCondition = false;
	if (totalDeltaPosition > 0) overshootCondition = totalDeltaPosition < overshootDeltaPosition;
	else overshootCondition = totalDeltaPosition > overshootDeltaPosition;
	if (overshootCondition) constantVelocity *= -1.0;

	//===== end of first condition =====

	//we need to get the sign of the in and out acceleration values
	//we do this by comparing the coast phase velocity with the rampIn and rampOut velocities
	double rampInAcceleration;
	if (startPoint.velocity < constantVelocity) rampInAcceleration = std::abs(startPoint.acceleration);
	else rampInAcceleration = -std::abs(startPoint.acceleration);
	double rampOutAcceleration;
	if (endPoint.velocity > constantVelocity) rampOutAcceleration = std::abs(endPoint.acceleration);
	else rampOutAcceleration = -std::abs(endPoint.acceleration);

	//if we are in the overshoot condition, this means that the coast phase velocity was inverted
	//we actually still need to find the value of the ciast phase velocity

	double rampInTransitionDeltaPosition = 0.0;
	double rampOutTransitionDeltaPosition = 0.0;
	double rampInTransitionVelocity = 0.0;
	double rampOutTransitionVelocity = 0.0;
	if (overshootCondition) {
		rampInTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * totalDeltaPosition) / (2.0 * rampInAcceleration - 2.0 * rampOutAcceleration);
		rampOutTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampInAcceleration * totalDeltaPosition) / (2.0 * rampOutAcceleration - 2.0 * rampInAcceleration);
		//sometimes give nan
		rampInTransitionVelocity = std::sqrt(std::pow(startPoint.velocity, 2.0) + 2.0 * rampInAcceleration * rampInTransitionDeltaPosition);
		rampOutTransitionVelocity = std::sqrt(std::pow(endPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * rampOutTransitionDeltaPosition);
		if (rampInTransitionVelocity > velocity) rampInTransitionVelocity = velocity;
		if (rampOutTransitionVelocity > velocity) rampOutTransitionVelocity = velocity;
		//we need to choose the correct ramp transition velocity, + or -
		if (constantVelocity < 0) rampOutTransitionVelocity *= -1.0;
		constantVelocity = rampOutTransitionVelocity;
	}

	ImGui::Text("ConstantVelocity: %.3f", constantVelocity);

	ImGui::Text("Acceleration: %s  Deceleration: %s", rampInAcceleration > 0.0 ? "+" : "-", rampOutAcceleration > 0.0 ? "+" : "-");
	ImGui::Text("overshootDeltaPosition: %.3f  totalDeltaPosition: %.3f  overshot: %s", overshootDeltaPosition, totalDeltaPosition, overshootCondition ? "true" : "false");


	//get the two delta p associated with the ramp in and ramp out phases
	double rampInDeltaPosition = (std::pow(constantVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);
	double rampOutDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(constantVelocity, 2.0)) / (2 * rampOutAcceleration);

	double totalRampDeltaPosition = rampInDeltaPosition + rampOutDeltaPosition;

	bool constantVelocityReachable = true;
	if (totalDeltaPosition > 0) constantVelocityReachable = totalRampDeltaPosition < totalDeltaPosition;
	else constantVelocityReachable = totalRampDeltaPosition > totalDeltaPosition;

	ImGui::Text("RampInDeltaPosition: %.3f  RampOutDeltaPosition: %.3f", rampInDeltaPosition, rampOutDeltaPosition);
	ImGui::Text("totalRampDeltaPosition: %.3f  ConstantVelocityReachable: %s", totalRampDeltaPosition, constantVelocityReachable ? "true" : "false");

	if (!constantVelocityReachable && !overshootCondition) {

		//since we can't reach the constant velocity, we calculate a new ramp transition velocity
		//the simplest way to do this is by first calculating the position delta of the ramp transition
		rampInTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * totalDeltaPosition) / (2.0 * rampInAcceleration - 2.0 * rampOutAcceleration);
		rampOutTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampInAcceleration * totalDeltaPosition) / (2.0 * rampOutAcceleration - 2.0 * rampInAcceleration);

		//we then can use this delta to calculate the transition velocity, taking care to set its sign equal to the initial constant velocity
		rampInTransitionVelocity = std::sqrt(std::pow(startPoint.velocity, 2.0) + 2.0 * rampInAcceleration * rampInTransitionDeltaPosition);
		rampOutTransitionVelocity = std::sqrt(std::pow(endPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * rampOutTransitionDeltaPosition);
		if (rampInTransitionVelocity > velocity) rampInTransitionVelocity = velocity;
		if (rampOutTransitionVelocity > velocity) rampOutTransitionVelocity = velocity;

		if ((startPoint.velocity > std::abs(velocity) && endPoint.velocity > std::abs(velocity)) || (startPoint.velocity < -std::abs(velocity) && endPoint.velocity < -std::abs(velocity))) {
			constantVelocity *= -1.0; //would this cause a double inversion?
		}

		if (constantVelocity < 0.0) rampInTransitionVelocity *= -1.0;

		rampInDeltaPosition = (std::pow(rampInTransitionVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);

		//we also need to recalculate the rampOut delta, since we can't reach constant velocity
		rampOutDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(rampInTransitionVelocity, 2.0)) / (2 * rampOutAcceleration);

		constantVelocity = rampInTransitionVelocity;
	}

	double rampInDeltaTime = (constantVelocity - startPoint.velocity) / rampInAcceleration;
	double rampOutDeltaTime = (endPoint.velocity - constantVelocity) / rampOutAcceleration;

	ImGui::Text("RampInTransitionDeltaPosition: %.3f", rampInTransitionDeltaPosition);
	ImGui::Text("RampOuTransitionDeltaPosition: %.3f", rampOutTransitionDeltaPosition);
	ImGui::Text("rampInTransitionVelocity: %.3f", rampInTransitionVelocity);
	ImGui::Text("rampOutTransitionVelocity: %.3f", rampOutTransitionVelocity);
	
	

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
	
														
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================
	



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