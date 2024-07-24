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

	const char* getString(){<
		std::string output;
		if(m_control){
		#if defined(STACATO_MACOS)
			output += "Cmd ";
		#else
			output += "Ctrl ";
		#endif
		}
		if(m_alt) output += "Alt ";
		if(m_shift) output += "Shift ";
		output += imGuiKeyToString(key);

		static char cstr[64];
		strcpy(cstr, output.c_str());
		return cstr;
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

	std::string imGuiKeyToString(ImGuiKey k){
		switch(k){
			case ImGuiKey_0: return "0";
			case ImGuiKey_1: return "1";
			case ImGuiKey_2: return "2";
			case ImGuiKey_3: return "3";
			case ImGuiKey_4: return "4";
			case ImGuiKey_5: return "5";
			case ImGuiKey_6: return "6";
			case ImGuiKey_7: return "7";
			case ImGuiKey_8: return "8";
			case ImGuiKey_9: return "9";
			case ImGuiKey_A: return "A";
			case ImGuiKey_B: return "B";
			case ImGuiKey_C: return "C";
			case ImGuiKey_D: return "D";
			case ImGuiKey_E: return "E";
			case ImGuiKey_F: return "F";
			case ImGuiKey_G: return "G";
			case ImGuiKey_H: return "H";
			case ImGuiKey_I: return "I";
			case ImGuiKey_J: return "J";
			case ImGuiKey_K: return "K";
			case ImGuiKey_L: return "L";
			case ImGuiKey_M: return "M";
			case ImGuiKey_N: return "N";
			case ImGuiKey_O: return "O";
			case ImGuiKey_P: return "P";
			case ImGuiKey_Q: return "Q";
			case ImGuiKey_R: return "R";
			case ImGuiKey_S: return "S";
			case ImGuiKey_T: return "T";
			case ImGuiKey_U: return "U";
			case ImGuiKey_V: return "V";
			case ImGuiKey_W: return "W";
			case ImGuiKey_X: return "X";
			case ImGuiKey_Y: return "Y";
			case ImGuiKey_Z: return "Z";
			default: return "???";
		}
	}
		
	bool m_alt = false;
	bool m_control = false;
	bool m_shift = false;
	ImGuiKey key;
	
};
