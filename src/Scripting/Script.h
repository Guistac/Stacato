#pragma once

#include <ofxLua.h>
#include <imgui.h>
#include <TextEditor.h>


class Script{
public:
	
	Script();
	
	void editor(ImVec2 size_arg);
	
	bool load(const char* filePath);
	void load(std::string& script);
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
	
	TextEditor textEditor;
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






		

