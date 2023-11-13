#include "EL7221-9014-MotionInterface.h"

void EL7221_9014::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL7221-9014")){
		if(ImGui::BeginTabBar("Drive")){
			if(ImGui::BeginTabItem("Control")){
				controlTab();
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Settings")){
				settingsTab();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::EndTabItem();
	}
}

void EL7221_9014::controlTab(){
		
	auto tableRowBool = [](std::string fieldName, bool data){
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", fieldName.c_str());
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%s", data ? "Yes" : "No");
	};
	
	auto tableRowInt = [](std::string fieldName, int data, std::string suffix){
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", fieldName.c_str());
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%i %s", data, suffix.c_str());
	};
	
	auto tableRowDouble = [](std::string fieldName, double data, std::string suffix){
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", fieldName.c_str());
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.3f %s", data, suffix.c_str());
	};
	
	auto tableRowString = [](std::string fieldName, std::string& data){
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", fieldName.c_str());
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%s", data.c_str());
	};
	
	if(ImGui::CollapsingHeader("Motor Nameplate")){
		if(ImGui::BeginTable("##MotorNameplate", 2, ImGuiTableFlags_Borders)){
			tableRowBool("Motor Identified", motorNameplate.b_motorIdentified);
			tableRowString("Type", motorNameplate.motorType);
			tableRowString("Serial Number", motorNameplate.serialNumber);
			tableRowDouble("Rated Current", motorNameplate.ratedCurrent_amps, "A");
			tableRowDouble("Max Current", motorNameplate.maxCurrent_amps, "A");
			tableRowDouble("Max Velocity", motorNameplate.maxVelocity_rps, "rev/s");
			tableRowDouble("Working Range", motorNameplate.workingRange_rev, "rev");
			tableRowInt("Velocity Encoder Resolution", motorNameplate.velocityResolution_rps, "inc");
			tableRowInt("Position Encoder Resolution", motorNameplate.positionResolution_rev, "inc");
			tableRowDouble("Torque Constant", motorNameplate.torqueConstant_mNmpA, "mNm/A");
			tableRowBool("Has Brake", motorNameplate.b_hasBrake);
			ImGui::EndTable();
		};
	}
	
	if(ImGui::CollapsingHeader("Status Word")){
		if(ImGui::BeginTable("##StatusWord", 2, ImGuiTableFlags_Borders)){
			tableRowBool("Ready To Switch On", statusWord.readyToSwitchOn);
			tableRowBool("Switched On", statusWord.switchedOn);
			tableRowBool("Operation Enabled", statusWord.operationEnabled);
			tableRowBool("Fault", statusWord.fault);
			tableRowBool("Quickstop", statusWord.quickstop);
			tableRowBool("Switch On Disabled", statusWord.switchOnDisabled);
			tableRowBool("Warning", statusWord.warning);
			tableRowBool("TxPdo Toggle", statusWord.TxPdoToggle);
			tableRowBool("Internal Limit Active", statusWord.internalLimitActive);
			tableRowBool("Command Value Followed", statusWord.commandValueFollowed);
			ImGui::EndTable();
		}
	}
	
	if(ImGui::CollapsingHeader("Control Word")){
		if(ImGui::BeginTable("##ControlWord", 2, ImGuiTableFlags_Borders)){
			tableRowBool("Switch On", controlWord.switchOn);
			tableRowBool("Enable Voltage", controlWord.enableVoltage);
			tableRowBool("Quickstop", controlWord.quickstop);
			tableRowBool("Enable Operation", controlWord.enableOperation);
			tableRowBool("Fault Reset", controlWord.faultReset);
			ImGui::EndTable();
		}
	}
	
	if(ImGui::CollapsingHeader("Errors")){
		if(ImGui::BeginTable("##Errors", 2, ImGuiTableFlags_Borders)){
			tableRowBool("Motor Connected", processData.b_motorConnected);
			tableRowBool("STO Active", actuator->actuatorProcessData.b_isEmergencyStopActive);
			tableRowBool("ADC Error", driverErrors.adc);
			tableRowBool("Overcurrent Error", driverErrors.overcurrent);
			tableRowBool("Undervoltage Error", driverErrors.undervoltage);
			tableRowBool("Overvoltage Error", driverErrors.overvoltage);
			tableRowBool("Overtemperature Error", driverErrors.overtemperature);
			tableRowBool("I2tAmplifier Error", driverErrors.i2tAmplifier);
			tableRowBool("I2tMotor Error", driverErrors.i2tMotor);
			tableRowBool("Encoder Error", driverErrors.encoder);
			tableRowBool("Watchdog Error", driverErrors.watchdog);
			ImGui::EndTable();
		}
	}

	if(ImGui::CollapsingHeader("Warnings")){
		if(ImGui::BeginTable("##Warnings", 2, ImGuiTableFlags_Borders)){
			tableRowBool("Undervoltage Warning", driverWarnings.undervoltage);
			tableRowBool("Overvoltage Warning", driverWarnings.overvoltage);
			tableRowBool("Overtemperature Warning", driverWarnings.overtemperature);
			tableRowBool("I2tAmplifier Warning", driverWarnings.i2tAmplifier);
			tableRowBool("I2tMotor Warning", driverWarnings.i2tMotor);
			tableRowBool("Encoder Warning", driverWarnings.encoder);
			ImGui::EndTable();
		}
	}

	
	ImGui::Separator();
	
	if(actuator->isEnabled()){
		if(ImGui::Button("Disable")) actuator->disable();
	}
	else {
		ImGui::BeginDisabled(!actuator->isReady());
		if(ImGui::Button("Enable")) actuator->enable();
		ImGui::EndDisabled();
	}
	
	if(ImGui::SliderFloat("##TargetVelocity", &velocitySliderValue, -actuator->getVelocityLimit(), actuator->getVelocityLimit())){
		actuator->setVelocityTarget(velocitySliderValue);
	}
	if(ImGui::IsItemDeactivatedAfterEdit()) {
		actuator->setVelocityTarget(0.0);
		velocitySliderValue = 0.0;
	}
	
	float velocityNormalized = actuator->getVelocityNormalized();
	float positionNormalized = actuator->getPositionNormalizedToWorkingRange();
	
	ImVec2 progressSize(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
	char progressString[64];
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, velocityNormalized < 0.0 ? Colors::red : Colors::green);
	snprintf(progressString, 64, "%.1frev/s", actuator->getVelocity());
	ImGui::ProgressBar(std::abs(velocityNormalized), progressSize, progressString);
	ImGui::PopStyleColor();
	snprintf(progressString, 64, "%.3frev", actuator->getPosition());
	ImGui::ProgressBar(positionNormalized, progressSize, progressString);
	snprintf(progressString, 64, "%.2fNm", actuator->getForce());
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::orange);
	ImGui::ProgressBar(actuator->getEffort(), progressSize, progressString);
	ImGui::PopStyleColor();
	
	
	if(ImGui::Button("First Setup")) firstSetup();
	ImGui::SameLine();
	if(ImGui::Button("Download Diagnostics")) downloadDiagnostics();
	
	if(ImGui::Button("Reset Encoder Position")) resetEncoderPosition();
	
	ImGui::Text("Status: %s", actuator->getStatusString().c_str());
	ImGui::Text("Position: %.3frev", actuator->getPosition());
	ImGui::Text("Velocity: %.2frev/s", actuator->getVelocity());
	ImGui::Text("Position Following Error: %.3frev", actuator->getFollowingError());
	ImGui::Text("Effort: %.2f%%", actuator->getEffort());
	ImGui::Text("Torque: %.2fNm", actuator->getForce());
	
}

