#include "EL7222-0010.h"

void EL7222_0010::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL7222-0010")){
		if(ImGui::BeginTabBar("Drive")){
				
			if(ImGui::BeginTabItem("Actuator 1")){
				actuator1->gui();
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Actuator 2")){
				actuator2->gui();
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Drive")){
				if(ImGui::Button("Download Diagnostics")) downloadDiagnostics();
				ImGui::EndTabItem();
			}
				
				
			ImGui::EndTabBar();
		}
		ImGui::EndTabItem();
	}
}
