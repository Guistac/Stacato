#include <pch.h>

#include "Gui/gui.h"
#include "Environnement/nodeGraph/ioNode.h"


namespace NodeEditor = ax::NodeEditor;

void ioData::dataGui() {
    static float dataFieldWidth = ImGui::GetTextLineHeight() * 4.0;
    ImGui::SetNextItemWidth(dataFieldWidth);
    if (isOutput()) ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
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
    ImGui::PopID();
    if (isOutput()) ImGui::PopItemFlag();
}

float ioData::getGuiWidth(bool alwaysShowValue) {
    static float iconSize = ImGui::GetTextLineHeight();                             //square size of the pin icons
    static float iconDummyWidth = ImGui::GetTextLineHeight() * 0.75;                //width the pin icon actually occupies
    static float dataFieldWidth = ImGui::GetTextLineHeight() * 4.0;
    //get the pin title width
    float pinTextWidth = ImGui::CalcTextSize(getName()).x;
    //if the pin is connected, don't display its value, but add space for an icon
    if (isConnected() && !alwaysShowValue)          return pinTextWidth + ImGui::GetStyle().ItemSpacing.x + iconDummyWidth;
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

void ioData::pinGui(bool alwaysShowValue) {
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

    if (isInput()) {
        NodeEditor::BeginPin(getUniqueID(), NodeEditor::PinKind::Input);
        NodeEditor::PinPivotAlignment(ImVec2(0.0, 0.5));
        ImGui::Dummy(glm::vec2(iconDummyWidth));
        //spacing.x
        glm::vec2 min = ImGui::GetItemRectMin();
        min.x -= iconSize * 0.15; //shift the visual position of the icon
        glm::vec2 max = min + glm::vec2(iconSize);
        DrawIcon(ImGui::GetWindowDrawList(), min, max, icon, isConnected(), ImColor(1.0f, 1.0f, 1.0f, 1.0f), ImColor(0.0f, 0.0f, 0.0f, 1.0f));
        ImGui::SameLine();
        ImGui::Text(getName());
        NodeEditor::EndPin();
        //spacing.x
        if (!isConnected() || alwaysShowValue) {
            ImGui::SameLine();
            dataGui();
        }
    }
    else if (isOutput()) {
        if (!isConnected() || alwaysShowValue) {
            dataGui();
            ImGui::SameLine();
        }
        //spacing.x
        NodeEditor::BeginPin(getUniqueID(), NodeEditor::PinKind::Output);
        NodeEditor::PinPivotAlignment(ImVec2(1.0, 0.5));
        ImGui::Text(getName());
        ImGui::SameLine();
        //spacing.x
        ImGui::Dummy(glm::vec2(iconDummyWidth));
        glm::vec2 min = ImGui::GetItemRectMin();
        min.x -= iconSize * 0.15; //shift the visual position of the icon
        glm::vec2 max = min + glm::vec2(iconSize);
        DrawIcon(ImGui::GetWindowDrawList(), min, max, icon, isConnected(), ImColor(1.0f, 1.0f, 1.0f, 1.0f), ImColor(0.0f, 0.0f, 0.0f, 1.0f));
        NodeEditor::EndPin();
    }
}

void ioNode::nodeGui(bool alwaysShowValue) {

    NodeEditor::BeginNode(getUniqueID());

    if (getType() != NodeType::NODEGROUPER) {
        //===== do some text size calculations to be able to cleanly align output nodes to the right =====

        //find the widest pin line
        float titleTextWidth = ImGui::CalcTextSize(getName()).x;
        float widestPin = 0;
        for (auto pin : getNodeInputData()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            float pinWidth = pin->getGuiWidth(alwaysShowValue);
            if (pinWidth > widestPin) widestPin = pinWidth;
        }
        for (auto pin : getNodeOutputData()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            float pinWidth = pin->getGuiWidth(alwaysShowValue) + ImGui::GetStyle().ItemSpacing.x; //spacing of the right align offset dummy
            if (pinWidth > widestPin) widestPin = pinWidth;
        }

        //compare title width and pin width to get the overall node width
        //get the width of the output pin block to be able to calculate an offset to right align all output pins
        static float nodePadding = NodeEditor::GetStyle().NodePadding.x;    //horizontal blank space at the inside edge of the node
        float pinSectionWidth = widestPin + 2 * nodePadding;
        float titleSectionWidth = titleTextWidth + 2 * nodePadding;
        float nodeWidth = titleSectionWidth > pinSectionWidth ? titleSectionWidth : pinSectionWidth;

        //===== draw colored background label and centered title =====

        ImU32 labelColor = ImColor(0.1f, 0.1f, 0.6f, 0.5f);
        ImVec2 nodePosition = NodeEditor::GetNodePosition(getUniqueID());       //top left coordinate of the node
        static float nodeBorderWidth = NodeEditor::GetStyle().NodeBorderWidth;  //offset from the border thickness of the node
        static float labelRounding = NodeEditor::GetStyle().NodeRounding;       //radius of the node rounded corners
        glm::vec2 rectMin = glm::vec2(nodePosition.x, nodePosition.y) + glm::vec2(nodeBorderWidth * 0.8);
        glm::vec2 rectMax = rectMin + glm::vec2(nodeWidth - 2 * nodeBorderWidth, ImGui::GetTextLineHeightWithSpacing());
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(rectMin, rectMax, labelColor, labelRounding, ImDrawFlags_RoundCornersTop);
        if (titleSectionWidth < nodeWidth) {
            ImGui::NewLine();
            float spacing = nodeWidth - titleTextWidth - 2 * nodePadding;
            spacing /= 2.0;
            ImGui::SameLine(spacing, 0);
        }
        ImGui::Text(getName());
        ImGui::Spacing();

        //===== draw input and output pins =====

        for (auto pin : getNodeInputData()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            pin->pinGui(alwaysShowValue);
        }
        ImGui::Spacing();
        for (auto pin : getNodeOutputData()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            float rightAlignSpacingWidth = nodeWidth - 2 * nodePadding - ImGui::GetStyle().ItemSpacing.x - pin->getGuiWidth(alwaysShowValue);
            ImGui::Dummy(glm::vec2(rightAlignSpacingWidth, 0));
            ImGui::SameLine();
            pin->pinGui(alwaysShowValue);
        }
    }
    else {
        ImGui::Text(getName());
        NodeEditor::Group(NodeEditor::GetNodeSize(getUniqueID()));
    }
    NodeEditor::EndNode();


    if (getType() == NodeType::NODEGROUPER && NodeEditor::BeginGroupHint(getUniqueID())) { 
        auto min = NodeEditor::GetGroupMin();
        auto max = NodeEditor::GetGroupMax();
        ImVec2 cursor = ImVec2(min.x + 8, min.y - ImGui::GetTextLineHeightWithSpacing() - 4);
        ImGui::SetCursorScreenPos(cursor);
        ImGui::BeginGroup();
        ImGui::TextUnformatted(getName());
        ImGui::EndGroup();
        NodeEditor::EndGroupHint();
    }
    
}

void ioNode::propertiesGui() {
    if (ImGui::BeginTabBar("PropertiesTabBar")) {

        nodeSpecificGui();

        if(ImGui::BeginTabItem("ioData")) {
            
            static auto displayDataTable = [](std::vector<std::shared_ptr<ioData>>& data, const char* tableName) {
                ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
                if (ImGui::BeginTable(tableName, 4, tableFlags)) {
                    ImGui::TableSetupColumn("Show");
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableSetupColumn("Value");
                    ImGui::TableHeadersRow();
                    int checkBoxID = 9999999;
                    for (auto data : data) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::PushID(checkBoxID++);
                        ImGui::Checkbox("##Visible", &data->isVisible());
                        ImGui::PopID();
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%s", data->getName());
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%s", data->getTypeName());
                        ImGui::TableSetColumnIndex(3);
                        switch (data->getType()) {
                        case DataType::BOOLEAN_VALUE:  ImGui::Text("%i", data->getBoolean()); break;
                        case DataType::INTEGER_VALUE:  ImGui::Text("%i", data->getInteger()); break;
                        case DataType::REAL_VALUE:     ImGui::Text("%.5f", data->getReal()); break;
                        }
                    }
                    ImGui::EndTable();
                }
            };

            ImGui::Text("Input Data:");
            displayDataTable(getNodeInputData(), "Input Data");
            ImGui::Separator();
            ImGui::Text("Output Data:");
            displayDataTable(getNodeOutputData(), "Output Data");
            ImGui::Separator();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}