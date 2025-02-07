#include <pch.h>
#include "StacatoCompact.h"

#include <imgui.h>

#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Fonts.h"

#include "Legato/Gui/Window.h"

#include "Gui/Utilities/CustomWidgets.h"
#include "Machine/Machine.h"

void StacatoCompact::gui(float height){
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	
	float joystickWidth = ImGui::GetTextLineHeight() * .2f;
	float joystickTipSize = ImGui::GetTextLineHeight() * .5f;
	
	auto drawJoystick = [&](std::shared_ptr<Joystick2X> joystick) -> bool{
		bool b_pressed = ImGui::InvisibleButton("joystick1", glm::vec2(height));
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		glm::vec2 center = (max + min) / 2.0;
		glm::vec2 joystickEndPosition = center + joystick->getPosition() * glm::vec2(height * .4f) * glm::vec2(1.f, -1.f);
		drawing->AddRectFilled(min, max, ImColor(Colors::darkGray), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
		drawing->AddCircleFilled(center, joystickWidth * .75f, ImColor(Colors::almostBlack));
		drawing->AddLine(center, joystickEndPosition, ImColor(Colors::almostBlack), joystickWidth);
		drawing->AddCircleFilled(joystickEndPosition, joystickTipSize * .5f, joystick->getPosition() == glm::vec2(0.f) ? ImColor(Colors::blue) : ImColor(Colors::darkRed));
		return b_pressed;
	};
	
	
	
	auto drawControlChannelMappings = [&](std::shared_ptr<ManualControlChannel> controlChannel){

		std::string mappingString;
		if(controlChannel->getSubscribers().empty()) {
			ImGui::PushFont(Fonts::sansLight20);
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			backgroundText("No Mapping", ImVec2(ImGui::GetTextLineHeight() * 5.0, height), Colors::darkGray);
			ImGui::PopStyleColor();
			ImGui::PopFont();
		}
		else{
			for(auto animatable : controlChannel->getSubscribers()){
				mappingString += std::string(animatable->getMachine()->getName()) + ", ";
			}
			mappingString.pop_back();
			mappingString.pop_back();

			ImGui::PushFont(Fonts::sansBold20);
			scrollingTextWithBackground("mappingdisplay", mappingString.c_str(), ImVec2(ImGui::GetTextLineHeight() * 5.0, height), Colors::darkGray, false, 0.0);
			ImGui::PopFont();
		}
	};

	
	if(drawJoystick(joystick)) controlChannel->requestAxisSelectionPopupOpen();
	else if(ImGui::IsItemHovered()) controlChannel->mappingListTooltip();
	controlChannel->mappingList();
	ImGui::SameLine();
	drawControlChannelMappings(controlChannel);
	
	ImGui::SameLine();

	
}
