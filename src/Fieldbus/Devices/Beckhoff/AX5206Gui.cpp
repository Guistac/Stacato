#include "AX5206.h"

void AX5206::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL7222-0010")){
		if(ImGui::BeginTabBar("Drive")){
			if(ImGui::BeginTabItem("Drive")){
				
				if(ImGui::Button("Read Invalid IDNs for PreOp->SafeOp")) getInvalidIDNsForSafeOp();
				if(ImGui::Button("Read Shutdown Errors")) getShutdownErrorList();
				if(ImGui::Button("Read Error History")) getErrorHistory();
				if(ImGui::Button("Read Diagnostics Message")) getDiagnosticsMessage();
				
				auto axisControlGui = [&](std::shared_ptr<Axis> servo, uint8_t axisNumber){
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
					ImGui::Text("Effort: %.1f%%", servo->actuatorProcessData.effortActual * 100.0);
					//ImGui::Text("Error: %i", processData.ax0_driveStatus.shutdownError);
					ImGui::PopID();
				};
				
				axisControlGui(axis0, 0);
				axisControlGui(axis1, 1);
				
				ImGui::EndTabItem();
				
			}
			if(ImGui::BeginTabItem("General")){
				invertSTO_param->gui(Fonts::sansBold15);
				invertDigitalIn0_param->gui(Fonts::sansBold15);
				invertDigitalIn1_param->gui(Fonts::sansBold15);
				invertDigitalIn2_param->gui(Fonts::sansBold15);
				invertDigitalIn3_param->gui(Fonts::sansBold15);
				invertDigitalIn4_param->gui(Fonts::sansBold15);
				invertDigitalIn5_param->gui(Fonts::sansBold15);
				invertDigitalIn6_param->gui(Fonts::sansBold15);
				invertDigitalOut7_param->gui(Fonts::sansBold15);
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Axis 0")){
				axis0->settingsGui();
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Axis 1")){
				axis1->settingsGui();
				ImGui::EndTabItem();
			}
			
			ImGui::EndTabBar();
		}
		ImGui::EndTabItem();
	}
}

void AX5206::Axis::settingsGui(){
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Axis %i", channel);
	ImGui::PopFont();
	motorType->gui(Fonts::sansBold15);
	velocityLimit_revps->gui(Fonts::sansBold15);
	accelerationLimit_revps2->gui(Fonts::sansBold15);
	positionFollowingErrorLimit_rev->gui(Fonts::sansBold15);
	currentLimit_amps->gui(Fonts::sansBold15);
	invertDirection_param->gui(Fonts::sansBold15);
}
