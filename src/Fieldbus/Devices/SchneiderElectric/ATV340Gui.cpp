#include <pch.h>
#include "ATV340.h"

#include <imgui.h>
#include "Gui/Assets/Fonts.h"

void ATV340::deviceSpecificGui(){
	
	if(ImGui::BeginTabItem("ATV340")){
		ImGui::Text("Hello ATV340");
		
		ImGui::Text("Current Error: %s", getErrorCodeString());
		
		ImGui::Checkbox("DO1", &digitalOut1);
		ImGui::Checkbox("DO2", &digitalOut2);
		ImGui::Checkbox("R1", &relayOut1);
		ImGui::Checkbox("R2", &relayOut2);
		
		
		
		ImGui::Separator();
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Process Data Configuration");
		ImGui::PopFont();
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::Text("4 options or less can be selected.");
		ImGui::PopStyleColor();
		
		for(auto parameter : pdoConfig.get()){
			parameter->gui();
			ImGui::SameLine();
			
			bool isEnabled = std::static_pointer_cast<BooleanParameter>(parameter)->value;
			if(isEnabled) ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("%s", parameter->getName());
			if(isEnabled) ImGui::PopFont();
		}
		
		
		
		
		ImGui::EndTabItem();
	}
}
