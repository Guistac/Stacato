#include <pch.h>

#include "ApplicationWindow.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Fieldbus/EtherCatFieldbus.h"
#include "Gui/Fieldbus/EtherCatGui.h"
#include "Project/Environnement.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"

#include <GLFW/glfw3.h>

namespace ApplicationWindow {



	void drawToolbar(float height) {

		glm::vec2 buttonSize(ImGui::GetTextLineHeight() * 4.0, ImGui::GetTextLineHeight() * 2.0);

		
		if(ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_ALT)){
			
			bool disableScan = EtherCatFieldbus::isCyclicExchangeActive() || Environnement::isSimulating() || !EtherCatFieldbus::isNetworkInitialized();
			if (disableScan) BEGIN_DISABLE_IMGUI_ELEMENT
			if (ImGui::Button("Scan", buttonSize)) EtherCatFieldbus::scanNetwork();
			if (disableScan) END_DISABLE_IMGUI_ELEMENT
				
		}else{
			
			bool disableStartButton = EtherCatFieldbus::isCyclicExchangeStarting() || (!Environnement::isSimulating() && !EtherCatFieldbus::isNetworkInitialized());
			if (disableStartButton) BEGIN_DISABLE_IMGUI_ELEMENT
				if (!EtherCatFieldbus::isCyclicExchangeActive()) {
					if (ImGui::Button("Start", buttonSize)) {
						EtherCatFieldbus::start();
						ImGui::OpenPopup("Starting EtherCAT Fieldbus");
					}
				}
				else {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					if (ImGui::Button("Stop", buttonSize)) EtherCatFieldbus::stop();
					ImGui::PopStyleColor();
				}
			if (disableStartButton) END_DISABLE_IMGUI_ELEMENT

		}
			
			
		ImGui::SameLine();
			
		static ToggleSwitch simulationToggle;
		static bool b_simulation = Environnement::isSimulating();
		
		if(simulationToggle.draw("SimulationSwitch", b_simulation, "simulation", "devices", buttonSize)) {
			b_simulation = !b_simulation;
			Environnement::setSimulation(b_simulation);
		}
			
			
		ImGui::SameLine();

		bool disableMachineToggleButton = !EtherCatFieldbus::isCyclicExchangeActive();

		if (disableMachineToggleButton) BEGIN_DISABLE_IMGUI_ELEMENT
		
		if (Environnement::areAllMachinesEnabled()) {
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
			BEGIN_DISABLE_IMGUI_ELEMENT
			ImGui::Button("Enable All", buttonSize);
			END_DISABLE_IMGUI_ELEMENT
		}
		else {
			if (Environnement::areNoMachinesEnabled()) ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
			else ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
			if (ImGui::Button("Enable All", buttonSize)) Environnement::enableAllMachines();
		}
		ImGui::PopStyleColor();
		
		ImGui::SameLine();
		
		if(Environnement::areNoMachinesEnabled()){
			BEGIN_DISABLE_IMGUI_ELEMENT
			ImGui::Button("Disable All", buttonSize);
			END_DISABLE_IMGUI_ELEMENT
		}else{
			if(ImGui::Button("Disable All", buttonSize)) Environnement::disableAllMachines();
		}
		
		if (disableMachineToggleButton) END_DISABLE_IMGUI_ELEMENT

		
		
	
		
		etherCatStartModal();
		
	}

}
