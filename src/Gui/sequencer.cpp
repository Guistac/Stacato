#include <pch.h>

#include "Gui.h"

#include "Motion/Curves/Position/D1PositionCurve.h"

#include "Utilities/CircularBuffer.h"

void sequencer() {
	
	static CircularBuffer circularBuffer(8);

	glm::vec2* data;
	size_t size = circularBuffer.getBuffer(&data);

	static int count = 0;

	if (ImGui::Button("Add")) {
		circularBuffer.addPoint(glm::vec2(count, Timing::getProgramTime_seconds()));
		count++;
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear")) {
		circularBuffer.clear();
		count = 0;
	}
	if (ImGui::BeginListBox("##test2", glm::vec2(500, 1000))) {
		for (int i = 0; i < size; i++) {
			glm::vec2& entry = data[i];
			int number = entry.x;
			ImGui::Text("[%i] x: %i   y: %.1f", i, number, entry.y);
		}
		ImGui::EndListBox();
	}




	static Motion::PositionCurve::D1::Point startPoint;
	static Motion::PositionCurve::D1::Point endPoint;
	static double maxVelocity = 3.0;
	static double maxAcceleration = 10.0;
	static double minPosition = -100000.0;
	static double maxPosition = 100000.0;

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
		init = false;
	}

	float inputWidth = 300.0;
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("StartPosition", &startPoint.position, 0.01, 0.1);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("StartTime", &startPoint.time, 0.01, 0.1);
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("EndPosition", &endPoint.position, 0.01, 0.1);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("EndTime", &endPoint.time, 0.01, 0.1);
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
	ImGui::InputDouble("maxV", &maxVelocity, 0.01, 0.1);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputDouble("maxA", &maxAcceleration, 0.01, 0.1);

	maxVelocity = std::max(maxVelocity, 0.0);
	maxAcceleration = std::max(maxAcceleration, 0.0);

	clamp(startPoint.acceleration, 0.0, maxAcceleration);
	clamp(endPoint.velocity, -maxVelocity, maxVelocity);
	clamp(endPoint.position, minPosition, maxPosition);
	clamp(endPoint.acceleration, 0.0, maxAcceleration);
	
	Motion::PositionCurve::D1::Interpolation profile;
	bool hasSolution = Motion::PositionCurve::D1::getTimeConstrainedInterpolation(startPoint, endPoint, maxVelocity, profile);

	int pointCount = 2000;
	double deltaT = (profile.rampOutEndTime - profile.rampInStartTime) / pointCount;
	std::vector<Motion::PositionCurve::D1::Point> points;
	for (int i = 0; i <= pointCount; i++) {
		double T = profile.rampInStartTime + i * deltaT;
		points.push_back(Motion::PositionCurve::D1::getInterpolationAtTime(T, profile));
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
	double rampInHandleX = rampInHandlePoints.back().x;
	double rampInHandleY = rampInHandlePoints.back().y;

	std::vector<glm::vec2> rampOutHandlePoints;
	rampOutHandlePoints.push_back(glm::vec2(profile.rampOutEndTime, profile.rampOutEndPosition));
	rampOutHandlePoints.push_back(glm::vec2(profile.rampOutEndTime, profile.rampOutEndPosition) + rampOutHandle);
	double rampOutHandleX = rampOutHandlePoints.back().x;
	double rampOutHandleY = rampOutHandlePoints.back().y;

	std::vector<double> velocityLimits = { -maxVelocity, maxVelocity };

	ImPlot::SetNextPlotLimits(-0.2, 1.5, -3.0, 3.0, ImGuiCond_FirstUseEver);
	if (ImPlot::BeginPlot("##motionCurve", 0, 0, ImVec2(ImGui::GetContentRegionAvail().x, 1400.0), ImPlotFlags_None)) {
		if (hasSolution) {
			ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 0.5, 1.0), 2.0);
			ImPlot::PlotHLines("Velocity Limits", &velocityLimits.front(), 2);
			ImPlot::DragPoint("target", &endPoint.time, &endPoint.position, true, glm::vec4(1.0, 1.0, 1.0, 1.0), 10.0);
			ImPlot::SetNextLineStyle(glm::vec4(0.5, 0.5, 0.5, 1.0), 2.0);
			ImPlot::PlotLine("acceleration", &points.front().time, &points.front().acceleration, pointCount + 1, 0, sizeof(Motion::PositionCurve::D1::Point));
			ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 1.0, 1.0), 4.0);
			ImPlot::PlotLine("velocity", &points.front().time, &points.front().velocity, pointCount + 1, 0, sizeof(Motion::PositionCurve::D1::Point));
			ImPlot::SetNextLineStyle(glm::vec4(1.0, 0.0, 0.0, 1.0), 4.0);
			ImPlot::PlotLine("position", &points.front().time, &points.front().position, pointCount + 1, 0, sizeof(Motion::PositionCurve::D1::Point));
			ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 1.0, 1.0), 2.0);
			ImPlot::PlotLine("RampIn", &rampInHandlePoints.front().x, &rampInHandlePoints.front().y, 2, 0, sizeof(glm::vec2));
			ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 1.0, 1.0), 2.0);
			ImPlot::PlotLine("RampOut", &rampOutHandlePoints.front().x, &rampOutHandlePoints.front().y, 2, 0, sizeof(glm::vec2));
			ImPlot::DragPoint("rampInEnd", &profile.rampInEndTime, &profile.rampInEndPosition, true, glm::vec4(1.0, 0.0, 0.0, 1.0), 4.0);
			ImPlot::DragPoint("rampOutBegin", &profile.rampOutStartTime, &profile.rampOutStartPosition, true, glm::vec4(1.0, 0.0, 0.0, 1.0), 4.0);
			ImPlot::DragPoint("RampInControlPoint", &rampInHandleX, &rampInHandleY, true, glm::vec4(1.0, 1.0, 1.0, 1.0), 8.0);
			ImPlot::DragPoint("RampOutControlPoint", &rampOutHandleX, &rampOutHandleY, true, glm::vec4(1.0, 1.0, 1.0, 1.0), 8.0);
		}
		ImPlot::EndPlot();
	}

	ImGui::Text("RampEndTime: %.3f", profile.rampOutEndTime);
	ImGui::SameLine();
	ImGui::Text("RampInAcceleration: %.3f", profile.rampInAcceleration);
	ImGui::SameLine();
	ImGui::Text("RampOutAcceleration: %.3f", profile.rampOutAcceleration);
	ImGui::SameLine();
	ImGui::Text("TransitionVelocity: %.3f", profile.coastVelocity);
	
}