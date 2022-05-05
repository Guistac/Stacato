#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>



class KeyboardShortcut{
public:

	enum class Modifier{
		NONE,
		SUPER,
		ALT,
		CONTROL,
		SHIFT
	};
	
	KeyboardShortcut(int key_, Modifier mod1, Modifier mod2 = Modifier::NONE, Modifier mod3 = Modifier::NONE, Modifier mod4 = Modifier::NONE) {
		key = key_;
		configure(mod1);
		configure(mod2);
		configure(mod3);
		configure(mod4);
	}
	
	bool isTriggered(){
		if(!ImGui::IsKeyPressed(key)) return false;
		
		static uint8_t f_super = 0x1;
		static uint8_t f_alt = 0x2;
		static uint8_t f_control = 0x4;
		static uint8_t f_shift = 0x8;
		
		uint8_t f_modifiers = 0x0;
		if(ImGui::IsKeyDown(GLFW_KEY_LEFT_SUPER) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_SUPER)) 	f_modifiers += f_super;
		if(ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_ALT)) 		f_modifiers += f_alt;
		if(ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_CONTROL)) f_modifiers += f_control;
		if(ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_SHIFT)) 	f_modifiers += f_shift;
		
		if(m_super) 	f_modifiers -= f_super;
		if(m_alt) 		f_modifiers -= f_alt;
		if(m_control) 	f_modifiers -= f_control;
		if(m_shift) 	f_modifiers -= f_shift;
		
		return f_modifiers == 0;
	}
	
private:
	
	void configure(Modifier mod){
		switch(mod){
			case Modifier::NONE: 	break;
			case Modifier::SUPER: 	m_super = true; 	break;
			case Modifier::ALT: 	m_alt = true; 		break;
			case Modifier::CONTROL: m_control = true; 	break;
			case Modifier::SHIFT: 	m_shift = true; 	break;
		}
	}
	
	bool m_super = false;
	bool m_alt = false;
	bool m_control = false;
	bool m_shift = false;
	int key;
	
};
