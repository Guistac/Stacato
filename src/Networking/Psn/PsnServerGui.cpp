#include <pch.h>

#include <imgui.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/HelpMarker.h"
#include "Scripting/Script.h"

#include "PsnServer.h"

#include "Gui/Utilities/CustomWidgets.h"

void PsnServer::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Network")){
		networkSettingsGui();
		ImGui::EndTabItem();
	}
	if(ImGui::BeginTabItem("Script")){
		scriptingGui();
		ImGui::EndTabItem();
	}
}


void PsnServer::networkSettingsGui(){
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Network Settings");
	ImGui::PopFont();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("PSN Server Name");
	ImGui::PopFont();
	serverName->gui();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Destination IP Address & Port");
	ImGui::PopFont();
	
	float octetFieldWidth = ImGui::CalcTextSize("000").x + ImGui::GetStyle().FramePadding.x * 2.f;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * .05f));
	ImGui::SetNextItemWidth(octetFieldWidth);
	destinationIp0->gui();
	ImGui::SameLine();
	ImGui::Text(".");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(octetFieldWidth);
	destinationIp1->gui();
	ImGui::SameLine();
	ImGui::Text(".");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(octetFieldWidth);
	destinationIp2->gui();
	ImGui::SameLine();
	ImGui::Text(".");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(octetFieldWidth);
	destinationIp3->gui();
	ImGui::SameLine();
	ImGui::Text(":");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::CalcTextSize("00000").x + ImGui::GetStyle().FramePadding.x * 2.f);
	ImGui::PopStyleVar();
	destinationPortNumber->gui();
	
	ImGui::SameLine();
	if(ImGui::Button("Reset to PSN Defaults")) setDefaultAddressSettings();
	
	if(!b_addressIsMulticast){
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::TextWrapped("Address is not multicast. (Most significant bit should be between 224 and 239)");
		ImGui::PopStyleColor();
	}
	
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Local Ip");
	ImGui::PopFont();
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * .05f));
	ImGui::SetNextItemWidth(octetFieldWidth);
	localIp0->gui();
	ImGui::SameLine();
	ImGui::Text(".");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(octetFieldWidth);
	localIp1->gui();
	ImGui::SameLine();
	ImGui::Text(".");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(octetFieldWidth);
	localIp2->gui();
	ImGui::SameLine();
	ImGui::Text(".");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(octetFieldWidth);
	localIp3->gui();
	ImGui::PopStyleVar();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Info Packet sending frequency");
	ImGui::PopFont();
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.f);
	infoSendingFrequency->gui();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Data Packet sending frequency");
	ImGui::PopFont();
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.f);
	dataSendingFrequency->gui();
}

void PsnServer::scriptingGui(){
	if(ImGui::Button("Compile")) script->compile();
	script->editor(ImGui::GetContentRegionAvail());
}

