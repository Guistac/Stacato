#include <pch.h>

#include "EtherCatGui.h"

#include <imgui.h>

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Images.h"
#include "Gui/Utilities/CustomWidgets.h"

void etherCatGui() {

	if (ImGui::BeginTabBar("##FieldbusTabBar")) {
		if (ImGui::BeginTabItem("Metrics")) {
			etherCatMetrics();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Parameters")) {
			etherCatParameters();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

}



void topologyConnectionIssues(){
	auto& wrongConnections = EtherCatFieldbus::getWrongConnections();
	float idVignetteSpacing = ImGui::GetTextLineHeight() * 0.2;
	for(int i = 0; i < wrongConnections.size(); i++){
		auto& wrongConnection = wrongConnections[i];
		auto& device = wrongConnection->device;
		
		startFrame(Colors::almostBlack, 1.0, Colors::red, ImVec2(ImGui::GetContentRegionAvail().x, 0));
		
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("%s", device->getName());
		ImGui::SameLine(.0f, idVignetteSpacing);
		device->deviceIdVignette();
		
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::TextWrapped("Device is not connected to the master via the network input port.");
		ImGui::PopFont();
		ImGui::TextWrapped("Current connection to the master is on port %s and should be on port %s.", EtherCatDevice::getPortName(wrongConnection->port), EtherCatDevice::getPortName(0));
		ImGui::PopStyleColor();
		
		endFrame();
	}
}

void networkTopology(){
	auto& topology = EtherCatFieldbus::getNetworkTopology();
	
	float idVignetteSpacing = ImGui::GetTextLineHeight() * 0.2;
	
	ImVec4 deviceBackgroundColor = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];
	float deviceOutlineWidth = 0.0;
	ImVec4 deviceOutlineColor = Colors::black;
	
	ImVec4 portBackgroundColor = Colors::darkRed;
	float portOutlineWidth = 0.0;
	ImVec4 portOutlineColor = Colors::black;
	
	float portFrameOffset = ImGui::GetTextLineHeight() * 1.0;
	float connectionLineOffset = ImGui::GetTextLineHeight() * 1.0;
	float connectionLineLength = ImGui::GetTextLineHeight() * 3.0;
	float connectionDisplayHeight = ImGui::GetTextLineHeight() * 1.5;
	float connectionDisplayWidth = ImGui::GetTextLineHeight() * 10.0;
	float connectionDisplayOffsetX = ImGui::GetTextLineHeight() * 3.0;
	float connectionDisplayOffsetY = (connectionLineLength - connectionDisplayHeight) / 2.0;
	ImVec4 connectionLineColor = Colors::red;
	float connectionLineWidth = ImGui::GetTextLineHeight() * 0.1;
	ImVec2 connectionFramePadding(ImGui::GetStyle().FramePadding.x, ImGui::GetTextLineHeight() * 0.1);
	
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	
	startFrame(deviceBackgroundColor, deviceOutlineWidth, deviceOutlineColor);
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Stacato");
	ImGui::PopFont();
	ImGui::SameLine(.0f, idVignetteSpacing);
	backgroundText("Master", ImVec2(0, ImGui::GetTextLineHeight()), Colors::red);
	endFrame();
	
	static auto getInstabilityColor = [](float instability) -> ImVec4{
		if(instability < 0.1) return Colors::green;
		else if(instability < 0.2) return Colors::yellow;
		else if(instability < 0.5) return Colors::orange;
		else if(instability < 1.0) return Colors::red;
		else return fmod(Timing::getProgramTime_seconds(), 0.5f) < .25f ? Colors::red : Colors::yellow;
	};
	
	static auto getInstabilityString = [](float instability) -> const char*{
		static char stabilityString[64];
		if(instability == 0.0) sprintf(stabilityString, "No Transmission Errors");
		else if(instability < 0.5) sprintf(stabilityString, "%.1f%%", instability * 100.0);
		else if(instability < 1.0) sprintf(stabilityString, "%.1f%% Inspect Cable", instability * 100.0);
		else sprintf(stabilityString, "Max Error Count Reached - Inspect Cable");
		return stabilityString;
	};
	
	static auto drawConnectionStatus = [](std::shared_ptr<EtherCatFieldbus::DeviceConnection> connection, ImVec2 size){
		ImVec4 barcolor = getInstabilityColor(connection->instability);
		ImVec4 backgroundColor = barcolor;
		backgroundColor.x *= 0.5;
		backgroundColor.y *= 0.5;
		backgroundColor.z *= 0.5;
		
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barcolor);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, backgroundColor);
		ImGui::ProgressBar(connection->instability, size, getInstabilityString(connection->instability));
		ImGui::PopStyleColor(2);
		
		if(ImGui::IsItemHovered()){
			ImGui::BeginTooltip();
			static auto drawDevicePortErrors = [](std::shared_ptr<EtherCatDevice> device, int port){
				ImGui::PushFont(Fonts::sansBold15);
				ImGui::Text("%s", device->getName());
				ImGui::PopFont();
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
				ImGui::Text("port %s", EtherCatDevice::getPortName(port));
				auto& portErrors = device->errorCounters.portErrors[port];
				ImGui::PopStyleColor();
				
				ImGui::Text("Frame Errors:");
				ImGui::SameLine();
				ImGui::PushFont(Fonts::sansBold15);
				float frame = 100.f * (float)portErrors.frameRxErrors / 255.f;
				ImGui::PushStyleColor(ImGuiCol_Text, getInstabilityColor(frame / 100.0));
				ImGui::Text("%.1f%%", frame);
				ImGui::PopFont();
				ImGui::PopStyleColor();
				
				ImGui::Text("Physical Errors:");
				ImGui::SameLine();
				ImGui::PushFont(Fonts::sansBold15);
				float physical = 100.f * (float)portErrors.physicalRxErrors / 255.f;
				ImGui::PushStyleColor(ImGuiCol_Text, getInstabilityColor(physical / 100.0));
				ImGui::Text("%.1f%%", physical);
				ImGui::PopFont();
				ImGui::PopStyleColor();
				
				ImGui::Text("Forwarded Errors:");
				ImGui::SameLine();
				ImGui::PushFont(Fonts::sansBold15);
				float forwarded = 100.f * (float)portErrors.forwardedRxErrors / 255.f;
				ImGui::PushStyleColor(ImGuiCol_Text, getInstabilityColor(forwarded / 100.0));
				ImGui::Text("%.1f%%", forwarded);
				ImGui::PopStyleColor();
				ImGui::PopFont();
			};
			
			static auto drawMasterFrameErrors = [](){
				
				ImGui::PushFont(Fonts::sansBold15);
				ImGui::Text("Stacato");
				ImGui::PopFont();
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
				ImGui::Text("Interface %s", EtherCatFieldbus::getActiveNetworkInterfaceCard()->description);
				ImGui::PopStyleColor();
				
				ImGui::Text("Frame Timeouts:");
				ImGui::SameLine();
				ImGui::PushFont(Fonts::sansBold15);
				float timeouts = 100.f * (float)EtherCatFieldbus::getCyclicFrameTimeoutCounter() / 255.f;
				timeouts = std::min(timeouts, 100.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, getInstabilityColor(timeouts / 100.0));
				ImGui::Text("%.1f%%", timeouts);
				ImGui::PopFont();
				ImGui::PopStyleColor();
				
				ImGui::Text("Frame Errors:");
				ImGui::SameLine();
				ImGui::PushFont(Fonts::sansBold15);
				float errors = 100.f * (float)EtherCatFieldbus::getCyclicFrameErrorCounter() / 255.f;
				errors = std::min(errors, 100.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, getInstabilityColor(errors / 100.0));
				ImGui::Text("%.1f%%", errors);
				ImGui::PopFont();
				ImGui::PopStyleColor();
				
			};
			
			if(connection->b_parentIsMaster) drawMasterFrameErrors();
			else drawDevicePortErrors(connection->parentDevice, connection->parentDevicePort);
			drawDevicePortErrors(connection->childDevice, connection->childDevicePort);
			ImGui::EndTooltip();
		}
		
		
		if(connection->b_wasDisconnected){
			ImGui::SameLine();
			ImGui::Image(Images::WarningIcon.getID(), ImVec2(size.y, size.y));
			if(ImGui::IsItemHovered()){
				ImGui::BeginTooltip();
				
				static auto drawDisconnections = [](std::shared_ptr<EtherCatDevice> device, int port){
					auto& portErrors = device->errorCounters.portErrors[port];
					
					ImGui::PushFont(Fonts::sansBold15);
					ImGui::Text("%s", device->getName());
					ImGui::PopFont();
					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
					ImGui::Text("port %s", EtherCatDevice::getPortName(port));
					ImGui::PopStyleColor();
					
					ImGui::Text("Disconnections:");
					ImGui::SameLine();
					ImGui::PushFont(Fonts::sansBold15);
					ImGui::PushStyleColor(ImGuiCol_Text, portErrors.lostLinks == 0 ? Colors::green : Colors::red);
					ImGui::Text("%i", (int)portErrors.lostLinks);
					ImGui::PopStyleColor();
					ImGui::PopFont();
				};
				
				if(!connection->b_parentIsMaster) drawDisconnections(connection->parentDevice, connection->parentDevicePort);
				drawDisconnections(connection->childDevice, connection->childDevicePort);
				ImGui::EndTooltip();
			}
		}
	};
	
	
	for(int i = 0; i < topology.size(); i++){
		
		auto& connection = topology[i];
		auto& childDevice = connection->childDevice;
		
		
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + portFrameOffset);
		
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, connectionFramePadding);
		startFrame(portBackgroundColor, portOutlineWidth, portOutlineColor, ImVec2(0,0), ImDrawFlags_RoundCornersBottom);
		ImGui::PopStyleVar();
		if(connection->b_parentIsMaster) ImGui::Text("Interface %s", EtherCatFieldbus::getActiveNetworkInterfaceCard()->description);
		else ImGui::Text("port %s", EtherCatDevice::getPortName(connection->parentDevicePort));
		endFrame();
		
		ImVec2 cursorBeforeCable = ImGui::GetCursorPos();
		
		glm::vec2 fwLineStart(ImGui::GetItemRectMin().x + connectionLineOffset, ImGui::GetItemRectMax().y - 1.0);
		glm::vec2 fwLineEnd(fwLineStart.x, fwLineStart.y + connectionLineLength + 2.0);
		
		drawing->AddLine(fwLineStart, fwLineEnd, ImColor(connectionLineColor), connectionLineWidth);
		
		ImGui::SetCursorPosY(cursorBeforeCable.y - ImGui::GetStyle().ItemSpacing.y + connectionDisplayOffsetY);
		ImGui::SetCursorPosX(cursorBeforeCable.x + connectionDisplayOffsetX);
		drawConnectionStatus(connection, ImVec2(connectionDisplayWidth, connectionDisplayHeight));
		
		ImGui::SetCursorPosX(cursorBeforeCable.x + portFrameOffset);
		ImGui::SetCursorPosY(cursorBeforeCable.y - ImGui::GetStyle().ItemSpacing.y + connectionLineLength);
		
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, connectionFramePadding);
		startFrame(portBackgroundColor, portOutlineWidth, portOutlineColor, ImVec2(0,0), ImDrawFlags_RoundCornersTop);
		ImGui::PopStyleVar();
		ImGui::Text("port %s", EtherCatDevice::getPortName(connection->childDevicePort));
		endFrame();
		
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
		
		startFrame(deviceBackgroundColor, deviceOutlineWidth, Colors::black);
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("%i - %s", connection->childDevice->getSlaveIndex(), connection->childDevice->getName());
		ImGui::PopFont();
		ImGui::SameLine(.0f, idVignetteSpacing);
		static char idString[32];
		switch(childDevice->identificationType){
			case EtherCatDevice::IdentificationType::STATION_ALIAS:
				sprintf(idString, "Alias #%i", childDevice->stationAlias);
				backgroundText(idString, ImVec2(0, ImGui::GetTextLineHeight()), childDevice->getEtherCatStateColor());
				break;
			case EtherCatDevice::IdentificationType::EXPLICIT_DEVICE_ID:
				sprintf(idString, "ID #%i", childDevice->explicitDeviceID);
				backgroundText(idString, ImVec2(0, ImGui::GetTextLineHeight()), childDevice->getEtherCatStateColor());
				break;
		}
		endFrame();
		
	}
}

















