#include <pch.h>

#include "Gui/Gui.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "NodeGraph/Utilities/NodeFactory.h"

void nodeAdder() {

    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetTextLineHeight() * 1.0);

    if(ImGui::BeginChild("##NodeList", ImGui::GetContentRegionAvail())) {

        ImGui::PushFont(Fonts::robotoBold20);
        ImGui::Text("Node Selector");
        ImGui::PopFont();
        ImGui::Separator();



        ImGui::PushFont(Fonts::robotoBold15);
        if (ImGui::CollapsingHeader("EtherCAT Slaves")) {

            ImGui::PushFont(Fonts::robotoRegular15);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));
            ImGui::Text("By Manufacturer");
            ImGui::PopStyleColor();
            for (auto& manufacturer : EtherCatDeviceFactory::getDevicesByManufacturer()) {
                if (ImGui::TreeNode(manufacturer.name)) {
                    for (auto& slave : manufacturer.devices) {
                        const char* deviceName = slave->getNodeName();
                        ImGui::Selectable(deviceName);
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                            ImGui::SetDragDropPayload("EtherCatDevice", &deviceName, sizeof(const char*));
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
                        const char* deviceName = slave->getNodeName();
                        ImGui::Selectable(deviceName);
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                            ImGui::SetDragDropPayload("EtherCatDevice", &deviceName, sizeof(const char*));
                            ImGui::Text(deviceName);
                            ImGui::EndDragDropSource();
                        }
                    }
                    ImGui::TreePop();
                }
            }


            ImGui::Separator();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));
            ImGui::Text("Detected Slaves");
            ImGui::PopStyleColor();
            static bool b_scanningNetwork = false;
            bool disableScanButton = EtherCatFieldbus::isCyclicExchangeActive();
            if (disableScanButton) BEGIN_DISABLE_IMGUI_ELEMENT
            if (ImGui::Button("Scan Network")) {
                std::thread etherCatNetworkScanner = std::thread([]() {
                    b_scanningNetwork = true;
                    EtherCatFieldbus::scanNetwork();
                    b_scanningNetwork = false;
                });
                etherCatNetworkScanner.detach();
            }
            if (disableScanButton) END_DISABLE_IMGUI_ELEMENT
            if (b_scanningNetwork) {
                ImGui::SameLine();
                ImGui::Text("Scanning...");
            }
            for (auto slave : EtherCatFieldbus::slaves_unassigned) {
                ImGui::Selectable(slave->getName());
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    ImGui::SetDragDropPayload("DetectedEtherCatDevice", &slave, sizeof(std::shared_ptr<EtherCatDevice>));
                    ImGui::Text(slave->getName());
                    ImGui::EndDragDropSource();
                }
            }



            ImGui::PopFont();
        }
        ImGui::PopFont();

        ImGui::PushFont(Fonts::robotoBold15);
        if (ImGui::CollapsingHeader("Machines")) {
            ImGui::PushFont(Fonts::robotoRegular15);
            for (auto machine : NodeFactory::getMachineTypes()) {
                const char* machineName = machine->getNodeName();
                ImGui::Selectable(machineName);
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    ImGui::SetDragDropPayload("Machine", &machineName, sizeof(const char*));
                    ImGui::Text(machineName);
                    ImGui::EndDragDropSource();
                }
            }
            ImGui::PopFont();
        }
        ImGui::PopFont();

        ImGui::PushFont(Fonts::robotoBold15);
        if (ImGui::CollapsingHeader("Network IO")) {
            ImGui::PushFont(Fonts::robotoRegular15);
            ImGui::Selectable("OSC");
            ImGui::Selectable("Artnet");
            ImGui::Selectable("PSN");
            ImGui::PopFont();
        }
        ImGui::PopFont();

        ImGui::PushFont(Fonts::robotoBold15);
        if (ImGui::CollapsingHeader("Data Processors")) {
            ImGui::PushFont(Fonts::robotoRegular15);
            for (auto category : NodeFactory::getNodesByCategory()) {
                if (ImGui::TreeNode(category.name)) {
                    for (Node* node : category.nodes) {
                        const char* nodeName = node->getNodeName();
                        ImGui::Selectable(nodeName);
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
        }
        ImGui::PopFont();

        ImGui::EndChild();
    }

    ImGui::PopStyleVar();
}

std::shared_ptr<Node> acceptDraggedNode() {
    if (ImGui::BeginDragDropTarget()) {
        const ImGuiPayload* payload;
        glm::vec2 mousePosition = ImGui::GetMousePos();
        payload = ImGui::AcceptDragDropPayload("EtherCatDevice");
        if (payload != nullptr && payload->DataSize == sizeof(const char*)) {
            const char* slaveDeviceName = *(const char**)payload->Data;
            std::shared_ptr<Node> newSlave = EtherCatDeviceFactory::getDeviceByName(slaveDeviceName);
            return newSlave;
        }
        payload = ImGui::AcceptDragDropPayload("DetectedEtherCatDevice");
        if (payload != nullptr && payload->DataSize == sizeof(std::shared_ptr<EtherCatDevice>)) {
            std::shared_ptr<EtherCatDevice> detectedSlave = *(std::shared_ptr<EtherCatDevice>*)payload->Data;
            std::vector<std::shared_ptr<EtherCatDevice>>& unassignedSlaves = EtherCatFieldbus::slaves_unassigned;
            for (int i = 0; i < unassignedSlaves.size(); i++) {
                if (unassignedSlaves[i] == detectedSlave) {
                    unassignedSlaves.erase(unassignedSlaves.begin() + i);
                    break;
                }
            }
            return detectedSlave;
        }
        payload = ImGui::AcceptDragDropPayload("ProcessorNode");
        if (payload != nullptr && payload->DataSize == sizeof(const char*)) {
            const char* nodeName = *(const char**)payload->Data;
            std::shared_ptr<Node> newNode = NodeFactory::getNodeByName(nodeName);
            return newNode;
        }
        payload = ImGui::AcceptDragDropPayload("Machine");
        if (payload != nullptr && payload->DataSize == sizeof(const char*)) {
            const char* machineName = *(const char**)payload->Data;
            std::shared_ptr<Node> newMachine = NodeFactory::getMachineByName(machineName);
            return newMachine;
        }
        ImGui::EndDragDropTarget();
    }
    return nullptr;
}



std::shared_ptr<Node> nodeAdderContextMenu() {

    std::shared_ptr<Node> output = nullptr;

    ImGui::MenuItem("Node Editor Menu", nullptr, false, false);
    ImGui::Separator();
    ImGui::MenuItem("EtherCAT devices", nullptr, false, false);
    if (ImGui::BeginMenu("By Manufaturer")) {
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

    std::shared_ptr<EtherCatDevice> selectedDetectedSlave = nullptr;
    if (!EtherCatFieldbus::slaves_unassigned.empty()) {
        if (ImGui::BeginMenu("Detected Slaves")) {
            for (auto detectedSlave : EtherCatFieldbus::slaves_unassigned) {
                if (ImGui::MenuItem(detectedSlave->getName())) {
                    output = detectedSlave;
                    selectedDetectedSlave = detectedSlave;
                }
            }
            ImGui::EndMenu();
        }
    }
    if (selectedDetectedSlave) {
        std::vector<std::shared_ptr<EtherCatDevice>>& unassignedSlaves = EtherCatFieldbus::slaves_unassigned;
        for (int i = 0; i < unassignedSlaves.size(); i++) {
            if (unassignedSlaves[i] == selectedDetectedSlave) {
                unassignedSlaves.erase(unassignedSlaves.begin() + i);
                break;
            }
        }
    }

    ImGui::Separator();

    if (ImGui::BeginMenu("Machine")) {
        for (auto machine : NodeFactory::getMachineTypes()) {
            if (ImGui::MenuItem(machine->getNodeName())) output = machine->getNewNodeInstance();
        }
        ImGui::EndMenu();
    }

    ImGui::Separator();

    if (ImGui::BeginMenu("Network")) {

        ImGui::EndMenu();
    }

    ImGui::Separator();

    ImGui::MenuItem("Processing Nodes", nullptr, false, false);
    for (auto category : NodeFactory::getNodesByCategory()) {
        if (ImGui::BeginMenu(category.name)) {
            for (auto device : category.nodes) {
                if (ImGui::MenuItem(device->getNodeName())) output = device->getNewNodeInstance();
            }
            ImGui::EndMenu();
        }
    }

    return output;
}

