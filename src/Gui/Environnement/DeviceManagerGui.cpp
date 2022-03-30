#include <pch.h>

#include <imgui.h>

#include "Environnement/Environnement.h"
#include "Fieldbus/EtherCatDevice.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/DraggableList.h"

namespace Environnement::Gui{

	void deviceManager() {
		   
		   static float listWidth = ImGui::GetTextLineHeight() * 15.0;
		   static float minlistWidth = ImGui::GetTextLineHeight() * 15.0;
		   
		   static DraggableList etherCatDeviceList;
		   std::vector<std::shared_ptr<EtherCatDevice>>& etherCatDevices = Environnement::getEtherCatDevices();
		   
		   std::shared_ptr<EtherCatDevice> clickedDevice = nullptr;
		   
		   ImGui::BeginGroup();
		   
		   ImGui::PushFont(Fonts::sansBold20);
		   ImGui::Text("EtherCat Device Manager");
		   ImGui::PopFont();
		   
		   if (etherCatDeviceList.beginList("##EtherCatDeviceList", glm::vec2(listWidth, ImGui::GetContentRegionAvail().y), 4.0)) {
			   for (int i = 0; i < etherCatDevices.size(); i++) {
				   std::shared_ptr<EtherCatDevice> etherCatDevice = etherCatDevices[i];
				   if (etherCatDeviceList.beginItem(glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing()))) {
					   
					   ImGui::PushFont(Fonts::sansBold15);
					   static char numberString[16];
					   if(!etherCatDevice->isDetected()) sprintf(numberString, "#-");
					   else sprintf(numberString, "#%i", etherCatDevice->getSlaveIndex());
					   
					   float headerStripWidth = ImGui::CalcTextSize(numberString).x + ImGui::GetStyle().ItemSpacing.x;
					   glm::vec2 min = ImGui::GetWindowPos();
					   glm::vec2 max = min + glm::vec2(headerStripWidth, ImGui::GetWindowSize().y);
					   glm::vec4 headerStripColor;
					   
					   if(etherCatDevice->isConnected()) headerStripColor = Colors::green;
					   else if(etherCatDevice->isDetected()) headerStripColor = Colors::yellow;
					   else headerStripColor = Colors::blue;
					   
					   ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(headerStripColor), 10.0, ImDrawFlags_RoundCornersLeft);
				   
					   ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x / 2.0);
					   int slaveIndex = etherCatDevice->getSlaveIndex();
					   if(!etherCatDevice->isDetected()) ImGui::PushStyleColor(ImGuiCol_Text, Colors::darkGray);
					   else ImGui::PushStyleColor(ImGuiCol_Text, Colors::white);
					   ImGui::Text("%s", numberString);
					   ImGui::PopStyleColor();
					   ImGui::PopFont();

					   ImGui::SameLine();
					   ImGui::Text("%s", etherCatDevice->getName());
					   
					   
					   if(etherCatDeviceList.endItem()) clickedDevice = etherCatDevice;
				   }
			   }
			   etherCatDeviceList.endList();
		   }
		   if(clickedDevice) Environnement::selectedEtherCatDevice = clickedDevice;
		   
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
		   
		   ImGui::BeginChild("##selectedDevice", ImGui::GetContentRegionAvail());
			   
		   if(Environnement::selectedEtherCatDevice){
			   ImGui::PushFont(Fonts::sansBold20);
			   ImGui::Text("%s", Environnement::selectedEtherCatDevice->getName());
			   ImGui::PopFont();
			   if(ImGui::BeginTabBar("selectedDeviceTabBar")){
				   Environnement::selectedEtherCatDevice->nodeSpecificGui();
				   ImGui::EndTabBar();
			   }
		   }else{
			   ImGui::PushFont(Fonts::sansBold20);
			   ImGui::Text("No Device Selected.");
			   ImGui::PopFont();
		   }
		   
		   ImGui::EndChild();
		   
	   }

}
