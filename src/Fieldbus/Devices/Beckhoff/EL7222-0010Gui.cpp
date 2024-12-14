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
				
				ImGui::PushFont(Fonts::sansBold20);
				ImGui::Text("Utilities");
				ImGui::PopFont();
				
				if(ImGui::Button("Detect Motors and Configure Drives")) readMotorNameplatesAndConfigureDrive();				
				if(ImGui::Button("Download Diagnostics")) downloadCompleteDiagnostics();
				
				ImGui::Separator();
				
				ImGui::PushFont(Fonts::sansBold20);
				ImGui::Text("Digital Input Inversion");
				ImGui::PopFont();
				
				for(auto parameter : pinInversionParameters) parameter->gui(parameter->value ? Fonts::sansBold15 : Fonts::sansRegular15);
				
				ImGui::EndTabItem();
			}
				
			ImGui::EndTabBar();
		}
		ImGui::EndTabItem();
	}
}

void ELM7231_9016::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL7222-0010")){
		
		if(ImGui::Button("Detect Motor and Configure Drive")) readMotorNameplatesAndConfigureDrive();
		if(ImGui::Button("Download Diagnostics")) downloadCompleteDiagnostics();
		
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
		
		ImGui::EndTabItem();
	}
}
