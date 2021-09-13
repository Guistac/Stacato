#include <pch.h>

#include "nodeGraph/ioNode.h"
#include "nodeGraph/nodeGraph.h"

#include "NodeEditorGui.h"


#include <imgui.h>
#include <imgui_node_editor.h>
#include "Gui/Framework/Fonts.h"

namespace NodeEditor = ax::NodeEditor;

float ioNode::getTitleWidth(bool isOutputSection) {
    if (getType() == IODEVICE && isOutputSection) {
        static float iconSize = ImGui::GetTextLineHeight();                             //square size of the pin icons
        static float iconDummyWidth = ImGui::GetTextLineHeight() * 0.75;                //width the pin icon actually occupies
        DeviceNode* thisDevice = (DeviceNode*)this;
        return ImGui::CalcTextSize(getName()).x + ImGui::GetStyle().ItemSpacing.x + iconDummyWidth;
    }
    else {
        return ImGui::CalcTextSize(getName()).x;
    }
}

void ioNode::titleGui(bool isOutputSection) {
    if (getType() == IODEVICE && isOutputSection) {
        static float iconSize = ImGui::GetTextLineHeight();                             //square size of the pin icons
        static float iconDummyWidth = ImGui::GetTextLineHeight() * 0.75;                //width the pin icon actually occupies
        DeviceNode* thisDevice = dynamic_cast<DeviceNode*>(this);
        NodeEditor::BeginPin(thisDevice->deviceLink->getUniqueID(), NodeEditor::PinKind::Output);
        NodeEditor::PinPivotAlignment(ImVec2(1.0, 0.5));
        ImGui::Text(getName());
        ImGui::SameLine();
        ImGui::Dummy(glm::vec2(iconDummyWidth, 1));
        glm::vec2 min = ImGui::GetItemRectMin();
        min.x -= iconSize * 0.15; //shift the visual position of the icon
        glm::vec2 max = min + glm::vec2(iconSize);
        DrawPinIcon(ImGui::GetWindowDrawList(), min, max, 5, thisDevice->deviceLink->isConnected(), ImColor(1.0f, 1.0f, 1.0f, 1.0f), ImColor(0.0f, 0.0f, 0.0f, 1.0f));
        NodeEditor::EndPin();
    }
    else {
        ImGui::Text(getName());
    }
}

