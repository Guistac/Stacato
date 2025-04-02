#include "MecanumMachine.h"

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"
#include "Motion/Interfaces.h"

void MecanumMachine::controlsGui() {
	
	ImGui::Checkbox("Local Enable Controls", &b_localControl);
	ImGui::Checkbox("Local Control Speed Mode", &b_localControlSpeedMode);
	ImGui::SliderFloat("Xvel", &localControl_X, -1.0, 1.0);
	ImGui::SliderFloat("Yvel", &localControl_Y, -1.0, 1.0);
	ImGui::SliderFloat("Rvel", &localControl_R, -1.0, 1.0);
	ImGui::PushFont(Fonts::sansBold42);
	if(ImGui::Button("STOP", ImVec2(ImGui::GetItemRectSize().x, ImGui::GetTextLineHeightWithSpacing()))){
		localControl_X = 0.0;
		localControl_Y = 0.0;
		localControl_R = 0.0;
	}
	ImGui::PopFont();
	
	
	ImGui::Text("Translation max velocity: %.1fmm/s", translationVelocityLimit);
	ImGui::Text("Rotation max velocity: %.1f°/s", rotationVelocityLimit);
	ImGui::Text("Translation X: %.1fmm/s", xProfile.getVelocity());
	ImGui::Text("Translation Y: %.1fmm/s", yProfile.getVelocity());
	ImGui::Text("Translation R: %.1fmm/s", rProfile.getVelocity());
	
	
	
	auto progressBarCentered = [](float val, std::string txt){
		ImDrawList* canvas = ImGui::GetWindowDrawList();
		ImGui::Dummy(ImVec2(ImGui::GetTextLineHeight() * 10.0, ImGui::GetFrameHeight()));
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		ImVec2 size = ImGui::GetItemRectSize();
		canvas->AddRectFilled(min, max, ImColor(ImGui::GetStyle().Colors[ImGuiCol_FrameBg]), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
		float halfX = size.x * 0.5;
		float midX = min.x + halfX;
		canvas->AddLine(ImVec2(midX, min.y), ImVec2(midX, max.y), ImColor(Colors::white), 1);
		float barSize = halfX * val;
		if(barSize > 1.0){
			canvas->AddRectFilled(ImVec2(midX, min.y), ImVec2(midX + halfX * val, max.y), ImColor(Colors::yellow), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersRight);
		}else if(barSize < -1.0){
			canvas->AddRectFilled(ImVec2(midX + halfX * val, min.y), ImVec2(midX, max.y), ImColor(Colors::yellow), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersLeft);
		}
		ImGui::SameLine();
		ImGui::Text("%s", txt.c_str());
	};
	
	ImVec2 velFeedbacksize(ImGui::GetTextLineHeight()*10.0, ImGui::GetFrameHeight());
	for(int i = 0; i < 4; i++){
		char txt[64];
		if(auto axis = axisMappings[i]->axis){
			snprintf(txt, 64, "%s %.2frev/s", axisMappings[i]->axisPin->displayString, axis->getVelocityActual());
			progressBarCentered(axis->getVelocityNormalizedToLimits(), txt);
		}
		else{
			char txt[64];
			snprintf(txt, 64, "%s not connected", axisMappings[i]->axisPin->displayString);
			progressBarCentered(0.0, txt);
		}
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
	
	
}
void MecanumMachine::axisGui() {}
void MecanumMachine::deviceGui() {}
void MecanumMachine::metricsGui() {}
void MecanumMachine::setupGui() {}
