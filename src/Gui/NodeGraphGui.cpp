#include <pch.h>

#include "GUI.h"

#include "Gui/Guiwindow.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatSlave.h"
#include "Environnement/Environnement.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_node_editor.h>


#include "Utilities/Timing.h"

#include <imnodes.h>

void nodeGraph() {

    

    ImNodes::BeginNodeEditor();
    ImNodesStyle& style = ImNodes::GetStyle();
    style.NodeCornerRounding = 10.0;
    style.PinCircleRadius = 10.0;
    style.PinHoverRadius = 20.0;
    style.PinOffset = 0.0;
    style.NodeBorderThickness = 2.0;
    style.NodePaddingHorizontal = 15.0;
    ImNodesIO& io = ImNodes::GetIO();
    io.AltMouseButton = 1;

    ImNodes::BeginNode(0);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Node Title");

    ImNodes::EndNodeTitleBar();

    ImNodes::BeginOutputAttribute(1, ImNodesPinShape_Triangle);
    ImGui::TextUnformatted("output pin 1");
    ImNodes::EndOutputAttribute();

    ImNodes::BeginOutputAttribute(2);
    ImGui::TextUnformatted("output pin 2");
    ImNodes::EndOutputAttribute();


    ImNodes::BeginInputAttribute(3);
    ImGui::TextUnformatted("Input Pin 1 aaaaaa");
    ImNodes::EndInputAttribute();

    ImNodes::EndNode();


    //glm::vec2 panning = ImNodes::EditorContextGetPanning();
    //Logger::warn("panning {} {}", panning.x, panning.y);
    //glm::vec2 panning;
    //panning.x = sin(Timing::getTime_seconds()) * 100.0;
    //panning.y = cos(Timing::getTime_seconds()) * 100.0;
    //ImNodes::EditorContextResetPanning(panning);
    glm::vec2 nodepos = ImNodes::GetNodeEditorSpacePos(0);
    Logger::warn("node0: {} {}", nodepos.x, nodepos.y);

    ImNodes::MiniMap();
    ImNodes::EndNodeEditor();
}












































































































































#ifdef LELELELEL

void nodeEditor();
void drawNodes(NodeGraph&);
void drawLinks(NodeGraph&);
void createLink(NodeGraph&);
void deleteLink(NodeGraph&);
void deleteNode(NodeGraph&);
void DrawIcon(ImDrawList* drawList, const glm::vec2& a, const glm::vec2& b, int type, bool filled, ImU32 color, ImU32 innerColor);

namespace NodeEditor = ax::NodeEditor;

void nodeGraph() {
	
	ImGui::BeginGroup();

	if (ImGui::BeginListBox("##NodeList", ImVec2(ImGui::GetTextLineHeight() * 15.0, ImGui::GetContentRegionAvail().y))) {

		if (ImGui::TreeNode("EtherCAT Slaves")) {
			ImGui::Button("Refresh Device List");
			for (auto slave : EtherCatFieldbus::slaves_unassigned) {

				bool selected = false;
				ImGui::Selectable(slave->getName(), &selected);

                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    
                    ImGui::SetDragDropPayload("EtherCatSlave", &slave, sizeof(std::shared_ptr<EtherCatSlave>));
					ImGui::Text("%s", slave->getName());
					ImGui::Text("Drop On Node Editor to Add Slave");

                    ImGui::EndDragDropSource();
                }
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Axis")) {
			bool selected = false;
			ImGui::Selectable("Linear Axis", &selected);
			ImGui::Selectable("Rotating Axis", &selected);
			ImGui::Selectable("State Machine", &selected);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Network IO")) {
			bool selected = false;
			ImGui::Selectable("OSC", &selected);
			ImGui::Selectable("Artnet", &selected);
			ImGui::Selectable("PSN", &selected);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Data Processors")) {
			bool selected = false;
			ImGui::Selectable("Condition", &selected);
			ImGui::Selectable("Add", &selected);
			ImGui::Selectable("Subtract", &selected);
			ImGui::Selectable("Multiply", &selected);
			ImGui::Selectable("Divide", &selected);
			ImGui::Selectable("Limit", &selected);
			ImGui::Selectable("Map", &selected);
			ImGui::TreePop();
		}

		ImGui::EndListBox();
	}

	ImGui::EndGroup();

	ImGui::SameLine();

    //========= VERTICAL SEPARATOR ==========

	ImGui::BeginGroup();

	nodeEditor();

    if (ImGui::BeginDragDropTarget()){
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EtherCatSlave");
        if (payload != nullptr && payload->DataSize == sizeof(std::shared_ptr<EtherCatSlave>)){
            std::shared_ptr<EtherCatSlave> slave = *(std::shared_ptr<EtherCatSlave>*)payload->Data;
			Environnement::add(slave);
        }
        ImGui::EndDragDropTarget();
    }

	if (ImGui::Button("Center View")) ax::NodeEditor::NavigateToContent();

	ImGui::EndGroup();
}

