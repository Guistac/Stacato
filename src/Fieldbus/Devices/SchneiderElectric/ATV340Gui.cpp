#include <pch.h>
#include "ATV340.h"

#include <imgui.h>

void ATV340::deviceSpecificGui(){
	
	if(ImGui::BeginTabItem("ATV340")){
		ImGui::Text("Hello ATV340");
		
		
		ImGui::Text("Current Error: %s", getErrorCodeString());
		
		ImGui::Checkbox("DO1", &digitalOut1);
		ImGui::Checkbox("DO2", &digitalOut2);
		ImGui::Checkbox("R1", &relayOut1);
		ImGui::Checkbox("R2", &relayOut2);
		
		ImGui::EndTabItem();
	}
}
