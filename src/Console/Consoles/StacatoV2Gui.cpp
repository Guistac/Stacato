#include <pch.h>
#include "StacatoV2.h"

#include <imgui.h>

#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Fonts.h"
#include "Machine/Machine.h"
#include "Gui/Utilities/CustomWidgets.h"

void StacatoV2::gui(float height)
{
	ImDrawList *drawing = ImGui::GetWindowDrawList();

	float joystickWidth = ImGui::GetTextLineHeight() * .2f;
	float joystickTipSize = ImGui::GetTextLineHeight() * .7f;

	auto drawJoystick = [&](std::shared_ptr<Joystick3X> joystick, std::shared_ptr<PushButton> deadmanButton, int id)
	{

		ImGui::PushID(id);
		bool b_pressed = ImGui::InvisibleButton("joystick", glm::vec2(height));
		ImGui::PopID();
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		glm::vec2 center = (max + min) / 2.0;

		glm::vec2 joystickPosition(joystick->getPosition().x, joystick->getPosition().y);
		glm::vec2 joystickEndPosition = center + joystickPosition * glm::vec2(height * .35f) * glm::vec2(1.f, -1.f);

		if (deadmanButton->isPressed()) drawing->AddRect(min, max, ImColor(Colors::white), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll, 3.0);
		drawing->AddRectFilled(min, max, ImColor(Colors::darkGray), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
		drawing->AddCircleFilled(center, joystickWidth * .75f, ImColor(Colors::almostBlack));
		drawing->AddLine(center, joystickEndPosition, ImColor(Colors::almostBlack), joystickWidth);
		drawing->AddCircleFilled(joystickEndPosition, joystickTipSize * .5f, joystickPosition == glm::vec2(0.f) ? ImColor(Colors::blue) : ImColor(Colors::red));

		float rotationRad = -M_PI * joystick->getPosition().z * 45.0 / 180;
		glm::vec2 r1(joystickEndPosition.x + sin(rotationRad) * joystickTipSize * .5, joystickEndPosition.y + cos(rotationRad) * joystickTipSize * .5);
		glm::vec2 r2(joystickEndPosition.x - sin(rotationRad) * joystickTipSize * .5, joystickEndPosition.y - cos(rotationRad) * joystickTipSize * .5);

		if (joystick->getPosition().z != 0.0)
			drawing->AddLine(r1, r2, joystick->getPosition().z == 0.0 ? ImColor(Colors::gray) : ImColor(Colors::white), 1.0);

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

	drawControlChannelMappings(leftJoystickControlChannel);
	ImGui::SameLine();
	bool b_leftPressed = drawJoystick(joystick3x_left, pushbutton_leftJoystickDeadman, 0);
	leftJoystickControlChannel->setPrefferedPopupPosition(ImGui::GetItemRectMin());
	if(b_leftPressed) leftJoystickControlChannel->requestAxisSelectionPopupOpen(true);
	else if (ImGui::IsItemHovered()) leftJoystickControlChannel->mappingListTooltip();
	ImGui::SameLine();
	leftJoystickControlChannel->mappingList();

	ImGui::SameLine();
	bool b_rightPressed = drawJoystick(joystick3x_right, pushbutton_rightJoystickDeadman, 1);
	rightJoystickControlChannel->setPrefferedPopupPosition(ImGui::GetItemRectMin());
	if(b_rightPressed) rightJoystickControlChannel->requestAxisSelectionPopupOpen(true);
	else if (ImGui::IsItemHovered()) rightJoystickControlChannel->mappingListTooltip();
	rightJoystickControlChannel->mappingList();
	ImGui::SameLine();
	drawControlChannelMappings(rightJoystickControlChannel);
}
