#include <pch.h>

#include "Gui/Gui.h"
#include "Fieldbus/EtherCatSlave.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "Environnement/NodeGraph/Utilities/ioNodeFactory.h"

std::shared_ptr<ioNode> nodeAdderContextMenu() {

    std::shared_ptr<ioNode> output = nullptr;

    ImGui::MenuItem("Node Editor Menu", nullptr, false, false);
    ImGui::Separator();
    if (ImGui::BeginMenu("EtherCAT Devices")){
        if (ImGui::BeginMenu("Schneider Electric")) {
            if (ImGui::MenuItem("Lexium32")) output = EtherCatDeviceFactory::getDeviceByName("LXM32M EtherCAT");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("EasyCAT")) {
            if (ImGui::MenuItem("EasyCAT+Uno")) output = EtherCatDeviceFactory::getDeviceByName("Artefact");
            if (ImGui::MenuItem("LedsAndButtons")) output = EtherCatDeviceFactory::getDeviceByName("LedsAndButtons");
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Axis")) {
        
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Network")) {
        
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Processing")) {

        ImGui::EndMenu();
    }
    return output;
}