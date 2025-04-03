#include "MecanumMachine.h"

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"
#include "Motion/Interfaces.h"

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
	
	
	ImGui::Text("Limits:  Translation %.1fmm/s  Rotation: %.2f°/s", translationVelocityLimit, rotationVelocityLimit);
	ImGui::Text("Relative Velocity    X: %.1fmm/s  Y: %.1fmm/s  R: %.2f°/s", translationVelocity_relative.x, translationVelocity_relative.y, angularVelocity);
	ImGui::Text("Absolute Velocity    X: %.1fmm/s  Y: %.1fmm/s  R: %.2f°/s", translationVelocity_absolute.x, translationVelocity_absolute.y, angularVelocity);
	ImGui::Text("Odometry Position    X: %.1fmm  Y: %.1fmm  R: %.2f°", estimatedPosition_absolute.x, estimatedPosition_absolute.y, estimatedHeading_degrees);
	
	
	
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
	
	/*
	ImGui::BeginGroup();
	ImVec2 velFeedbacksize(ImGui::GetTextLineHeight()*10.0, ImGui::GetFrameHeight());
	for(int i = 0; i < 4; i++){
		char txt[64];
		if(auto axis = axisMappings[i]->axis){
			snprintf(txt, 64, "%s %.2frev/s", axisMappings[i]->axisPin->displayString, axis->getVelocityActual());
			progressBarCentered(axis->getVelocityNormalizedToLimits(), axis->getVelocityTargetNormalizedToLimits(), txt);
		}
		else{
			char txt[64];
			snprintf(txt, 64, "%s not connected", axisMappings[i]->axisPin->displayString);
			progressBarCentered(0.0, 0.0, txt);
		}
	}
	ImGui::EndGroup();
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
	
	linearVelocityLimit_H->gui(Fonts::sansBold15);
	angularVelocityLimit_H->gui(Fonts::sansBold15);
	linearVelocityLimit_L->gui(Fonts::sansBold15);
	angularVelocityLimit_L->gui(Fonts::sansBold15);
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Acceleration Settings");
	ImGui::PopFont();
	linearAcceleration->gui(Fonts::sansBold15);
	angularAcceleration->gui(Fonts::sansBold15);
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
