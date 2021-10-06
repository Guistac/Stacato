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
	
	//first condition evaluation, do we overshoot the target when using only the larger of the two acceleration values ?
	double largerAcceleration;
	if (std::abs(startPoint.acceleration > std::abs(endPoint.acceleration))) largerAcceleration = std::abs(startPoint.acceleration);
	else largerAcceleration = std::abs(endPoint.acceleration);
	//make sure to get the correct sign of the larger acceleration
	if (endPoint.velocity < startPoint.velocity) largerAcceleration *= -1.0;
	double overshootDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2.0 * largerAcceleration);



	//overshoot condition might be on when the transition velocity is higher than the move velocity
	bool overshootCondition = false;

	//determine the sign of accelerations and constant velocity
	double constantVelocity;
	if (totalDeltaPosition > 0) {
		if (totalDeltaPosition > overshootDeltaPosition) {
			constantVelocity = std::abs(velocity);
		}
		else {
			//instead of just flipping the constant velocity
			//we should check if juste flipping the rampin (or rampout?) acceleration is enough
			constantVelocity = -std::abs(velocity);
			overshootCondition = true;
		}
	}
	else {
		if (totalDeltaPosition < overshootDeltaPosition) {
			constantVelocity = -std::abs(velocity);
		}
		else {
			//same here, check with one inverted acceleration first
			constantVelocity = std::abs(velocity);
			overshootCondition = true;
		}
	}

	//use the constant velocity to get the signs of in and out accelerations
	double rampInAcceleration;
	if (startPoint.velocity < constantVelocity) rampInAcceleration = std::abs(startPoint.acceleration);
	else rampInAcceleration = -std::abs(startPoint.acceleration);

	double rampOutAcceleration;
	if (endPoint.velocity > constantVelocity) rampOutAcceleration = std::abs(endPoint.acceleration);
	else rampOutAcceleration = -std::abs(endPoint.acceleration);

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

	double rampInDeltaTime = (constantVelocity - startPoint.velocity) / rampInAcceleration;
	double rampOutDeltaTime = (endPoint.velocity - constantVelocity) / rampOutAcceleration;

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

		rampInDeltaTime = (rampInTransitionVelocity - startPoint.velocity) / rampInAcceleration;
		rampOutDeltaTime = (endPoint.velocity - rampInTransitionVelocity) / rampOutAcceleration;
	}

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