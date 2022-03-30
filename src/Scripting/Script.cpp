#include <pch.h>
#include "Script.h"

#include <imgui.h>
#include <TextEditor.h>



Script::Script(){
	textEditor = new TextEditor();
	textEditor->SetImGuiChildIgnored(true);
	textEditor->SetHandleKeyboardInputs(true);
	auto language = TextEditor::LanguageDefinition::Lua();
	textEditor->SetLanguageDefinition(language);
}

Script::~Script(){
	delete textEditor;
}


void Script::editor(ImVec2 size_arg){
	
	ImGui::BeginChild("ScriptEditor", size_arg, false, ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar()){
		if (ImGui::BeginMenu("File")){
			if (ImGui::MenuItem("Save")) save();
			//if (ImGui::MenuItem("Reload Saved")) reloadAndRunScript();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")){
			bool ro = textEditor->IsReadOnly();
			if (ImGui::MenuItem("Read-only mode", nullptr, &ro)) textEditor->SetReadOnly(ro);
			ImGui::Separator();
			if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && textEditor->CanUndo())) textEditor->Undo();
			if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && textEditor->CanRedo())) textEditor->Redo();
			ImGui::Separator();
			if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, textEditor->HasSelection())) textEditor->Copy();
			if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && textEditor->HasSelection())) textEditor->Cut();
			if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && textEditor->HasSelection())) textEditor->Delete();
			if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr)) textEditor->Paste();
			ImGui::Separator();
			if (ImGui::MenuItem("Select all", nullptr, nullptr)) textEditor->SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(textEditor->GetTotalLines(), 0));
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")){
			if (ImGui::MenuItem("Dark palette")) textEditor->SetPalette(TextEditor::GetDarkPalette());
			if (ImGui::MenuItem("Light palette")) textEditor->SetPalette(TextEditor::GetLightPalette());
			if (ImGui::MenuItem("Retro blue palette")) textEditor->SetPalette(TextEditor::GetRetroBluePalette());
			ImGui::EndMenu();
		}
		if(ImGui::Button("Run")) run();
		if(ImGui::Button("Stop")) stop();
		ImGui::EndMenuBar();
	}
	
	float f_separatorHeight = ImGui::GetTextLineHeight() / 2.0;
	float f_editorHeight = ImGui::GetContentRegionAvail().y - f_separatorHeight - f_consoleHeight;
	float f_width = ImGui::GetContentRegionAvail().x;
	
	ImGui::BeginChild("##TextEditor",
					  ImVec2(f_width, f_editorHeight),
					  false,
					  ImGuiWindowFlags_HorizontalScrollbar);
	textEditor->Render("TextEditor");
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
	
	for(auto& message : consoleMessages){
		switch(message.type){
			case ScriptFlag::INFO:
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 1.0, 0.5, 1.0));
				break;
			case ScriptFlag::COMPILER_ERROR:
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 0.5, 0.5, 1.0));
				break;
			case ScriptFlag::RUNTIME_ERROR:
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 0.5, 1.0));
				break;
		}
		ImGui::TextWrapped("%s", message.string);
		ImGui::PopStyleColor();
	}
	
	ImGui::EndChild();
	ImGui::PopStyleColor();
	
	ImGui::EndChild();
}




















void Script::load(){
	std::ifstream inputFileStream(scriptFilePath);
	std::stringstream buffer;
	buffer << inputFileStream.rdbuf();
	script = buffer.str();
	textEditor->SetText(script);
}

void Script::save(){
	std::ofstream outputFileStream(scriptFilePath);
	script = textEditor->GetText();
	outputFileStream << script;
	outputFileStream.close();
}




