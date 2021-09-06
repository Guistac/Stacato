#include <pch.h>
#include "LedsAndButtons.h"

#include <imgui.h>

void LedsAndButtons::deviceSpecificGui() {
	if (ImGui::BeginTabItem("LedsAndButtons Configuration")) {
		
		bool b0 = button0.getBoolean();
		bool b1 = button1.getBoolean();
		bool b2 = button2.getBoolean();
		bool b3 = button3.getBoolean();
		int l0 = led0.getInteger();
		int l1 = led1.getInteger();
		int l2 = led2.getInteger();
		int l3 = led3.getInteger();
		int l4 = led4.getInteger();

		ImGui::Checkbox("Button 0", &b0);
		ImGui::Checkbox("Button 1", &b1);
		ImGui::Checkbox("Button 2", &b2);
		ImGui::Checkbox("Button 3", &b3);
		ImGui::SliderInt("Led 0", &l0, 0, 255);
		ImGui::SliderInt("Led 1", &l1, 0, 255);
		ImGui::SliderInt("Led 2", &l2, 0, 255);
		ImGui::SliderInt("Led 3", &l3, 0, 255);
		ImGui::SliderInt("Led 4", &l4, 0, 255);

		led0.set((long long int)l0);
		led1.set((long long int)l1);
		led2.set((long long int)l2);
		led3.set((long long int)l3);
		led4.set((long long int)l4);

		ImGui::EndTabItem();
	}
}