void nodeEditor() {
    NodeGraph& nodeGraph = Environnement::nodeGraph;
	NodeEditor::SetCurrentEditor(GuiWindow::nodeEditorContext);
	NodeEditor::Begin("Node Editor", ImVec2(0, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeight() * 1.7));

    drawNodes(nodeGraph);
    drawLinks(nodeGraph);

    // Handle creation action, returns true if editor want to create new object (node or link)
    if (NodeEditor::BeginCreate(ImVec4(1.0, 1.0, 1.0, 1.0), 1.0)) {
        createLink(nodeGraph); //TODO: don't allow duplicate links
    }
    // Wraps up object creation action handling.
    NodeEditor::EndCreate();


    // Handle deletion action
    if (NodeEditor::BeginDelete()) {
        deleteLink(nodeGraph);
        deleteNode(nodeGraph);
    }
    NodeEditor::EndDelete(); // Wrap up deletion action

    static NodeEditor::NodeId contextNodeId = 0;
    static NodeEditor::LinkId contextLinkId = 0;
    static NodeEditor::PinId  contextPinId = 0;

    NodeEditor::Suspend();
    if (NodeEditor::ShowNodeContextMenu(&contextNodeId))		ImGui::OpenPopup("Node Context Menu");
    else if (NodeEditor::ShowPinContextMenu(&contextPinId))		ImGui::OpenPopup("Pin Context Menu");
    else if (NodeEditor::ShowLinkContextMenu(&contextLinkId))	ImGui::OpenPopup("Link Context Menu");
    else if (NodeEditor::ShowBackgroundContextMenu())			ImGui::OpenPopup("Background Context Menu");
    NodeEditor::Resume();

    NodeEditor::Suspend();
    if (ImGui::BeginPopup("Node Context Menu")) {
        ImGui::Text("Node Context Menu, Node#%i", contextNodeId);
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Pin Context Menu")) {
        ImGui::Text("Pin Context Menu, Pin#%i", contextPinId);
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Link Context Menu")) {
        ImGui::Text("Link Context Menu, Link#%i", contextLinkId);
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Background Context Menu")) {
        ImGui::Text("Background Context Menu");
        ImGui::EndPopup();
    }
    NodeEditor::Resume();


	NodeEditor::End();
}



