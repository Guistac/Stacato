#include <pch.h>

namespace Environnement::Gui{

	void machineManager(){
		
		static float listWidth = ImGui::GetTextLineHeight() * 15.0;
		static float minlistWidth = ImGui::GetTextLineHeight() * 10.0;
		
		static DraggableList machineList;
		std::shared_ptr<Machine> clickedMachine = nullptr;
		
		ImGui::BeginGroup();
		
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Machine Manager");
		ImGui::PopFont();
		
		std::vector<std::shared_ptr<Machine>>& machines = Environnement::getMachines();
		if (machineList.beginList("##MachineList", glm::vec2(listWidth, ImGui::GetContentRegionAvail().y), 4.0)) {
			for (int i = 0; i < machines.size(); i++) {
				std::shared_ptr<Machine> machine = machines[i];
				if (machineList.beginItem(glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing()), machine == Environnement::selectedMachine)) {

					ImGui::PushFont(Fonts::robotoBold15);
					float shortNameWidth = ImGui::CalcTextSize(machine->shortName).x;
					ImGui::PopFont();
					
					float headerStripWidth = shortNameWidth + ImGui::GetStyle().ItemSpacing.x;
					glm::vec2 min = ImGui::GetWindowPos();
					glm::vec2 max = min + glm::vec2(headerStripWidth, ImGui::GetWindowSize().y);
					glm::vec4 headerStripColor;
					
					if(machine->isEnabled()) headerStripColor = Colors::green;
					else if(machine->isReady()) headerStripColor = Colors::yellow;
					else headerStripColor = Colors::blue;
					
					ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(headerStripColor), 10.0, ImDrawFlags_RoundCornersLeft);

					ImGui::PushFont(Fonts::robotoBold15);
					ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x / 2.0);
					ImGui::Text("%s", machine->shortName);
					ImGui::PopFont();

					ImGui::SameLine();
					ImGui::Text("%s", machine->getName());
					if (machineList.endItem()) clickedMachine = machine;
				}
			}
			machineList.endList();
		}
		if (machineList.wasReordered()) {
			int oldIndex, newIndex;
			machineList.getReorderedItemIndex(oldIndex, newIndex);
			std::shared_ptr<Machine> tmp = machines[oldIndex];
			machines.erase(machines.begin() + oldIndex);
			machines.insert(machines.begin() + newIndex, tmp);
		}
		if(clickedMachine) Environnement::selectedMachine = clickedMachine;
		
		ImGui::EndGroup();
		
		
		
		
		static float splitterWidth = ImGui::GetTextLineHeight() * 0.5;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::SameLine();
		ImGui::InvisibleButton("VerticalSplitter", glm::vec2(splitterWidth, ImGui::GetContentRegionAvail().y));
		if (ImGui::IsItemActive()) listWidth += ImGui::GetIO().MouseDelta.x;
		if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		if (listWidth < minlistWidth) listWidth = minlistWidth;
		
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		float middle = (min.x + max.x) / 2.0;
		ImGui::GetWindowDrawList()->AddLine(glm::vec2(middle, min.y), glm::vec2(middle, max.y), ImColor(Colors::darkGray), ImGui::GetTextLineHeight() * 0.1);
		ImGui::SameLine();
		ImGui::PopStyleVar();
		
		
		
		
		ImGui::SameLine();
		
		ImGui::BeginChild("##selectedmachine", ImGui::GetContentRegionAvail());
			
		if(Environnement::selectedMachine){
			ImGui::PushFont(Fonts::robotoBold20);
			ImGui::Text("%s", Environnement::selectedMachine->getName());
			ImGui::PopFont();
			if(ImGui::BeginTabBar("selectedMachineTabBar")){
				Environnement::selectedMachine->nodeSpecificGui();
				ImGui::EndTabBar();
			}
		}else{
			ImGui::PushFont(Fonts::robotoBold20);
			ImGui::Text("No Machine Selected.");
			ImGui::PopFont();
		}
		
		ImGui::EndChild();
		
	}

}
