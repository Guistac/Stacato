#include <pch.h>

#include "Gui/Environnement/NodeGraph/NodeGraphGui.h"

#include <imgui.h>
#include <imgui_node_editor.h>

#include "Environnement/NodeGraph/NodeGraph.h"

bool NodePin::shouldDisplayDataGui() {
    if (b_noDataField) return false;
    else if (b_forceDataField) return true;
    else if (isConnected()) return false;
    else return true;
}

void NodePin::dataGui() {
    static float dataFieldWidth = ImGui::GetTextLineHeight() * 4.0;
    ImGui::SetNextItemWidth(dataFieldWidth);

	ImGui::BeginDisabled(b_disableDataField);

    ImGui::PushID(getUniqueID());
    switch (dataType) {
		case DataType::BOOLEAN:
			ImGui::Checkbox("##", getSharedPointer<bool>().get());
			break;
		case DataType::INTEGER:
			ImGui::InputInt("##", getSharedPointer<int>().get(), 0, 0);
			break;
		case DataType::REAL:
			ImGui::InputDouble("##", getSharedPointer<double>().get(), 0.0, 0.0, "%.3f");
			break;
		default:
			break;
    }
    ImGui::PopID();
	ImGui::EndDisabled();
}

float NodePin::getGuiWidth() {
    static float iconSize = ImGui::GetTextLineHeight();                             //square size of the pin icons
    static float iconDummyWidth = ImGui::GetTextLineHeight() * 0.75;                //width the pin icon actually occupies
    static float dataFieldWidth = ImGui::GetTextLineHeight() * 4.0;
    //get the pin title width
    float pinTextWidth = ImGui::CalcTextSize(getDisplayString()).x;

    //if the pin is connected, don't display its value, but add space for an icon
    if (!shouldDisplayDataGui())  return pinTextWidth + ImGui::GetStyle().ItemSpacing.x + iconDummyWidth;
    //if it is connected and the type is boolean, add the width and spacing for a checkbox and icon
    else if (dataType == DataType::BOOLEAN)  return pinTextWidth + 2 * ImGui::GetStyle().ItemSpacing.x + iconDummyWidth + ImGui::GetFrameHeight();
    //if it is connected and is not a boolean, add the width and spacing for an input field and icon
    else                                            return pinTextWidth + 2 * ImGui::GetStyle().ItemSpacing.x + iconDummyWidth + dataFieldWidth;
}


enum pinIcon {
    ARROW = 0,
    CIRCLE_ARROW_OUT = 1,
    SQUARED_SQUARE = 3,
    ROUNDED_SQUARED = 4,
    DIAMOND = 5
};

void NodePin::pinGui() {
    static float iconSize = ImGui::GetTextLineHeight();                             //square size of the pin icons
    static float iconDummyWidth = ImGui::GetTextLineHeight() * 0.75;                //width the pin icon actually occupies
    static float dataFieldWidth = ImGui::GetTextLineHeight() * 4.0;

    pinIcon icon;
    switch (dataType) {
		case DataType::BOOLEAN: icon = ROUNDED_SQUARED; break;
		case DataType::INTEGER: icon = DIAMOND; break;
		case DataType::REAL: icon = ARROW; break;
		default: icon = CIRCLE_ARROW_OUT; break;
    }

    glm::vec4 pinColor = ImGui::GetStyle().Colors[ImGuiCol_Text];

    //decide if we should show a data field
    bool showDataField = shouldDisplayDataGui();

    if (isInput()) {
        if (!b_disablePin) {
			ax::NodeEditor::BeginPin(getUniqueID(), ax::NodeEditor::PinKind::Input);
			ax::NodeEditor::PinPivotAlignment(ImVec2(0.0, 0.5));
        }
        ImGui::Dummy(glm::vec2(iconDummyWidth));
        //spacing.x
        if (!b_disablePin) {
            glm::vec2 min = ImGui::GetItemRectMin();
            min.x -= iconSize * 0.15; //shift the visual position of the icon
            glm::vec2 max = min + glm::vec2(iconSize);
			Environnement::NodeGraph::Gui::DrawPinIcon(ImGui::GetWindowDrawList(),
													   min,
													   max,
													   icon,
													   isConnected(),
													   ImColor(pinColor),
													   ImColor(0.0f, 0.0f, 0.0f, 1.0f));
        }
        ImGui::SameLine();
        ImGui::Text("%s", getDisplayString());
        if (!b_disablePin) {
			ax::NodeEditor::EndPin();
        }
        //spacing.x
        if (shouldDisplayDataGui()) {
            ImGui::SameLine();
            dataGui();
        }
    }
    else if (isOutput()) {
        if (shouldDisplayDataGui()) {
            dataGui();
            ImGui::SameLine();
        }
        //spacing.x
        if (!b_disablePin) {
			assert(getUniqueID() > 0);
			ax::NodeEditor::BeginPin(getUniqueID(), ax::NodeEditor::PinKind::Output);
			ax::NodeEditor::PinPivotAlignment(ImVec2(1.0, 0.5));
        }
        ImGui::Text("%s", getDisplayString());
        ImGui::SameLine();
        //spacing.x
        ImGui::Dummy(glm::vec2(iconDummyWidth));
        if (!b_disablePin) {
            glm::vec2 min = ImGui::GetItemRectMin();
            min.x -= iconSize * 0.15; //shift the visual position of the icon
            glm::vec2 max = min + glm::vec2(iconSize);
			Environnement::NodeGraph::Gui::DrawPinIcon(ImGui::GetWindowDrawList(),
													   min,
													   max,
													   icon,
													   isConnected(),
													   ImColor(pinColor),
													   ImColor(0.0f, 0.0f, 0.0f, 1.0f));
			ax::NodeEditor::EndPin();
        }
    }
}
