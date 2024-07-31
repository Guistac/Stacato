#include "AX5206.h"

void AX5206::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL7222-0010")){
		if(ImGui::BeginTabBar("Drive")){
			if(ImGui::BeginTabItem("Drive")){
				
				if(ImGui::Button("Test")){
					
					struct IDN_List{
						uint16_t actualLength = 0; //number of bytes in the idns array
						uint16_t maxLength = 0;
						uint16_t idns[10000] = {0};
					};
					
					IDN_List test;
					int size = 20000;
					bool ret = readSercos_Array(25, (uint8_t*)&test, size);
					
					for(int i = 0; i < test.actualLength; i++){
						if(test.idns[i] == 0) continue;
						if(test.idns[i] > 32768) Logger::error("P-0-{}", test.idns[i] - 32768);
						else Logger::error("S-0-{}", test.idns[i]);
					}
					
					Logger::warn("{}", test.actualLength);
				}
				
				if(ImGui::Button("Scan Feedback Axis 0")){
					if(writeSercos_U16(163, 3, 0, true)){
						Logger::info("Scanning feedback on axis 0");
					}else Logger::warn("Failed to scan feedback");
				}
				if(ImGui::Button("Scan Feedback Axis 1")){
					if(writeSercos_U16(163, 3, 1, true)){
						Logger::info("Scanning feedback on axis 1");
					}else Logger::warn("Failed to scan feedback");
				}
				
				if(ImGui::Button("Show Feedback Type")){
					uint16_t data0[2048] = {0};
					uint16_t data1[2048] = {0};
					int size0 = 2048;
					int size1 = 2048;
					bool ret0 = readSercos_Array(151, (uint8_t*)data0, size0, 0, true);
					bool ret1 = readSercos_Array(151, (uint8_t*)data1, size1, 1, true);
					
					std::string mot0;
					std::string mot1;
					readSercos_String(53, mot0, 0, true);
					readSercos_String(53, mot1, 1, true);
					
					Logger::warn("Feedback scan {} {}", ret0, ret1);
				}
				
				
				ImGui::PushID("Axis0");
				ImGui::PushFont(Fonts::sansBold20);
				ImGui::Text("Axis 0");
				ImGui::PopFont();
				ImGui::Checkbox("realtimeBit1", &ax0_driveControl.realtimeBit1);
				ImGui::Checkbox("realtimeBit2", &ax0_driveControl.realtimeBit2);
				ImGui::Checkbox("syncBit", &ax0_driveControl.syncBit);
				ImGui::Checkbox("haltRestart", &ax0_driveControl.haltRestart);
				ImGui::Checkbox("enable", &ax0_driveControl.enable);
				ImGui::Checkbox("driveOnOff", &ax0_driveControl.driveOnOff);
				ImGui::InputScalar("operatingMode", ImGuiDataType_U8, &ax0_driveControl.operatingMode);
				ImGui::Text("driveStatusWord: %X", acknowledgeTelegram.ax0_driveStatusWord);
				ImGui::TreePush("status0");
				ImGui::Text("followsCommand: %i", ax0_driveStatus.followsCommand);
				ImGui::Text("realtimeBit1: %i", ax0_driveStatus.realtimeBit1);
				ImGui::Text("realtimeBit2: %i", ax0_driveStatus.realtimeBit2);
				ImGui::Text("operatingMode: %i", ax0_driveStatus.operatingMode);
				ImGui::Text("infoChange: %i", ax0_driveStatus.infoChange);
				ImGui::Text("warningChange: %i", ax0_driveStatus.warningChange);
				ImGui::Text("shutdownError: %i", ax0_driveStatus.shutdownError);
				ImGui::Text("status: %i", ax0_driveStatus.status);
				ImGui::TreePop();
				ImGui::Text("positionFeedbackValue1: %X",acknowledgeTelegram.ax0_positionFeedbackValue1);
				ImGui::Text("velocityFeedbackValue: %X",acknowledgeTelegram.ax0_velocityFeedbackValue);
				ImGui::Text("torqueFeedbackValue: %X",acknowledgeTelegram.ax0_torqueFeedbackValue);
				ImGui::Text("followingDistance: %X",acknowledgeTelegram.ax0_followingDistance);
				ImGui::PopID();
				
				
				
				ImGui::PushID("Axis1");
				ImGui::PushFont(Fonts::sansBold20);
				ImGui::Text("Axis 1");
				ImGui::PopFont();
				ImGui::Checkbox("realtimeBit1", &ax1_driveControl.realtimeBit1);
				ImGui::Checkbox("realtimeBit2", &ax1_driveControl.realtimeBit2);
				ImGui::Checkbox("syncBit", &ax1_driveControl.syncBit);
				ImGui::Checkbox("haltRestart", &ax1_driveControl.haltRestart);
				ImGui::Checkbox("enable", &ax1_driveControl.enable);
				ImGui::Checkbox("driveOnOff", &ax1_driveControl.driveOnOff);
				ImGui::InputScalar("operatingMode", ImGuiDataType_U8, &ax0_driveControl.operatingMode);
				ImGui::Text("driveStatusWord: %X", acknowledgeTelegram.ax1_driveStatusWord);
				ImGui::TreePush("status1");
				ImGui::Text("followsCommand: %i", ax1_driveStatus.followsCommand);
				ImGui::Text("realtimeBit1: %i", ax1_driveStatus.realtimeBit1);
				ImGui::Text("realtimeBit2: %i", ax1_driveStatus.realtimeBit2);
				ImGui::Text("operatingMode: %i", ax1_driveStatus.operatingMode);
				ImGui::Text("infoChange: %i", ax1_driveStatus.infoChange);
				ImGui::Text("warningChange: %i", ax1_driveStatus.warningChange);
				ImGui::Text("shutdownError: %i", ax1_driveStatus.shutdownError);
				ImGui::Text("status: %i", ax1_driveStatus.status);
				ImGui::TreePop();
				ImGui::Text("positionFeedbackValue1: %X",acknowledgeTelegram.ax1_positionFeedbackValue1);
				ImGui::Text("velocityFeedbackValue: %X",acknowledgeTelegram.ax1_velocityFeedbackValue);
				ImGui::Text("torqueFeedbackValue: %X",acknowledgeTelegram.ax1_torqueFeedbackValue);
				ImGui::Text("followingDistance: %X",acknowledgeTelegram.ax1_followingDistance);
				ImGui::PopID();
				
				ImGui::NewLine();
				ImGui::Text("Digital Inputs: %X", acknowledgeTelegram.digitalInputsState);
				
				
				ImGui::EndTabItem();
				
			}
			ImGui::EndTabBar();
		}
		ImGui::EndTabItem();
	}
}
