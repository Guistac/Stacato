#pragma once

#include <imgui.h>
#include <imgui_internal.h>

void verticalProgressBar(float fraction, const ImVec2& size_arg);

class ToggleSwitch{
public:
	
	ToggleSwitch();

	bool draw(const char* ID, bool& data, const char* option1, const char* option2, ImVec2 size);
	
	double toggleTime_seconds = 0.0;
	const double travelTime_seconds = 0.1;
};

//—————————————————————————————————————
//			Pictogram Buttons
//—————————————————————————————————————

bool buttonCross(const char* id, float size = 0);

bool buttonPlay(const char* id, float size = 0);
bool buttonPause(const char* id, float size = 0);
bool buttonStop(const char* id, float size = 0);
bool buttonSTOP(const char* id, float size = 0);

bool buttonArrowLeft(const char* id, float size = 0);
bool buttonArrowRight(const char* id, float size = 0);

bool buttonArrowRightStop(const char* id, float size = 0);
bool buttonArrowLeftStop(const char* id, float size = 0);
bool buttonArrowDownStop(const char* id, float size = 0);

namespace ListManagerWidget{
	enum class Interaction{
		NONE,
		MOVE_UP,
		MOVE_DOWN,
		DELETE
	};
	Interaction draw(bool disableMoveUp, bool disableMoveDown, const char* ID = "", float buttonHeight = 0.0);
};

namespace UpDownButtons{
	enum class Interaction{
		NONE,
		UP,
		DOWN
	};
	Interaction draw(const char* ID = "", float butonSize = 0.0, bool disableUp = false, bool disableDown = false);
};

bool nextButton(const char* ID = "", float buttonSize = 0.0, bool withStop = false, ImDrawFlags drawFlags = ImDrawFlags_None);
bool previousButton(const char* ID = "", float buttonSize = 0.0, bool withStop = false, ImDrawFlags drawFlags = ImDrawFlags_None);

//—————————————————————————————————————
//			Background Text
//—————————————————————————————————————

void backgroundText(const char* text);
void backgroundText(const char* text, ImVec2 size);
void backgroundText(const char* text, ImVec4 backgroundColor);
void backgroundText(const char* text, ImVec2 size, ImVec4 backgroundColor);
void backgroundText(const char* text, ImVec4 backgroundColor, ImVec4 textColor);
void backgroundText(const char* text, ImVec2 size, ImVec4 backgroundColor, ImVec4 textColor);

//—————————————————————————————————————
//			Time Entry
//—————————————————————————————————————

bool timeEntryWidgetSeconds(const char* ID, float height, double& time_seconds);
bool timeEntryWidgetMicroseconds(const char* ID, float height, long long int time_microseconds);
