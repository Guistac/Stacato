#include <pch.h>

#include "EtherCatJunctions.h"

#include <imgui.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

void CU1124::deviceSpecificGui() {
	if(ImGui::BeginTabItem("CU1124")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}

void CU1128::deviceSpecificGui() {
	if(ImGui::BeginTabItem("CU1128")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}

void EK1100::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EK1100")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}

void EK1310::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EK1310")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}

void EK1122::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EK1122")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}

void EK1521_0010::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EK1521-0010")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}

void CU1521_0010::deviceSpecificGui() {
	if(ImGui::BeginTabItem("CU1521-0010")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}

void EL2008::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL2008")){
		for(int i = 0; i < 8; i++){
			ImGui::PushID(i);
			signalInversionParams[i]->gui(Fonts::sansBold15);
			ImGui::PopID();
		}
		ImGui::EndTabItem();
	}
}

void EL1008::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL1008")){
		for(int i = 0; i < 8; i++){
			ImGui::PushID(i);
			signalInversionParams[i]->gui(Fonts::sansBold15);
			ImGui::PopID();
		}
		ImGui::EndTabItem();
	}
}

void EPP1008_0001::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EPP1008-0001")){
		for(int i = 0; i < 8; i++){
			ImGui::PushID(i);
			signalInversionParams[i]->gui(Fonts::sansBold15);
			ImGui::PopID();
		}
		ImGui::EndTabItem();
	}
}


void EL2624::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL2624")){
		for(int i = 0; i < 4; i++){
			ImGui::PushID(i);
			signalInversionParams[i]->gui(Fonts::sansBold15);
			ImGui::PopID();
		}
		ImGui::EndTabItem();
	}
}

void EL3078::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL3078")){
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Analog Input Channel Settings");
		ImGui::PopFont();
		
		if(ImGui::BeginTable("##ChannelSetting", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_RowBg)){
			ImGui::TableSetupColumn("Ch#");
			ImGui::TableSetupColumn("Input Type");
			ImGui::TableSetupColumn("Filter");
			ImGui::TableHeadersRow();
			
			ImVec2 chNameSize = ImVec2(ImGui::GetTextLineHeight() * 2.0, ImGui::GetFrameHeight());
			float inputTypeWidth = ImGui::GetTextLineHeight() * 5.0;
			float filterSettingWidth = ImGui::GetTextLineHeight() * 5.0;
			for(int i = 0; i < 8; i++){
				ImGui::PushID(i);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				std::string channelName = "Ch" + std::to_string(i+1);
				ImGui::PushFont(Fonts::sansBold15);
				backgroundText(channelName.c_str(), chNameSize, Colors::gray, Colors::black);
				ImGui::PopFont();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(inputTypeWidth);
				channelSettings[i].inputType->gui();
				ImGui::TableSetColumnIndex(2);
				channelSettings[i].enableFilter->gui();
				ImGui::SameLine();
				ImGui::SetNextItemWidth(filterSettingWidth);
				ImGui::BeginDisabled(!channelSettings[i].enableFilter->value);
				channelSettings[i].filterSetting->gui();
				ImGui::EndDisabled();
				ImGui::PopID();
			}
		
			
			ImGui::EndTable();
		}
		
		
		ImGui::EndTabItem();
	}
}


void EL5001::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL5001")){
		
		ssiFrameSize->gui(Fonts::sansBold15);
		multiturnResolution->gui(Fonts::sansBold15);
		singleturnResolution->gui(Fonts::sansBold15);
		ssiCoding_parameter->gui(Fonts::sansBold15);
		ssiBaudrate_parameter->gui(Fonts::sansBold15);
		inhibitTime->gui(Fonts::sansBold15);
		
		ImGui::Separator();
		
		hasResetSignal_Param->gui(Fonts::sansBold15);
		resetSignalTime_Param->gui(Fonts::sansBold15);
		
		ImGui::Separator();
		
		invertDirection_Param->gui(Fonts::sansBold15);
		centerOnZero_Param->gui(Fonts::sansBold15);
		reportOfflineState_Param->gui(Fonts::sansBold15);
		
		float minRange = encoder->feedbackConfig.positionLowerWorkingRangeBound;
		float maxRange = encoder->feedbackConfig.positionUpperWorkingRangeBound;
		float pos = encoder->feedbackProcessData.positionActual;
		float vel = encoder->feedbackProcessData.velocityActual;
		
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::Text("Working Range: %.f to %.f revolutions", minRange, maxRange);
		ImGui::PopStyleColor();
		
		ImGui::Separator();
		
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Realtime encoder data");
		ImGui::PopFont();

		//ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		//ImGui::TextWrapped("%s", encoder->getStatusString().c_str());
		//ImGui::PopStyleColor();
		
		float positionInWorkingRange = (pos - minRange) / (maxRange - minRange);
		float velocityNormalized = vel / 10.0;
		
		ImVec2 progressSize(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
		
		static char statusString[64];
		snprintf(statusString, 64, "%.3f rev", pos);
		ImGui::ProgressBar(positionInWorkingRange, progressSize, statusString);
		
		snprintf(statusString, 64, "%.2f rev/s", vel);
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, velocityNormalized > 0.0 ? Colors::green : Colors::red);
		ImGui::ProgressBar(std::abs(velocityNormalized), progressSize, statusString);
		ImGui::PopStyleColor();
		
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::Text("Raw SSI Value: %i", ssiValue);
		ImGui::PopStyleColor();
		
		if(ImGui::Button("Reset Position")) encoder->overridePosition(0.0);
		ImGui::SameLine();
		if(ImGui::Button("Clear Offset")) {
			positionOffset_rev = 0.0;
			updateEncoderWorkingRange();
		}
		
		ImGui::EndTabItem();
	}
	if(ImGui::BeginTabItem("Debug Data")){
		
		ImGui::Text("Frame Format: %s", frameFormatString.c_str());
		std::stringstream hexStream;
		hexStream << "0x" << std::hex << ssiValue;
		std::string hex = hexStream.str();
		std::string bin = std::bitset<32>(ssiValue).to_string();
		
		ImGui::Text("Counter:");
		ImGui::Text("Rev: %.3fr", encoder->feedbackProcessData.positionActual);
		ImGui::Text("Deg: %.3fd", encoder->feedbackProcessData.positionActual * 360.0);
		ImGui::Text("Int: %i", ssiValue);
		ImGui::Text("Hex: %s", hex.c_str());
		ImGui::Text("Bin: %s", bin.c_str());
		
		ImGui::Separator();
		
		ImGui::Text("Data Error : %i", b_dataError);
		ImGui::Text("Frame Error : %i", b_frameError);
		ImGui::Text("Power Failure : %i", b_powerFailure);
		ImGui::Text("Data Mismatch : %i", b_dataMismatch);
		ImGui::Text("Sync Error : %i", b_syncError);
		ImGui::Text("TxPDO State : %i", b_txPdoState);
		ImGui::Text("TxPDO Toggle : %i", b_txPdoToggle);
		
		ImGui::Separator();
		
		ImGui::Text("Lower Working Range Bound : %.3f rev", encoder->feedbackConfig.positionLowerWorkingRangeBound);
		ImGui::Text("Upper Working Range Bound : %.3f rev", encoder->feedbackConfig.positionUpperWorkingRangeBound);
		
		ImGui::EndTabItem();
	}
}
