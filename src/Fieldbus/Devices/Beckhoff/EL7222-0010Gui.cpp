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
