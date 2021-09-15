#include <pch.h>

#include <imgui.h>
#include <imgui_node_editor.h>
#include <imgui_internal.h>

#include "Gui/NodeEditor/NodeEditorGui.h"

#include "nodeGraph/ioNode.h"
#include "nodeGraph/nodeGraph.h"

namespace NodeEditor = ax::NodeEditor;

bool ioData::shouldDisplayDataGui() {
    if (b_noDataField) return false;
    else if (b_forceDataField) return true;
    else if (isOutput()) return parentNode->parentNodeGraph->b_showOutputValues;
    else if (isConnected()) return false;
    else return true;
}

void ioData::dataGui() {
    static float dataFieldWidth = ImGui::GetTextLineHeight() * 4.0;
    ImGui::SetNextItemWidth(dataFieldWidth);

    if(b_disableDataField) ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

    ImGui::PushID(getUniqueID());
    switch (getType()) {
    case DataType::BOOLEAN_VALUE:
        ImGui::Checkbox("##", &booleanValue);
        break;
    case DataType::INTEGER_VALUE:
        ImGui::InputScalar("##", ImGuiDataType_S64, &integerValue);
        break;
    case DataType::REAL_VALUE:
        ImGui::InputDouble("##", &realValue, 0.0, 0.0, "%.3f");
        break;
    }
    if (ImGui::IsItemEdited()) {
        parentNode->parentNodeGraph->evaluate(parentNode);
    }
    ImGui::PopID();
    if (b_disableDataField) ImGui::PopItemFlag();
}

float ioData::getGuiWidth() {
    static float iconSize = ImGui::GetTextLineHeight();                             //square size of the pin icons
    static float iconDummyWidth = ImGui::GetTextLineHeight() * 0.75;                //width the pin icon actually occupies
    static float dataFieldWidth = ImGui::GetTextLineHeight() * 4.0;
    //get the pin title width
    float pinTextWidth = ImGui::CalcTextSize(getName()).x;

    //if the pin is connected, don't display its value, but add space for an icon
    if (!shouldDisplayDataGui())  return pinTextWidth + ImGui::GetStyle().ItemSpacing.x + iconDummyWidth;
    //if it is connected and the type is boolean, add the width and spacing for a checkbox and icon
    else if (getType() == DataType::BOOLEAN_VALUE)  return pinTextWidth + 2 * ImGui::GetStyle().ItemSpacing.x + iconDummyWidth + ImGui::GetFrameHeight();
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

void ioData::pinGui() {
    static float iconSize = ImGui::GetTextLineHeight();                             //square size of the pin icons
    static float iconDummyWidth = ImGui::GetTextLineHeight() * 0.75;                //width the pin icon actually occupies
    static float dataFieldWidth = ImGui::GetTextLineHeight() * 4.0;

    pinIcon icon;
    switch (getType()) {
    case DataType::BOOLEAN_VALUE: icon = ROUNDED_SQUARED; break;
    case DataType::INTEGER_VALUE: icon = DIAMOND; break;
    case DataType::REAL_VALUE: icon = ARROW; break;
    default: icon = CIRCLE_ARROW_OUT; break;
    }

    //decide if we should show a data field
    bool showDataField = shouldDisplayDataGui();

    if (isInput()) {
        if (!b_disablePin) {
            NodeEditor::BeginPin(getUniqueID(), NodeEditor::PinKind::Input);
            NodeEditor::PinPivotAlignment(ImVec2(0.0, 0.5));
        }
        ImGui::Dummy(glm::vec2(iconDummyWidth));
        //spacing.x
        if (!b_disablePin) {
            glm::vec2 min = ImGui::GetItemRectMin();
            min.x -= iconSize * 0.15; //shift the visual position of the icon
            glm::vec2 max = min + glm::vec2(iconSize);
            DrawPinIcon(ImGui::GetWindowDrawList(), min, max, icon, isConnected(), ImColor(1.0f, 1.0f, 1.0f, 1.0f), ImColor(0.0f, 0.0f, 0.0f, 1.0f));
        }
        ImGui::SameLine();
        ImGui::Text(getName());
        if (!b_disablePin) {
            NodeEditor::EndPin();
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
            NodeEditor::BeginPin(getUniqueID(), NodeEditor::PinKind::Output);
            NodeEditor::PinPivotAlignment(ImVec2(1.0, 0.5));
        }
        ImGui::Text(getName());
        ImGui::SameLine();
        //spacing.x
        ImGui::Dummy(glm::vec2(iconDummyWidth));
        if (!b_disablePin) {
            glm::vec2 min = ImGui::GetItemRectMin();
            min.x -= iconSize * 0.15; //shift the visual position of the icon
            glm::vec2 max = min + glm::vec2(iconSize);
            DrawPinIcon(ImGui::GetWindowDrawList(), min, max, icon, isConnected(), ImColor(1.0f, 1.0f, 1.0f, 1.0f), ImColor(0.0f, 0.0f, 0.0f, 1.0f));
            NodeEditor::EndPin();
        }
    }
}