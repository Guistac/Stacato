#pragma once

#include <ofxLua.h>
#include <imgui.h>


class TextEditor;

class Script{
public:
	
	Script();
	~Script();
	
	void editor(ImVec2 size_arg);
	
	void load();
	void save();
	
	void run();
	bool isRunning();
	void stop();
	
	void callFunction(const char* functionName);
	
	
private:
	
	TextEditor* textEditor;
	ofxLua lua;
	
	std::string scriptFilePath;
	std::string script;
	
	lua_State* L = NULL;
	
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






		

