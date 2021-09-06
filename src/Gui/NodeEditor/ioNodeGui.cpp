#include <pch.h>

#include "Gui/gui.h"
#include "Environnement/nodeGraph/ioNode.h"


namespace NodeEditor = ax::NodeEditor;

void ioNode::nodeGui() {

    NodeEditor::BeginNode(getUniqueID());

    //===== do some text size calculations to be able to cleanly align output nodes to the right =====

    ImVec2 nodePosition = NodeEditor::GetNodePosition(getUniqueID());         //top left coordinate of the node
    static float pinSpacing = ImGui::GetTextLineHeight() / 2.0;                     //horizontal space between input and output pins
    static float nodePadding = NodeEditor::GetStyle().NodePadding.x;                //horizontal blank space at the inside edge of the node
    static float borderOffset = NodeEditor::GetStyle().NodeBorderWidth / 1.5f;      //offset from the border thickness of the node
    static float labelRounding = NodeEditor::GetStyle().NodeRounding;               //radius of the node rounded corners
    static float iconSize = ImGui::GetTextLineHeight();                             //square size of the pin icons
    static float iconDummyWidth = ImGui::GetTextLineHeight() * 0.75;                //width the pin icon actually occupies

    //find the widest input pin text
    //find the widest output pin text
    float titleTextWidth = ImGui::CalcTextSize(getName()).x;
    float inputPinTextWidth = 0;
    for (auto pin : getNodeInputData()) {
        if (!pin->isVisible() && !pin->isConnected()) continue;
        float pinTextWidth = ImGui::CalcTextSize(pin->getName()).x;
        if (pinTextWidth > inputPinTextWidth) inputPinTextWidth = pinTextWidth;
    }
    float outputPinTextWidth = 0;
    for (auto pin : getNodeOutputData()) {
        if (!pin->isVisible() && !pin->isConnected()) continue;
        float pinTextWidth = ImGui::CalcTextSize(pin->getName()).x;
        if (pinTextWidth > outputPinTextWidth) outputPinTextWidth = pinTextWidth;
    }

    //calculate the width of the pin section
    //calculate the width of the title section
    float pinSectionWidth = inputPinTextWidth                           //width of the input pin text
        + outputPinTextWidth                        //width of the output pin text
        + 2 * NodeEditor::GetStyle().NodePadding.x  //padding at the two edges of the nodes
        + 2 * iconDummyWidth                        //width of the icons
        + 5 * ImGui::GetStyle().ItemSpacing.x;      //spacings between icon and text (2) input and output groups and dummy spacer (2) and right align padding and outputpin (1)
    +pinSpacing;                               //spacing between input and output pins
    float titleSectionWidth = titleTextWidth + 2 * nodePadding;

    //compare title width and pin width to get the overall node width
    //get the width of the output pin block to be able to calculate an offset to right align all pins
    float nodeWidth;
    float outputPinGroupWidth;
    if (titleSectionWidth > pinSectionWidth) {
        nodeWidth = titleSectionWidth;
        outputPinGroupWidth = titleSectionWidth                     //overall node width
            - inputPinTextWidth                     //Width of the input pin text
            - nodePadding                           //left padding edge 
            - iconDummyWidth                        //icon width
            - ImGui::GetStyle().ItemSpacing.x;      //icon to text spacing
        -pinSpacing;                           //spacing between input and output pins
    }
    else {
        nodeWidth = pinSectionWidth;
        outputPinGroupWidth = outputPinTextWidth                    //Width of the input pin text
            + nodePadding                           //right padding edge
            + iconDummyWidth                        //icon width
            + 4 * ImGui::GetStyle().ItemSpacing.x;  //group spacing, right align padding to pin spacing, text to icon spacing
        +pinSpacing;                           //spacing between input and output pins
    }

    ImU32 labelColor = ImColor();
    labelColor = ImColor(0.1f, 0.1f, 0.6f, 0.5f);

    //===== draw colored background label and centered title =====

    glm::vec2 rectMin = glm::vec2(nodePosition.x, nodePosition.y) + glm::vec2(borderOffset);
    glm::vec2 rectMax = glm::vec2(nodePosition) + glm::vec2(nodeWidth - borderOffset, ImGui::GetTextLineHeightWithSpacing() + nodePadding);
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

    //===== draw input pins =====

    ImGui::BeginGroup();
    for (auto pin : getNodeInputData()) {
        if (!pin->isVisible() && !pin->isConnected()) continue;
        NodeEditor::BeginPin(pin->getUniqueID(), NodeEditor::PinKind::Input);
        NodeEditor::PinPivotAlignment(ImVec2(0.0, 0.5));
        ImGui::Dummy(glm::vec2(iconDummyWidth));
        glm::vec2 min = ImGui::GetItemRectMin();
        min.x -= iconSize * 0.15; //shift the visual position of the icon
        glm::vec2 max = min + glm::vec2(iconSize);
        DrawIcon(ImGui::GetWindowDrawList(), min, max, 0, false, ImColor(1.0f, 1.0f, 1.0f, 1.0f), ImColor(0.0f, 0.0f, 0.0f, 1.0f));
        ImGui::SameLine();
        ImGui::Text(pin->getName());
        NodeEditor::EndPin();
    }
    ImGui::EndGroup();

    //===== Vertical Spacer between input and output pins =====

    ImGui::SameLine();
    ImGui::Dummy(glm::vec2(pinSpacing));
    ImGui::SameLine();

    //===== draw output pins =====

    ImGui::BeginGroup();
    for (auto pin : getNodeOutputData()) {
        if (!pin->isVisible() && !pin->isConnected()) continue;
        //calculate offset to right align pins
        float rightAlignSpacingWidth = outputPinGroupWidth                  //overall width of the output pins
            - nodePadding                           //right edge node padding
            - ImGui::CalcTextSize(pin->getName()).x //width of the pin text
            - ImGui::GetStyle().ItemSpacing.x * 4   //center spacing (2), spacing between offset and pin (1), spacing between text and icon (1)
            - iconDummyWidth                        //actual width of the icon
            - pinSpacing;                           //width of the vertical input and output pin separator
//apply offset
        ImGui::Dummy(glm::vec2(rightAlignSpacingWidth, 0));
        ImGui::SameLine();
        NodeEditor::BeginPin(pin->getUniqueID(), NodeEditor::PinKind::Output);
        NodeEditor::PinPivotAlignment(ImVec2(1.0, 0.5));
        ImGui::Text(pin->getName());
        ImGui::SameLine();
        ImGui::Dummy(glm::vec2(iconDummyWidth));
        glm::vec2 min = ImGui::GetItemRectMin();
        min.x -= iconSize * 0.15; //shift the visual position of the icon
        glm::vec2 max = min + glm::vec2(iconSize);
        DrawIcon(ImGui::GetWindowDrawList(), min, max, 0, false, ImColor(1.0f, 1.0f, 1.0f, 1.0f), ImColor(0.0f, 0.0f, 0.0f, 1.0f));
        NodeEditor::EndPin();
    }
    ImGui::EndGroup();

    /*
    if (node->getType() == Node::Type::Container && node->getSubtype() == Node::Subtype::Group)
        NodeEditor::Group(NodeEditor::GetNodeSize(node->getUniqueID()));
    */

    NodeEditor::EndNode();

    /*
    if (node->getType() == Node::Type::Container && NodeEditor::BeginGroupHint(node->getUniqueID())) {
        auto min = NodeEditor::GetGroupMin();
        auto max = NodeEditor::GetGroupMax();
        ImVec2 cursor = ImVec2(min.x + 8, min.y - ImGui::GetTextLineHeightWithSpacing() - 4);
        ImGui::SetCursorScreenPos(cursor);
        ImGui::BeginGroup();
        ImGui::TextUnformatted(node->getName().c_str());
        ImGui::EndGroup();
    }
    NodeEditor::EndGroupHint();
    */
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