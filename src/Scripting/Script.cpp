#include <pch.h>
#include "Script.h"

#include <fstream>

#include <imgui.h>
#include <TextEditor.h>

#include "Gui/Assets/Fonts.h"

#include "LoggingLibrary.h"

LuaScript::LuaScript(const char* n){
	name = n;
	textEditor.SetImGuiChildIgnored(true);
	textEditor.SetHandleKeyboardInputs(true);
	auto language = TextEditor::LanguageDefinition::Lua();
	textEditor.SetLanguageDefinition(language);
	textEditor.SetPalette(TextEditor::GetDarkPalette());
}


void LuaScript::editor(ImVec2 size_arg){
		
	float f_separatorHeight = ImGui::GetTextLineHeight() / 2.0;
	float f_editorHeight = ImGui::GetContentRegionAvail().y - f_separatorHeight - f_consoleHeight;
	float f_width = ImGui::GetContentRegionAvail().x;
	
	ImGui::BeginChild("##TextEditor",
					  ImVec2(f_width, f_editorHeight),
					  false,
					  ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::PushFont(Fonts::mono14);
	textEditor.Render("TextEditor");
	ImGui::PopFont();
	ImGui::EndChild();
	
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::InvisibleButton("##Separator", ImVec2(f_width, f_separatorHeight));
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	float f_middle = (max.y + min.y) / 2.0;
	ImGui::GetWindowDrawList()->AddLine(ImVec2(min.x, f_middle), ImVec2(max.x, f_middle), ImColor(1.0f, 1.0f, 1.0f, 0.5f), ImGui::GetTextLineHeight() / 8.0);
	if (ImGui::IsItemActive()) f_consoleHeight -= ImGui::GetIO().MouseDelta.y;
	f_consoleHeight = std::max(f_consoleHeight, ImGui::GetTextLineHeight() * 2.0f);
	if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
	ImGui::PopStyleVar();
	
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::BeginChild("##console", ImGui::GetContentRegionAvail(), false);
	ImGui::PopStyleColor();
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(0,0));
	for(auto& message : consoleMessages){

		switch(message.type){
			case ScriptFlag::INFO:
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 1.0, 0.5, 1.0));
				break;
			case ScriptFlag::WARNING:
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0f, 0.0, 1.0));
				break;
			case ScriptFlag::COMPILER_ERROR:
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 0.4, 0.0, 1.0));
				break;
			case ScriptFlag::RUNTIME_ERROR:
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 0.0, 0.0, 1.0));
				break;
		}
		ImGui::TextWrapped("%s", message.string);
		ImGui::PopStyleColor();
	}
	ImGui::PopStyleVar();
	
	ImGui::SetScrollHereY(1.0f);
	
	ImGui::EndChild();
}




















bool LuaScript::load(const char* filePath){
	scriptFilePath = filePath;
	if(!std::filesystem::exists(std::filesystem::path(filePath))) return false;
	std::ifstream inputFileStream(scriptFilePath);
	std::stringstream buffer;
	buffer << inputFileStream.rdbuf();
	textEditor.SetText(buffer.str());
	return true;
}

void LuaScript::load(std::string& script){
	scriptFilePath = "";
	textEditor.SetText(script);
}

void LuaScript::save(const char* filePath){
	std::ofstream outputFileStream(filePath);
	outputFileStream << textEditor.GetText();
	outputFileStream.close();
}

std::string LuaScript::getScriptText(){
	std::string txt = textEditor.GetText();
	//textEditor appends a newline at each file end for some reason
	txt.erase(txt.end() - 1, txt.end());
	return txt;
}

void LuaScript::reloadSaved(){
	if(!std::filesystem::exists(std::filesystem::path(scriptFilePath))) return;
	std::ifstream inputFileStream(scriptFilePath);
	std::stringstream buffer;
	buffer << inputFileStream.rdbuf();
	textEditor.SetText(buffer.str());
}


