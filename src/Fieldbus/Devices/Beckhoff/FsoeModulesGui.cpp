#include "FsoeModules.h"

#include <imgui.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

void EL2912::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL2912")){
		ImGui::Text("This device has no settings or controls");
		ImGui::InputScalar("FSOE Address", ImGuiDataType_U16, &fsoeAddress);
		if(ImGui::Button("show safety parameters")) downladSafetyParameters();
		ImGui::Text("Safe Output 1: %i (ErrAck: %i)", processData.safeOutput1, processData.safeOutput1ErrAck);
		ImGui::Text("Safe Output 2: %i (ErrAck: %i)", processData.safeOutput2, processData.safeOutput2ErrAck);
		ImGui::Text("Fault1: %i  Fault2: %i", processData.safeOutput1Fault, processData.safeOutput2Fault);
		
		
		if(isStateOperational()){
			ImGui::BeginGroup();
			ImGui::Text("Outputs");
			for(int i = 0; i < identity->Obytes; i++){
				ImGui::Text("[%i]: %x", i, identity->outputs[i]);
			}
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();
			ImGui::Text("Inputs");
			for(int i = 0; i < identity->Ibytes; i++){
				ImGui::Text("[%i]: %x", i, identity->inputs[i]);
			}
			ImGui::EndGroup();
		}
		
		ImGui::EndTabItem();
	}
}

void EL1904::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL1904")){
		ImGui::Text("This device has no settings or controls");
		ImGui::InputScalar("FSOE Address", ImGuiDataType_U16, &fsoeAddress);
		if(ImGui::Button("show safety parameters")) downladSafetyParameters();
		
		for(int i = 0; i < 4; i++){
			pinInversionParams[i]->gui(Fonts::sansBold15);
		}
		
		if(isStateOperational()){
			
			ImGui::BeginGroup();
			ImGui::Text("Outputs");
			for(int i = 0; i < identity->Obytes; i++){
				ImGui::Text("[%i]: %x", i, identity->outputs[i]);
			}
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();
			ImGui::Text("Inputs");
			for(int i = 0; i < identity->Ibytes; i++){
				ImGui::Text("[%i]: %x", i, identity->inputs[i]);
			}
			ImGui::EndGroup();
		}
		
		ImGui::EndTabItem();
	}
}