void Script::run() {
	if(isRunning()) stop();
	
	//initialize lua state and load librairies
	L = luaL_newstate();
	if(L == nullptr) return;
	luaL_openlibs(L);
	//luaopen_of(L);
	//luaopen_glm(L);
	//CoolContainer::Lua::openlib(L);
	lua_settop(L, 0); //clear stack since opening libs leaves tables on the stack
		
	clearConsole();
	clearEditorErrors();
	
	script = textEditor->GetText();
	int ret = luaL_loadstring(L, script.c_str());
	switch(ret) {
		case LUA_OK:
			addConsoleMessage("Compilation Successfull, Starting Script...", ScriptFlag::INFO);
			break;
		case LUA_ERRSYNTAX: {
			const char* luaErrorString = lua_tostring(L, -1);
			std::string errorMessage = "Syntax Error :\n" + std::string(luaErrorString);
			addConsoleMessage(errorMessage.c_str(), ScriptFlag::COMPILER_ERROR);
			handleScriptError(luaErrorString, ScriptFlag::COMPILER_ERROR);
			stop();
			return;
		}
		case LUA_ERRMEM: {
			addConsoleMessage("Lua Memory Error !", ScriptFlag::COMPILER_ERROR);
			stop();
			return;
		}
	}
	
	ret = lua_pcall(L, 0, LUA_MULTRET, 0);
	if(ret != LUA_OK) {
		const char* luaErrorString = lua_tostring(L, -1);
		std::string errorMessage = "Runtime error :\n" + std::string(luaErrorString);
		addConsoleMessage(errorMessage.c_str(), ScriptFlag::RUNTIME_ERROR);
		handleScriptError(luaErrorString, ScriptFlag::RUNTIME_ERROR);
		stop();
	}
}

bool Script::isRunning(){
	return L != nullptr;
}

void Script::stop(){
	if(L != nullptr) {
		addConsoleMessage("Exiting Script...", ScriptFlag::INFO);
		lua_close(L);
		L = nullptr;
		addConsoleMessage("Script Closed.", ScriptFlag::INFO);
	}
}


