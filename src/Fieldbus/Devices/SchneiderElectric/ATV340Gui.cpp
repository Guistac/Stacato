#include <pch.h>
#include "ATV340.h"

#include <imgui.h>

void ATV340::deviceSpecificGui(){
	
	if(ImGui::BeginTabItem("ATV340")){
		ImGui::Text("Hello ATV340");
		
		if(ImGui::Button("Configure Motor")) configureMotor();
		
		ImGui::EndTabItem();
	}
}
