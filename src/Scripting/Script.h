#pragma once

#include <lua.hpp>
#include <TextEditor.h>

class LuaScript{
public:
	
	LuaScript(const char* name);
	std::string name;
	
	void editor(ImVec2 size_arg);
	
	bool load(const char* filePath);
	void load(std::string& script);
	void save(const char* filePath);
	void reloadSaved();
	std::string getScriptText();
	
	bool compile(bool hideSuccessMessage = false);
	void compileAndRun();
	bool isRunning();
	void stop();
	
	bool checkHasFunction(const char* functionName);
	void callFunction(const char* functionName);
	
	typedef std::function<void(lua_State*)> LoadLibrariesCallback;
	LoadLibrariesCallback loadLibCallback = [](lua_State* L){};
	void setLoadLibrairiesCallback(LoadLibrariesCallback cb){ loadLibCallback = cb; }
	 
	void logInfo(const char* message);
	void logWarning(const char* message);
	void clearConsole();
	
private:
	
	TextEditor textEditor;
	lua_State* L = NULL;
	
	std::string scriptFilePath;
	
	float f_consoleHeight = 150.0;
	
	enum class ScriptFlag{
		INFO,
		WARNING,
		COMPILER_ERROR,
		RUNTIME_ERROR
	};
	
	struct ConsoleMessage{
		ScriptFlag type;
		char string[512];
	};
	std::vector<ConsoleMessage> consoleMessages;
	void addConsoleMessage(const char* string, ScriptFlag t);
	
	struct EditorError{
		ScriptFlag type;
		char string[512];
	};
	std::vector<EditorError> editorErrors;
	void handleScriptError(const char* string, ScriptFlag t);
	void clearEditorErrors();
	
};






		