void drawNodes(NodeGraph& nodeGraph) {

    for (ioNode* node : nodeGraph.getNodes()) {

        NodeEditor::BeginNode(node->getUniqueID());

        float pinSpacing = 20.0;

        std::vector<ioData*>& inputPins = node->getNodeInputData();
        std::vector<ioData*>& outputPins = node->getNodeOutputData();

        ImVec2 nodePosition = NodeEditor::GetNodePosition(node->getUniqueID());
        float nodePadding = NodeEditor::GetStyle().NodePadding.x;
        float borderOffset = NodeEditor::GetStyle().NodeBorderWidth / 1.5f;
        float labelRounding = NodeEditor::GetStyle().NodeRounding - borderOffset;

        //calculate width of node, skipping hidden pins that are not connected
        int inputPinCount = (int)inputPins.size();
        int outputPinCount = (int)outputPins.size();
        float nodeContentWidth = 0.0;
        int in = 0;
        int out = 0;
        while (in < inputPinCount || out < outputPinCount) {
            float rowWidth = 0.0;
            //while (in < inputPinCount && inputPins[in]->isHidden() && !inputPins[in]->isConnected()) in++;
            //while (out < outputPinCount && outputPins[out]->isHidden() && !outputPins[out]->isConnected()) out++;
            if (in < inputPinCount) rowWidth += ImGui::CalcTextSize(inputPins[in]->getName()).x;
            if (out < outputPinCount) rowWidth += ImGui::CalcTextSize(outputPins[out]->getName()).x;
            if (in < inputPinCount && out < outputPinCount) rowWidth += pinSpacing;
            else if (in >= inputPinCount) rowWidth += nodePadding;
            if (rowWidth > nodeContentWidth) nodeContentWidth = rowWidth;
            in++;
            out++;
        }

        float titleWidth = ImGui::CalcTextSize(node->getName()).x;
        bool isTitleMasterWidth = titleWidth >= nodeContentWidth;
        float nodeWidth = isTitleMasterWidth ? titleWidth : nodeContentWidth;

        ImU32 labelColor = ImColor();
        /*
        switch (node->getSubtype()) {
        case Node::Subtype::AudioInterface:
            labelColor = ImColor(0.6f, 0.1f, 0.1f, 0.5f);
            break;
        case Node::Subtype::AudioProcessor:
            labelColor = ImColor(0.1f, 0.1f, 0.6f, 0.5f);
            break;
        case Node::Subtype::MidiInterface:
            labelColor = ImColor(0.1f, 0.6f, 0.1f, 0.5f);
            break;
        case Node::Subtype::MidiProcessor:
            labelColor = ImColor(0.4f, 0.6f, 0.1f, 0.5f);
            break;
        case Node::Subtype::DataInterface: break;
        case Node::Subtype::DataProcessor: break;
        case Node::Subtype::VideoInterface: break;
        case Node::Subtype::VideoProcessor: break;
        case Node::Subtype::Group:
            labelColor = ImColor(0.7f, 0.7f, 0.7f, 0.5f);
            break;
        }
        */
        labelColor = ImColor(0.1f, 0.1f, 0.6f, 0.5f);

        //draw centered title and colored background label
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 rectMin = ImVec2(nodePosition.x + borderOffset, nodePosition.y + borderOffset);
        ImVec2 rectMax = ImVec2(nodePosition.x + nodeWidth + nodePadding * 2 - borderOffset, nodePosition.y + ImGui::GetTextLineHeight() + nodePadding * 2 - borderOffset);
        drawList->AddRectFilled(rectMin, rectMax, labelColor, labelRounding, ImDrawFlags_RoundCornersTop);
        if (isTitleMasterWidth) ImGui::Text(node->getName());
        else {
            float titleOffset = (nodeContentWidth - titleWidth) / 2;
            ImGui::NewLine();
            ImGui::SameLine(titleOffset);
            ImGui::Text(node->getName());
        }

        ImGui::Spacing();

        //draw all input and output pins, skipping hidden ones that are not connected
        in = 0;
        out = 0;
        while (in < inputPinCount || out < outputPinCount) {
            //while (in < inputPinCount && inputPins[in]->isHidden() && !inputPins[in]->isConnected()) in++;
            //while (out < outputPinCount && outputPins[out]->isHidden() && !outputPins[out]->isConnected()) out++;
            if (in < inputPinCount) {
                NodeEditor::BeginPin(inputPins[in]->getUniqueID(), NodeEditor::PinKind::Input);

                NodeEditor::PinPivotAlignment(ImVec2(0, 0.5));

                glm::vec2 min = ImGui::GetCursorScreenPos();
                glm::vec2 size = ImGui::CalcTextSize(inputPins[in]->getName());

                /*
                float padding = 2.0;
                ImVec2 points[] = {
                    glm::vec2(min.x - padding, min.y - padding),
                    glm::vec2(min.x + size.x + padding, min.y - padding),
                    glm::vec2(min.x + size.x + padding, min.y + size.y + padding),
                    glm::vec2(min.x - padding, min.y + size.y + padding),
                    glm::vec2(min.x - padding - size.y / 2 - padding, min.y + size.y / 2)
                };

                ImGui::GetWindowDrawList()->AddConvexPolyFilled(points, 5, ImColor(0.5, 0.1, 0.1, 0.5f));
                */

                ImGui::GetWindowDrawList()->AddRectFilled(min, min + size, ImColor(0.5f, 0.1f, 0.1f, 0.5f), 5.f);

                ImGui::Text(inputPins[in]->getName());

                glm::vec2 topRight = min + glm::vec2(size.x, 0);
                glm::vec2 iconSize = glm::vec2(ImGui::GetTextLineHeight());

                DrawIcon(ImGui::GetWindowDrawList(), topRight, topRight + iconSize, 0, false, ImColor(1.0f, 1.0f, 1.0f, 1.0f), ImColor(0.0f, 0.0f, 0.0f, 1.0f));

                NodeEditor::EndPin();
            }
            if (out < outputPinCount) {
                if (in >= inputPinCount) ImGui::NewLine();
                float textWidth = ImGui::CalcTextSize(outputPins[out]->getName()).x;
                float offset = nodeWidth - textWidth;
                ImGui::SameLine(offset);
                NodeEditor::BeginPin(outputPins[out]->getUniqueID(), NodeEditor::PinKind::Output);
                NodeEditor::PinPivotAlignment(ImVec2(1, 0.5));

                ImVec2 min = ImGui::GetCursorScreenPos();
                ImVec2 size = ImGui::CalcTextSize(outputPins[out]->getName());

                float padding = 2.0;

                ImVec2 points[] = {
                    ImVec2(min.x - padding, min.y - padding),
                    ImVec2(min.x + size.x + padding, min.y - padding),
                    ImVec2(min.x + size.x + padding + size.y / 2 + padding, min.y + size.y / 2),
                    ImVec2(min.x + size.x + padding, min.y + size.y + padding),
                    ImVec2(min.x - padding, min.y + size.y + padding)
                };

                ImGui::GetWindowDrawList()->AddConvexPolyFilled(points, 5, ImColor(0.5, 0.1, 0.1, 0.5f));


                ImGui::Text(outputPins[out]->getName());
                NodeEditor::EndPin();
            }
            in++;
            out++;
        }

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
}

void drawLinks(NodeGraph& nodeGraph) {
    for (ioLink* link : nodeGraph.getLinks())
        NodeEditor::Link(link->getUniqueID(),
            link->getOutputPin()->getUniqueID(),
            link->getInputPin()->getUniqueID(),
            ImColor(1.0f, 1.0f, 1.0f),
            1.0);
}

void createLink(NodeGraph& nodeGraph) {
    NodeEditor::PinId pin1Id, pin2Id;
    if (NodeEditor::QueryNewLink(&pin1Id, &pin2Id)) {
        if (pin1Id && pin2Id) {
            ioData* pin1 = nodeGraph.getIoData(pin1Id.Get());
            ioData* pin2 = nodeGraph.getIoData(pin2Id.Get());
            if (pin1 && pin2 && nodeGraph.isConnectionValid(pin1, pin2)) {
                if (NodeEditor::AcceptNewItem(ImColor(1.0f, 1.0f, 1.0f), 3.0)) {
                    ioLink* link = nodeGraph.connect(pin1, pin2);
                    NodeEditor::Link(link->getUniqueID(), pin1Id, pin2Id);
                }
            }
            else NodeEditor::RejectNewItem(ImColor(1.0f, 0.0f, 0.0f), 3.0);
        }
    }
}

void deleteLink(NodeGraph& nodeGraph) {
    NodeEditor::LinkId deletedLinkId;
    while (NodeEditor::QueryDeletedLink(&deletedLinkId)) {
        if (NodeEditor::AcceptDeletedItem()) {
            ioLink* deletedLink = nodeGraph.getIoLink(deletedLinkId.Get());
            if (deletedLink) nodeGraph.disconnect(deletedLink);
        }
    }
}

void deleteNode(NodeGraph& nodeGraph) {
    NodeEditor::NodeId deletedNodeId;
    while (NodeEditor::QueryDeletedNode(&deletedNodeId)) {
        ioNode* deletedNode = nodeGraph.getIoNode(deletedNodeId.Get());
        if (NodeEditor::AcceptDeletedItem()) {
            nodeGraph.removeIoNode(deletedNode);
        }
        else NodeEditor::RejectDeletedItem();
    }
}









void DrawIcon(ImDrawList* drawList, const glm::vec2& a, const glm::vec2& b, int type, bool filled, ImU32 color, ImU32 innerColor)
{
    auto rect = ImRect(a, b);
    auto rect_x = rect.Min.x;
    auto rect_y = rect.Min.y;
    auto rect_w = rect.Max.x - rect.Min.x;
    auto rect_h = rect.Max.y - rect.Min.y;
    auto rect_center_x = (rect.Min.x + rect.Max.x) * 0.5f;
    auto rect_center_y = (rect.Min.y + rect.Max.y) * 0.5f;
    auto rect_center = glm::vec2(rect_center_x, rect_center_y);
    const auto outline_scale = rect_w / 24.0f;
    const auto extra_segments = static_cast<int>(2 * outline_scale); // for full circle

    if (type == 0)
    {
        const auto origin_scale = rect_w / 24.0f;

        const auto offset_x = 1.0f * origin_scale;
        const auto offset_y = 0.0f * origin_scale;
        const auto margin = (filled ? 2.0f : 2.0f) * origin_scale;
        const auto rounding = 0.1f * origin_scale;
        const auto tip_round = 0.7f; // percentage of triangle edge (for tip)
        //const auto edge_round = 0.7f; // percentage of triangle edge (for corner)
        const auto canvas = ImRect(
            rect.Min.x + margin + offset_x,
            rect.Min.y + margin + offset_y,
            rect.Max.x - margin + offset_x,
            rect.Max.y - margin + offset_y);
        const auto canvas_x = canvas.Min.x;
        const auto canvas_y = canvas.Min.y;
        const auto canvas_w = canvas.Max.x - canvas.Min.x;
        const auto canvas_h = canvas.Max.y - canvas.Min.y;

        const auto left = canvas_x + canvas_w * 0.5f * 0.3f;
        const auto right = canvas_x + canvas_w - canvas_w * 0.5f * 0.3f;
        const auto top = canvas_y + canvas_h * 0.5f * 0.2f;
        const auto bottom = canvas_y + canvas_h - canvas_h * 0.5f * 0.2f;
        const auto center_y = (top + bottom) * 0.5f;
        //const auto angle = AX_PI * 0.5f * 0.5f * 0.5f;

        const auto tip_top = glm::vec2(canvas_x + canvas_w * 0.5f, top);
        const auto tip_right = glm::vec2(right, center_y);
        const auto tip_bottom = glm::vec2(canvas_x + canvas_w * 0.5f, bottom);

        drawList->PathLineTo(glm::vec2(left, top) + glm::vec2(0, rounding));
        drawList->PathBezierCurveTo(
            glm::vec2(left, top),
            glm::vec2(left, top),
            glm::vec2(left, top) + glm::vec2(rounding, 0));
        drawList->PathLineTo(tip_top);
        drawList->PathLineTo(tip_top + (tip_right - tip_top) * tip_round);
        drawList->PathBezierCurveTo(
            tip_right,
            tip_right,
            tip_bottom + (tip_right - tip_bottom) * tip_round);
        drawList->PathLineTo(tip_bottom);
        drawList->PathLineTo(glm::vec2(left, bottom) + glm::vec2(rounding, 0));
        drawList->PathBezierCurveTo(
            glm::vec2(left, bottom),
            glm::vec2(left, bottom),
            glm::vec2(left, bottom) - glm::vec2(0, rounding));

        if (!filled)
        {
            if (innerColor & 0xFF000000)
                drawList->AddConvexPolyFilled(drawList->_Path.Data, drawList->_Path.Size, innerColor);

            drawList->PathStroke(color, true, 2.0f * outline_scale);
        }
        else
            drawList->PathFillConvex(color);
    }
    else
    {
        auto triangleStart = rect_center_x + 0.32f * rect_w;

        auto rect_offset = -static_cast<int>(rect_w * 0.25f * 0.25f);

        rect.Min.x += rect_offset;
        rect.Max.x += rect_offset;
        rect_x += rect_offset;
        rect_center_x += rect_offset * 0.5f;
        rect_center.x += rect_offset * 0.5f;

        if (type == 1)
        {
            const auto c = rect_center;

            if (!filled)
            {
                const auto r = 0.5f * rect_w / 2.0f - 0.5f;

                if (innerColor & 0xFF000000)
                    drawList->AddCircleFilled(c, r, innerColor, 12 + extra_segments);
                drawList->AddCircle(c, r, color, 12 + extra_segments, 2.0f * outline_scale);
            }
            else
                drawList->AddCircleFilled(c, 0.5f * rect_w / 2.0f, color, 12 + extra_segments);
        }

        if (type == 2)
        {
            if (filled)
            {
                const auto r = 0.5f * rect_w / 2.0f;
                const auto p0 = rect_center - glm::vec2(r, r);
                const auto p1 = rect_center + glm::vec2(r, r);

                drawList->AddRectFilled(p0, p1, color, 0, 15 + extra_segments);
            }
            else
            {
                const auto r = 0.5f * rect_w / 2.0f - 0.5f;
                const auto p0 = rect_center - glm::vec2(r, r);
                const auto p1 = rect_center + glm::vec2(r, r);

                if (innerColor & 0xFF000000)
                    drawList->AddRectFilled(p0, p1, innerColor, 0, 15 + extra_segments);

                drawList->AddRect(p0, p1, color, 0, 15 + extra_segments, 2.0f * outline_scale);
            }
        }

        if (type == 3)
        {
            const auto r = 0.5f * rect_w / 2.0f;
            const auto w = ceilf(r / 3.0f);

            const auto baseTl = glm::vec2(floorf(rect_center_x - w * 2.5f), floorf(rect_center_y - w * 2.5f));
            const auto baseBr = glm::vec2(floorf(baseTl.x + w), floorf(baseTl.y + w));

            auto tl = baseTl;
            auto br = baseBr;
            for (int i = 0; i < 3; ++i)
            {
                tl.x = baseTl.x;
                br.x = baseBr.x;
                drawList->AddRectFilled(tl, br, color);
                tl.x += w * 2;
                br.x += w * 2;
                if (i != 1 || filled)
                    drawList->AddRectFilled(tl, br, color);
                tl.x += w * 2;
                br.x += w * 2;
                drawList->AddRectFilled(tl, br, color);

                tl.y += w * 2;
                br.y += w * 2;
            }

            triangleStart = br.x + w + 1.0f / 24.0f * rect_w;
        }

        if (type == 4)
        {
            if (filled)
            {
                const auto r = 0.5f * rect_w / 2.0f;
                const auto cr = r * 0.5f;
                const auto p0 = rect_center - glm::vec2(r, r);
                const auto p1 = rect_center + glm::vec2(r, r);

                drawList->AddRectFilled(p0, p1, color, cr, 15);
            }
            else
            {
                const auto r = 0.5f * rect_w / 2.0f - 0.5f;
                const auto cr = r * 0.5f;
                const auto p0 = rect_center - glm::vec2(r, r);
                const auto p1 = rect_center + glm::vec2(r, r);

                if (innerColor & 0xFF000000)
                    drawList->AddRectFilled(p0, p1, innerColor, cr, 15);

                drawList->AddRect(p0, p1, color, cr, 15, 2.0f * outline_scale);
            }
        }
        else if (type == 5)
        {
            if (filled)
            {
                const auto r = 0.607f * rect_w / 2.0f;
                const auto c = rect_center;

                drawList->PathLineTo(c + glm::vec2(0, -r));
                drawList->PathLineTo(c + glm::vec2(r, 0));
                drawList->PathLineTo(c + glm::vec2(0, r));
                drawList->PathLineTo(c + glm::vec2(-r, 0));
                drawList->PathFillConvex(color);
            }
            else
            {
                const auto r = 0.607f * rect_w / 2.0f - 0.5f;
                const auto c = rect_center;

                drawList->PathLineTo(c + glm::vec2(0, -r));
                drawList->PathLineTo(c + glm::vec2(r, 0));
                drawList->PathLineTo(c + glm::vec2(0, r));
                drawList->PathLineTo(c + glm::vec2(-r, 0));

                if (innerColor & 0xFF000000)
                    drawList->AddConvexPolyFilled(drawList->_Path.Data, drawList->_Path.Size, innerColor);

                drawList->PathStroke(color, true, 2.0f * outline_scale);
            }
        }
        else
        {
            const auto triangleTip = triangleStart + rect_w * (0.45f - 0.32f);

            drawList->AddTriangleFilled(
                ImVec2(ceilf(triangleTip), rect_y + rect_h * 0.5f),
                ImVec2(triangleStart, rect_center_y + 0.15f * rect_h),
                ImVec2(triangleStart, rect_center_y - 0.15f * rect_h),
                color);
        }
    }
}

#endif