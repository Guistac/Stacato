#include <pch.h>
#include "LedsAndButtons.h"

#include <imgui.h>

void LedsAndButtons::deviceSpecificGui() {
	if (ImGui::BeginTabItem("LedsAndButtons Configuration")) {
		
		bool b0 = button0.getBool();
		bool b1 = button1.getBool();
		bool b2 = button2.getBool();
		bool b3 = button3.getBool();

		ImGui::Checkbox("Button 0", &b0);
		ImGui::Checkbox("Button 1", &b1);
		ImGui::Checkbox("Button 2", &b2);
		ImGui::Checkbox("Button 3", &b3);

		int l0 = led0.getUnsignedByte();
		int l1 = led1.getUnsignedByte();
		int l2 = led2.getUnsignedByte();
		int l3 = led3.getUnsignedByte();
		int l4 = led4.getUnsignedByte();

		ImGui::SliderInt("Led 0", &l0, 0, 255);
		ImGui::SliderInt("Led 1", &l1, 0, 255);
		ImGui::SliderInt("Led 2", &l2, 0, 255);
		ImGui::SliderInt("Led 3", &l3, 0, 255);
		ImGui::SliderInt("Led 4", &l4, 0, 255);

		led0 = (uint8_t)l0;
		led1 = (uint8_t)l1;
		led2 = (uint8_t)l2;
		led3 = (uint8_t)l3;
		led4 = (uint8_t)l4;

		ImGui::EndTabItem();
	}
}