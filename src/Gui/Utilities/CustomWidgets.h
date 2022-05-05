#pragma once

struct ImVec2;
struct ImVec4;

void verticalProgressBar(float fraction, const ImVec2& size_arg);

bool buttonCross(const char* id, float size = 0);

bool buttonPlay(const char* id, float size = 0);

bool buttonPause(const char* id, float size = 0);

bool buttonArrowLeft(const char* id, float size = 0);

bool buttonArrowRight(const char* id, float size = 0);

class ToggleSwitch{
public:
	
	ToggleSwitch();

	bool draw(const char* ID, bool& data, const char* option1, const char* option2, ImVec2 size);
	
	double toggleTime_seconds = 0.0;
	const double travelTime_seconds = 0.1;	
};

namespace ListManagerWidget{
	enum class Interaction{
		NONE,
		MOVE_UP,
		MOVE_DOWN,
		DELETE
	};
	Interaction draw(bool disableMoveUp, bool disableMoveDown, const char* ID = "", float buttonHeight = 0.0);
};


void backgroundText(const char* text);
void backgroundText(const char* text, ImVec2 size);
void backgroundText(const char* text, ImVec4 backgroundColor);
void backgroundText(const char* text, ImVec2 size, ImVec4 backgroundColor);
void backgroundText(const char* text, ImVec4 backgroundColor, ImVec4 textColor);
void backgroundText(const char* text, ImVec2 size, ImVec4 backgroundColor, ImVec4 textColor);
