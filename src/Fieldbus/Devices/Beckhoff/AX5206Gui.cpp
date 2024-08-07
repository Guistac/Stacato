#include "AX5206.h"

void AX5206::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL7222-0010")){
		if(ImGui::BeginTabBar("Drive")){
			if(ImGui::BeginTabItem("Drive")){
				
				if(ImGui::Button("Read Invalid IDNs for PreOp->SafeOp")) getInvalidIDNsForSafeOp();
				if(ImGui::Button("Read Shutdown Errors")) getShutdownErrorList();
				if(ImGui::Button("Read Error History")) getErrorHistory();
				if(ImGui::Button("Read Diagnostics Message")) getDiagnosticsMessage();
				
				ImGui::Text("STO: %s", processData.b_stoActive ? "Safe" : "Clear");
				ImGui::Text("DI0: %i   DI1: %i   DI2: %i   DI3: %i   DI4: %i   DI5: %i   DI6: %i",
							processData.digitalInput0,
							processData.digitalInput1,
							processData.digitalInput2,
							processData.digitalInput3,
							processData.digitalInput4,
							processData.digitalInput5,
							processData.digitalInput6);
				
				double velMin = -90;
				double velMax = 90.0;
				
				
				auto axisControlGui = [&](std::shared_ptr<AX5206_Servo> servo, uint8_t axisNumber){
					ImGui::PushID(axisNumber);
					ImGui::PushFont(Fonts::sansBold20);
					ImGui::Text("Axis %i", axisNumber);
					ImGui::PopFont();
					if(servo->isEnabled()) {
						if(ImGui::Button("Disable")) servo->disable();
					}
					else {
						ImGui::BeginDisabled(!servo->isReady());
						if(ImGui::Button("Enable")) servo->enable();
						ImGui::EndDisabled();
					}
					ImGui::SameLine();
					ImGui::Text("%s", servo->getStatusString().c_str());
					if(ImGui::SliderFloat("Velocity", &servo->guiVelocitySliderValue, velMin, velMax)) servo->setVelocityTarget(servo->guiVelocitySliderValue);
					if(ImGui::IsItemDeactivatedAfterEdit()) {
						servo->setVelocityTarget(0.0);
						servo->guiVelocitySliderValue = 0.0;
					}
					ImGui::Text("Velocity: %.3frev/s", servo->feedbackProcessData.velocityActual);
					ImGui::Text("Position: %.3frev", servo->feedbackProcessData.positionActual);
					ImGui::Text("Effort: %.1f%%", servo->actuatorProcessData.effortActual);
					//ImGui::Text("Error: %i", processData.ax0_driveStatus.shutdownError);
					ImGui::PopID();
				};
				
				axisControlGui(servo0, 0);
				axisControlGui(servo1, 1);
				
				ImGui::EndTabItem();
				
			}
			ImGui::EndTabBar();
		}
		ImGui::EndTabItem();
	}
}
