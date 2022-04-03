#pragma once

class ofFbo;
class TextEditor;

#include <string>
#include <vector>

#include <ofxLua.h>
#include <imgui.h>

class ofApplet {
public:
	
	ofApplet();
	~ofApplet();
	
	void canvas(ImVec2 size_arg = ImVec2(0,0), float rounding = 0.0);
	void editor(const char* windowName);
	
private:
		
	bool b_isSetup = false;
	void setupScript();
	void loadScript();
	void saveScript();
	void runScript();
	void reloadAndRunScript();
	TextEditor* textEditor;
	const char* scriptFilePath = "scripts/graphicsExample.lua";
	std::string script;
	ofxLua lua;
	
	void resizeFrameBuffer(int width, int height);
	ofFbo* frameBuffer;
	int frameBufferWidth = 0;
	int frameBufferHeight = 0;
	
	int mousePosX = 0;
	int mousePosY = 0;
	bool b_mousePressed = false;
	
	lua_State* L = NULL;
	void initializeScript();
	bool isScriptValid();
	void doScriptString(std::string& str);
	void callScriptSetup();
	void callScriptUpdate();
	void callScriptExit();
	void stopScript();
	
	float f_consoleHeight;
	
	enum class ScriptFlag{
		INFO,
		COMPILER_ERROR,
		RUNTIME_ERROR
	};
	struct ConsoleMessage{
		ScriptFlag type;
		char string[512];
	};
	std::vector<ConsoleMessage> consoleMessages;
	void addConsoleMessage(const char* string, ScriptFlag t);
	void clearConsole();
	
	struct EditorError{
		ScriptFlag type;
		char string[512];
	};
	std::vector<EditorError> editorErrors;
	void handleScriptError(const char* string, ScriptFlag t);
	void clearEditorErrors();
};