void ioNode::nodeGui() {

    static float nodePadding = NodeEditor::GetStyle().NodePadding.x;    //horizontal blank space at the inside edge of the node
    static float nodeBorderWidth = NodeEditor::GetStyle().NodeBorderWidth;  //offset from the border thickness of the node
    static float labelRounding = NodeEditor::GetStyle().NodeRounding;       //radius of the node rounded corners
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    if (getType() == NodeType::CONTAINER) {

        NodeEditor::BeginNode(getUniqueID());
        ImGui::Text(getName());
        NodeEditor::Group(NodeEditor::GetNodeSize(getUniqueID()));
        NodeEditor::EndNode();
        if (NodeEditor::BeginGroupHint(getUniqueID())) {
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
    else if (!b_isSplit) {

        if (b_wasSplit) b_wasSplit = false;

        NodeEditor::BeginNode(getUniqueID());

        //===== do some text size calculations to be able to cleanly align output nodes to the right =====

        getTitleWidth(true);

        //find the widest pin line
        ImGui::PushFont(Fonts::robotoBold15);
        float titleTextWidth = getTitleWidth(true);
        ImGui::PopFont();
        float widestPin = 0;
        for (auto pin : getNodeInputData()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            float pinWidth = pin->getGuiWidth();
            if (pinWidth > widestPin) widestPin = pinWidth;
        }
        for (auto pin : getNodeOutputData()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            float pinWidth = pin->getGuiWidth() + ImGui::GetStyle().ItemSpacing.x; //spacing of the right align offset dummy
            if (pinWidth > widestPin) widestPin = pinWidth;
        }

        //compare title width and pin width to get the overall node width
        //get the width of the output pin block to be able to calculate an offset to right align all output pins
        float pinSectionWidth = widestPin + 2 * nodePadding;
        float titleSectionWidth = titleTextWidth + 2 * nodePadding;
        float nodeWidth = titleSectionWidth > pinSectionWidth ? titleSectionWidth : pinSectionWidth;

        //===== draw colored background label and centered title =====

        ImU32 labelColor = ImColor(0.1f, 0.1f, 0.6f, 0.5f);
        ImVec2 nodePosition = NodeEditor::GetNodePosition(getUniqueID());       //top left coordinate of the node
        glm::vec2 rectMin = glm::vec2(nodePosition.x, nodePosition.y) + glm::vec2(nodeBorderWidth * 0.8);
        glm::vec2 rectMax = rectMin + glm::vec2(nodeWidth - 2 * nodeBorderWidth, ImGui::GetTextLineHeightWithSpacing());
        drawList->AddRectFilled(rectMin, rectMax, labelColor, labelRounding, ImDrawFlags_RoundCornersTop);

        ImGui::NewLine();
        float spacing = (nodeWidth - titleTextWidth - 2 * nodePadding) / 2.0;
        ImGui::SameLine(spacing, 0);
        ImGui::PushFont(Fonts::robotoBold15);
        titleGui(true);
        ImGui::PopFont();
        ImGui::Spacing();

        //===== draw input and output pins =====

        for (auto pin : getNodeInputData()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            pin->pinGui();
        }
        ImGui::Spacing();
        for (auto pin : getNodeOutputData()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            float rightAlignSpacingWidth = nodeWidth - 2 * nodePadding - ImGui::GetStyle().ItemSpacing.x - pin->getGuiWidth();
            ImGui::Dummy(glm::vec2(rightAlignSpacingWidth, 0));
            ImGui::SameLine();
            pin->pinGui();
        }
        
        NodeEditor::EndNode();

    }
    else {

        //====== SPLIT NODE ======
        //node is split into two graphical nodes
        //an input and output node
    
        static const char* outputLabel = "Outputs";
        static const char* inputLabel = "Inputs";
        static float outputLabelWidth = ImGui::CalcTextSize(outputLabel).x;
        static float inputLabelWidth = ImGui::CalcTextSize(inputLabel).x;


        //===== SPLIT INPUT NODE =====

        NodeEditor::BeginNode(getUniqueID());

        ImGui::PushFont(Fonts::robotoBold15);
        float inputTitleTextWidth = getTitleWidth(false);
        ImGui::PopFont();
        if (outputLabelWidth > inputTitleTextWidth) inputTitleTextWidth = outputLabelWidth;
        float widestInputPin = 0;
        for (auto pin : getNodeInputData()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            float pinWidth = pin->getGuiWidth();
            if (pinWidth > widestInputPin) widestInputPin = pinWidth;
        }

        float inputPinSectionWidth = widestInputPin + 2 * nodePadding;
        float inputTitleSectionWidth = inputTitleTextWidth + 2 * nodePadding;
        float inputNodeWidth = inputTitleSectionWidth > inputPinSectionWidth ? inputTitleSectionWidth : inputPinSectionWidth;

        ImU32 labelColor = ImColor(0.1f, 0.1f, 0.6f, 0.5f);
        ImVec2 nodePosition = NodeEditor::GetNodePosition(getUniqueID());       //top left coordinate of the node
        glm::vec2 inputTitleRectMin = glm::vec2(nodePosition.x, nodePosition.y) + glm::vec2(nodeBorderWidth * 0.8);
        glm::vec2 inputTitleRectMax = inputTitleRectMin + glm::vec2(inputNodeWidth - 2 * nodeBorderWidth, ImGui::GetTextLineHeightWithSpacing() + ImGui::GetTextLineHeight());
        drawList->AddRectFilled(inputTitleRectMin, inputTitleRectMax, labelColor, labelRounding, ImDrawFlags_RoundCornersTop);
        
        ImGui::NewLine();
        float inputTitleSpacing = (inputNodeWidth - inputTitleTextWidth - 2 * nodePadding) / 2.0;
        ImGui::SameLine(inputTitleSpacing, 0);
        ImGui::PushFont(Fonts::robotoBold15);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(0, -ImGui::GetTextLineHeight() * 0.15));
        titleGui(false);
        ImGui::PopStyleVar();
        ImGui::PopFont();
        ImGui::NewLine();
        float outputLabelSpacing = (inputNodeWidth - outputLabelWidth - 2 * nodePadding) / 2.0;
        ImGui::SameLine(outputLabelSpacing);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
        ImGui::Text(outputLabel);
        ImGui::PopStyleColor();
        ImGui::Spacing();

        for (auto pin : getNodeInputData()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            pin->pinGui();
        }

        NodeEditor::EndNode();

        int splitNodeID = -getUniqueID();

        if (!b_wasSplit) {
            b_wasSplit = true;
            glm::vec2 inputNodePosition = NodeEditor::GetNodePosition(getUniqueID());
            glm::vec2 inputNodeSize = NodeEditor::GetNodeSize(getUniqueID());
            glm::vec2 outputNodePosition = inputNodePosition;
            outputNodePosition.y += inputNodeSize.y;
            NodeEditor::SetNodePosition(splitNodeID, outputNodePosition);
        }

        //===== SPLIT OUTPUT NODE ======


        NodeEditor::BeginNode(splitNodeID);

        ImGui::PushFont(Fonts::robotoBold15);
        float outputTitleTextWidth = getTitleWidth(true);
        ImGui::PopFont();
        if (inputLabelWidth > outputTitleTextWidth) outputTitleTextWidth = inputLabelWidth;
        float widestOutputPin = 0;
        for (auto pin : getNodeOutputData()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            float pinWidth = pin->getGuiWidth();
            if (pinWidth > widestOutputPin) widestOutputPin = pinWidth;
        }

        float outputPinSectionWidth = widestOutputPin + 2 * nodePadding;
        float outputTitleSectionWidth = outputTitleTextWidth + 2 * nodePadding;
        float outputNodeWidth = outputTitleSectionWidth > outputPinSectionWidth ? outputTitleSectionWidth : outputPinSectionWidth;

        labelColor = ImColor(0.1f, 0.1f, 0.6f, 0.5f);
        nodePosition = NodeEditor::GetNodePosition(splitNodeID);       //top left coordinate of the node
        glm::vec2 outputTextRectMin = glm::vec2(nodePosition.x, nodePosition.y) + glm::vec2(nodeBorderWidth * 0.8);
        glm::vec2 outputTextRectMax = outputTextRectMin + glm::vec2(outputNodeWidth - 2 * nodeBorderWidth, ImGui::GetTextLineHeightWithSpacing() + ImGui::GetTextLineHeight());
        drawList->AddRectFilled(outputTextRectMin, outputTextRectMax, labelColor, labelRounding, ImDrawFlags_RoundCornersTop);
        
        ImGui::NewLine();
        float outputTitleSpacing = (outputNodeWidth - outputTitleTextWidth - 2 * nodePadding) / 2.0;
        ImGui::SameLine(outputTitleSpacing, 0);
        ImGui::PushFont(Fonts::robotoBold15);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(0, -ImGui::GetTextLineHeight() * 0.15));
        titleGui(true);
        ImGui::PopStyleVar();
        ImGui::PopFont();
        ImGui::NewLine();
        float inputLabelSpacing = (outputNodeWidth - inputLabelWidth - 2 * nodePadding) / 2.0;
        ImGui::SameLine(inputLabelSpacing, 0);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
        ImGui::Text(inputLabel);
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::Spacing();
        for (auto pin : getNodeOutputData()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            float rightAlignSpacingWidth = outputNodeWidth - 2 * nodePadding - ImGui::GetStyle().ItemSpacing.x - pin->getGuiWidth();
            ImGui::Dummy(glm::vec2(rightAlignSpacingWidth, 0));
            ImGui::SameLine();
            pin->pinGui();
        }

        NodeEditor::EndNode();

    }
}