void EL7221_9014::settingsTab(){
		
	float frameHeight = ImGui::GetFrameHeight();
	ImGui::PushFont(Fonts::sansBold15);
	ImVec2 offset(ImGui::GetStyle().CellPadding.y, (frameHeight - ImGui::GetTextLineHeight()) / 2.0);
	ImGui::PopFont();
	
	auto parameterTableRow = [&](std::shared_ptr<Parameter> param){
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImVec2 cursorPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(cursorPos.x + offset.x, cursorPos.y + offset.y));
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("%s", param->getName());
		ImGui::PopFont();
		ImGui::TableSetColumnIndex(1);
		param->gui();
	};
	
	if(ImGui::BeginTable("##driveSettings", 2, ImGuiTableFlags_RowBg)){
		parameterTableRow(driveSettings.velocityLimit);
		parameterTableRow(driveSettings.accelerationLimit);
		parameterTableRow(driveSettings.currentLimit);
		parameterTableRow(driveSettings.invertDirection);
		parameterTableRow(driveSettings.positionFollowingErrorWindow);
		parameterTableRow(driveSettings.positionFollowingErrorTimeout);
		parameterTableRow(driveSettings.faultReaction);
		parameterTableRow(driveSettings.haltRampDeceleration);
		ImGui::EndTable();
	}
	
	if(ImGui::Button("Upload Parameters")) uploadParameters();
	
}


//TODO:
//-CSP Mode
