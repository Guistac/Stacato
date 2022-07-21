#include <pch.h>

#include "NodeGraphGui.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_node_editor.h>

#include "Environnement/NodeGraph/NodeGraph.h"
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

namespace NodeEditor = ax::NodeEditor;

float Node::getTitleWidth() {
    ImGui::PushFont(Fonts::sansBold15);
    float width = ImGui::CalcTextSize(getName()).x;
    ImGui::PopFont();
    return width;
}

void Node::titleGui() {
    ImGui::PushFont(Fonts::sansBold15);
    ImGui::Text("%s", getName());
    ImGui::PopFont();
}

void Node::nodeGui() {

    static float nodePadding = NodeEditor::GetStyle().NodePadding.x;    //horizontal blank space at the inside edge of the node
    static float nodeBorderWidth = NodeEditor::GetStyle().NodeBorderWidth;  //offset from the border thickness of the node
    static float labelRounding = NodeEditor::GetStyle().NodeRounding;       //radius of the node rounded corners
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    glm::vec4 labelColor = glm::vec4(0.1, 0.1, 0.6, 0.5);
    bool nodeIsOffline = false;

    if (getType() == Node::Type::IODEVICE) {
        std::shared_ptr<Device> device = std::static_pointer_cast<Device>(shared_from_this());
        nodeIsOffline = !device->isDetected() && !device->isConnected();
        switch (device->getDeviceType()) {
            case Device::Type::ETHERCAT_DEVICE:
                labelColor = glm::vec4(0.7, 0.2, 0.0, 0.5f);
                break;
            case Device::Type::NETWORK_DEVICE:
                labelColor = glm::vec4(0.6f, 0.1f, 0.6f, 0.5f);
                break;
            case Device::Type::USB_DEVICE:
                labelColor = glm::vec4(0.1f, 0.1f, 0.6f, 0.5f);
                break;
        }
    }

    if (nodeIsOffline) {
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
    }

    if (getType() == Node::Type::CONTAINER) {

        NodeEditor::BeginNode(getUniqueID());
        ImGui::Text("%s", getName());
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

        getTitleWidth();

        //find the widest pin line
        float titleTextWidth = getTitleWidth();
        float widestPin = 0;
        for (auto pin : getInputPins()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            float pinWidth = pin->getGuiWidth();
            if (pinWidth > widestPin) widestPin = pinWidth;
        }
        for (auto pin : getOutputPins()) {
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

        ImVec2 nodePosition = NodeEditor::GetNodePosition(getUniqueID());       //top left coordinate of the node
        glm::vec2 rectMin = glm::vec2(nodePosition.x, nodePosition.y) + glm::vec2(nodeBorderWidth * 0.8);
        glm::vec2 rectMax = rectMin + glm::vec2(nodeWidth - 2 * nodeBorderWidth, ImGui::GetTextLineHeightWithSpacing());
        drawList->AddRectFilled(rectMin, rectMax, ImColor(labelColor), labelRounding, ImDrawFlags_RoundCornersTop);

        ImGui::NewLine();
        float spacing = (nodeWidth - titleTextWidth - 2 * nodePadding) / 2.0;
        ImGui::SameLine(spacing, 0);
        titleGui();
        ImGui::Spacing();

        //===== draw input and output pins =====

        for (auto pin : getInputPins()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            pin->pinGui();
        }
        ImGui::Spacing();
        for (auto pin : getOutputPins()) {
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

        float inputTitleTextWidth = getTitleWidth();
        if (outputLabelWidth > inputTitleTextWidth) inputTitleTextWidth = outputLabelWidth;
        float widestInputPin = 0;
        for (auto pin : getInputPins()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            float pinWidth = pin->getGuiWidth();
            if (pinWidth > widestInputPin) widestInputPin = pinWidth;
        }

        float inputPinSectionWidth = widestInputPin + 2 * nodePadding;
        float inputTitleSectionWidth = inputTitleTextWidth + 2 * nodePadding;
        float inputNodeWidth = inputTitleSectionWidth > inputPinSectionWidth ? inputTitleSectionWidth : inputPinSectionWidth;

        ImVec2 nodePosition = NodeEditor::GetNodePosition(getUniqueID());       //top left coordinate of the node
        glm::vec2 inputTitleRectMin = glm::vec2(nodePosition.x, nodePosition.y) + glm::vec2(nodeBorderWidth * 0.8);
        glm::vec2 inputTitleRectMax = inputTitleRectMin + glm::vec2(inputNodeWidth - 2 * nodeBorderWidth, ImGui::GetTextLineHeightWithSpacing() + ImGui::GetTextLineHeight());
        drawList->AddRectFilled(inputTitleRectMin, inputTitleRectMax, ImColor(labelColor), labelRounding, ImDrawFlags_RoundCornersTop);
        
        ImGui::NewLine();
        float inputTitleSpacing = (inputNodeWidth - inputTitleTextWidth - 2 * nodePadding) / 2.0;
        ImGui::SameLine(inputTitleSpacing, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(0, -ImGui::GetTextLineHeight() * 0.15));
        titleGui();
        ImGui::PopStyleVar();
        ImGui::NewLine();
        float outputLabelSpacing = (inputNodeWidth - outputLabelWidth - 2 * nodePadding) / 2.0;
        ImGui::SameLine(outputLabelSpacing);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
        ImGui::Text("%s", outputLabel);
        ImGui::PopStyleColor();
        ImGui::Spacing();

        for (auto pin : getInputPins()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            pin->pinGui();
        }

        NodeEditor::EndNode();

        int splitNodeID = INT_MAX - getUniqueID();
		//int splitNodeID = -getUniqueID();

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

        float outputTitleTextWidth = getTitleWidth();
        if (inputLabelWidth > outputTitleTextWidth) outputTitleTextWidth = inputLabelWidth;
        float widestOutputPin = 0;
        for (auto pin : getOutputPins()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            float pinWidth = pin->getGuiWidth();
            if (pinWidth > widestOutputPin) widestOutputPin = pinWidth;
        }

        float outputPinSectionWidth = widestOutputPin + 2 * nodePadding;
        float outputTitleSectionWidth = outputTitleTextWidth + 2 * nodePadding;
        float outputNodeWidth = outputTitleSectionWidth > outputPinSectionWidth ? outputTitleSectionWidth : outputPinSectionWidth;

        nodePosition = NodeEditor::GetNodePosition(splitNodeID);       //top left coordinate of the node
        glm::vec2 outputTextRectMin = glm::vec2(nodePosition.x, nodePosition.y) + glm::vec2(nodeBorderWidth * 0.8);
        glm::vec2 outputTextRectMax = outputTextRectMin + glm::vec2(outputNodeWidth - 2 * nodeBorderWidth, ImGui::GetTextLineHeightWithSpacing() + ImGui::GetTextLineHeight());
        drawList->AddRectFilled(outputTextRectMin, outputTextRectMax, ImColor(labelColor), labelRounding, ImDrawFlags_RoundCornersTop);
        
        ImGui::NewLine();
        float outputTitleSpacing = (outputNodeWidth - outputTitleTextWidth - 2 * nodePadding) / 2.0;
        ImGui::SameLine(outputTitleSpacing, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(0, -ImGui::GetTextLineHeight() * 0.15));
        titleGui();
        ImGui::PopStyleVar();
        ImGui::NewLine();
        float inputLabelSpacing = (outputNodeWidth - inputLabelWidth - 2 * nodePadding) / 2.0;
        ImGui::SameLine(inputLabelSpacing, 0);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
        ImGui::Text("%s", inputLabel);
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::Spacing();
        for (auto pin : getOutputPins()) {
            if (!pin->isVisible() && !pin->isConnected()) continue;
            float rightAlignSpacingWidth = outputNodeWidth - 2 * nodePadding - ImGui::GetStyle().ItemSpacing.x - pin->getGuiWidth();
            ImGui::Dummy(glm::vec2(rightAlignSpacingWidth, 0));
            ImGui::SameLine();
            pin->pinGui();
        }

        NodeEditor::EndNode();
		
    }

    if (nodeIsOffline) {
        ImGui::PopStyleColor();
    }
}




void Node::propertiesGui() {
    if (ImGui::BeginTabBar("PropertiesTabBar")) {

        nodeSpecificGui();
        
        if(ImGui::BeginTabItem("Node")) {
            
            static auto displayDataTable = [](std::vector<std::shared_ptr<NodePin>>& data, const char* tableName) {
                ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;
                if (ImGui::BeginTable(tableName, 4, tableFlags)) {
                    ImGui::TableSetupColumn("Show");
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableSetupColumn("Value");
                    ImGui::TableHeadersRow();
                    int checkBoxID = 9999999;
                    for (auto data : data) {
                        ImGui::PushID(data->getSaveString());
                        
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::PushID(checkBoxID++);
                        ImGui::Checkbox("##Visible", &data->isVisible());
                        ImGui::PopID();
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 10.0);
                        ImGui::InputText("##NameEdit", data->displayString, 64);
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%s", Enumerator::getDisplayString(data->dataType));
                        ImGui::TableSetColumnIndex(3);
                        ImGui::Text("%s", data->getValueString());

                        ImGui::PopID();
                    }
                    ImGui::EndTable();
                }
            };

            ImGui::Text("Node Name");
            ImGui::InputText("##nodeCustomName", (char*)getName(), 128);

            if (getType() == Node::Type::IODEVICE) {
                ImGui::Checkbox("Split Inputs and Outputs", &b_isSplit);
            }

            ImGui::Separator();

            if (hasInputs()) {
                ImGui::Text("Input Data:");
                displayDataTable(getInputPins(), "Input Data");
                ImGui::Separator();
            }
            if (hasOutputs()) {
                ImGui::Text("Output Data:");
                displayDataTable(getOutputPins(), "Output Data");
                ImGui::Separator();
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

glm::vec2 Node::getNodeGraphPosition() {
    return NodeEditor::GetNodePosition(getUniqueID());
}

void Node::getSplitNodeGraphPosition(glm::vec2& in, glm::vec2& out) {
    in = NodeEditor::GetNodePosition(getUniqueID());
    out = NodeEditor::GetNodePosition(INT_MAX - getUniqueID());
}

void Node::restoreSavedPosition() {
    NodeEditor::SetNodePosition(getUniqueID(), savedPosition);
    if (isSplit()) NodeEditor::SetNodePosition(INT_MAX - getUniqueID(), savedSplitPosition);
}
