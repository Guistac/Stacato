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
            std::thread ioModuleDetectionHandler([this]() { configureFromDeviceModules(); });
            ioModuleDetectionHandler.detach();
        }
        ImGui::SameLine();
        ImGui::Text(getDataTransferState(configureFromDeviceModulesDownloadStatus)->displayName);
        
        for (int i = 0; i < modules.size(); i++) {
            Module& module = modules[i];
            ImGui::PushID(i);
            if (ImGui::TreeNode(getModuleType(module.moduleType)->displayName)) {
                //ImGui::Text(module.name);
                //ImGui::Text("Inputs: ByteCount: %i  BitCount: %i", module.inputByteCount, module.inputBitCount);
                //ImGui::Text("Outputs: ByteCount: %i  BitCount: %i", module.outputByteCount, module.outputBitCount);
                for (int j = 0; j < module.inputs.size(); j++) {
                    ModuleParameter& parameter = module.inputs[j];
                    ImGui::Text("Input %i: InputByte: %i  InputBit: %i  BitSize: %i", i, parameter.ioMapByteOffset, parameter.ioMapBitOffset, parameter.bitSize);
                    std::shared_ptr<NodePin> NodePin = parameter.nodePin;
                    //ImGui::Text("%s (%s)", NodePin->getName(), NodePin->getTypeName());
                }
                for (int j = 0; j < module.outputs.size(); j++) {
                    ModuleParameter& parameter = module.outputs[j];
                    ImGui::Text("Output %i: OutputByte: %i  OutputBit: %i  BitSize: %i", i, parameter.ioMapByteOffset, parameter.ioMapBitOffset, parameter.bitSize);
                    std::shared_ptr<NodePin> NodePin = parameter.nodePin;
                    //ImGui::Text("%s (%s)", NodePin->getName(), NodePin->getTypeName());
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::EndTabItem();
    }
}