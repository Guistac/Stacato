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

	auto getAbsMin = [](double a, double b) -> double {
		if (std::abs(a) < std::abs(b)) return std::abs(a);
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
			if (in < rangeB) { in = rangeB; return true;}
			else if (in > rangeA) { in = rangeA; return true; }
		}
		else {
			if (in < rangeA) { in = rangeA; return true; }
			else if (in > rangeB) { in = rangeB; return true; }
		}
		return false;
	};


	double totalDeltaPosition = endPoint.position - startPoint.position;

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
		if (totalDeltaPosition > 0.0 && totalDeltaPosition < overshootDeltaPosition) return true;
		else if (totalDeltaPosition < 0.0 && totalDeltaPosition > overshootDeltaPosition) return true;
		else return false;
	};

	auto isTargetOvershotWithSmallerAcceleration = [&]()->bool {
		double overshootEdgeCaseAcceleration = getAbsMin(rampInAcceleration, rampOutAcceleration) * getSignMultiplierFromDouble(totalDeltaVelocity);
		double overshootDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2.0 * overshootEdgeCaseAcceleration);
		if (totalDeltaPosition > 0.0 && totalDeltaPosition < overshootDeltaPosition) return true;
		else if (totalDeltaPosition < 0.0 && totalDeltaPosition > overshootDeltaPosition) return true;
		else return false;
	};

	auto isTargetVelocityReachable = [&]()->bool {
		double rampInDeltaPosition = (std::pow(transitionVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);
		double rampOutDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(transitionVelocity, 2.0)) / (2 * rampOutAcceleration);
		double totalRampDeltaPosition = rampInDeltaPosition + rampOutDeltaPosition;
		if (totalDeltaPosition > 0.0) return totalRampDeltaPosition < totalDeltaPosition;
		else return totalRampDeltaPosition > totalDeltaPosition;
	};

	auto getAbsRampTangentVelocity = [&]()->double {
		double rampInTangentDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * totalDeltaPosition) / (2.0 * (rampInAcceleration - rampOutAcceleration));
		double rampOutTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampInAcceleration * totalDeltaPosition) / (2.0 * (rampOutAcceleration - rampInAcceleration));
		double rampTangentVelocity = std::sqrt(std::pow(startPoint.velocity, 2.0) + 2.0 * rampInAcceleration * rampInTangentDeltaPosition);
		double rampOutTransitionVelocity = std::sqrt(std::pow(endPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * rampOutTransitionDeltaPosition);
		if (isnan(rampTangentVelocity) || rampTangentVelocity < 0.0) rampTangentVelocity = std::numeric_limits<double>::infinity();
		//this function potentially returns garbage values, if the values are out of range they need to be handled
		return rampTangentVelocity;
	};

	bool hasConstantVelocityPhase = true;
	bool weirdCondition = false;

	if (isTargetOvershot()) {
		//here we absolutely cannot reach the target by moving in the same direction as the total delta, we invert the transition velocity sign and get new acceleration signs
		invert(transitionVelocity);
		updateAccelerationSigns();
		//we then get a new tagent transition velocity value
		transitionVelocity = getAbsRampTangentVelocity() * getSignMultiplierFromDouble(transitionVelocity);
		//we make sure the velocity doesn't exceed the requested velocity
		if (std::abs(transitionVelocity) < std::abs(requestedVelocity)) {
			hasConstantVelocityPhase = false;
		}
		else {
			clamp(transitionVelocity, -requestedVelocity, requestedVelocity);
			hasConstantVelocityPhase = true;
		}
		updateAccelerationSigns();
	}
	
	else if (isTargetOvershotWithSmallerAcceleration()) {
		//weird transition case
		//we know the position is reachable without inverting the direction but we need to get new acceleration signs without knowing the transition velocity
		if (totalDeltaPosition > 0.0) {
			if (totalDeltaVelocity < 0.0) {
				rampInAcceleration = -std::abs(rampInAcceleration);
				rampOutAcceleration = -std::abs(rampOutAcceleration);
			}
			else {
				rampInAcceleration = std::abs(rampInAcceleration);
				rampOutAcceleration = -std::abs(rampOutAcceleration);
			}
		}
		else {
			if (totalDeltaVelocity > 0.0) {
				rampInAcceleration = std::abs(rampInAcceleration);
				rampOutAcceleration = std::abs(rampOutAcceleration);
			}
			else {
				rampInAcceleration = -std::abs(rampInAcceleration);
				rampOutAcceleration = std::abs(rampOutAcceleration);
			}
		}
		transitionVelocity = getAbsRampTangentVelocity() * getSignMultiplierFromDouble(transitionVelocity);
		hasConstantVelocityPhase = false;
		weirdCondition = true;
	}
	
	else if (!isTargetVelocityReachable()) {
		//this can mean multiple things, depending on the tangent ramp transition velocity
		double absRampTangentVelocity = getAbsRampTangentVelocity();
		if (absRampTangentVelocity < std::abs(requestedVelocity)) {
			//either we can get a fast transition with a new velocity that is below the requested one
			//in this case we transition between the ramps with no coast phase
			transitionVelocity = absRampTangentVelocity * getSignMultiplierFromDouble(totalDeltaPosition);
			updateAccelerationSigns();
			hasConstantVelocityPhase = false;
		}
		else {
			//or we can get a fast transition with a velocity that is above the requetsed one
			//in which case we absolutely cannot reach the target by moving in the same direction as the total delta
			//so we flip the velocity and do the calculation again
			invert(transitionVelocity);
			updateAccelerationSigns();
			transitionVelocity = getAbsRampTangentVelocity() * getSignMultiplierFromDouble(transitionVelocity);
			//make sure the tangent transition velocity doesn't exceed the requested velocity
			if (std::abs(transitionVelocity) < std::abs(requestedVelocity)) {
				hasConstantVelocityPhase = false;
			}
			else {
				clamp(transitionVelocity, -requestedVelocity, requestedVelocity);
				hasConstantVelocityPhase = true;
			}
			updateAccelerationSigns();
		}
	}

	double rampInDeltaPosition = (std::pow(transitionVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);
	double rampOutDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(transitionVelocity, 2.0)) / (2 * rampOutAcceleration);
	double rampInDeltaTime = (transitionVelocity - startPoint.velocity) / rampInAcceleration;
	double rampOutDeltaTime = (endPoint.velocity - transitionVelocity) / rampOutAcceleration;

	double rampInEndPosition;
	double rampInEndTime;
	double rampOutStartPosition;
	double rampOutStartTime;
	double curveEndTime;

	if (hasConstantVelocityPhase) {
		rampInEndPosition = startPoint.position + rampInDeltaPosition;
		rampInEndTime = startPoint.time + rampInDeltaTime;
		rampOutStartPosition = endPoint.position - rampOutDeltaPosition;
		rampOutStartTime = rampInEndTime + (rampOutStartPosition - rampInEndPosition) / transitionVelocity;
		curveEndTime = rampOutStartTime + rampOutDeltaTime;
	}
	else {
		rampInEndPosition = startPoint.position + rampInDeltaPosition;
		rampInEndTime = startPoint.time + rampInDeltaTime;
		rampOutStartPosition = rampInEndPosition;
		rampOutStartTime = rampInEndTime;
		curveEndTime = rampOutStartTime + rampOutDeltaTime;
	}


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
		ImPlot::PlotLine("acceleration", &points.front().time, &points.front().acceleration, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));

		ImPlot::EndPlot();
	}


	ImGui::Text("weirdCondition: %s", weirdCondition ? "true" : "false");
	ImGui::Text("HasConstantVelocityPhase: %s", hasConstantVelocityPhase ? "true" : "false");
	ImGui::Separator();
	ImGui::Text("RampInEndPosition: %.3f", rampInEndPosition);
	ImGui::Text("rampInEndTime: %.3f", rampInEndTime);
	ImGui::Text("rampOutStartPosition: %.3f", rampOutStartPosition);
	ImGui::Text("rampOutStartTime: %.3f", rampOutStartTime);
	ImGui::Text("curveEndTime: %.3f", curveEndTime);
	ImGui::Text("curveEndPosition: %.3f", endPoint.position);
	ImGui::Separator();
	ImGui::Text("RampInAcceleration: %.3f", rampInAcceleration);
	ImGui::Text("RampOutAcceleration: %.3f", rampOutAcceleration);
	ImGui::Text("TransitionVelocity: %.3f", transitionVelocity);

	
}