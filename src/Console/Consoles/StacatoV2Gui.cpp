#include <pch.h>
#include "StacatoV2.h"

#include <imgui.h>

#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Fonts.h"

void StacatoV2::gui(float height){
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	
	float joystickWidth = ImGui::GetTextLineHeight() * .2f;
	float joystickTipSize = ImGui::GetTextLineHeight() * .7f;
	
	auto drawJoystick = [&](std::shared_ptr<Joystick3X> joystick, int id){
		ImGui::PushID(id);
		bool b_pressed = ImGui::InvisibleButton("joystick1", glm::vec2(height));
		ImGui::PopID();
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		glm::vec2 center = (max + min) / 2.0;
		
		glm::vec2 joystickPosition(joystick->getPosition().x, joystick->getPosition().y);
		glm::vec2 joystickEndPosition = center + joystickPosition * glm::vec2(height * .35f) * glm::vec2(1.f, -1.f);
		
		drawing->AddRectFilled(min, max, ImColor(Colors::darkGray), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
		drawing->AddCircleFilled(center, joystickWidth * .75f, ImColor(Colors::almostBlack));
		drawing->AddLine(center, joystickEndPosition, ImColor(Colors::almostBlack), joystickWidth);
		drawing->AddCircleFilled(joystickEndPosition, joystickTipSize * .5f, joystickPosition == glm::vec2(0.f) ? ImColor(Colors::blue) : ImColor(Colors::red));
		
		float rotationRad = -M_PI * joystick->getPosition().z * 45.0 / 180;
		glm::vec2 r1(joystickEndPosition.x + sin(rotationRad) * joystickTipSize * .5, joystickEndPosition.y + cos(rotationRad) * joystickTipSize * .5);
		glm::vec2 r2(joystickEndPosition.x - sin(rotationRad) * joystickTipSize * .5, joystickEndPosition.y - cos(rotationRad) * joystickTipSize * .5);
		
		drawing->AddLine(r1, r2, joystick->getPosition().z == 0.0 ? ImColor(Colors::gray) : ImColor(Colors::white), 1.0);

		return b_pressed;
	};
	
	if(drawJoystick(joystick3x_left, 0)) leftJoystickControlChannel->openMappingList();
	else if(ImGui::IsItemHovered()) leftJoystickControlChannel->mappingListTooltip();
	leftJoystickControlChannel->mappingList();
	
	ImGui::SameLine();
	if(drawJoystick(joystick3x_right, 1)) rightJoystickControlChannel->openMappingList();
	else if(ImGui::IsItemHovered()) rightJoystickControlChannel->mappingListTooltip();
	rightJoystickControlChannel->mappingList();
}
