#include <pch.h>
#include "ConsoleStarmania.h"

#include <imgui.h>

#include "Gui/Assets/Colors.h"

void ConsoleStarmania::gui(float height){
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	
	float joystickWidth = ImGui::GetTextLineHeight() * .3f;
	
	auto drawJoystick = [&](std::shared_ptr<Joystick2X> joystick){
		ImGui::InvisibleButton("joystick1", glm::vec2(height));
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		glm::vec2 center = (max - min) / 2.0;
		drawing->AddRectFilled(min, max, ImColor(Colors::darkGray), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
		drawing->AddCircleFilled(center, joystickWidth * 2.f, ImColor(Colors::almostBlack));
		drawing->AddLine(center, center + glm::vec2(height * .5f) * joystickLeft->getPosition(), ImColor(Colors::almostBlack));
		drawing->AddCircleFilled(center + glm::vec2(height * .5f) * joystickLeft->getPosition(), joystickWidth * 3.f, ImColor(Colors::darkRed));
	};
	
	drawJoystick(joystickLeft);
	ImGui::SameLine();
	drawJoystick(joystickRight);
	
}
