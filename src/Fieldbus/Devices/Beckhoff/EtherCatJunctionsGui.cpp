#include <pch.h>

#include "EtherCatJunctions.h"

#include <imgui.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

void CU1124::deviceSpecificGui() {
	if(ImGui::BeginTabItem("CU1124")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}

void CU1128::deviceSpecificGui() {
	if(ImGui::BeginTabItem("CU1128")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}

void EK1100::deviceSpecificGui() {
	if(ImGui::BeginTabItem("CU1128")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}

void EL2008::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL2008")){
		
		for(int i = 0; i < 8; i++){
			ImGui::PushID(i);
			
			char name[32];
			snprintf(name, 32, "Channel %i", i + 1);
			ImGui::Checkbox(name, &outputs[i]);
			
			ImGui::PopID();
		}
		
		ImGui::EndTabItem();
	}
}

void EL5001::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL5001")){
	
		ssiFrameSize->gui(Fonts::sansBold15);
		multiturnResolution->gui(Fonts::sansBold15);
		singleturnResolution->gui(Fonts::sansBold15);
		ssiCoding_parameter->gui(Fonts::sansBold15);
		ssiBaudrate_parameter->gui(Fonts::sansBold15);
		ImGui::Text("Frame Format: %s", frameFormatString.c_str());
		
		ImGui::Separator();
		
		ImGui::Text("Counter: %i", ssiValue);
		ImGui::Text("Data Error : %i", b_dataError);
		ImGui::Text("Frame Error : %i", b_frameError);
		ImGui::Text("Power Failure : %i", b_powerFailure);
		ImGui::Text("Data Mismatch : %i", b_dataMismatch);
		ImGui::Text("Sync Error : %i", b_syncError);
		ImGui::Text("TxPDO State : %i", b_txPdoState);
		ImGui::Text("TxPDO Toggle : %i", b_txPdoToggle);
		
		ImGui::EndTabItem();
	}
}