bool LuaScript::compile(bool hideSuccessMessage){
	if(isRunning()) stop();
	
	//initialize lua state and load librairies
	L = luaL_newstate();
	if(L == nullptr) return false;
	luaL_openlibs(L);
	Scripting::LogLibrary::openLib(L, this);
	loadLibCallback(L);
	lua_settop(L, 0); //clear stack since opening libs leaves tables on the stack
	
	//reset console
	clearConsole();
	clearEditorErrors();
	
	//compile script string
	std::string script = textEditor.GetText();
	int ret = luaL_loadstring(L, script.c_str());
	switch(ret) {
		case LUA_OK:
			if(!hideSuccessMessage) addConsoleMessage("Script compiled successfully !", ScriptFlag::INFO);
			return true;
		case LUA_ERRSYNTAX: {
			const char* luaErrorString = lua_tostring(L, -1);
			std::string errorMessage = "Script compilation failed : Syntax Error :\n" + std::string(luaErrorString);
			addConsoleMessage(errorMessage.c_str(), ScriptFlag::COMPILER_ERROR);
			handleScriptError(luaErrorString, ScriptFlag::COMPILER_ERROR);
			stop();
			return false;
		}
		case LUA_ERRMEM: {
			addConsoleMessage("Script compilation failed : Lua Memory Error !", ScriptFlag::COMPILER_ERROR);
			stop();
			return false;
		}
	}
}

void LuaScript::compileAndRun() {
	if(!compile(true)) return;
	addConsoleMessage("Starting Script...", ScriptFlag::INFO);
	if(lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
		const char* luaErrorString = lua_tostring(L, -1);
		std::string errorMessage = "Runtime error :\n" + std::string(luaErrorString);
		addConsoleMessage(errorMessage.c_str(), ScriptFlag::RUNTIME_ERROR);
		handleScriptError(luaErrorString, ScriptFlag::RUNTIME_ERROR);
		stop();
	}
}

bool LuaScript::isRunning(){
	return L != nullptr;
}

void LuaScript::stop(){
	if(L != nullptr) {
		lua_close(L);
		L = nullptr;
		addConsoleMessage("Script Stopped", ScriptFlag::INFO);
	}
}


bool LuaScript::checkHasFunction(const char* functionName){
	if(!isRunning()) return false;
	lua_getglobal(L, functionName);
	bool b_hasFunction = lua_type(L, -1) == LUA_TFUNCTION;
	lua_pop(L, 1);
	return b_hasFunction;
}

void LuaScript::callFunction(const char* functionName){
	if(!isRunning()) return;
	lua_getglobal(L, functionName);
	if(lua_type(L, -1) != LUA_TFUNCTION) {
		std::string errorMessage = "Script does not contain " + std::string(functionName) + "() function";
		addConsoleMessage(errorMessage.c_str(), ScriptFlag::RUNTIME_ERROR);
		stop();
		return;
	}
	int result = lua_pcall(L, 0, 0, 0);
	if(result != LUA_OK) {
		const char* luaErrorString = lua_tostring(L, -1);
		std::string errorMessage = "Runtime error in function" + std::string(functionName) + "() :\n" + std::string(luaErrorString);
		addConsoleMessage(errorMessage.c_str(), ScriptFlag::RUNTIME_ERROR);
		handleScriptError(luaErrorString, ScriptFlag::RUNTIME_ERROR);
		stop();
	}
}





void LuaScript::logInfo(const char* message){
	addConsoleMessage(message, ScriptFlag::INFO);
}

void LuaScript::logWarning(const char* message){
	addConsoleMessage(message, ScriptFlag::WARNING);
}


void LuaScript::addConsoleMessage(const char* string, ScriptFlag t){
	consoleMessages.push_back(ConsoleMessage());
	ConsoleMessage& message = consoleMessages.back();
	message.type = t;
	strcpy(message.string, string);
}

void LuaScript::clearConsole(){
	consoleMessages.clear();
}

void LuaScript::handleScriptError(const char* errorString, ScriptFlag t){
	
	size_t stringLength = strlen(errorString);
	int lineStringIndex = -1;
	for(int i = 0; i < stringLength; i++){
		if(errorString[i] == ':' && i < stringLength - 1){
			lineStringIndex = i+1;
			break;
		}
	}
	if(lineStringIndex == -1) return;
	
	int lineStringEndIndex = -1;
	for(int i = lineStringIndex; i < stringLength; i++){
		if(errorString[i] == ':'){
			lineStringEndIndex = i;
			break;
		}
	}
	if(lineStringEndIndex == -1) return;
	
	size_t lineStringLength = lineStringEndIndex - lineStringIndex;
	char lineString[lineStringLength + 1];
	strncpy(lineString, errorString + lineStringIndex, lineStringLength);
	lineString[lineStringLength] = 0;
	int lineNumber = strtol(lineString, nullptr, 10);
	
	std::map<int, std::string> errorMarkers = {{lineNumber, std::string(errorString + lineStringEndIndex + 2)}};
	textEditor.SetErrorMarkers(errorMarkers);
}

void LuaScript::clearEditorErrors(){
	std::map<int, std::string> errorMarkers;
	textEditor.SetErrorMarkers(errorMarkers);
}
