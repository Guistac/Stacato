#include <pch.h>

#include <imgui.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "OscDevice.h"

void OscDevice::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Network")){
		networkGui();
		ImGui::EndTabItem();
	}
	if(ImGui::BeginTabItem("Data")){
		dataGui();
		ImGui::EndTabItem();
	}
}

void OscDevice::networkGui(){
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("UDP Socket");
	ImGui::PopFont();
	ImGui::Text("Remote IP");
	ImGui::InputInt4("##RemoteIP", remoteIP);
	ImGui::Text("Remote Port");
	ImGui::InputInt("##RemotePort", &remotePort);
	ImGui::Text("Listening Port");
	ImGui::InputInt("##ListeningPort", &listeningPort);
}

void OscDevice::dataGui(){
	
	float tableWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;
	
	ImGuiTableFlags messageTableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit;
	
	ImGui::BeginGroup();
	
	ImGui::Text("Outgoing Messages");
	
	if(ImGui::BeginTable("##outgoingMessageList", 3, messageTableFlags)){
		
		ImGui::TableSetupColumn("Manage");
		ImGui::TableSetupColumn("Message");
		ImGui::TableSetupColumn("Arguments");
		ImGui::TableHeadersRow();
		
		for(int i = 0; i < outgoingMessages.size(); i++){
			auto& message = outgoingMessages[i];
			ImGui::PushID(i);
			ImGui::TableNextRow();
			if(isMessageSelected(message)) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(Colors::blue));
			ImGui::TableSetColumnIndex(1);
			ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns;
			if (ImGui::Selectable(message->address, false, selectable_flags, ImVec2(0.0, 0.0))) selectMessage(message);
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%i", (int)message->arguments.size());
			ImGui::PopID();
		}
		
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		if(ImGui::Button("New##Outgoing")) addOutgoingMessage();
		
		ImGui::EndTable();
	}
	
	ImGui::EndGroup();
	
	ImGui::SameLine();
	
	ImGui::BeginGroup();
	
	ImGui::Text("Incoming Messages");
	
	if(ImGui::BeginTable("##incomingMessageList", 3, messageTableFlags)){
		
		ImGui::TableSetupColumn("Manage");
		ImGui::TableSetupColumn("Message");
		ImGui::TableSetupColumn("Arguments");
		ImGui::TableHeadersRow();
		
		for(int i = 0; i < incomingMessages.size(); i++){
			auto& message = incomingMessages[i];
			ImGui::PushID(i);
			ImGui::TableNextRow();
			if(isMessageSelected(message)) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(Colors::blue));
			ImGui::TableSetColumnIndex(1);
			ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns;
			if (ImGui::Selectable(message->address, false, selectable_flags, ImVec2(0.0, 0.0))) selectMessage(message);
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%i", (int)message->arguments.size());
			ImGui::PopID();
		}
		
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		if(ImGui::Button("New##Incoming")) addIncomingMessage();
		
		ImGui::EndTable();
	}
	
	ImGui::EndGroup();
	
	ImGui::Separator();
	
	if(selectedMessage == nullptr) {
		ImGui::Text("No Message Selected.");
		return;
	}
	
	ImGui::PushFont(Fonts::robotoBold20);
	switch(selectedMessage->type){
		case OSC::MessageType::OUTGOING_MESSAGE:
			ImGui::Text("Outgoing Message :");
			break;
		case OSC::MessageType::INCOMING_MESSAGE:
			ImGui::Text("Incoming Messages :");
			break;
	}
	ImGui::PopFont();
	
	ImGui::Text("Address :");
	ImGui::InputText("##address", selectedMessage->address, 256);
	
	if(selectedMessage->type == OSC::MessageType::OUTGOING_MESSAGE){
		ImGui::Text("Sending Frequency :");
		ImGui::InputDouble("##freq", &selectedMessage->outputFrequency_Hertz, 0.0, 0.0, "%.1f Hz");
		
		ImGui::Checkbox("Include OSC Timestamp", &selectedMessage->b_includeTimestamp);
	}
	
	ImGui::Text("Arguments :");
	if(ImGui::BeginTable("##arguments", 4, messageTableFlags)){
		ImGui::TableSetupColumn("Manage");
		ImGui::TableSetupColumn("Type");
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Value");
		ImGui::TableHeadersRow();
		
		for(int i = 0; i < selectedMessage->arguments.size(); i++){
			ImGui::TableNextRow();
			ImGui::PushID(i);
			
			auto& argument = selectedMessage->arguments[i];
			
			ImGui::TableSetColumnIndex(0);
			//buttons
			
			ImGui::TableSetColumnIndex(1);
			
			if(ImGui::BeginCombo("##ArgumentTypeSelector", OSC::getArgumentType(argument->type)->displayName)){
				for(auto& argType : OSC::getArgumentTypes()){
					if(ImGui::Selectable(argType.displayName, argument->type == argType.type)){
						argument->setType(argType.type);
					}
				}
				ImGui::EndCombo();
			}
			
			ImGui::PopID();
		}
		
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		if(ImGui::Button("Add Argument")) addArgument(selectedMessage);
		
		ImGui::EndTable();
	}
	
}
