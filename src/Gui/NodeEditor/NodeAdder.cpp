#include <pch.h>

#include "Gui/Gui.h"
#include "Fieldbus/EtherCatSlave.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "NodeGraph/Utilities/ioNodeFactory.h"

std::shared_ptr<ioNode> nodeAdderContextMenu() {

    std::shared_ptr<ioNode> output = nullptr;

    ImGui::MenuItem("Node Editor Menu", nullptr, false, false);
    ImGui::Separator();
    ImGui::MenuItem("EtherCAT devices", nullptr, false, false);
    if (ImGui::BeginMenu("By Manufaturer")){
        for (auto manufacturer : EtherCatDeviceFactory::getDevicesByManufacturer()) {
            if (ImGui::BeginMenu(manufacturer.name)) {
                for (auto device : manufacturer.devices) {
                    if (ImGui::MenuItem(device->getNodeName())) output = device->getNewDeviceInstance();
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("By Category")) {
        for (auto manufacturer : EtherCatDeviceFactory::getDevicesByCategory()) {
            if (ImGui::BeginMenu(manufacturer.name)) {
                for (auto device : manufacturer.devices) {
                    if (ImGui::MenuItem(device->getNodeName())) output = device->getNewDeviceInstance();
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMenu();
    }

    ImGui::Separator();

    if (ImGui::BeginMenu("Axis")) {
        for (auto axis : ioNodeFactory::getAxisTypes()) {
            if (ImGui::MenuItem(axis->getNodeName())) output = axis->getNewNodeInstance();
        }
        ImGui::EndMenu();
    }
    
    ImGui::Separator();
    
    if (ImGui::BeginMenu("Network")) {
        
        ImGui::EndMenu();
    }

    ImGui::Separator();

    ImGui::MenuItem("Processing Nodes", nullptr, false, false);
    for (auto category : ioNodeFactory::getNodesByCategory()) {
        if (ImGui::BeginMenu(category.name)) {
            for (auto device : category.nodes) {
                if (ImGui::MenuItem(device->getNodeName())) output = device->getNewNodeInstance();
            }
            ImGui::EndMenu();
        }
    }

    return output;
}



void nodeAdder() {

    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetTextLineHeight() * 1.0);

    if(ImGui::BeginChild("##NodeList", ImGui::GetContentRegionAvail())) {

        ImGui::PushFont(Fonts::robotoBold20);
        ImGui::Text("Node Selector");
        ImGui::PopFont();
        ImGui::Separator();

        ImGui::PushFont(Fonts::robotoBold15);
        if (ImGui::TreeNode("EtherCAT Slaves")) {
            ImGui::PushFont(Fonts::robotoRegular15);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));
            ImGui::Text("By Manufacturer");
            ImGui::PopStyleColor();
            for (auto& manufacturer : EtherCatDeviceFactory::getDevicesByManufacturer()) {
                if (ImGui::TreeNode(manufacturer.name)) {
                    for (auto& slave : manufacturer.devices) {
                        bool selected = false;
                        const char* deviceName = slave->getNodeName();
                        ImGui::Selectable(deviceName, &selected);
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                            ImGui::SetDragDropPayload("EtherCatSlave", &deviceName, sizeof(const char*));
                            ImGui::Text(deviceName);
                            ImGui::EndDragDropSource();
                        }
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::Separator();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));
            ImGui::Text("By Category");
            ImGui::PopStyleColor();
            for (auto& manufacturer : EtherCatDeviceFactory::getDevicesByCategory()) {
                if (ImGui::TreeNode(manufacturer.name)) {
                    for (auto& slave : manufacturer.devices) {
                        bool selected = false;
                        const char* deviceName = slave->getNodeName();
                        ImGui::Selectable(deviceName, &selected);
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                            ImGui::SetDragDropPayload("EtherCatSlave", &deviceName, sizeof(const char*));
                            ImGui::Text(deviceName);
                            ImGui::EndDragDropSource();
                        }
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::PopFont();
            ImGui::TreePop();
        }
        ImGui::PopFont();

        ImGui::Separator();

        ImGui::PushFont(Fonts::robotoBold15);
        if (ImGui::TreeNode("Axis")) {
            ImGui::PushFont(Fonts::robotoRegular15);
            for (auto axis : ioNodeFactory::getAxisTypes()) {
                bool selected = false;
                const char* axisName = axis->getNodeName();
                ImGui::Selectable(axisName, &selected);
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    ImGui::SetDragDropPayload("Axis", &axisName, sizeof(const char*));
                    ImGui::Text(axisName);
                    ImGui::EndDragDropSource();
                }
            }
            ImGui::PopFont();
            ImGui::TreePop();
        }
        ImGui::PopFont();

        ImGui::Separator();

        ImGui::PushFont(Fonts::robotoBold15);
        if (ImGui::TreeNode("Network IO")) {
            ImGui::PushFont(Fonts::robotoRegular15);
            bool selected = false;
            ImGui::Selectable("OSC", &selected);
            ImGui::Selectable("Artnet", &selected);
            ImGui::Selectable("PSN", &selected);
            ImGui::PopFont();
            ImGui::TreePop();
        }
        ImGui::PopFont();

        ImGui::Separator();

        ImGui::PushFont(Fonts::robotoBold15);
        if (ImGui::TreeNode("Data Processors")) {
            ImGui::PushFont(Fonts::robotoRegular15);
            for (auto category : ioNodeFactory::getNodesByCategory()) {
                if (ImGui::TreeNode(category.name)) {
                    for (ioNode* node : category.nodes) {
                        bool selected = false;
                        const char* nodeName = node->getNodeName();
                        ImGui::Selectable(nodeName, &selected);
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                            ImGui::SetDragDropPayload("ProcessorNode", &nodeName, sizeof(const char*));
                            ImGui::Text(nodeName);
                            ImGui::EndDragDropSource();
                        }
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::PopFont();
            ImGui::TreePop();
        }
        ImGui::PopFont();

        ImGui::EndChild();
    }

    ImGui::PopStyleVar();
}

std::shared_ptr<ioNode> acceptDraggedNode() {
    if (ImGui::BeginDragDropTarget()) {
        const ImGuiPayload* payload;
        glm::vec2 mousePosition = ImGui::GetMousePos();
        payload = ImGui::AcceptDragDropPayload("EtherCatSlave");
        if (payload != nullptr && payload->DataSize == sizeof(const char*)) {
            const char* slaveDeviceName = *(const char**)payload->Data;
            std::shared_ptr<ioNode> newSlave = EtherCatDeviceFactory::getDeviceByName(slaveDeviceName);
            return newSlave;
        }
        payload = ImGui::AcceptDragDropPayload("ProcessorNode");
        if (payload != nullptr && payload->DataSize == sizeof(const char*)) {
            const char* nodeName = *(const char**)payload->Data;
            std::shared_ptr<ioNode> newNode = ioNodeFactory::getIoNodeByName(nodeName);
            return newNode;
        }
        payload = ImGui::AcceptDragDropPayload("Axis");
        if (payload != nullptr && payload->DataSize == sizeof(const char*)) {
            const char* axisName = *(const char**)payload->Data;
            std::shared_ptr<ioNode> newAxis = ioNodeFactory::getAxisByName(axisName);
            return newAxis;
        }
        ImGui::EndDragDropTarget();
    }
    return nullptr;
}