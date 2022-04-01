#pragma once

#include <ofxLua.h>
#include <imgui.h>


class TextEditor;

class Script{
public:
	
	Script();
	~Script();
	
	void editor(ImVec2 size_arg);
	
	bool load(const char* filePath);
	void save(const char* filePath);
	void reloadSaved();
	
	bool compile();
	void run();
	bool isRunning();
	void stop();
	
	bool checkHasFunction(const char* functionName);
	void callFunction(const char* functionName);
	
	typedef void(*LoadLibrariesCallback)(lua_State* L);
	LoadLibrariesCallback loadLibrairies = nullptr;
	void setLoadLibrairiesCallback(LoadLibrariesCallback cb){ loadLibrairies = cb; }
	
private:
	
	TextEditor* textEditor;
	lua_State* L = NULL;
	
	std::string scriptFilePath;
	
	float f_consoleHeight = 150.0;
	
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






		

