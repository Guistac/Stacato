#include <pch.h>

#include "ApplicationWindow.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Fieldbus/EtherCatFieldbus.h"
#include "Gui/Fieldbus/EtherCatGui.h"
#include "Environnement/Environnement.h"

#include "Gui.h"
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"

#include <GLFW/glfw3.h>

#include "Networking/NetworkDevice.h"

namespace Gui {



	void toolbar(float height) {

		glm::vec2 buttonSize(ImGui::GetTextLineHeight() * 4.0, ImGui::GetTextLineHeight() * 2.0);

		
		
		
		if(ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_ALT)){
			
			bool disableScan = EtherCatFieldbus::isCyclicExchangeActive() || Environnement::isSimulating() || !EtherCatFieldbus::isNetworkInitialized();
			ImGui::BeginDisabled(disableScan);
			if (ImGui::Button("Scan", buttonSize)) EtherCatFieldbus::scanNetwork();
			ImGui::EndDisabled();
				
		}else{
			
			bool disableStartButton = !Environnement::isReady() || Environnement::isStarting();
			ImGui::BeginDisabled(disableStartButton);
			if(Environnement::isStarting()){
				ImGui::BeginDisabled();
				ImGui::Button("Starting", buttonSize);
				ImGui::EndDisabled();
			}
			else if(!Environnement::isRunning()){
				if(ImGui::Button("Start", buttonSize)){
					if(!Environnement::isSimulating()) ImGui::OpenPopup("Starting Environnement");
					Environnement::start();
				}
			}else{
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
				if(ImGui::Button("Stop", buttonSize)) Environnement::stop();
				ImGui::PopStyleColor();
			}
			ImGui::EndDisabled();

		}
			
		if(!Environnement::isEditorHidden()){
		
			ImGui::SameLine();
				
			static ToggleSwitch simulationToggle;
			bool b_simulation = Environnement::isSimulating();
			
			bool disableSimulationSwitch = Environnement::isRunning() || Environnement::isStarting();
			ImGui::BeginDisabled(disableSimulationSwitch);
			if(simulationToggle.draw("SimulationSwitch", b_simulation, "Simulation", "Hardware", buttonSize)) {
				b_simulation = !b_simulation;
				Environnement::setSimulation(b_simulation);
			}
			ImGui::EndDisabled();
				
		}
			
		ImGui::SameLine();

		bool disableMachineToggleButtons = !Environnement::isRunning();

		ImGui::BeginDisabled(disableMachineToggleButtons);
		
		if (Environnement::areAllMachinesEnabled()) {
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
			ImGui::BeginDisabled();
			ImGui::Button("Enable All", buttonSize);
			ImGui::EndDisabled();
		}
		else {
			if (Environnement::areNoMachinesEnabled()) ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
			else ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
			if (ImGui::Button("Enable All", buttonSize)) Environnement::enableAllMachines();
		}
		ImGui::PopStyleColor();
		
		ImGui::SameLine();
		
		if(Environnement::areNoMachinesEnabled()){
			ImGui::BeginDisabled();
			ImGui::Button("Disable All", buttonSize);
			ImGui::EndDisabled();
		}else{
			if(ImGui::Button("Disable All", buttonSize)) Environnement::disableAllMachines();
		}
		
		ImGui::EndDisabled();

		
		ImGui::SameLine();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		glm::vec2 separatorStart(ImGui::GetCursorPos().x, ImGui::GetWindowPos().y);
		glm::vec2 separatorEnd(separatorStart.x, separatorStart.y + height);
		drawList->AddLine(separatorStart, separatorEnd, ImColor(Colors::gray));
		
		ImGui::Dummy(glm::vec2(0));
		
		
			
		//========== Status Displays ==========
			
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		
		glm::vec4 etherCatStatusColor;
		if(EtherCatFieldbus::isRunning()) etherCatStatusColor = Colors::green;
		else if(EtherCatFieldbus::isStarting()) etherCatStatusColor = Colors::yellow;
		else if(!EtherCatFieldbus::hasNetworkInterface()) etherCatStatusColor = Colors::red;
		else if(EtherCatFieldbus::slaves.empty()) etherCatStatusColor = Colors::orange;
		else etherCatStatusColor = Colors::blue;
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, etherCatStatusColor);
		ImGui::Button("EtherCAT", buttonSize);
		ImGui::PopStyleColor();
		
		for(auto& networkDevice : Environnement::getNetworkDevices()){
			ImGui::SameLine();
			glm::vec4 networkDeviceStatusColor;
			if(networkDevice->isConnected()) networkDeviceStatusColor = Colors::green;
			else if(networkDevice->isDetected()) networkDeviceStatusColor = Colors::blue;
			else networkDeviceStatusColor = Colors::red;
			ImGui::PushStyleColor(ImGuiCol_Button, networkDeviceStatusColor);
			ImGui::Button(networkDevice->getName(), buttonSize);
			ImGui::PopStyleColor();
		}
		
		ImGui::PopItemFlag();
	
		
		etherCatStartModal();
		
	}

}
