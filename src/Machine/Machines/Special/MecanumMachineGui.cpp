#include "MecanumMachine.h"

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"
#include "Motion/Interfaces.h"


void drawArrowRotated(float s, float rotationDegrees){
	float rad = s * 0.5;
	ImGui::InvisibleButton("Test", ImVec2(s,s));
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	ImVec2 size = ImGui::GetItemRectSize();
	ImVec2 mid = ImVec2(min.x + size.x * 0.5, min.y + size.y * 0.5);
	auto canvas = ImGui::GetWindowDrawList();
	canvas->AddCircleFilled(mid, rad, ImColor(ImGui::GetStyle().Colors[ImGuiCol_FrameBg]));
	
	float angleRadians = -M_PI * rotationDegrees / 180.0;
	angleRadians -= M_PI_2;
	float triangleSize = s*0.6;
	float startRadius = -rad;
	float endRadius = rad;
	glm::vec2 center = mid;
	
	float lineEndRadius = endRadius - triangleSize * .5f;
	glm::vec2 start(startRadius * std::cos(angleRadians), startRadius * std::sin(angleRadians));
	glm::vec2 end(lineEndRadius * std::cos(angleRadians), lineEndRadius * std::sin(angleRadians));
	start += center;
	end += center;
	
	glm::vec2 trianglePoints[3] = {
		center + glm::rotate(glm::vec2(endRadius, 0), angleRadians),
		center + glm::rotate(glm::vec2(endRadius - triangleSize, triangleSize * .4f), angleRadians),
		center + glm::rotate(glm::vec2(endRadius - triangleSize, -triangleSize * .4f), angleRadians)
	};
	
	canvas->AddLine(start, end, ImColor(Colors::white), s*0.1);
	canvas->AddTriangleFilled(trianglePoints[0],
							   trianglePoints[1],
							   trianglePoints[2],
							   ImColor(Colors::white));
}

