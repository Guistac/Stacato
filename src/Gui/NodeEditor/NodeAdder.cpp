#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "NodeGraph/Utilities/NodeFactory.h"

void nodeAdder() {

    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetTextLineHeight() * 1.0);

    if(ImGui::BeginChild("##NodeList", ImGui::GetContentRegionAvail())) {

        ImGui::PushFont(Fonts::robotoBold20);
        ImGui::Text("Environnement Editor");
        ImGui::PopFont();
        ImGui::Separator();



        ImGui::PushFont(Fonts::robotoBold15);
        if (ImGui::CollapsingHeader("EtherCAT Devices")) {

            ImGui::PushFont(Fonts::robotoRegular15);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));
            ImGui::Text("By Manufacturer");
            ImGui::PopStyleColor();
            for (auto& manufacturer : EtherCatDeviceFactory::getDevicesByManufacturer()) {
                if (ImGui::TreeNode(manufacturer.name)) {
                    for (auto& device : manufacturer.devices) {
                        const char* deviceDisplayName = device->getName();
                        ImGui::Selectable(deviceDisplayName);
                        const char* deviceSaveName = device->getSaveName();
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                            ImGui::SetDragDropPayload("EtherCatDevice", &deviceSaveName, sizeof(const char*));
                            ImGui::Text(deviceDisplayName);
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
                    for (auto& device : manufacturer.devices) {
                        const char* deviceDisplayName = device->getName();
                        ImGui::Selectable(deviceDisplayName);
                        const char* deviceSaveName = device->getSaveName();
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                            ImGui::SetDragDropPayload("EtherCatDevice", &deviceSaveName, sizeof(const char*));
                            ImGui::Text(deviceDisplayName);
                            ImGui::EndDragDropSource();
                        }
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::Separator();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));
            ImGui::Text("Detected Devices");
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
            for (auto device : EtherCatFieldbus::slaves_unassigned) {
                const char* deviceDisplayName = device->getName();
                ImGui::Selectable(deviceDisplayName);
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    ImGui::SetDragDropPayload("DetectedEtherCatDevice", &device, sizeof(std::shared_ptr<EtherCatDevice>));
                    ImGui::Text(deviceDisplayName);
                    ImGui::EndDragDropSource();
                }
            }
            ImGui::PopFont();
        }
        ImGui::PopFont();

        ImGui::PushFont(Fonts::robotoBold15);
        if (ImGui::CollapsingHeader("Motion")) {
            if(ImGui::TreeNode("Axis")){
                ImGui::PushFont(Fonts::robotoRegular15);
                for (auto axis : NodeFactory::getAllAxisTypes()) {
                    const char* axisDisplayName = axis->getName();
                    ImGui::Selectable(axisDisplayName);
                    const char* axisSaveName = axis->getSaveName();
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                        ImGui::SetDragDropPayload("Axis", &axisSaveName, sizeof(const char*));
                        ImGui::Text(axisDisplayName);
                        ImGui::EndDragDropSource();
                    }
                }
                ImGui::TreePop();
                ImGui::PopFont();
            }



            if (ImGui::TreeNode("Machines")) {
                for (auto& category : NodeFactory::getMachinesByCategory()) {
                    if (ImGui::TreeNode(category.name)) {
                        ImGui::PushFont(Fonts::robotoRegular15);
                        for (auto& machine : category.nodes) {
                            const char* machineDisplayName = machine->getName();
                            ImGui::Selectable(machineDisplayName);
                            const char* machineSaveName = machine->getSaveName();
                            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                                ImGui::SetDragDropPayload("Machine", &machineSaveName, sizeof(const char*));
                                ImGui::Text(machineDisplayName);
                                ImGui::EndDragDropSource();
                            }
                        }
                        ImGui::PopFont();
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
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
                        const char* nodeDisplayName = node->getName();
                        const char* nodeSaveName = node->getSaveName();
                        ImGui::Selectable(nodeDisplayName);
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                            ImGui::SetDragDropPayload("ProcessorNode", &nodeSaveName, sizeof(const char*));
                            ImGui::Text(nodeDisplayName);
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
            const char* deviceName = *(const char**)payload->Data;
            std::shared_ptr<Node> newSlave = EtherCatDeviceFactory::getDeviceBySaveName(deviceName);
            return newSlave;
        }
        payload = ImGui::AcceptDragDropPayload("DetectedEtherCatDevice");
        if (payload != nullptr && payload->DataSize == sizeof(std::shared_ptr<EtherCatDevice>)) {
            std::shared_ptr<EtherCatDevice> detectedSlave = *(std::shared_ptr<EtherCatDevice>*)payload->Data;
            EtherCatFieldbus::removeFromUnassignedSlaves(detectedSlave);
            return detectedSlave;
        }
        payload = ImGui::AcceptDragDropPayload("Axis");
        if (payload != nullptr && payload->DataSize == sizeof(const char*)) {
            const char* axisSaveName = *(const char**)payload->Data;
            std::shared_ptr<Node> newAxis = NodeFactory::getAxisBySaveName(axisSaveName);
            return newAxis;
        }
        payload = ImGui::AcceptDragDropPayload("Machine");
        if (payload != nullptr && payload->DataSize == sizeof(const char*)) {
            const char* machineSaveName = *(const char**)payload->Data;
            std::shared_ptr<Node> newMachine = NodeFactory::getMachineBySaveName(machineSaveName);
            return newMachine;
        }
        payload = ImGui::AcceptDragDropPayload("ProcessorNode");
        if (payload != nullptr && payload->DataSize == sizeof(const char*)) {
            const char* nodeSaveName = *(const char**)payload->Data;
            std::shared_ptr<Node> newNode = NodeFactory::getNodeBySaveName(nodeSaveName);
            return newNode;
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
                    if (ImGui::MenuItem(device->getName())) output = device->getNewDeviceInstance();
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
                    if (ImGui::MenuItem(device->getName())) output = device->getNewDeviceInstance();
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
                    EtherCatFieldbus::removeFromUnassignedSlaves(selectedDetectedSlave);
                }
            }
            ImGui::EndMenu();
        }
    }

    ImGui::Separator();

    if (ImGui::BeginMenu("Motion")) {
        if (ImGui::BeginMenu("Axis")) {
            for (auto axis : NodeFactory::getAllAxisTypes()) {
                if (ImGui::MenuItem(axis->getName())) output = axis->getNewNodeInstance();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Machines")) {
            for (auto& category : NodeFactory::getMachinesByCategory()) {
                if (ImGui::BeginMenu(category.name)) {
                
                    for (auto& machine : category.nodes) {
                        
                        if (ImGui::MenuItem(machine->getName())) output = machine->getNewNodeInstance();
                    
                    }

                    ImGui::EndMenu();
                }
            }
            ImGui::EndMenu();
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
                if (ImGui::MenuItem(device->getName())) output = device->getNewNodeInstance();
            }
            ImGui::EndMenu();
        }
    }

    return output;
}

