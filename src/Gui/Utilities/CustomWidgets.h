#pragma once

struct ImVec2;
struct ImVec4;

void verticalProgressBar(float fraction, const ImVec2& size_arg);

bool buttonCross(const char* id, float size = 0);


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

namespace BackgroundText{
	void draw(const char* text);
	void draw(const char* text, ImVec2 size);
	void draw(const char* text, ImVec4 backgroundColor);
	void draw(const char* text, ImVec2 size, ImVec4 backgroundColor);
	void draw(const char* text, ImVec4 backgroundColor, ImVec4 textColor);
	void draw(const char* text, ImVec2 size, ImVec4 backgroundColor, ImVec4 textColor);
};