void MecanumMachine::drawWheelWidget(){
	auto verticalBar = [](float val1, float val2, std::string txt){
		ImDrawList* canvas = ImGui::GetWindowDrawList();
		ImGui::Dummy(ImVec2(ImGui::GetTextLineHeight() * 2.0, ImGui::GetTextLineHeight() * 4.0));
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		ImVec2 size = ImGui::GetItemRectSize();
		canvas->AddRectFilled(min, max, ImColor(ImGui::GetStyle().Colors[ImGuiCol_FrameBg]), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
		float halfX = size.x * 0.5;
		float halfY = size.y * 0.5;
		float midX = min.x + halfX;
		float midY = min.y + halfY;
		canvas->AddLine(ImVec2(min.x, midY), ImVec2(max.x, midY), ImColor(Colors::transparentWhite), 1);
		float barSize1 = halfY * val1;
		float barSize2 = halfY * val2;
		float rounding = ImGui::GetStyle().FrameRounding;
		if(barSize1 < -1.0)
			canvas->AddRectFilled(ImVec2(min.x, midY), ImVec2(midX, midY - barSize1), ImColor(Colors::yellow), rounding, ImDrawFlags_RoundCornersBottomLeft);
		if(barSize1 > 1.0)
			canvas->AddRectFilled(ImVec2(min.x, midY - barSize1), ImVec2(midX, midY), ImColor(Colors::yellow), rounding, ImDrawFlags_RoundCornersTopLeft);
		
		if(barSize2 < -1.0)
			canvas->AddRectFilled(ImVec2(midX, midY), ImVec2(max.x, midY - barSize2), ImColor(Colors::white), rounding, ImDrawFlags_RoundCornersBottomRight);
		else if(barSize2 > 1.0)
			canvas->AddRectFilled(ImVec2(midX, midY - barSize2), ImVec2(max.x, midY), ImColor(Colors::white), rounding, ImDrawFlags_RoundCornersTopRight);
	};
	
	auto drawAxis = [&](std::shared_ptr<AxisMapping> mapping){
		if(mapping->axis == nullptr) {
			verticalBar(0.0, 0.0, "NC");
			return;
		}
		double actual = mapping->axis->getVelocityNormalizedToLimits();
		double target = mapping->axis->getVelocityTargetNormalizedToLimits();
		if(!mapping->invertAxis->value) {
			actual *= -1.0;
			target *= -1.0;
		}
		verticalBar(actual, target, mapping->axisPin->displayString);
	};
	
	ImGui::BeginGroup();
	drawAxis(axisMappings[0]);
	ImGui::SameLine();
	drawAxis(axisMappings[1]);
	drawAxis(axisMappings[2]);
	ImGui::SameLine();
	drawAxis(axisMappings[3]);
	ImGui::EndGroup();
}


void MecanumMachine::controlsGui() {
	
	if(ImGui::Checkbox("LocalControls", &b_localControl)){
		localControl_X = 0.0;
		localControl_Y = 0.0;
		localControl_R = 0.0;
	}
	if(b_localControl){
		ImGui::SameLine();
		ImGui::Checkbox("Speed Mode", &b_localControlSpeedMode);
		ImGui::SameLine();
		ImGui::Checkbox("Move Mode", &b_localControlMoveMode);
		ImGui::BeginGroup();
		ImGui::SliderFloat("##Xvel", &localControl_X, -1.0, 1.0, "%.2f%% X Velocity");
		ImGui::SliderFloat("##Yvel", &localControl_Y, -1.0, 1.0, "%.2f%% Y Velocity");
		ImGui::SliderFloat("##Rvel", &localControl_R, -1.0, 1.0, "%.2f%% R Velocity");
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::PushFont(Fonts::sansBold42);
		if(ImGui::Button("STOP", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetItemRectSize().y))){
			localControl_X = 0.0;
			localControl_Y = 0.0;
			localControl_R = 0.0;
		}
		ImGui::PopFont();
	}
	
	
	ImGui::Text("Limits:  Translation %.1fmm/s  Rotation: %.2f°/s", translationVelocityLimitCurrent, rotationVelocityLimitCurrent);
	ImGui::Text("Relative Velocity    X: %.1fmm/s  Y: %.1fmm/s  R: %.2f°/s", translationVelocity_relative.x, translationVelocity_relative.y, angularVelocity);
	ImGui::Text("Absolute Velocity    X: %.1fmm/s  Y: %.1fmm/s  R: %.2f°/s", translationVelocity_absolute.x, translationVelocity_absolute.y, angularVelocity);
	ImGui::Text("Odometry Position    X: %.1fmm  Y: %.1fmm  R: %.2f°", estimatedPosition_absolute.x, estimatedPosition_absolute.y, estimatedHeading_degrees);
	
	
	/*
	auto progressBarCentered = [](float val1, float val2, std::string txt){
		ImDrawList* canvas = ImGui::GetWindowDrawList();
		ImGui::Dummy(ImVec2(ImGui::GetTextLineHeight() * 10.0, ImGui::GetFrameHeight()));
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		ImVec2 size = ImGui::GetItemRectSize();
		canvas->AddRectFilled(min, max, ImColor(ImGui::GetStyle().Colors[ImGuiCol_FrameBg]), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
		float halfX = size.x * 0.5;
		float halfY = size.y * 0.5;
		float midX = min.x + halfX;
		float midY = min.y + halfY;
		canvas->AddLine(ImVec2(midX, min.y), ImVec2(midX, max.y), ImColor(Colors::white), 1);
		
		float barSize1 = halfX * val1;
		float barSize2 = halfX * val2;
		if(barSize1 > 1.0)
			canvas->AddRectFilled(ImVec2(midX, min.y), ImVec2(midX + halfX * val1, midY), ImColor(Colors::yellow), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersTopRight);
		else if(barSize1 < -1.0)
			canvas->AddRectFilled(ImVec2(midX + halfX * val1, min.y), ImVec2(midX, midY), ImColor(Colors::yellow), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersTopLeft);
		if(barSize2 > 1.0)
			canvas->AddRectFilled(ImVec2(midX, midY), ImVec2(midX + halfX * val2, max.y), ImColor(Colors::white), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersBottomRight);
		else if(barSize2 < -1.0)
			canvas->AddRectFilled(ImVec2(midX + halfX * val2, midY), ImVec2(midX, max.y), ImColor(Colors::white), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersBottomLeft);
		
		ImGui::SameLine();
		backgroundText(txt.c_str(), ImVec2(ImGui::GetTextLineHeight() * 6.0, ImGui::GetFrameHeight()), Colors::gray, Colors::black);
	};
	*/
	
	auto verticalBar = [](float val1, float val2, std::string txt){
		ImDrawList* canvas = ImGui::GetWindowDrawList();
		ImGui::Dummy(ImVec2(ImGui::GetTextLineHeight() * 2.0, ImGui::GetTextLineHeight() * 4.0));
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		ImVec2 size = ImGui::GetItemRectSize();
		canvas->AddRectFilled(min, max, ImColor(ImGui::GetStyle().Colors[ImGuiCol_FrameBg]), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
		float halfX = size.x * 0.5;
		float halfY = size.y * 0.5;
		float midX = min.x + halfX;
		float midY = min.y + halfY;
		canvas->AddLine(ImVec2(min.x, midY), ImVec2(max.x, midY), ImColor(Colors::transparentWhite), 1);
		float barSize1 = halfY * val1;
		float barSize2 = halfY * val2;
		float rounding = ImGui::GetStyle().FrameRounding;
		if(barSize1 < -1.0)
			canvas->AddRectFilled(ImVec2(min.x, midY), ImVec2(midX, midY - barSize1), ImColor(Colors::yellow), rounding, ImDrawFlags_RoundCornersBottomLeft);
		if(barSize1 > 1.0)
			canvas->AddRectFilled(ImVec2(min.x, midY - barSize1), ImVec2(midX, midY), ImColor(Colors::yellow), rounding, ImDrawFlags_RoundCornersTopLeft);
		
		if(barSize2 < -1.0)
			canvas->AddRectFilled(ImVec2(midX, midY), ImVec2(max.x, midY - barSize2), ImColor(Colors::white), rounding, ImDrawFlags_RoundCornersBottomRight);
		else if(barSize2 > 1.0)
			canvas->AddRectFilled(ImVec2(midX, midY - barSize2), ImVec2(max.x, midY), ImColor(Colors::white), rounding, ImDrawFlags_RoundCornersTopRight);
	};
	
	auto drawAxis = [&](std::shared_ptr<AxisMapping> mapping){
		if(mapping->axis == nullptr) {
			verticalBar(0.0, 0.0, "NC");
			return;
		}
		double actual = mapping->axis->getVelocityNormalizedToLimits();
		double target = mapping->axis->getVelocityTargetNormalizedToLimits();
		if(!mapping->invertAxis->value) {
			actual *= -1.0;
			target *= -1.0;
		}
		verticalBar(actual, target, mapping->axisPin->displayString);
	};
	
	ImGui::BeginGroup();
	drawAxis(axisMappings[0]);
	ImGui::SameLine();
	drawAxis(axisMappings[1]);
	drawAxis(axisMappings[2]);
	ImGui::SameLine();
	drawAxis(axisMappings[3]);
	ImGui::EndGroup();
	
	
	
	{
		float s = ImGui::GetItemRectSize().y;
		float rad = s * 0.5;
		ImGui::SameLine();
		ImGui::InvisibleButton("Test", ImVec2(s,s));
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		ImVec2 size = ImGui::GetItemRectSize();
		ImVec2 mid = ImVec2(min.x + size.x * 0.5, min.y + size.y * 0.5);
		auto canvas = ImGui::GetWindowDrawList();
		canvas->AddCircleFilled(mid, rad, ImColor(ImGui::GetStyle().Colors[ImGuiCol_FrameBg]));
		auto drawArrowRotated = [](ImDrawList* drawing, glm::vec2 center, float startRadius, float endRadius, float angleRadians, float triangleSize, float lineWidth, ImVec4 color){
			angleRadians -= M_PI_2;
			
			float lineEndRadius = endRadius - triangleSize * .5f;
			glm::vec2 start(startRadius * std::cos(angleRadians), startRadius * std::sin(angleRadians));
			glm::vec2 end(lineEndRadius * std::cos(angleRadians), lineEndRadius * std::sin(angleRadians));
			start += center;
			end += center;
			
			glm::vec2 trianglePoints[3] = {
				center + glm::rotate(glm::vec2(endRadius, 0), angleRadians),
				center + glm::rotate(glm::vec2(endRadius - triangleSize, triangleSize * .4f), angleRadians),
				center + glm::rotate(glm::vec2(endRadius - triangleSize, -triangleSize * .4f), angleRadians)
			};
			
			drawing->AddLine(start, end, ImColor(color), lineWidth);
			drawing->AddTriangleFilled(trianglePoints[0],
									   trianglePoints[1],
									   trianglePoints[2],
									   ImColor(color));
		};
		drawArrowRotated(canvas, mid, -rad, rad, -M_PI * estimatedHeading_degrees / 180.0, s*0.6, s*0.1, Colors::white);
	}
	
	{
	
		ImGui::InvisibleButton("Test", ImGui::GetContentRegionAvail());
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		ImVec2 size = ImGui::GetItemRectSize();
		glm::vec2 mid = glm::vec2(min.x + size.x * 0.5, min.y + size.y * 0.5);
		auto canvas = ImGui::GetWindowDrawList();
		canvas->AddRectFilled(min, max, ImColor(ImGui::GetStyle().Colors[ImGuiCol_FrameBg]), ImGui::GetStyle().FrameRounding);
		float toScreenMultiplier = 0.1;
		glm::vec2 screenPos = mid + estimatedPosition_absolute * glm::vec2(toScreenMultiplier, -toScreenMultiplier);
		
		auto drawCenteredTriangleRotated = [](ImDrawList* canvas, glm::vec2 center, float size, float angleRadians, ImVec4 color){
			glm::vec2 trianglePoints[3] = {
				center + glm::rotate(glm::vec2(0.0, -size * 0.6), angleRadians),
				center + glm::rotate(glm::vec2(size*0.5, size * 0.4), angleRadians),
				center + glm::rotate(glm::vec2(-size*0.5, size * 0.4), angleRadians)
			};
			canvas->AddTriangleFilled(trianglePoints[0],
									  trianglePoints[1],
									  trianglePoints[2],
									  ImColor(color));
			canvas->AddCircleFilled(center, 2, ImColor(glm::vec4(0.0, 0.0, 0.0, 1.0)));
		};
		
		float maxDistanceY = (size.y * 0.5) / toScreenMultiplier;
		float maxDistanceX = (size.x * 0.5) / toScreenMultiplier;
		for(float i = 0.0; i < maxDistanceY; i += 1000.0){
			float offset = i * toScreenMultiplier;
			ImColor c = ImColor(ImVec4(1.0, 1.0, 1.0, 0.1));
			canvas->AddLine(ImVec2(min.x, mid.y + offset), ImVec2(max.x, mid.y + offset), c);
			canvas->AddLine(ImVec2(min.x, mid.y - offset), ImVec2(max.x, mid.y - offset), c);
		}
		for(float i = 0.0; i < maxDistanceX; i += 1000.0){
			float offset = i * toScreenMultiplier;
			ImColor c = ImColor(ImVec4(1.0, 1.0, 1.0, 0.1));
			canvas->AddLine(ImVec2(mid.x + offset, min.y), ImVec2(mid.x + offset, max.y), c);
			canvas->AddLine(ImVec2(mid.x - offset, min.y), ImVec2(mid.x - offset, max.y), c);
		}
		
		std::vector<ImVec2> points;
		positionHistoryMutex.lock();
		positionHistory.data(points);
		positionHistoryMutex.unlock();
		
		std::vector<ImVec2> displayPoints(points.size());
		for(int i = 0; i < points.size(); i++) displayPoints[i] = ImVec2(mid.x + points[i].x * toScreenMultiplier, mid.y - points[i].y * toScreenMultiplier);
		canvas->AddPolyline(displayPoints.data(), int(displayPoints.size()), ImColor(Colors::transparentWhite), ImDrawFlags_None, 2.0);
		
		drawCenteredTriangleRotated(canvas, screenPos, ImGui::GetTextLineHeight(), M_PI * estimatedHeading_degrees / 180.0, Colors::white);
		canvas->AddCircle(mid, 4, ImColor(Colors::black), 16, 1.0);
	}
}
void MecanumMachine::settingsGui() {
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Mecanum Wheel Settings");
	ImGui::PopFont();
	
	if(ImGui::BeginTable("##ChannelSetting", 6, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_RowBg)){
		ImGui::TableSetupColumn("Axis");
		ImGui::TableSetupColumn("Diameter");
		ImGui::TableSetupColumn("Position [X,Y]");
		ImGui::TableSetupColumn("Wheel Type");
		ImGui::TableSetupColumn("Invert");
		ImGui::TableSetupColumn("Friction Vector");
		ImGui::TableHeadersRow();
		ImVec2 axisNameSize = ImVec2(ImGui::GetTextLineHeight() * 2.0, ImGui::GetFrameHeight());
		for(int i = 0; i < 4; i++){
			ImGui::PushID(i);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::string channelName = axisMappings[i]->axisPin->displayString;
			ImGui::PushFont(Fonts::sansBold15);
			backgroundText(channelName.c_str(), axisNameSize, Colors::gray, Colors::black);
			ImGui::PopFont();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4);
			axisMappings[i]->wheelDiameter->gui();
			ImGui::TableSetColumnIndex(2);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 8.0);
			axisMappings[i]->wheelPosition->gui();
			ImGui::TableSetColumnIndex(3);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.5);
			axisMappings[i]->wheelType->gui();
			ImGui::TableSetColumnIndex(4);
			axisMappings[i]->invertAxis->gui();
			ImGui::TableSetColumnIndex(5);
			ImGui::Text("X%.1f Y%.1f mm/rev", axisMappings[i]->frictionVector.x, axisMappings[i]->frictionVector.y);
			ImGui::PopID();
		}
		ImGui::EndTable();
	}
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Velocity Settings");
	ImGui::PopFont();
	
	linearVelocityLimit->gui(Fonts::sansBold15);
	angularVelocityLimit->gui(Fonts::sansBold15);
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Acceleration Settings");
	ImGui::PopFont();
	linearAccelerationLimit->gui(Fonts::sansBold15);
	angularAccelerationLimit->gui(Fonts::sansBold15);
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Other Settings");
	ImGui::PopFont();
	brakeApplyTime->gui(Fonts::sansBold15);
	enableTimeout->gui(Fonts::sansBold15);
	headingCorrectFactor->gui(Fonts::sansBold15);
	
	
}
void MecanumMachine::axisGui() {}
void MecanumMachine::deviceGui() {}
void MecanumMachine::metricsGui() {}
void MecanumMachine::setupGui() {}

