#include <pch.h>

#include <imgui.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/HelpMarker.h"
#include "Scripting/Script.h"

#include "ArtnetNode.h"

#include "Gui/Utilities/CustomWidgets.h"

void ArtNetNode::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Network")){
		networkSettingsGui();
		ImGui::EndTabItem();
	}
	if(ImGui::BeginTabItem("Script")){
		scriptingGui();
		ImGui::EndTabItem();
	}
}


void ArtNetNode::networkSettingsGui(){
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Network Settings");
	ImGui::PopFont();
	
	broadcast->gui();
	ImGui::SameLine();
	ImGui::Text("Broadcast");
	
	if(broadcast->value){
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Destination Port");
		ImGui::PopFont();
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("88888").x);
		portNumber->gui();
	}else{
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Destination IP Address & Port");
		ImGui::PopFont();
		
		float octetFieldWidth = ImGui::CalcTextSize("000").x + ImGui::GetStyle().FramePadding.x * 2.f;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * .05f));
		ImGui::SetNextItemWidth(octetFieldWidth);
		ipAddress0->gui();
		ImGui::SameLine();
		ImGui::Text(".");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(octetFieldWidth);
		ipAddress1->gui();
		ImGui::SameLine();
		ImGui::Text(".");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(octetFieldWidth);
		ipAddress2->gui();
		ImGui::SameLine();
		ImGui::Text(".");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(octetFieldWidth);
		ipAddress3->gui();
		ImGui::SameLine();
		ImGui::Text(":");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("00000").x + ImGui::GetStyle().FramePadding.x * 2.f);
		ImGui::PopStyleVar();
		portNumber->gui();
		
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Network");
		ImGui::PopFont();
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * .05f));
		ImGui::SetNextItemWidth(octetFieldWidth);
		networkIpAddress0->gui();
		ImGui::SameLine();
		ImGui::Text(".");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(octetFieldWidth);
		networkIpAddress1->gui();
		ImGui::SameLine();
		ImGui::Text(".");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(octetFieldWidth);
		networkIpAddress2->gui();
		ImGui::SameLine();
		ImGui::Text(".");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(octetFieldWidth);
		networkIpAddress3->gui();
		ImGui::PopStyleVar();
		
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Network Mask");
		ImGui::PopFont();
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * .05f));
		ImGui::SetNextItemWidth(octetFieldWidth);
		networkMask0->gui();
		ImGui::SameLine();
		ImGui::Text(".");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(octetFieldWidth);
		networkMask1->gui();
		ImGui::SameLine();
		ImGui::Text(".");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(octetFieldWidth);
		networkMask2->gui();
		ImGui::SameLine();
		ImGui::Text(".");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(octetFieldWidth);
		networkMask3->gui();
		ImGui::PopStyleVar();
	}
	
	if(ImGui::Button("Reset default ArtNet port")) resetToDefaultArtNetPort();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Sending frequency");
	ImGui::PopFont();
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.f);
	sendingFrequency->gui();
}

void ArtNetNode::scriptingGui(){
	if(ImGui::Button("Compile")) script->compile();
	script->editor(ImGui::GetContentRegionAvail());
}

