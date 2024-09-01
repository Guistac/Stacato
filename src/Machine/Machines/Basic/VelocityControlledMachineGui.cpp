#include <pch.h>

#include "VelocityControlledMachine.h"
#include "Motion/Axis/AxisNode.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "Environnement/NodeGraph/DeviceNode.h"

#include "Environnement/Environnement.h"

static bool b_invalidValue = false;
static void pushInvalidValue(bool doit){
	if(doit){
		b_invalidValue = true;
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::PushFont(Fonts::sansBold15);
	}
}

static void popInvalidValue(){
	if(b_invalidValue){
		b_invalidValue = false;
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}
}

void VelocityControlledMachine::controlsGui() {
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Manual Controls");
	ImGui::PopFont();
	
	if(isAxisConnected()){
		Units::Type axisUnitType = getAxisInterface()->getPositionUnit()->unitType;
		verticalWidgetGui();
		ImGui::Separator();
	}
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Setup Controls");
	ImGui::PopFont();
	
	setupGui();
	
}


void VelocityControlledMachine::settingsGui() {
	
	if(!isAxisConnected()){
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::Text("No Axis is Connected.");
		ImGui::PopStyleColor();
		return;
	}
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Axis Settings");
	ImGui::PopFont();
	
	invertAxis->gui();
	ImGui::SameLine();
	ImGui::TextWrapped("Invert Axis");
	
	invertControlGui->gui();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Invert Control Widget");
	ImGui::PopFont();
	
}

void VelocityControlledMachine::axisGui() {}

void VelocityControlledMachine::deviceGui() {}

void VelocityControlledMachine::metricsGui() {}








void VelocityControlledMachine::ControlWidget::gui(){
	glm::vec2 headerCursorPos = machine->reserveSpaceForMachineHeaderGui();
	
	if(!machine->isAxisConnected()) {
		machine->machineHeaderGui(headerCursorPos);
		ImGui::TextWrapped("No Axis Connected");
		return;
	}
	
	ImGui::BeginGroup();
	
	machine->verticalWidgetGui();

	ImGui::EndGroup();
	
	float widgetWidth = ImGui::GetItemRectSize().x;
	machine->machineHeaderGui(headerCursorPos, widgetWidth);
	machine->machineStateControlGui(widgetWidth);
}


//————————————————————————————————————————————— WIDGET GUI —————————————————————————————————————————————

void VelocityControlledMachine::verticalWidgetGui(){
		
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2));
	ImGui::BeginDisabled(!isEnabled());

	animatableVelocity->manualControlsVerticalGui(ImGui::GetTextLineHeight() * 10.f, nullptr, invertControlGui->value);

	float controlsHeight = ImGui::GetItemRectSize().y;
	if(controlsHeight <= 0.0) controlsHeight = 1.0;

	ImGui::SameLine();

	float actualEffort = 0.0;
	if(isAxisConnected()) actualEffort = getAxisInterface()->getEffortActual();
	float effortProgress = actualEffort;
	while(effortProgress > 1.0) effortProgress -= 1.0;
	if(actualEffort > 2.0) {
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::red);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::yellow);
	}else if(actualEffort > 1.0){
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::yellow);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::green);
	}else{
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetColorU32(ImGuiCol_FrameBg));
	}

	verticalProgressBar(effortProgress, ImVec2(ImGui::GetTextLineHeight() * .5f, controlsHeight));
	ImGui::PopStyleColor(2);

	ImGui::EndDisabled();
	ImGui::PopStyleVar();
	
}