void listNetworkConnections(){
	
	auto& topology = EtherCatFieldbus::getNetworkTopology();
	
	float idVignetteSpacing = ImGui::GetTextLineHeight() * 0.2;
	
	ImVec4 deviceBackgroundColor = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];
	float deviceOutlineWidth = 0.0;
	ImVec4 deviceOutlineColor = Colors::black;
	
	ImVec4 portBackgroundColor = Colors::darkRed;
	float portOutlineWidth = 0.0;
	ImVec4 portOutlineColor = Colors::black;
	
	float portFrameOffset = ImGui::GetTextLineHeight() * 1.0;
	float connectionLineOffset = ImGui::GetTextLineHeight() * 1.0;
	float connectionLineLength = ImGui::GetTextLineHeight() * 3.0;
	float connectionDisplayHeight = ImGui::GetTextLineHeight() * 1.5;
	float connectionDisplayWidth = ImGui::GetTextLineHeight() * 10.0;
	float connectionDisplayOffsetX = ImGui::GetTextLineHeight() * 3.0;
	float connectionDisplayOffsetY = (connectionLineLength - connectionDisplayHeight) / 2.0;
	ImVec4 connectionLineColor = Colors::red;
	float connectionLineWidth = ImGui::GetTextLineHeight() * 0.1;
	ImVec2 connectionFramePadding(ImGui::GetStyle().FramePadding.x, ImGui::GetTextLineHeight() * 0.1);
	
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	

	
	static auto getInstabilityColor = [](float instability) -> ImVec4{
		if(instability < 0.1) return Colors::green;
		else if(instability < 0.2) return Colors::yellow;
		else if(instability < 0.5) return Colors::orange;
		else if(instability < 1.0) return Colors::red;
		else return fmod(Timing::getProgramTime_seconds(), 0.5f) < .25f ? Colors::red : Colors::yellow;
	};
	
	static auto getInstabilityString = [](float instability) -> const char*{
		static char stabilityString[64];
		if(instability == 0.0) sprintf(stabilityString, "No Transmission Errors");
		else if(instability < 0.5) sprintf(stabilityString, "%.1f%%", instability * 100.0);
		else if(instability < 1.0) sprintf(stabilityString, "%.1f%% Inspect Cable", instability * 100.0);
		else sprintf(stabilityString, "Max Error Count Reached - Inspect Cable");
		return stabilityString;
	};
	
	static auto drawConnectionStatus = [](std::shared_ptr<EtherCatFieldbus::DeviceConnection> connection, ImVec2 size){
		ImVec4 barcolor = getInstabilityColor(connection->instability);
		ImVec4 backgroundColor = barcolor;
		backgroundColor.x *= 0.5;
		backgroundColor.y *= 0.5;
		backgroundColor.z *= 0.5;
		
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barcolor);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, backgroundColor);
		ImGui::ProgressBar(connection->instability, size, getInstabilityString(connection->instability));
		ImGui::PopStyleColor(2);
		
		if(ImGui::IsItemHovered()){
			ImGui::BeginTooltip();
			static auto drawDevicePortErrors = [](std::shared_ptr<EtherCatDevice> device, int port){
				ImGui::PushFont(Fonts::sansBold15);
				ImGui::Text("%s", device->getName());
				ImGui::PopFont();
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
				ImGui::Text("port %s", EtherCatDevice::getPortName(port));
				auto& portErrors = device->errorCounters.portErrors[port];
				ImGui::PopStyleColor();
				
				ImGui::Text("Frame Errors:");
				ImGui::SameLine();
				ImGui::PushFont(Fonts::sansBold15);
				float frame = 100.f * (float)portErrors.frameRxErrors / 255.f;
				ImGui::PushStyleColor(ImGuiCol_Text, getInstabilityColor(frame / 100.0));
				ImGui::Text("%.1f%%", frame);
				ImGui::PopFont();
				ImGui::PopStyleColor();
				
				ImGui::Text("Physical Errors:");
				ImGui::SameLine();
				ImGui::PushFont(Fonts::sansBold15);
				float physical = 100.f * (float)portErrors.physicalRxErrors / 255.f;
				ImGui::PushStyleColor(ImGuiCol_Text, getInstabilityColor(physical / 100.0));
				ImGui::Text("%.1f%%", physical);
				ImGui::PopFont();
				ImGui::PopStyleColor();
				
				ImGui::Text("Forwarded Errors:");
				ImGui::SameLine();
				ImGui::PushFont(Fonts::sansBold15);
				float forwarded = 100.f * (float)portErrors.forwardedRxErrors / 255.f;
				ImGui::PushStyleColor(ImGuiCol_Text, getInstabilityColor(forwarded / 100.0));
				ImGui::Text("%.1f%%", forwarded);
				ImGui::PopStyleColor();
				ImGui::PopFont();
			};
			
			static auto drawMasterFrameErrors = [](){
				
				ImGui::PushFont(Fonts::sansBold15);
				ImGui::Text("Stacato");
				ImGui::PopFont();
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
				ImGui::Text("Interface %s", EtherCatFieldbus::getActiveNetworkInterfaceCard()->description);
				ImGui::PopStyleColor();
				
				ImGui::Text("Dropped Frames:");
				ImGui::SameLine();
				double droppedFramesNormalized = 100.0 * double(EtherCatFieldbus::getMetrics().droppedFrameCount) / double(EtherCatFieldbus::getMetrics().frameCount);
				ImGui::Text("%i / %i (%.3f%%)",
							(int)EtherCatFieldbus::getMetrics().droppedFrameCount,
							(int)EtherCatFieldbus::getMetrics().frameCount,
							droppedFramesNormalized);
				
				ImGui::Text("Frame Errors:");
				ImGui::SameLine();
				ImGui::PushFont(Fonts::sansBold15);
				ImGui::Text("%i", EtherCatFieldbus::getCyclicFrameErrorCounter());
				ImGui::PopFont();

				
			};
			
			if(connection->b_parentIsMaster) drawMasterFrameErrors();
			else drawDevicePortErrors(connection->parentDevice, connection->parentDevicePort);
			drawDevicePortErrors(connection->childDevice, connection->childDevicePort);
			ImGui::EndTooltip();
		}
		
		
		if(connection->b_wasDisconnected){
			ImGui::SameLine();
			ImGui::Image(Images::WarningIcon.getID(), ImVec2(size.y, size.y));
			if(ImGui::IsItemHovered()){
				ImGui::BeginTooltip();
				
				static auto drawDisconnections = [](std::shared_ptr<EtherCatDevice> device, int port){
					auto& portErrors = device->errorCounters.portErrors[port];
					
					ImGui::PushFont(Fonts::sansBold15);
					ImGui::Text("%s", device->getName());
					ImGui::PopFont();
					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
					ImGui::Text("port %s", EtherCatDevice::getPortName(port));
					ImGui::PopStyleColor();
					
					ImGui::Text("Disconnections:");
					ImGui::SameLine();
					ImGui::PushFont(Fonts::sansBold15);
					ImGui::PushStyleColor(ImGuiCol_Text, portErrors.lostLinks == 0 ? Colors::green : Colors::red);
					ImGui::Text("%i", (int)portErrors.lostLinks);
					ImGui::PopStyleColor();
					ImGui::PopFont();
				};
				
				if(!connection->b_parentIsMaster) drawDisconnections(connection->parentDevice, connection->parentDevicePort);
				drawDisconnections(connection->childDevice, connection->childDevicePort);
				ImGui::EndTooltip();
			}
		}
	};
	
	
	for(int i = 0; i < topology.size(); i++){
		
		auto& connection = topology[i];
		auto& parentDevice = connection->parentDevice;
		auto& childDevice = connection->childDevice;
		
		if(parentDevice == nullptr){
			
			startFrame(deviceBackgroundColor, deviceOutlineWidth, deviceOutlineColor);
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("Stacato");
			ImGui::PopFont();
			ImGui::SameLine(.0f, idVignetteSpacing);
			backgroundText("Master", ImVec2(0, ImGui::GetTextLineHeight()), Colors::red);
			endFrame();
			
		}else if(i > 0 && topology[i - 1]->childDevice != parentDevice){
				
			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();
			
			startFrame(deviceBackgroundColor, deviceOutlineWidth, Colors::black);
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("%i - %s", parentDevice->getSlaveIndex(), parentDevice->getName());
			ImGui::PopFont();
			ImGui::SameLine(.0f, idVignetteSpacing);
			static char idString[32];
			switch(parentDevice->identificationType){
				case EtherCatDevice::IdentificationType::STATION_ALIAS:
					sprintf(idString, "Alias #%i", parentDevice->stationAlias);
					backgroundText(idString, ImVec2(0, ImGui::GetTextLineHeight()), parentDevice->getEtherCatStateColor());
					break;
				case EtherCatDevice::IdentificationType::EXPLICIT_DEVICE_ID:
					sprintf(idString, "ID #%i", parentDevice->explicitDeviceID);
					backgroundText(idString, ImVec2(0, ImGui::GetTextLineHeight()), parentDevice->getEtherCatStateColor());
					break;
			}
			endFrame();
		}
		
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + portFrameOffset);
		
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, connectionFramePadding);
		startFrame(portBackgroundColor, portOutlineWidth, portOutlineColor, ImVec2(0,0), ImDrawFlags_RoundCornersBottom);
		ImGui::PopStyleVar();
		if(connection->b_parentIsMaster) ImGui::Text("Interface %s", EtherCatFieldbus::getActiveNetworkInterfaceCard()->description);
		else ImGui::Text("port %s", EtherCatDevice::getPortName(connection->parentDevicePort));
		endFrame();
		
		ImVec2 cursorBeforeCable = ImGui::GetCursorPos();
		
		glm::vec2 fwLineStart(ImGui::GetItemRectMin().x + connectionLineOffset, ImGui::GetItemRectMax().y - 1.0);
		glm::vec2 fwLineEnd(fwLineStart.x, fwLineStart.y + connectionLineLength + 2.0);
		
		drawing->AddLine(fwLineStart, fwLineEnd, ImColor(connectionLineColor), connectionLineWidth);
		
		ImGui::SetCursorPosY(cursorBeforeCable.y - ImGui::GetStyle().ItemSpacing.y + connectionDisplayOffsetY);
		ImGui::SetCursorPosX(cursorBeforeCable.x + connectionDisplayOffsetX);
		drawConnectionStatus(connection, ImVec2(connectionDisplayWidth, connectionDisplayHeight));
		
		ImGui::SetCursorPosX(cursorBeforeCable.x + portFrameOffset);
		ImGui::SetCursorPosY(cursorBeforeCable.y - ImGui::GetStyle().ItemSpacing.y + connectionLineLength);
		
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, connectionFramePadding);
		startFrame(portBackgroundColor, portOutlineWidth, portOutlineColor, ImVec2(0,0), ImDrawFlags_RoundCornersTop);
		ImGui::PopStyleVar();
		ImGui::Text("port %s", EtherCatDevice::getPortName(connection->childDevicePort));
		endFrame();
		
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
		
		startFrame(deviceBackgroundColor, deviceOutlineWidth, Colors::black);
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("%i - %s", connection->childDevice->getSlaveIndex(), connection->childDevice->getName());
		ImGui::PopFont();
		ImGui::SameLine(.0f, idVignetteSpacing);
		static char idString[32];
		switch(childDevice->identificationType){
			case EtherCatDevice::IdentificationType::STATION_ALIAS:
				sprintf(idString, "Alias #%i", childDevice->stationAlias);
				backgroundText(idString, ImVec2(0, ImGui::GetTextLineHeight()), childDevice->getEtherCatStateColor());
				break;
			case EtherCatDevice::IdentificationType::EXPLICIT_DEVICE_ID:
				sprintf(idString, "ID #%i", childDevice->explicitDeviceID);
				backgroundText(idString, ImVec2(0, ImGui::GetTextLineHeight()), childDevice->getEtherCatStateColor());
				break;
		}
		endFrame();
		
	}
	
}



