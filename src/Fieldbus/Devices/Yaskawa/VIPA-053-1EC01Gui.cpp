#include <pch.h>

#include "VIPA-053-1EC01.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>
#include "Gui/Framework/Colors.h"
#include "Gui/Framework/Fonts.h"



void VIPA_053_1EC01::deviceSpecificGui() {
    if (ImGui::BeginTabItem("VIPA")) {
        if (ImGui::Button("Detect I/O Modules")) {
            std::thread ioModuleDetectionHandler([this]() { detectIoModules(); });
            ioModuleDetectionHandler.detach();
        }
        for (int i = 0; i < detectedModules.size(); i++) {
            VIPAModule& module = detectedModules[i];
            ImGui::PushID(i);
            if (ImGui::TreeNode(module.name)) {
                for (int i = 0; i < module.inputParameters.size(); i++) {
                    VIPAparameter& parameter = module.inputParameters[i];
                    ImGui::Text("Input %i: Index: 0x%X  Subindex: 0x%X  BitSize: %i", i, parameter.mappingModule, parameter.subindex, parameter.bitCount);
                }
                for (int i = 0; i < module.outputParameters.size(); i++) {
                    VIPAparameter& parameter = module.outputParameters[i];
                    ImGui::Text("Output %i: Index: 0x%X  Subindex: 0x%X  BitSize: %i", i, parameter.mappingModule, parameter.subindex, parameter.bitCount);
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::EndTabItem();
    }
}