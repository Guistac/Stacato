#pragma once

#include <imgui.h>
#include <config.h>


class KeyboardShortcut{
public:

	enum class Modifier{
		NONE,
		CTRL_CMD,
		ALT,
		SHIFT
	};
	
	KeyboardShortcut(ImGuiKey key_, Modifier mod1 = Modifier::CTRL_CMD, Modifier mod2 = Modifier::NONE, Modifier mod3 = Modifier::NONE) {
		key = key_;
		configure(mod1);
		configure(mod2);
		configure(mod3);
	}
	
	bool isTriggered(){
		if(!ImGui::IsKeyPressed(key)) return false;

		static uint8_t f_alt = 0x1;
		static uint8_t f_control = 0x2;
		static uint8_t f_shift = 0x4;
		
		uint8_t f_modifiers = 0x0;
		if(ImGui::IsKeyDown(ImGuiKey_ModAlt)) 		f_modifiers += f_alt;
		if(ImGui::IsKeyDown(ImGuiKey_ModShift)) 	f_modifiers += f_shift;
		if(ImGui::IsKeyDown(ImGuiKey_ModCtrl))		f_modifiers += f_control;
		
		if(m_alt) 		f_modifiers -= f_alt;
		if(m_control) 	f_modifiers -= f_control;
		if(m_shift) 	f_modifiers -= f_shift;
		
		return f_modifiers == 0;
	}
	
private:
	
	void configure(Modifier mod){
		switch(mod){
			case Modifier::NONE: 		break;
			case Modifier::ALT: 		m_alt = true; 		break;
			case Modifier::CTRL_CMD: 	m_control = true; 	break;
			case Modifier::SHIFT: 		m_shift = true; 	break;
		}
	}
	
	bool m_alt = false;
	bool m_control = false;
	bool m_shift = false;
	ImGuiKey key;
	
};
