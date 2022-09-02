#include <pch.h>
#include "ConsoleStarmania.h"

#include <imgui.h>

#include "Gui/Assets/Colors.h"

void ConsoleStarmania::gui(float height){
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	
	float joystickWidth = ImGui::GetTextLineHeight() * .2f;
    float joystickTipSize = ImGui::GetTextLineHeight() * .5f;
	
	auto drawJoystick = [&](std::shared_ptr<Joystick2X> joystick){
		ImGui::InvisibleButton("joystick1", glm::vec2(height));
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		glm::vec2 center = (max + min) / 2.0;
        glm::vec2 joystickEndPosition = center + joystick->getPosition() * glm::vec2(height * .4f) * glm::vec2(1.f, -1.f);
		drawing->AddRectFilled(min, max, ImColor(Colors::darkGray), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
        drawing->AddCircleFilled(center, joystickWidth * .75f, ImColor(Colors::almostBlack));
		drawing->AddLine(center, joystickEndPosition, ImColor(Colors::almostBlack), joystickWidth);
		drawing->AddCircleFilled(joystickEndPosition, joystickTipSize * .5f, joystick->getPosition() == glm::vec2(0.f) ? ImColor(Colors::blue) : ImColor(Colors::darkRed));
	};
	
	drawJoystick(joystickLeft);
	ImGui::SameLine();
	drawJoystick(joystickRight);
	
}