#include "Stacato/StacatoGui.h"
#include "Legato/Application.h"

void MecanumMachine::MecanumWidget::gui(){
	
	ImVec2 widgetSize(ImGui::GetTextLineHeight() * 20.0, ImGui::GetTextLineHeight() * 14.7);
	
	if(ImGui::BeginChild("Widget", widgetSize)){
		
		machine->drawWheelWidget();
		float mh = ImGui::GetItemRectSize().y;
		ImGui::SameLine();
		float offset = 180.0;
		drawArrowRotated(ImGui::GetItemRectSize().y, machine->b_absoluteMoveMode ? machine->estimatedHeading_degrees + offset : offset);
		ImGui::SameLine();
		
		float rm = ImGui::GetContentRegionAvail().x;
		ImGui::BeginGroup();
		ImGui::PushFont(Fonts::sansBold20);
		
		float bh = (mh - ImGui::GetStyle().ItemSpacing.y * 2.0) / 3.0;
		ImVec2 fds = ImVec2(rm, bh);
		
		if(ImGui::Button("Shutdown", fds)){
			Application::requestShutdown();
		}
		
		backgroundText(machine->b_absoluteMoveMode ? "Absolute" : "Relative", fds,
					   machine->b_absoluteMoveMode ? Colors::black : Colors::gray,
					   machine->b_absoluteMoveMode ? Colors::white : Colors::black);
		
		switch(machine->state){
			case DeviceState::ENABLED:
				backgroundText("Enabled", fds, Colors::green, Colors::white);
				break;
			case DeviceState::ENABLING:
				backgroundText("Enabling...", fds, Colors::darkGreen, Colors::white);
				break;
			case DeviceState::DISABLING:
				backgroundText("Disabling", fds, Colors::darkYellow, Colors::white);
				break;
			case DeviceState::READY:
				backgroundText("Ready", fds, Colors::yellow, Colors::black);
				break;
			case DeviceState::NOT_READY:
				if(machine->isEmergencyStopped()) backgroundText("STO", fds, Timing::getBlink(1.0) ? Colors::red : Colors::yellow, Colors::white);
				else if(machine->b_brakeOverride) backgroundText("Brakes", fds, Timing::getBlink(2.0) ? Colors::yellow : Colors::white, Colors::black);
				else backgroundText("Not Ready", fds, Colors::red, Colors::white);
				break;
			case DeviceState::OFFLINE:
			default:
				backgroundText("Offline", fds, Colors::blue, Colors::white);
				break;
		}
		
		ImGui::PopFont();
		
		ImGui::EndGroup();
		
		float w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;
		float h = ImGui::GetFrameHeight();
		for(int i = 0; i < 4; i++){
			if(machine->axisMappings[i]->axis){
				auto axis = machine->axisMappings[i]->axis;
				ImVec4 color;
				if(axis->hasFault()) color = Colors::orange;
				else if(axis->isEnabled()) color = Colors::green;
				else if(axis->isReady()) color = Colors::yellow;
				else if(axis->isOnline()) color = Colors::red;
				else color = Colors::blue;
				char txt[128];
				snprintf(txt, 128, "%s: %s", machine->axisMappings[i]->axisPin->displayString, machine->axisMappings[i]->axis->getStatusString().c_str());
				ImGui::PushID(i);
				scrollingTextWithBackground("##txt", txt, ImVec2(w,h), color);
				ImGui::PopID();
				if(i == 0 || i == 2) ImGui::SameLine();
			}
		}
		
		float sw = ImGui::GetContentRegionAvail().x;
		
		bool paramEdit = false;
		ImGui::SetNextItemWidth(sw);
		ImGui::SliderFloat("##LowSpeedAdjust", &machine->lowSpeed_userAdjust, 10.0, 50.0, "Low Speed %.1f%%");
		paramEdit |= ImGui::IsItemDeactivatedAfterEdit();
		ImGui::SetNextItemWidth(sw);
		ImGui::SliderFloat("##AccelAdjust", &machine->globalAcceleration_userAdjust, 50.0, 150.0, "Acceleration %.1f%%");
		paramEdit |= ImGui::IsItemDeactivatedAfterEdit();
		
		if(paramEdit){
			auto saveAfterDelay = [](){
				std::thread saveThread([](){
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					Stacato::Gui::save();
				});
				saveThread.detach();
			};
			saveAfterDelay();
		}
		
		ImGui::EndChild();
	}
}
