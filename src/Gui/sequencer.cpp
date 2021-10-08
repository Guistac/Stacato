#include <pch.h>

#include "Gui.h"

#include "Motion/MotionCurve.h"

void sequencer() {

	/*

	static MotionCurve::MotionConstraints constraints;
	static MotionCurve::CurvePoint startPoint;
	static MotionCurve::CurvePoint endPoint;
	static double reqvelocity = 0.2;

	static bool init = true;
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
		constraints.minPosition = -1000000.0;
		constraints.maxPosition = 10000000.0;
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
	ImGui::InputDouble("V", &reqvelocity, 0.01, 0.1);

	MotionCurve::CurveProfile profile = MotionCurve::getVelocityContrainedProfile(startPoint, endPoint, reqvelocity, constraints);

	int pointCount = 200;
	double deltaT = (profile.rampOutEndTime - profile.rampInStartTime) / pointCount;
	std::vector<MotionCurve::CurvePoint> points;
	for (int i = 0; i <= pointCount; i++) {
		double T = profile.rampInStartTime + i * deltaT;
		points.push_back(MotionCurve::getCurvePointAtTime(T, profile));
	}

	glm::vec2 rampInHandle(1.0, startPoint.velocity);
	rampInHandle = glm::normalize(rampInHandle);
	//rampInHandle *= 1.0;// * startPoint.acceleration;

	glm::vec2 rampOutHandle(-1.0, -endPoint.velocity);
	glm::normalize(rampOutHandle);
	//rampOutHandle *= 1.0;// *endPoint.acceleration;

	double velocityTangentLength = 1.0;
	std::vector<glm::vec2> rampInHandlePoints;
	rampInHandlePoints.push_back(glm::vec2(profile.rampInStartTime, profile.rampInStartPosition));
	rampInHandlePoints.push_back(glm::vec2(profile.rampInStartTime, profile.rampInStartPosition) + rampInHandle);

	std::vector<glm::vec2> rampOutHandlePoints;
	rampOutHandlePoints.push_back(glm::vec2(profile.rampOutEndTime, profile.rampOutEndPosition));
	rampOutHandlePoints.push_back(glm::vec2(profile.rampOutEndTime, profile.rampOutEndPosition) + rampOutHandle);

	double minreqdvel = -reqvelocity;
	if (ImPlot::BeginPlot("##curveTest", 0, 0, ImVec2(ImGui::GetContentRegionAvail().x, 1500.0), ImPlotFlags_None)) {
		ImPlot::DragLineY("end", &endPoint.position);
		ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 0.5, 1.0), 2.0);
		ImPlot::PlotHLines("##maxV", &reqvelocity, 1);
		ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 0.5, 1.0), 2.0);
		ImPlot::PlotHLines("##minV", &minreqdvel, 1);
		ImPlot::SetNextLineStyle(glm::vec4(0.5, 0.5, 0.5, 1.0), 2.0);
		ImPlot::PlotLine("acceleration", &points.front().time, &points.front().acceleration, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));
		ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 1.0, 1.0), 4.0);
		ImPlot::PlotLine("velocity", &points.front().time, &points.front().velocity, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 0.0, 0.0, 1.0), 4.0);
		ImPlot::PlotLine("position", &points.front().time, &points.front().position, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 1.0, 1.0), 2.0);
		ImPlot::PlotLine("RampIn", &rampInHandlePoints.front().x, &rampInHandlePoints.front().y, 2, 0, sizeof(glm::vec2));
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 1.0, 1.0), 2.0);
		ImPlot::PlotLine("RampOut", &rampOutHandlePoints.front().x, &rampOutHandlePoints.front().y, 2, 0, sizeof(glm::vec2));
		ImPlot::DragPoint("rampBegin", &profile.rampInStartTime, &profile.rampInStartPosition);
		ImPlot::DragPoint("rampInEnd", &profile.rampInEndTime, &profile.rampInEndPosition);
		ImPlot::DragPoint("rampOutBegin", &profile.rampOutStartTime, &profile.rampOutStartPosition);
		ImPlot::DragPoint("rampOutEnd", &profile.rampOutEndTime, &profile.rampOutEndPosition);
		ImPlot::EndPlot();
	}

	*/



	static MotionCurve::MotionConstraints constraints;
	static MotionCurve::CurvePoint startPoint;
	static MotionCurve::CurvePoint endPoint;

	static bool init = true;
	if (init) {
		startPoint.position = 0.0;
		startPoint.time = 0.0;
		startPoint.velocity = 0.0;
		startPoint.acceleration = 100.0;
		endPoint.position = 1.0;
		endPoint.time = 1.0;
		endPoint.velocity = 0.0;
		endPoint.acceleration = 100.0;
		constraints.maxAcceleration = 10000.0;
		constraints.maxVelocity = 100000.0;
		constraints.minPosition = -1000000.0;
		constraints.maxPosition = 10000000.0;
		init = false;
	}

	float inputWidth = 300.0;
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("Vi", &startPoint.velocity, 0.01, 0.1);
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("Ai", &startPoint.acceleration, 0.01, 0.1);
	ImGui::Separator();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("Vo", &endPoint.velocity, 0.01, 0.1);
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("Ao", &endPoint.acceleration, 0.01, 0.1);

	


	auto square = [](double in) -> double { return std::pow(in, 2.0); };

	double pi = startPoint.position;
	double ti = startPoint.time;
	double vi = startPoint.velocity;
	double ai = std::abs(startPoint.acceleration);

	double po = endPoint.position;
	double vo = endPoint.velocity;
	double ao = std::abs(endPoint.acceleration);

	double dt = endPoint.time - startPoint.time;
	double dp = endPoint.position - startPoint.position;
	double dv = endPoint.velocity - startPoint.velocity;


	//is the target too close to be reached using only the highest of the two acceleration values ?
	auto isTargetOvershotWithLargerAcceleration = [&]()->bool {
		//get the absolute value of the larger acceleration
		double amax;
		if (std::abs(ai) > std::abs(ao)) amax = std::abs(ai);
		else amax = std::abs(ao);
		//sign the acceleration depending on 
		if (dv < 0) amax *= -1.0;
		double overshootDeltaPosition = (square(vo) - square(vi)) / (2.0 * amax);
		if (dp > 0) return dp < overshootDeltaPosition;
		else return dp > overshootDeltaPosition;
	};

	auto isTargetOvershotWithSmallerAcceleration = [&]()->bool {
		//get the absolute value of the smaller acceleration
		double amin;
		if (std::abs(ai) < std::abs(ao)) amin = std::abs(ai);
		else amin = std::abs(ao);
		//sign the acceleration depending on 
		if (dv < 0) amin *= -1.0;
		double overshootDeltaPosition = (square(vo) - square(vi)) / (2.0 * amin);
		if (dp > 0) return dp < overshootDeltaPosition;
		else return dp > overshootDeltaPosition;
	};



	static bool ai_sign = true;
	static bool ao_sign = false;
	static bool rootTermSign = true;

	MotionCurve::CurveProfile profile;

	double aTerm;
	double bTerm;
	double cTerm;
	double rTerm;

	auto solveCurve = [&](MotionCurve::CurveProfile& profile) -> bool {

		//initialize acceleration signs
		if (!ai_sign) ai = -std::abs(ai);
		else ai = std::abs(ai);
		if (!ao_sign) ao = -std::abs(ao);
		else ao = std::abs(ao);

		//============ QUADRATIC FUNCTION FOR CONSTANT VELOCITY VALUE ============

		//Quadratic equation constants for ax² + bx + c = 0
		double a = ai - ao; //solution undefined if == 0
		double b = 2.0 * (ao * vi - ai * vo + ai * ao * dt);
		double c = ai * square(vo) - ao * square(vi) - 2.0 * ai * ao * dp;

		aTerm = a;
		bTerm = b;
		cTerm = c;

		//quadratic formula root term (solution undefined if < 0)
		double r = square(b) - 4.0 * a * c;

		rTerm = r;

		//quadratic solution
		double vt_plus = (-b + sqrt(r)) / (2.0 * a);
		double vt_minus = (-b - sqrt(r)) / (2.0 * a);

		//=========================================================================

		//using the final transition velocity value and signed acceleration values
		//we can get time and position deltas as well as absolute position and time points for the ramps
		double vt = 0.0;
		if (rootTermSign) vt = vt_plus;
		else vt = vt_minus;

		double dti = (vt - vi) / ai;
		double dto = (vo - vt) / ao;
		double dpi = vi * dti + ai * square(dti) / 2.0;
		double dpo = vt * dto + ao * square(dto) / 2.0;

		double rampInEndPosition = pi + dpi;
		double rampInEndTime = ti + dti;
		double rampOutStartPosition = po - dpo;
		double rampOutStartTime = rampInEndTime + (rampOutStartPosition - rampInEndPosition) / vt;
		double to = rampOutStartTime + dto;

		profile.rampInStartTime = startPoint.time;			//time of curve start
		profile.rampInStartPosition = startPoint.position;	//position of curve start
		profile.rampInStartVelocity = startPoint.velocity;	//velocity at curve start
		profile.rampInAcceleration = ai;	//acceleration of curve
		profile.rampInEndPosition = rampInEndPosition;		//position of curve after acceleration phase
		profile.rampInEndTime = rampInEndTime;			    //time of acceleration end
		profile.coastVelocity = vt;			//velocity of constant velocity phase
		profile.rampOutStartPosition = rampOutStartPosition;//position of deceleration start
		profile.rampOutStartTime = rampOutStartTime;		//time of deceleration start
		profile.rampOutAcceleration = ao;	//deceleration of curve
		profile.rampOutEndTime = to;			    //time of curve end
		profile.rampOutEndPosition = endPoint.position;		//position of curve end
		profile.rampOutEndVelocity = endPoint.velocity;		//velocity of curve end

		//checks for a successfull solve
		if (a == 0.0) return false;
		if (r < 0.0) return false;
		if (dti < 0.0) return false;
		if (dto < 0.0) return false;
		if (dti > to - dto) return false;
		return true;
	};

	
	bool solved = false;
	for (int i = 0; i < 8; i++) {
		switch (i) {
			case 0: //000
				ai_sign = false;
				ao_sign = false;
				rootTermSign = false;
				break;
			case 1: //001
				ai_sign = false;
				ao_sign = false;
				rootTermSign = true;
				break;
			case 2: //010
				ai_sign = false;
				ao_sign = true;
				rootTermSign = false;
				break;
			case 3: //011
				ai_sign = false;
				ao_sign = true;
				rootTermSign = true;
				break;
			case 4: //100
				ai_sign = true;
				ao_sign = false;
				rootTermSign = false;
				break;
			case 5: //101
				ai_sign = true;
				ao_sign = false;
				rootTermSign = true;
				break;
			case 6: //110
				ai_sign = true;
				ao_sign = true;
				rootTermSign = false;
				break;
			case 7: //111
				ai_sign = true;
				ao_sign = true;
				rootTermSign = true;
				break;
		}
		solved = solveCurve(profile);
		if (solved) break;
	}
	
	

	
	/*
	ImGui::Checkbox("RootTermSign", &rootTermSign);
	ImGui::SameLine();
	ImGui::Checkbox("RampInAcceleration", &ai_sign);
	ImGui::SameLine();
	ImGui::Checkbox("RampOutAcceleration", &ao_sign);
	bool solved = solveCurve(profile);
	*/

	ImGui::Text("%s", solved ? "Solved" : "Failed to Solve");
	ImGui::SameLine();
	ImGui::Text("%.3fx^2 + %.3fx + %.3f", aTerm, bTerm, cTerm);
	ImGui::SameLine();
	ImGui::Text("Root Term: %.3f", rTerm);
	
	ImGui::Text("deltaP: %.3f", dp);
	ImGui::SameLine();
	ImGui::Text("deltaT: %.3f", dt);
	ImGui::SameLine();
	ImGui::Text("deltaV: %.3f", dv);

	ImGui::Text("RootTermSign%s", rootTermSign ? "+" : "-");
	ImGui::SameLine();
	ImGui::Text("ai%s", ai_sign ? "+" : "-");
	ImGui::SameLine();
	ImGui::Text("ao%s", ao_sign ? "+" : "-");






	int pointCount = 200;
	double deltaT = (profile.rampOutEndTime - profile.rampInStartTime) / pointCount;
	std::vector<MotionCurve::CurvePoint> points;
	for (int i = 0; i <= pointCount; i++) {
		double T = profile.rampInStartTime + i * deltaT;
		points.push_back(MotionCurve::getCurvePointAtTime(T, profile));
	}

	glm::vec2 rampInHandle(1.0, startPoint.velocity);
	rampInHandle = glm::normalize(rampInHandle);
	//rampInHandle *= 1.0;// * startPoint.acceleration;

	glm::vec2 rampOutHandle(-1.0, -endPoint.velocity);
	glm::normalize(rampOutHandle);
	//rampOutHandle *= 1.0;// *endPoint.acceleration;

	double velocityTangentLength = 1.0;
	std::vector<glm::vec2> rampInHandlePoints;
	rampInHandlePoints.push_back(glm::vec2(profile.rampInStartTime, profile.rampInStartPosition));
	rampInHandlePoints.push_back(glm::vec2(profile.rampInStartTime, profile.rampInStartPosition) + rampInHandle);

	std::vector<glm::vec2> rampOutHandlePoints;
	rampOutHandlePoints.push_back(glm::vec2(profile.rampOutEndTime, profile.rampOutEndPosition));
	rampOutHandlePoints.push_back(glm::vec2(profile.rampOutEndTime, profile.rampOutEndPosition) + rampOutHandle);

	ImPlot::SetNextPlotLimits(-0.2, 1.5, -3.0, 3.0, ImGuiCond_FirstUseEver);
	if (ImPlot::BeginPlot("##curveTest", 0, 0, ImVec2(ImGui::GetContentRegionAvail().x, 1100.0), ImPlotFlags_None)) {
		ImPlot::DragPoint("target", &endPoint.time, &endPoint.position, true, glm::vec4(1.0, 1.0, 1.0, 1.0), 10.0);
		ImPlot::SetNextLineStyle(glm::vec4(0.5, 0.5, 0.5, 1.0), 2.0);
		ImPlot::PlotLine("acceleration", &points.front().time, &points.front().acceleration, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));
		ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 1.0, 1.0), 4.0);
		ImPlot::PlotLine("velocity", &points.front().time, &points.front().velocity, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 0.0, 0.0, 1.0), 4.0);
		ImPlot::PlotLine("position", &points.front().time, &points.front().position, pointCount + 1, 0, sizeof(MotionCurve::CurvePoint));
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 1.0, 1.0), 2.0);
		ImPlot::PlotLine("RampIn", &rampInHandlePoints.front().x, &rampInHandlePoints.front().y, 2, 0, sizeof(glm::vec2));
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 1.0, 1.0), 2.0);
		ImPlot::PlotLine("RampOut", &rampOutHandlePoints.front().x, &rampOutHandlePoints.front().y, 2, 0, sizeof(glm::vec2));
		ImPlot::DragPoint("rampBegin", &profile.rampInStartTime, &profile.rampInStartPosition);
		ImPlot::DragPoint("rampInEnd", &profile.rampInEndTime, &profile.rampInEndPosition);
		ImPlot::DragPoint("rampOutBegin", &profile.rampOutStartTime, &profile.rampOutStartPosition);
		ImPlot::DragPoint("rampOutEnd", &profile.rampOutEndTime, &profile.rampOutEndPosition);
		ImPlot::EndPlot();
	}
}