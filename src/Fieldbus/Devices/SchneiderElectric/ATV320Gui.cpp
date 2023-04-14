#include "ATV320.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "Gui/Utilities/CustomWidgets.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

void ATV320::deviceSpecificGui(){
	
	if(ImGui::BeginTabItem("ATV320")){
		statusGui();
		ImGui::Separator();
		if(ImGui::BeginTabBar("ATV320TabBar")){
			if(ImGui::BeginTabItem("Controls")){
				controlsGui();
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Settings")){
				settingsGui();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::EndTabItem();
	}
}


void ATV320::controlsGui(){
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Velocity Control");
	ImGui::PopFont();
	
	ImGui::BeginDisabled(!actuator->isEnabled() || actuatorPin->isConnected());
	
	int16_t max = maxVelocityRPM->getValue();
	int16_t min = -maxVelocityRPM->getValue();
	float velocityTarget_rps = 0.0;
	float maxVel = maxVelocityRPM->getValue() / 60.0;
	ImGui::SliderFloat("##VelocityTarget", &velocityTarget_rps, maxVel, -maxVel);
	if(ImGui::IsItemActive()) actuator->setVelocityTarget(velocityTarget_rpm);
	else if(ImGui::IsItemDeactivatedAfterEdit()) actuator->setVelocityTarget(0.0);

	
	ImGui::InvisibleButton("velocityIndicator", ImGui::GetItemRectSize());
	glm::vec2 minIndicator = ImGui::GetItemRectMin();
	glm::vec2 maxIndicator = ImGui::GetItemRectMax();
	glm::vec2 sizeIndicator = ImGui::GetItemRectSize();
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	drawing->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::GetColorU32(ImGuiCol_Button), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	float velocityNormalized = (float)velocityActual_rpm / max;
	float sizeIndicatorWidthHalf = sizeIndicator.x / 2.0;
	float velocityWidth = velocityNormalized * sizeIndicatorWidthHalf;
	if(velocityNormalized > 0.0f){
		glm::vec2 start(minIndicator.x + sizeIndicatorWidthHalf, minIndicator.y);
		glm::vec2 end(start.x + velocityWidth, maxIndicator.y);
		drawing->AddRectFilled(start, end, ImGui::GetColorU32(ImGuiCol_PlotHistogram), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	}else if(velocityNormalized < 0.0f){
		glm::vec2 start(minIndicator.x + sizeIndicatorWidthHalf + velocityWidth, minIndicator.y);
		glm::vec2 end(minIndicator.x + sizeIndicatorWidthHalf, maxIndicator.y);
		drawing->AddRectFilled(start, end, ImGui::GetColorU32(ImGuiCol_PlotHistogram), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	}
	glm::vec2 minCenter = glm::vec2(minIndicator.x + sizeIndicatorWidthHalf, minIndicator.y);
	glm::vec2 maxCenter = glm::vec2(minIndicator.x + sizeIndicatorWidthHalf, maxIndicator.y);
	ImColor centerColor = b_velocityTargetReached ? ImColor(.0f, 1.0f, .0f, 1.f) : ImColor(1.f, 1.f, 1.f, 1.f);
	drawing->AddRectFilled(minCenter, maxCenter, centerColor, 2.0);
	
	ImGui::EndDisabled();
}


void ATV320::settingsGui(){
	ImGui::Text("Acceleration Ramp");
	accelerationRampTime->gui();
	ImGui::Text("Deceleration Ramp");
	decelerationRampTime->gui();
	ImGui::Text("Max Velocity");
	maxVelocityRPM->gui();
    ImGui::Text("Low Speed");
    lowSpeedHertz->gui();
	
	ImGui::TextWrapped("Max Velocity is %.1f rev/s", actuator->getVelocityLimit());
	ImGui::TextWrapped("Max Acceleration is %.1f rev/s\xc2\xb2", actuator->getAccelerationLimit());
	ImGui::TextWrapped("Max Decleration is %.1f rev/s\xc2\xb2", actuator->getDecelerationLimit());
	
	invertDirection->gui();
	ImGui::SameLine();
	ImGui::TextWrapped("Invert Direction of Motion");
	
	ImGui::Separator();
	
	ImGui::Text("Slowdown Frequency");
	slowdownVelocityHertz->gui();
}

void ATV320::statusGui(){
	float doubleWidgetWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;
	float maxDoubleWidgetWidth = ImGui::GetTextLineHeight() * 15.0;
	if (doubleWidgetWidth > maxDoubleWidgetWidth) doubleWidgetWidth = maxDoubleWidgetWidth;

	float tripleWidgetWidth = (ImGui::GetContentRegionAvail().x - 2 * ImGui::GetStyle().ItemSpacing.x) / 3.0;
	float maxTripleWidgetWidth = ImGui::GetTextLineHeight() * 15.0;
	if (tripleWidgetWidth > maxTripleWidgetWidth) tripleWidgetWidth = maxTripleWidgetWidth;

	glm::vec2 statusDisplaySize(tripleWidgetWidth, ImGui::GetTextLineHeight() * 2.0);

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(0));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 0.0, 0.0, 0.0));
	ImGui::Button("Network Status", glm::vec2(statusDisplaySize.x, 0));
	ImGui::SameLine();
	ImGui::Button("EtherCAT status", glm::vec2(statusDisplaySize.x, 0));
	ImGui::SameLine();
	ImGui::Button("Servo Status", glm::vec2(statusDisplaySize.x, 0));
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	ImGui::PopItemFlag();

	
	ImGui::PushFont(Fonts::sansBold15);
	
	const char* networkStatusString = isConnected() ? "Online" : (isDetected() ? "Detected" : "Offline");
	glm::vec4 networkStatusColor = isConnected() ? Colors::green : (isDetected() ? Colors::yellow : Colors::red);
	backgroundText(networkStatusString, statusDisplaySize, networkStatusColor);

	ImGui::SameLine();
	backgroundText(getEtherCatStateChar(), statusDisplaySize, getEtherCatStateColor());

	ImGui::SameLine();
	
	backgroundText(getShortStatusString().c_str(), statusDisplaySize, getStatusColor());
	
	if(ImGui::IsItemHovered()){
		ImGui::BeginTooltip();
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::Text("%s", getStatusString().c_str());
		ImGui::PopStyleColor();
		ImGui::EndTooltip();
	}

	ImGui::PopFont();

	glm::vec2 commandButtonSize(doubleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);

	ImGui::BeginDisabled(!actuator->isReady() || actuatorPin->isConnected());
	if (actuator->isEnabled()) {
		if (ImGui::Button("Disable", commandButtonSize)) actuator->disable();
	}
	else if(b_hasFault){
		if(ImGui::Button("Reset Faults & Enable", commandButtonSize)) actuator->enable();
	}
	else {
		if (ImGui::Button("Enable", commandButtonSize)) actuator->enable();
	}
	ImGui::SameLine();
	if (ImGui::Button("Quick Stop", commandButtonSize)) actuator->quickstop();
	ImGui::EndDisabled();
}
