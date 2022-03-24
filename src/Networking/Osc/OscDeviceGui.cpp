#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/HelpMarker.h"

#include "OscDevice.h"

#include "Gui/Utilities/CustomWidgets.h"

void OscDevice::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Connection")){
		networkGui();
		ImGui::EndTabItem();
	}
	if(ImGui::BeginTabItem("Outgoing")){
		outgoingMessagesGui();
		ImGui::EndTabItem();
	}
	if(ImGui::BeginTabItem("Incoming")){
		incomingMessagesGui();
		ImGui::EndTabItem();
	}
}



void OscDevice::networkGui(){
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("UDP Socket Settings");
	ImGui::PopFont();
	
	ImGui::SameLine();
	if(beginHelpMarker("(help)")){
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Remote Address:");
		ImGui::PopFont();
		
		ImGui::TextWrapped("The IPv4 address and port to which outgoing messages are sent.");
		
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Local Address:");
		ImGui::PopFont();
		
		ImGui::TextWrapped("The Listening Port is the port on which incoming messages are received.\n"
						   "The local IPv4 address is set through the operating system network preferences.\n");
		
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::TextWrapped("Note: Only one Network Socket with the same IP and ports can be active at any given time.");
		ImGui::PopStyleColor();
		
		endHelpMarker();
	}
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Remote IP");
	ImGui::PopFont();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(0.0, ImGui::GetStyle().ItemSpacing.y));
	for(int i = 0; i < 4; i++){
		if(i != 0) ImGui::SameLine();
		ImGui::PushID(i);
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 2.0);
		ImGui::InputScalar("##ipv4", ImGuiDataType_U8, &remoteIP[i]);
		ImGui::PopID();
		if(i == 3) continue;
		ImGui::SameLine();
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text(".");
		ImGui::PopFont();
	}
	ImGui::PopStyleVar();
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Remote Port");
	ImGui::PopFont();
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 6.0);
	ImGui::InputScalar("##RemotePort", ImGuiDataType_U16, &remotePort);
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Listening Port");
	ImGui::PopFont();
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 6.0);
	ImGui::InputScalar("##ListeningPort", ImGuiDataType_U16, &listeningPort);
}



void OscDevice::outgoingMessagesGui(){
	
	ImGuiTableFlags messageTableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit;
	
	std::shared_ptr<OSC::Message> deletedMessage = nullptr;
	std::shared_ptr<OSC::Message> movedUpMessage = nullptr;
	std::shared_ptr<OSC::Message> movedDownMessage = nullptr;
	
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
			ImGui::TableSetColumnIndex(0);
			switch(ListManagerWidget::draw(i == 0, i == outgoingMessages.size() - 1)){
				case ListManagerWidget::Interaction::NONE: break;
				case ListManagerWidget::Interaction::DELETE: deletedMessage = message; break;
				case ListManagerWidget::Interaction::MOVE_UP: movedUpMessage = message; break;
				case ListManagerWidget::Interaction::MOVE_DOWN: movedDownMessage = message; break;
			}
			ImGui::TableSetColumnIndex(1);
			ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns;
			if (ImGui::Selectable(message->path, false, selectable_flags, ImVec2(0.0, 0.0))) selectMessage(message);
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%i", (int)message->arguments.size());
			ImGui::PopID();
		}
		
		if(outgoingMessages.empty()) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("No Outgoing Messages");
		}
		
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(0.0));
		if(ImGui::Button("New", glm::vec2(ImGui::GetTextLineHeight() * 3.0, ImGui::GetTextLineHeight()))) addOutgoingMessage();
		ImGui::PopStyleVar();
		
		ImGui::EndTable();
	}
	
	if(deletedMessage) deleteMessage(deletedMessage);
	else if(movedUpMessage) moveMessageUp(movedUpMessage);
	else if(movedDownMessage) moveMessageDown(movedDownMessage);
	
	ImGui::Separator();
	
	if(selectedOutgoingMessage == nullptr) {
		ImGui::Text("No Message Selected.");
		return;
	}else{
		messageGui(selectedOutgoingMessage);
	}
}




