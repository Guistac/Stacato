#include <pch.h>

#include "Gui.h"

#include "Motion/MotionCurve.h"

void sequencer() {
	
	static MotionCurve::MotionConstraints constraints;
	static MotionCurve::CurvePoint startPoint;
	static MotionCurve::CurvePoint endPoint;

	static bool init = true;
	if (init) {
		startPoint.position = 0.0;
		startPoint.time = 0.0;
		startPoint.velocity = 0.0;
		startPoint.acceleration = 4.0;
		endPoint.position = 1.0;
		endPoint.time = 1.0;
		endPoint.velocity = 0.0;
		endPoint.acceleration = 4.0;
		constraints.maxAcceleration = 100.0;
		constraints.maxVelocity = 2.0;
		constraints.minPosition = -1000000.0;
		constraints.maxPosition = 10000000.0;
		init = false;
	}

	float inputWidth = 300.0;
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("TargetPosition", &endPoint.position, 0.01, 0.1);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("TargetTime", &endPoint.time, 0.01, 0.1);
	ImGui::Separator();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("Vi", &startPoint.velocity, 0.01, 0.1);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("Ai", &startPoint.acceleration, 0.01, 0.1);
	ImGui::Separator();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("Vo", &endPoint.velocity, 0.01, 0.1);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("Ao", &endPoint.acceleration, 0.01, 0.1);
	ImGui::Separator();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("maxV", &constraints.maxVelocity, 0.01, 0.1);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("maxA", &constraints.maxAcceleration, 0.01, 0.1);

	constraints.maxVelocity = std::max(constraints.maxVelocity, 0.0);
	constraints.maxAcceleration = std::max(constraints.maxAcceleration, 0.0);

	auto clamp = [](double in, double rangeA, double rangeB) -> double {
		if (rangeA < rangeB) {
			if (in < rangeA) return rangeA;
			else if (in > rangeB) return rangeB;
		}
		else {
			if (in < rangeB) return rangeB;
			else if (in > rangeA) return rangeA;
		}
		return in;
	};

	startPoint.acceleration = clamp(startPoint.acceleration, 0.0, constraints.maxAcceleration);
	endPoint.velocity = clamp(endPoint.velocity, -constraints.maxVelocity, constraints.maxVelocity);
	endPoint.position = clamp(endPoint.position, constraints.minPosition, constraints.maxPosition);
	endPoint.acceleration = clamp(endPoint.acceleration, 0.0, constraints.maxAcceleration);
	
	MotionCurve::CurveProfile profile = MotionCurve::getTimeConstrainedProfile(startPoint, endPoint, constraints);


	int pointCount = 2000;
	double deltaT = (profile.rampOutEndTime - profile.rampInStartTime) / pointCount;
	std::vector<MotionCurve::CurvePoint> points;
	for (int i = 0; i <= pointCount; i++) {
		double T = profile.rampInStartTime + i * deltaT;
		points.push_back(MotionCurve::getCurvePointAtTime(T, profile));
	}

	glm::vec2 rampInHandle(1.0, startPoint.velocity);
	rampInHandle = glm::normalize(rampInHandle);
	rampInHandle *= startPoint.acceleration * 0.1;

	glm::vec2 rampOutHandle(-1.0, -endPoint.velocity);
	glm::normalize(rampOutHandle);
	rampOutHandle *= endPoint.acceleration * 0.1;

	double velocityTangentLength = 1.0;
	std::vector<glm::vec2> rampInHandlePoints;
	rampInHandlePoints.push_back(glm::vec2(profile.rampInStartTime, profile.rampInStartPosition));
	rampInHandlePoints.push_back(glm::vec2(profile.rampInStartTime, profile.rampInStartPosition) + rampInHandle);

	std::vector<glm::vec2> rampOutHandlePoints;
	rampOutHandlePoints.push_back(glm::vec2(profile.rampOutEndTime, profile.rampOutEndPosition));
	rampOutHandlePoints.push_back(glm::vec2(profile.rampOutEndTime, profile.rampOutEndPosition) + rampOutHandle);

	std::vector<double> velocityLimits = { -constraints.maxVelocity, constraints.maxVelocity };

	ImPlot::SetNextPlotLimits(-0.2, 1.5, -3.0, 3.0, ImGuiCond_FirstUseEver);
	if (ImPlot::BeginPlot("##motionCurve", 0, 0, ImVec2(ImGui::GetContentRegionAvail().x, 1400.0), ImPlotFlags_None)) {
		ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 0.5, 1.0), 2.0);
		ImPlot::PlotHLines("Velocity Limits", &velocityLimits.front(), 2);
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
		ImPlot::DragPoint("RampInControlPoint", &rampInHandlePoints.back().x, &rampInHandlePoints.back().y, true, glm::vec4(1.0, 1.0, 1.0, 1.0), 4.0);
		ImPlot::DragPoint("RampOutControlPoint", &rampOutHandlePoints.front().x, &rampOutHandlePoints.front().y, true, glm::vec4(1.0, 1.0, 1.0, 1.0), 4.0);
		ImPlot::EndPlot();
	}

	ImGui::Text("RampEndTime: %.3f", profile.rampOutEndTime);
	ImGui::SameLine();
	ImGui::Text("RampInAcceleration: %.3f", profile.rampInAcceleration);
	ImGui::SameLine();
	ImGui::Text("RampOutAcceleration: %.3f", profile.rampOutAcceleration);

	
}