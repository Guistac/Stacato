#include <pch.h>

#include "Gui.h"

#include "Motion/MotionCurve.h"

void sequencer() {

	static bool init = true;

	static MotionCurve::MotionConstraints constraints;
	static MotionCurve::CurvePoint startPoint;
	static MotionCurve::CurvePoint endPoint;
	static double requestedVelocity = 0.2;

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
	ImGui::InputDouble("V", &requestedVelocity, 0.01, 0.1);

	
	
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================
	
	auto getAbsMax = [](double a, double b) -> double {
		if (std::abs(a) > std::abs(b)) return std::abs(a);
		return std::abs(b);
	};

	auto getSignMultiplierFromDouble = [](double a) -> double {
		if (a > 0.0) return 1.0;
		return -1.0;
	};

	auto invert = [](double& a) {
		a *= -1.0;
	};

	auto clamp = [](double& in, double rangeA, double rangeB) -> bool {
		if (rangeA > rangeB) {
			if (in < rangeB) {
				in = rangeB;
				return true;
			}
			else if (in > rangeA) {
				in = rangeA;
				return true;
			}
		}
		else {
			if (in < rangeA) {
				in = rangeA;
				return true;
			}
			else if (in > rangeB) {
				in = rangeB;
				return true;
			}
		}
		return false;
	};


	double totalDeltaPosition = endPoint.position - startPoint.position;
	bool totalDeltaPositionSign = totalDeltaPosition > 0.0;

	double totalDeltaVelocity = endPoint.velocity - startPoint.velocity;
	bool totalDeltaVelocitySign = totalDeltaVelocity > 0.0;

	double transitionVelocity = std::abs(requestedVelocity) * getSignMultiplierFromDouble(totalDeltaPosition);
	double rampInAcceleration = std::abs(startPoint.acceleration);
	double rampOutAcceleration = std::abs(endPoint.acceleration);

	auto updateAccelerationSigns = [&]() {
		if (startPoint.velocity < transitionVelocity) rampInAcceleration = std::abs(rampInAcceleration);
		else rampInAcceleration = -std::abs(rampInAcceleration);
		if (transitionVelocity < endPoint.velocity) rampOutAcceleration = std::abs(rampOutAcceleration);
		else rampOutAcceleration = -std::abs(rampOutAcceleration);
	};

	updateAccelerationSigns();

	auto isTargetOvershot = [&]()->bool{
		double overshootEdgeCaseAcceleration = getAbsMax(rampInAcceleration, rampOutAcceleration) * getSignMultiplierFromDouble(totalDeltaVelocity);
		double overshootDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2.0 * overshootEdgeCaseAcceleration);
		if (totalDeltaPositionSign && totalDeltaPosition < overshootDeltaPosition) return true;
		else if (!totalDeltaPositionSign && totalDeltaPosition > overshootDeltaPosition) return true;
		else return false;
	};

	auto isTargetVelocityReachable = [&]()->bool {
		double rampInDeltaPosition = (std::pow(transitionVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);
		double rampOutDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(transitionVelocity, 2.0)) / (2 * rampOutAcceleration);
		double totalRampDeltaPosition = rampInDeltaPosition + rampOutDeltaPosition;
		if (totalDeltaPositionSign > 0) return totalRampDeltaPosition < totalDeltaPosition;
		else return totalRampDeltaPosition > totalDeltaPosition;
	};

	auto getAbsRampTangentVelocity = [&]()->double {
		double rampInTangentDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * totalDeltaPosition) / (2.0 * (rampInAcceleration - rampOutAcceleration));
		//double rampOutTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampInAcceleration * totalDeltaPosition) / (2.0 * rampOutAcceleration - 2.0 * rampInAcceleration);
		double rampTangentVelocity = std::sqrt(std::pow(startPoint.velocity, 2.0) + 2.0 * rampInAcceleration * rampInTangentDeltaPosition);
		//double rampOutTransitionVelocity = std::sqrt(std::pow(endPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * rampOutTransitionDeltaPosition);
		if (isnan(rampTangentVelocity) || rampTangentVelocity < 0.0) rampTangentVelocity = std::numeric_limits<double>::infinity();
		//this function potentially returns garbage values, if the values are out of range they need to be handled
		return rampTangentVelocity;
	};

	bool overshot = false;
	bool targetvelocityNotReachable = false;
	bool transitionVelocityExceeded = false;
	bool isRampTangentVelocityBug = false;

	if (isTargetOvershot()) {
		overshot = true;
		//here we absolutely cannot reach the target by moving in the same direction as the total delta, we invert the transition velocity sign and get new acceleration signs
		invert(transitionVelocity);
		updateAccelerationSigns();
		//we then get a new tagent transition velocity value
		double rampTangentVelocity = getAbsRampTangentVelocity() * getSignMultiplierFromDouble(transitionVelocity);
		clamp(rampTangentVelocity, -requestedVelocity, requestedVelocity);
		transitionVelocity = rampTangentVelocity;
		updateAccelerationSigns();
	}
	else if (!isTargetVelocityReachable()) {
		targetvelocityNotReachable = true;
		//this can mean multiple things, depending on the tangent ramp transition velocity
		double absRampTangentVelocity = getAbsRampTangentVelocity();
		if (absRampTangentVelocity < std::abs(requestedVelocity)) {
			//either we can get a fast transition with a new velocity that is below the requested one
			//in this case we transition between the ramps with no coast phase
			transitionVelocity = absRampTangentVelocity * getSignMultiplierFromDouble(totalDeltaPosition);
			updateAccelerationSigns();
		}
		else {
			transitionVelocityExceeded = true;
			//or we can get a fast transition with a velocity that is above the requetsed one
			//in which case we absolutely cannot reach the target by moving in the same direction as the total delta
			//so we flip the velocity and do the calculation again
			invert(transitionVelocity);
			updateAccelerationSigns();
			transitionVelocity = getAbsRampTangentVelocity() * getSignMultiplierFromDouble(transitionVelocity);
			//make sure the tangent transition velocity doesn't exceed the requested velocity
			if (clamp(transitionVelocity, -requestedVelocity, requestedVelocity)) updateAccelerationSigns();
		}
	}

	double rampInDeltaPosition = (std::pow(transitionVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);
	double rampOutDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(transitionVelocity, 2.0)) / (2 * rampOutAcceleration);
	double rampInDeltaTime = (transitionVelocity - startPoint.velocity) / rampInAcceleration;
	double rampOutDeltaTime = (endPoint.velocity - transitionVelocity) / rampOutAcceleration;

	double rampInEndPosition = startPoint.position + rampInDeltaPosition;
	double rampInEndTime = startPoint.time + rampInDeltaTime;
	double rampOutStartPosition = endPoint.position - rampOutDeltaPosition;
	double rampOutStartTime = rampInEndTime + (rampOutStartPosition - rampInEndPosition) / transitionVelocity;
	double curveEndTime = rampOutStartTime + rampOutDeltaTime;

	MotionCurve::CurveProfile profile;
	profile.rampInStartTime = startPoint.time;			//time of curve start
	profile.rampInStartPosition = startPoint.position;	//position of curve start
	profile.rampInStartVelocity = startPoint.velocity;	//velocity at curve start
	profile.rampInAcceleration = rampInAcceleration;	//acceleration of curve
	profile.rampInEndPosition = rampInEndPosition;		//position of curve after acceleration phase
	profile.rampInEndTime = rampInEndTime;			    //time of acceleration end
	profile.coastVelocity = transitionVelocity;			//velocity of constant velocity phase
	profile.rampOutStartPosition = rampOutStartPosition;//position of deceleration start
	profile.rampOutStartTime = rampOutStartTime;		//time of deceleration start
	profile.rampOutAcceleration = rampOutAcceleration;	//deceleration of curve
	profile.rampOutEndTime = curveEndTime;			    //time of curve end
	profile.rampOutEndPosition = endPoint.position;		//position of curve end
	profile.rampOutEndVelocity = endPoint.velocity;		//velocity of curve end


	
	/*

	//calculate total delta P of the curve (negative for negative moves)
	double totalDeltaPosition = endPoint.position - startPoint.position;
	
	//assume the coast phase velocity sign is the same as the total deltaV (this may change depending on later checks)
	double transitionVelocity;
	if (totalDeltaPosition > 0) transitionVelocity = std::abs(velocity);
	else transitionVelocity = -std::abs(velocity);

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
	if (overshootCondition) transitionVelocity *= -1.0;

	//===== end of first condition evaluation =====

	//we need to get the sign of the in and out acceleration values
	//we do this by comparing the coast phase velocity with the rampIn and rampOut velocities
	double rampInAcceleration;
	if (startPoint.velocity < transitionVelocity) rampInAcceleration = std::abs(startPoint.acceleration);
	else rampInAcceleration = -std::abs(startPoint.acceleration);
	double rampOutAcceleration;
	if (endPoint.velocity > transitionVelocity) rampOutAcceleration = std::abs(endPoint.acceleration);
	else rampOutAcceleration = -std::abs(endPoint.acceleration);


	//====== overshoot condition ======
	//if we are in the overshoot condition, it means the coast phase velocity was inverted
	//we actually still need to find the value of the coast phase velocity

	if (overshootCondition) {
		double rampInTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * totalDeltaPosition) / (2.0 * rampInAcceleration - 2.0 * rampOutAcceleration);
		//double rampOutTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampInAcceleration * totalDeltaPosition) / (2.0 * rampOutAcceleration - 2.0 * rampInAcceleration);
		double rampInTransitionVelocity = std::sqrt(std::pow(startPoint.velocity, 2.0) + 2.0 * rampInAcceleration * rampInTransitionDeltaPosition);
		//rampOutTransitionVelocity = std::sqrt(std::pow(endPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * rampOutTransitionDeltaPosition);
		if (rampInTransitionVelocity > velocity) rampInTransitionVelocity = velocity; //don't let the speed exceed the initial requested velocity
		//if (rampOutTransitionVelocity > velocity) rampOutTransitionVelocity = velocity;
		//we need to choose the correct ramp transition velocity, + or -
		if (transitionVelocity < 0) transitionVelocity = rampInTransitionVelocity * -1.0;
		else transitionVelocity = rampInTransitionVelocity;
	}


	//====== second condition evaluation ======
	
	//we need to know if we can actually reach the demanded coast phase velocity given the current parameters
	//we do this by finding the delta position from rampIn and rampOut to the coast phase velocity
	//if the sum of those ramp position deltas is greater than the total position delta, the velocity can not be reached
	double rampInDeltaPosition = (std::pow(transitionVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);
	double rampOutDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(transitionVelocity, 2.0)) / (2 * rampOutAcceleration);
	double totalRampDeltaPosition = rampInDeltaPosition + rampOutDeltaPosition;
	bool constantVelocityReachable;
	if (totalDeltaPosition > 0) constantVelocityReachable = totalRampDeltaPosition < totalDeltaPosition;
	else constantVelocityReachable = totalRampDeltaPosition > totalDeltaPosition;

	if (!constantVelocityReachable && !overshootCondition) {

		double rampInTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * totalDeltaPosition) / (2.0 * rampInAcceleration - 2.0 * rampOutAcceleration);
		//double rampOutTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampInAcceleration * totalDeltaPosition) / (2.0 * rampOutAcceleration - 2.0 * rampInAcceleration);

		double rampInTransitionVelocity = std::sqrt(std::pow(startPoint.velocity, 2.0) + 2.0 * rampInAcceleration * rampInTransitionDeltaPosition);
		//double rampOutTransitionVelocity = std::sqrt(std::pow(endPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * rampOutTransitionDeltaPosition);

		//rampInTransitionVelocity was an absolute value since it came from solving a square root
		//we assign the same sign as the total position delta
		if (totalDeltaPosition < 0) rampInTransitionVelocity *= -1.0;

		//if the velocity can not be reached and the required transition velocity exceeds the requested velocity
		//we need to limits that velocity and invert it
		//we effectively move backwards before going towards the target to reach it at its required position and velocity
		if (std::abs(rampInTransitionVelocity) > std::abs(velocity)) {
			if (rampInTransitionVelocity > 0.0) rampInTransitionVelocity = -std::abs(velocity);
			else rampInTransitionVelocity = std::abs(velocity);
			//since the velocity changed, we need to adjust the signs of accelerations
			transitionVelocity = rampInTransitionVelocity;
			if (startPoint.velocity < transitionVelocity) rampInAcceleration = std::abs(startPoint.acceleration);
			else rampInAcceleration = -std::abs(startPoint.acceleration);
			if (endPoint.velocity > transitionVelocity) rampOutAcceleration = std::abs(endPoint.acceleration);
			else rampOutAcceleration = -std::abs(endPoint.acceleration);
			//we then also need to recalculate the transition velocity, since the acceration signs have changed
			rampInTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * totalDeltaPosition) / (2.0 * rampInAcceleration - 2.0 * rampOutAcceleration);
			//double rampOutTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampInAcceleration * totalDeltaPosition) / (2.0 * rampOutAcceleration - 2.0 * rampInAcceleration);
			rampInTransitionVelocity = std::sqrt(std::pow(startPoint.velocity, 2.0) + 2.0 * rampInAcceleration * rampInTransitionDeltaPosition);
			//double rampOutTransitionVelocity = std::sqrt(std::pow(endPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * rampOutTransitionDeltaPosition);
			//and make sure to limit it and reassign the correct sign
			if (rampInTransitionVelocity > std::abs(velocity)) rampInTransitionVelocity = std::abs(velocity);
			if (transitionVelocity < 0) rampInTransitionVelocity *= -1.0;
		}

		transitionVelocity = rampInTransitionVelocity;
		rampInDeltaPosition = (std::pow(transitionVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);
		rampOutDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(transitionVelocity, 2.0)) / (2 * rampOutAcceleration);
	}

	double rampInDeltaTime = (transitionVelocity - startPoint.velocity) / rampInAcceleration;
	double rampOutDeltaTime = (endPoint.velocity - transitionVelocity) / rampOutAcceleration;
	
	

	//end position of ramp-In is start + ramping delta
	double rampInEndPosition = startPoint.position + rampInDeltaPosition;

	//end time of ramp-In is startTime + rampIn delta time
	double rampInEndTime = startPoint.time + rampInDeltaTime;

	//ramp out start position is end position minus rampOut Delta
	double rampOutStartPosition = endPoint.position - rampOutDeltaPosition;

	//ramp out start time is rampIn end time + time of constant velocity coast
	double rampOutStartTime = rampInEndTime + (rampOutStartPosition - rampInEndPosition) / transitionVelocity;

	//end of curve time is ramp out start time + ramp out delta time
	double curveEndTime = rampOutStartTime + rampOutDeltaTime;

	*/

														
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

	ImGui::Text("overshot: %i", overshot);
	ImGui::Text("targetvelocityNotReachable: %i", targetvelocityNotReachable);
	ImGui::Text("transitionVelocityExceeded: %i", transitionVelocityExceeded);
	ImGui::Text("isRampTangentVelocityBug: %i", isRampTangentVelocityBug);

	/*
	ImGui::Text("RampInDeltaPosition: %.3f  RampOutDeltaPosition: %.3f", rampInDeltaPosition, rampOutDeltaPosition);
	ImGui::Text("totalRampDeltaPosition: %.3f  ConstantVelocityReachable: %s", totalRampDeltaPosition, constantVelocityReachable ? "true" : "false");

	
	ImGui::Text("Acceleration: %s  Deceleration: %s", rampInAcceleration > 0.0 ? "+" : "-", rampOutAcceleration > 0.0 ? "+" : "-");
	ImGui::Text("overshootDeltaPosition: %.3f  totalDeltaPosition: %.3f  overshot: %s", overshootDeltaPosition, totalDeltaPosition, overshootCondition ? "true" : "false");



	ImGui::Text("constantVelocity: %.3f", transitionVelocity);

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
	*/
	
}