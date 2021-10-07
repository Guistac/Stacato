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

	auto getSignMultiplierFromDouble = [](double a) -> double {
		if (a > 0.0) return 1.0;
		return -1.0;
	};

	auto clamp = [](double& in, double rangeA, double rangeB) {
		if (rangeA > rangeB) {
			if (in < rangeB) in = rangeB;
			else if (in > rangeA) in = rangeA;
		}
		else {
			if (in < rangeA) in = rangeA;
			else if (in > rangeB) in = rangeB;
		}
	};

	const double totalDeltaPosition = endPoint.position - startPoint.position;
	const double totalDeltaVelocity = endPoint.velocity - startPoint.velocity;
	const double rampInVelocity = startPoint.velocity;
	const double rampOutVelocity = endPoint.velocity;

	double transitionVelocity;
	double rampInAcceleration;
	double rampOutAcceleration;

	//compare the rampIn and rampOut velocities with the transition velocity to set the signs if the rampIn and rampOut acceleration values
	auto updateAccelerationSigns = [&]() {
		if (startPoint.velocity < transitionVelocity) rampInAcceleration = std::abs(rampInAcceleration);
		else rampInAcceleration = -std::abs(rampInAcceleration);
		if (transitionVelocity < endPoint.velocity) rampOutAcceleration = std::abs(rampOutAcceleration);
		else rampOutAcceleration = -std::abs(rampOutAcceleration);
	};

	//is the target too close to be reached using only the highest of the two acceleration values ?
	auto isTargetOvershotWithLargerAcceleration = [&]()->bool{
		//get the absolute value of the larger acceleration
		double largerAcceleration;
		if (std::abs(rampInAcceleration) > std::abs(rampOutAcceleration)) largerAcceleration = std::abs(rampInAcceleration);
		else largerAcceleration = std::abs(rampOutAcceleration);
		//sign the acceleration based on the total velocity delta
		largerAcceleration *= getSignMultiplierFromDouble(totalDeltaVelocity);
		double overshootDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2.0 * largerAcceleration);
		if (totalDeltaPosition > 0.0 && totalDeltaPosition < overshootDeltaPosition) return true;
		else if (totalDeltaPosition < 0.0 && totalDeltaPosition > overshootDeltaPosition) return true;
		else return false;
	};

	//is the target too close to be reached using only the lowest of the two acceleration values ?
	auto isTargetOvershotWithSmallerAcceleration = [&]()->bool {
		//get the absolute value of the larger acceleration
		double smallerAcceleration;
		if (std::abs(rampInAcceleration) < std::abs(rampOutAcceleration)) smallerAcceleration = std::abs(rampInAcceleration);
		else smallerAcceleration = std::abs(rampOutAcceleration);
		//sign the acceleration based on the total velocity delta
		smallerAcceleration *= getSignMultiplierFromDouble(totalDeltaVelocity);
		double overshootDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2.0 * smallerAcceleration);
		if (totalDeltaPosition > 0.0 && totalDeltaPosition < overshootDeltaPosition) return true;
		else if (totalDeltaPosition < 0.0 && totalDeltaPosition > overshootDeltaPosition) return true;
		else return false;
	};

	//is the target close enough for the movement to reach the requested velocity ?
	auto isTargetVelocityReachable = [&]()->bool {
		double rampInDeltaPosition = (std::pow(transitionVelocity, 2.0) - std::pow(startPoint.velocity, 2.0)) / (2 * rampInAcceleration);
		double rampOutDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(transitionVelocity, 2.0)) / (2 * rampOutAcceleration);
		double totalRampDeltaPosition = rampInDeltaPosition + rampOutDeltaPosition;
		if (totalDeltaPosition > 0.0) return totalRampDeltaPosition < totalDeltaPosition;
		else return totalRampDeltaPosition > totalDeltaPosition;
	};

	//get the velocity at the transition point between two tangent velocity ramps
	auto getAbsRampTangentVelocity = [&]()->double {
		double rampInTangentDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * totalDeltaPosition) / (2.0 * (rampInAcceleration - rampOutAcceleration));
		double rampOutTransitionDeltaPosition = (std::pow(endPoint.velocity, 2.0) - std::pow(startPoint.velocity, 2.0) - 2.0 * rampInAcceleration * totalDeltaPosition) / (2.0 * (rampOutAcceleration - rampInAcceleration));
		double rampTangentVelocity = std::sqrt(std::pow(startPoint.velocity, 2.0) + 2.0 * rampInAcceleration * rampInTangentDeltaPosition);
		double rampOutTransitionVelocity = std::sqrt(std::pow(endPoint.velocity, 2.0) - 2.0 * rampOutAcceleration * rampOutTransitionDeltaPosition);
		if (isnan(rampTangentVelocity) || rampTangentVelocity < 0.0) rampTangentVelocity = std::numeric_limits<double>::infinity();
		//this function potentially returns garbage values, if the values are out of range they need to be handled
		return rampTangentVelocity;
	};

	//set the default transition velocity sign based on the movement direction
	transitionVelocity = std::abs(requestedVelocity) * getSignMultiplierFromDouble(totalDeltaPosition);
	//initialize the default acceleration values
	rampInAcceleration = std::abs(startPoint.acceleration);
	rampOutAcceleration = std::abs(endPoint.acceleration);
	//get default acceleration signs based on the requested transition velocity
	updateAccelerationSigns();
	//by default, we assume these signs are correct and a basic move with acceleration to transition velocity followed by deceleration is possible
	//however, several cases need different parameters and the following condition checking will separate and handle them
	//some moves will not be able to reach the requested velocity, other moves will invert the acceleration signs or even the transition velocity sign
	bool requestedVelocityReacheable;

	//the most specific case is handled first
	if (isTargetOvershotWithLargerAcceleration()) {
		//here the target is too close to be reached using only the max acceleration value
		//we need invert the transition velocity and get new acceleration signs
		//we effectively backtrack to come back to the target at the correct end point position and velocity
		transitionVelocity *= -1.0;
		updateAccelerationSigns();
		//we then get a new transition velocity value that represents the fastest way to get to the target
		transitionVelocity = getAbsRampTangentVelocity() * getSignMultiplierFromDouble(transitionVelocity);
		requestedVelocityReacheable = std::abs(transitionVelocity) >= std::abs(requestedVelocity);
		//we make sure the velocity doesn't exceed the requested velocity
		if(requestedVelocityReacheable) clamp(transitionVelocity, -requestedVelocity, requestedVelocity);
		//we get new acceleration signs in case they need to change
		updateAccelerationSigns();
	}	
	else if (!isTargetVelocityReachable()) {
		//here the target can be reached, but is too close for the requested velocity to be reached
		//this can have multiple outcomes:
		if (isTargetOvershotWithSmallerAcceleration()) {
			//here the target is not reachable using only the smaller of the two acceleration values
			//it can still be reached by using a blend of the two accelerations, and without inverting the direction of motion
			//but we need to get new acceleration signs without knowing the transition velocity
			//the velocity in that segment will be somewhere between the input and outputvelocity
			//we can use the average of the two to find out acceleration signs and get a real transition velocity values later
			transitionVelocity = (startPoint.velocity + endPoint.velocity) / 2.0;
			updateAccelerationSigns();
		}
		//find the fastest possible transition velocity
		double absRampTangentVelocity = getAbsRampTangentVelocity();
		if (absRampTangentVelocity < std::abs(requestedVelocity)) {
			//here we can get a transition velocity that is below the requested one
			//in this case we keep the same movement direction, but without reaching the requested velocity
			transitionVelocity = absRampTangentVelocity * getSignMultiplierFromDouble(transitionVelocity);
			updateAccelerationSigns();
			requestedVelocityReacheable = false;
		}
		else {
			//in this case, the fastest transition velocity is higher than the requested one
			//here we absolutely cannot reach the target by moving in the same direction as the total delta
			//so we flip the velocity and do the calculation again
			transitionVelocity *= -1.0;
			updateAccelerationSigns();
			transitionVelocity = getAbsRampTangentVelocity() * getSignMultiplierFromDouble(transitionVelocity);
			requestedVelocityReacheable = std::abs(transitionVelocity) >= std::abs(requestedVelocity);
			//make sure the tangent transition velocity doesn't exceed the requested velocity
			if (requestedVelocityReacheable) clamp(transitionVelocity, -requestedVelocity, requestedVelocity);
			//we get new acceleration signs in case they need to change
			updateAccelerationSigns();
		}
	}
	else {
		//this is the basic case for regular moves
		//no additional logic is required
		requestedVelocityReacheable = true;
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

	if (requestedVelocityReacheable) {
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
	std::vector<glm::vec2> rampInVelocityPoints;
	rampInVelocityPoints.push_back(glm::vec2(profile.rampInStartTime, profile.rampInStartPosition));
	rampInVelocityPoints.push_back(glm::vec2(profile.rampInStartTime + velocityTangentLength, profile.rampInStartPosition + profile.rampInStartVelocity * velocityTangentLength));

	std::vector<glm::vec2> rampOutVelocityPoints;
	rampOutVelocityPoints.push_back(glm::vec2(profile.rampOutEndTime, profile.rampOutEndPosition));
	rampOutVelocityPoints.push_back(glm::vec2(profile.rampOutEndTime - velocityTangentLength, profile.rampOutEndPosition - profile.rampOutEndVelocity * velocityTangentLength));

	if (ImPlot::BeginPlot("##curveTest", 0, 0, ImVec2(ImGui::GetContentRegionAvail().x, 800.0), ImPlotFlags_None)) {
	
		ImPlot::DragLineY("end", &endPoint.position);
	
		ImPlot::PlotLine("RampIn Velocity", &rampInVelocityPoints.front().x, &rampInVelocityPoints.front().y, 2, 0, sizeof(glm::vec2));
		ImPlot::PlotLine("RampOut Velocity", &rampOutVelocityPoints.front().x, &rampOutVelocityPoints.front().y, 2, 0, sizeof(glm::vec2));

		ImPlot::DragPoint("rampBegin", &profile.rampInStartTime, &profile.rampInStartPosition);
		ImPlot::DragPoint("rampInEnd", &profile.rampInEndTime, &profile.rampInEndPosition);
		ImPlot::DragPoint("rampOutBegin", &profile.rampOutStartTime, &profile.rampOutStartPosition);
		ImPlot::DragPoint("rampOutEnd", &profile.rampOutEndTime, &profile.rampOutEndPosition);

		ImPlot::PlotLine("position", &points.front().time, &points.front().position, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));
		ImPlot::PlotLine("velocity", &points.front().time, &points.front().velocity, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));
		ImPlot::PlotLine("acceleration", &points.front().time, &points.front().acceleration, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));

		ImPlot::EndPlot();
	}

	ImGui::Text("HasConstantVelocityPhase: %s", requestedVelocityReacheable ? "true" : "false");
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