void drawDeviceConnections(std::shared_ptr<EtherCatDevice> device){
	if(ImGui::CollapsingHeader(device->getName())){
		ImGui::TreePush();
		for(auto connection : device->connections){
			if(connection->parentDevice == device){
				drawDeviceConnections(connection->childDevice);
			}
		}
		ImGui::TreePop();
	}
}

void unfoldNetwork(){
	for(auto& etherCatDevice : EtherCatFieldbus::getDevices()){
		bool b_deviceHasParentDevice = false;
		for(auto& connection : etherCatDevice->connections){
			if(connection->childDevice == etherCatDevice) {
				b_deviceHasParentDevice = true;
				break;
			}
		}
		if(!b_deviceHasParentDevice) drawDeviceConnections(etherCatDevice);
	}
}








void EtherCatNetworkWindow::onDraw(){
		
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("EtherCAT Network");
	ImGui::PopFont();
	
	if(!EtherCatFieldbus::isRunning()){
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::Text("Network diagnostics are not available while the fieldbus is not running.");
		ImGui::PopStyleColor();
	}
	else if(!EtherCatFieldbus::getWrongConnections().empty()){
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::Text("The Network has illegal connections.");
		ImGui::PopStyleColor();
		ImGui::PopFont();
		ImGui::TextWrapped("Network diagnostics are unavailable while these issues are not resolved. Fix the following issues and restart the fieldbus.");
		
		ImGui::BeginChild("ConnectionIssues");
		topologyConnectionIssues();
		ImGui::EndChild();
	}else{
		ImGui::SameLine();
		if(ImGui::Button("Reset Error Counters")) EtherCatFieldbus::resetErrorCounters();
		
		ImGui::Separator();
		ImGui::BeginChild("Topology");
		//networkTopology();
		listNetworkConnections();
		/*
		ImGui::NewLine();
		ImGui::Separator();
		ImGui::Separator();
		ImGui::Separator();
		ImGui::NewLine();
		unfoldNetwork();
		 */
		ImGui::EndChild();
	}
	
}