void ioNode::propertiesGui() {
    if (ImGui::BeginTabBar("PropertiesTabBar")) {

        nodeSpecificGui();

        if(ImGui::BeginTabItem("Node")) {
            
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
                        ImGui::Text("%s", data->getValueString());
                    }
                    ImGui::EndTable();
                }
            };

            ImGui::Text("Node Name");
            ImGui::InputText("##nodeCustomName", (char*)getName(), 128);

            if (getType() == NodeType::IODEVICE) {
                ImGui::Checkbox("Split Inputs and Outputs", &b_isSplit);
            }

            ImGui::Separator();

            if (hasInputs()) {
                ImGui::Text("Input Data:");
                displayDataTable(getNodeInputData(), "Input Data");
                ImGui::Separator();
            }
            if (hasOutputs()) {
                ImGui::Text("Output Data:");
                displayDataTable(getNodeOutputData(), "Output Data");
                ImGui::Separator();
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

glm::vec2 ioNode::getNodeGraphPosition() {
    return NodeEditor::GetNodePosition(getUniqueID());
}

void ioNode::getSplitNodeGraphPosition(glm::vec2& in, glm::vec2& out) {
    in = NodeEditor::GetNodePosition(getUniqueID());
    out = NodeEditor::GetNodePosition(-getUniqueID());
}

void ioNode::restoreSavedPosition() {
    NodeEditor::SetNodePosition(getUniqueID(), savedPosition);
    if (isSplit()) NodeEditor::SetNodePosition(-getUniqueID(), savedSplitPosition);
}