void Script::callFunction(const char* functionName){
	if(!isRunning()) return;
	lua_getglobal(L, functionName);
	if(lua_type(L, -1) != LUA_TFUNCTION) {
		lua_pop(L, 1);
		std::string errorMessage = "Script does not have function " + std::string(functionName) + "()";
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







void Script::addConsoleMessage(const char* string, ScriptFlag t){
	consoleMessages.push_back(ConsoleMessage());
	ConsoleMessage& message = consoleMessages.back();
	message.type = t;
	strcpy(message.string, string);
}

void Script::clearConsole(){
	consoleMessages.clear();
}

void Script::handleScriptError(const char* errorString, ScriptFlag t){
	
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
	textEditor->SetErrorMarkers(errorMarkers);
}

void Script::clearEditorErrors(){
	std::map<int, std::string> errorMarkers;
	textEditor->SetErrorMarkers(errorMarkers);
}







/*
 namespace CoolContainer{

	 class CoolThing{
	 public:
		 CoolThing(const char* n){
			 strcpy(name, n);
			 nameLength = strlen(name);
		 }
		 char name[128];
		 int nameLength;
		 void doAction(){ std::cout << "Called from C: I am " << name << std::endl; }
		 int getNameLength() { return nameLength; }
	 };

	 std::vector<CoolThing> coolThings = {
		 CoolThing("firstThing"),
		 CoolThing("anotherThing"),
		 CoolThing("weirdThing"),
		 CoolThing("lastThing")
	 };

	 int getThingCount(){
		 return coolThings.size();
	 }

	 CoolThing* getThing(int index){
		 return &coolThings[index];
	 }

	 CoolThing* getThingByName(const char* n){
		 for(int i = 0; i < getThingCount(); i++) if(strcmp(n, coolThings[i].name) == 0) return &coolThings[i];
		 return nullptr;
	 }

	 namespace Lua{

		 int getCount(lua_State* L){
			 lua_pushinteger(L, CoolContainer::getThingCount());
			 return 1;
		 }

		 int get(lua_State* L){
			 int index = luaL_checkinteger(L, -1);
			 luaL_argcheck(L, index >= 1 && index <= getThingCount(), -1, "getThing() array index out of range");
			 
			 CoolThing** userData = (CoolThing**)lua_newuserdata(L, sizeof(CoolThing*));
			 *userData = getThing(index-1);
			 luaL_getmetatable(L, "CoolThing.Type");
			 lua_setmetatable(L, -2);
			 
			 return 1;
		 }
	 
		 int getByName(lua_State* L){
			 const char* name = luaL_checkstring(L, -1);
			 CoolThing* thing = getThingByName(name);
			 if(thing != nullptr) {
				 CoolThing** userData = (CoolThing**)lua_newuserdata(L, sizeof(CoolThing*));
				 *userData = thing;
				 luaL_getmetatable(L, "CoolThing.Type");
				 lua_setmetatable(L, -2);
			 }else{
				 lua_pushnil(L);
			 }
			 return 1;
		 }

		 int thing_getName(lua_State* L){
			 CoolThing** userData = (CoolThing**)luaL_checkudata(L, 1, "CoolThing.Type");
			 CoolThing* thing = *userData;
			 luaL_argcheck(L, thing != nullptr, -1, "GetThingName() thing is nullptr");
			 lua_pushstring(L, thing->name);
			 return 1;
		 }

		 int thing_doAction(lua_State* L){
			 CoolThing** userData = (CoolThing**)luaL_checkudata(L, 1, "CoolThing.Type");
			 CoolThing* thing = *userData;
			 luaL_argcheck(L, thing != nullptr, -1, "doThingAction() thing is nullptr");
			 thing->doAction();
			 return 0;
		 }
	 
		 int thing_getNameLength(lua_State* L){
			 CoolThing** userData = (CoolThing**)luaL_checkudata(L, 1, "CoolThing.Type");
			 CoolThing* thing = *userData;
			 luaL_argcheck(L, thing != nullptr, -1, "doThingAction() thing is nullptr");
			 lua_pushinteger(L, thing->getNameLength());
			 return 1;
		 }

		 int thing_toString(lua_State* L){
			 CoolThing** userData = (CoolThing**)luaL_checkudata(L, 1, "CoolThing.Type");
			 CoolThing* thing = *userData;
			 luaL_argcheck(L, thing!= nullptr, -1, "toString() thing is nullptr");
			 lua_pushfstring(L, "Thing with name %s of length %d", thing->name, thing->getNameLength());
			 return 1;
		 }

		 int openlib(lua_State* L){
			 
			 //metatable for CoolThing
			 luaL_newmetatable(L, "CoolThing.Type");
			 
			 //index table for CoolThing.Type Metatable
			 lua_newtable(L);
			 
			 lua_pushcfunction(L, thing_getName);
			 lua_setfield(L, -2, "getName");
			 lua_pushcfunction(L, thing_doAction);
			 lua_setfield(L, -2, "doAction");
			 lua_pushcfunction(L, thing_getNameLength);
			 lua_setfield(L, -2, "getNameLength");
			 
			 //set index table key
			 lua_setfield(L, -2, "__index");
		 
			 //set tostring function key
			 lua_pushcfunction(L, thing_toString);
			 lua_setfield(L, -2, "__tostring");
			 
			 //pop the CoolThing metatable off the stack
			 lua_pop(L, 1);
			 
			 //declare library table
			 lua_newtable(L);
			 
			 //declare library functions
			 lua_pushcfunction(L, getCount);
			 lua_setfield(L, -2, "getCount");
			 lua_pushcfunction(L, get);
			 lua_setfield(L, -2, "get");
			 lua_pushcfunction(L, getByName);
			 lua_setfield(L, -2, "getByName");
			 
			 //set library name
			 lua_setglobal(L, "Things");
			 
			 return 1;
		 }
	 }


 };
 */
