#include "CommandZ.h"

#include <imgui.h>

void testUndoHistory(){
	
	static std::vector<std::shared_ptr<Parameter<double>>> parameters = {
		std::make_shared<Parameter<double>>(1.0, "parameter 0"),
		std::make_shared<Parameter<double>>(2.0, "parameter 1"),
		std::make_shared<Parameter<double>>(3.0, "parameter 2"),
		std::make_shared<Parameter<double>>(4.0, "parameter 3"),
		std::make_shared<Parameter<double>>(5.0, "parameter 4")
	};
	
	static std::vector<std::shared_ptr<Parameter<int>>> intParams = {
		std::make_shared<Parameter<int>>(1, "integer 0"),
		std::make_shared<Parameter<int>>(2, "integer 1"),
		std::make_shared<Parameter<int>>(3, "integer 2"),
		std::make_shared<Parameter<int>>(4, "integer 3")
	};
	
	static std::vector<std::shared_ptr<Parameter<bool>>> boolParams = {
		std::make_shared<Parameter<bool>>(true, "boolean 0"),
		std::make_shared<Parameter<bool>>(true, "boolean 1"),
		std::make_shared<Parameter<bool>>(true, "boolean 2"),
		std::make_shared<Parameter<bool>>(true, "boolean 3")
	};
	
	static const char* lastModifiedParameterName = "";
	
	ImGui::BeginDisabled(!CommandHistory::canUndo());
	if(ImGui::Button("Undo")) CommandHistory::undo();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!CommandHistory::canRedo());
	if(ImGui::Button("Redo")) CommandHistory::redo();
	ImGui::EndDisabled();
	
	
	int deletedParameterIndex = -1;
	
	for(int i = 0; i < parameters.size(); i++){
		auto& parameter = parameters[i];
		ImGui::PushID(i);
		if(ImGui::Button("X")) deletedParameterIndex = i;
		ImGui::SameLine();
		parameter->gui(100);
		if(parameter->changed()) lastModifiedParameterName = parameter->name.c_str();
		ImGui::PopID();
	}
	
	for(int i = 0; i < intParams.size(); i++){
		auto& parameter = intParams[i];
		ImGui::PushID(i);
		parameter->gui(100);
		if(parameter->changed()) lastModifiedParameterName = parameter->name.c_str();
		ImGui::PopID();
	}
	
	for(int i = 0; i < boolParams.size(); i++){
		auto& parameter = boolParams[i];
		ImGui::PushID(i);
		parameter->gui(100);
		if(parameter->changed()) lastModifiedParameterName = parameter->name.c_str();
		ImGui::PopID();
	}
	
	
	if(deletedParameterIndex >= 0){
		auto& deletedParameter = parameters[deletedParameterIndex];
		auto command = std::make_shared<RemoveParameterFromListCommand<double>>(deletedParameter, &parameters, deletedParameterIndex);
		command->execute();
		CommandHistory::push(command);
	}
	
	
	if(ImGui::Button("+")){
		std::string name = "parameter " + std::to_string(parameters.size());
		auto newParameter = std::make_shared<Parameter<double>>(0.0, name);
		auto command = std::make_shared<AddParameterToListCommand<double>>(newParameter, &parameters, parameters.size());
		command->execute();
		CommandHistory::push(command);
	}
	
	if(ImGui::BeginListBox("History")){
		
		int topIndex = CommandHistory::getUndoableCommandCount();
		std::vector<std::shared_ptr<Command>>& history = CommandHistory::get();
		bool b_disabled = false;
		
		for(int i = 0; i < history.size(); i++){
			if(i == topIndex){
				ImGui::BeginDisabled();
				b_disabled = true;
			}
			ImGui::Selectable(history[i]->name.c_str());
		}
		if(b_disabled) ImGui::EndDisabled();
		
		ImGui::EndListBox();
	}
	
	ImGui::Text("Last Changed: %s", lastModifiedParameterName);
	
}
 