void OscDevice::incomingMessagesGui(){
	
	
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
	ImGui::TextWrapped("Incoming OSC Messages are not supported yet.");
	ImGui::PopStyleColor();
	
	ImGui::BeginDisabled();
	
	ImGui::Text("Incoming Messages");
	
	ImGuiTableFlags messageTableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit;
	
	std::shared_ptr<OSC::Message> deletedMessage = nullptr;
	std::shared_ptr<OSC::Message> movedUpMessage = nullptr;
	std::shared_ptr<OSC::Message> movedDownMessage = nullptr;
	
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
			ImGui::TableSetColumnIndex(0);
			switch(ListManagerWidget::draw(i == 0, i == incomingMessages.size() - 1)){
				case ListManagerWidget::Interaction::NONE: break;
				case ListManagerWidget::Interaction::DELETE: deletedMessage = message; break;
				case ListManagerWidget::Interaction::MOVE_UP: movedUpMessage = message; break;
				case ListManagerWidget::Interaction::MOVE_DOWN: movedDownMessage = message; break;
			}
			ImGui::TableSetColumnIndex(1);
			ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns;
			if (ImGui::Selectable(message->path, false, selectable_flags, ImVec2(0.0, 0.0))) selectMessage(message);
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%i", (int)message->arguments.size());
			ImGui::PopID();
		}
		
		
		if(incomingMessages.empty()) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("No Incoming Messages");
		}
		
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(0.0));
		if(ImGui::Button("New", glm::vec2(ImGui::GetTextLineHeight() * 3.0, ImGui::GetTextLineHeight()))) addIncomingMessage();
		ImGui::PopStyleVar();
		
		ImGui::EndTable();
	}
	
	if(deletedMessage) deleteMessage(deletedMessage);
	else if(movedUpMessage) moveMessageUp(movedUpMessage);
	else if(movedDownMessage) moveMessageDown(movedDownMessage);
	
	ImGui::EndDisabled();
	
	ImGui::Separator();
	
	if(selectedIncomingMessage == nullptr) {
		ImGui::Text("No Message Selected.");
		return;
	}else{
		messageGui(selectedIncomingMessage);
	}
	
}




void OscDevice::messageGui(std::shared_ptr<OSC::Message> msg){
	
	ImGui::PushFont(Fonts::robotoBold20);
	switch(msg->type){
		case OSC::MessageType::OUTGOING_MESSAGE:
			ImGui::Text("Outgoing Message :");
			break;
		case OSC::MessageType::INCOMING_MESSAGE:
			ImGui::Text("Incoming Message :");
			break;
	}
	ImGui::PopFont();
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("OSC Path :");
	ImGui::PopFont();
	ImGui::InputText("##Path", msg->path, 256);
	
	if(msg->type == OSC::MessageType::OUTGOING_MESSAGE){
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Sending Frequency :");
		ImGui::PopFont();
		ImGui::InputDouble("##freq", &msg->outputFrequency_Hertz, 0.0, 0.0, "%.1f Hz");
		
		ImGui::Checkbox("Include OSC Timestamp", &msg->b_includeTimestamp);
	}
	
	std::shared_ptr<OSC::Argument> deletedArgument = nullptr;
	std::shared_ptr<OSC::Argument> movedUpArgument = nullptr;
	std::shared_ptr<OSC::Argument> movedDownArgument = nullptr;
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("OSC Arguments :");
	ImGui::PopFont();
	
	ImGuiTableFlags messageTableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit;
	
	if(ImGui::BeginTable("##arguments", 4, messageTableFlags)){
		ImGui::TableSetupColumn("Manage");
		ImGui::TableSetupColumn("Type");
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Value");
		ImGui::TableHeadersRow();
		
		for(int i = 0; i < msg->arguments.size(); i++){
			ImGui::TableNextRow();
			ImGui::PushID(i);
			
			auto& argument = msg->arguments[i];
			
			ImGui::TableSetColumnIndex(0);
			switch(ListManagerWidget::draw(i == 0, i == msg->arguments.size() - 1, "", ImGui::GetFrameHeight())){
				case ListManagerWidget::Interaction::NONE: break;
				case ListManagerWidget::Interaction::DELETE: deletedArgument = argument; break;
				case ListManagerWidget::Interaction::MOVE_UP: movedUpArgument = argument; break;
				case ListManagerWidget::Interaction::MOVE_DOWN: movedDownArgument = argument; break;
			}
			
			ImGui::TableSetColumnIndex(1);
			
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 5.0);
			if(ImGui::BeginCombo("##ArgumentTypeSelector", OSC::getArgumentType(argument->type)->displayName)){
				for(auto& argType : OSC::getArgumentTypes()){
					if(ImGui::Selectable(argType.displayName, argument->type == argType.type)){
						argument->setType(argType.type);
					}
				}
				ImGui::EndCombo();
			}
			
			ImGui::TableSetColumnIndex(2);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 12.0);
			ImGui::InputText("##name", (char*)argument->pin->getDisplayString(), 256);
			
			ImGui::TableSetColumnIndex(3);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			argument->pin->dataGui();
			
			ImGui::PopID();
		}
		
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		if(ImGui::Button("Add", glm::vec2(ImGui::GetFrameHeight() * 3.0, ImGui::GetFrameHeight()))) addArgument(msg);
		
		ImGui::EndTable();
	}
	
	if(deletedArgument) removeArgument(msg, deletedArgument);
	else if(movedUpArgument) moveArgumentUp(msg, movedUpArgument);
	else if(movedDownArgument) moveArgumentDown(msg, movedDownArgument);
	
}
