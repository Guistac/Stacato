#include "CommandZ.h"

#include <imgui.h>

namespace CommandHistory{

	std::vector<std::shared_ptr<Command>> history;
	size_t undoableCommandCount = 0;

	void push(std::shared_ptr<Command> command){
		if(undoableCommandCount < history.size()) history.erase(history.begin() + undoableCommandCount, history.end());
		history.push_back(command);
		undoableCommandCount++;
	}

	void undo(){
		undoableCommandCount--;
		history[undoableCommandCount]->undo();
	}

	void redo(){
		history[undoableCommandCount]->execute();
		undoableCommandCount++;
	}


	bool canUndo(){
		return undoableCommandCount > 0;
	}

	bool canRedo(){
		return undoableCommandCount < history.size();
	}

	std::vector<std::shared_ptr<Command>>& get(){
		return history;
	}

	size_t getTopIndex(){
		void (*test)() = nullptr;
		return undoableCommandCount;
	}
	
};






class DoubleParameter : public std::enable_shared_from_this<DoubleParameter>{
public:
	
	class EditValueCommand : public Command{
	public:
		std::shared_ptr<DoubleParameter> parameter;
		double newValue;
		double previousValue;
		EditValueCommand(std::shared_ptr<DoubleParameter> parameter_){
			parameter = parameter_;
			newValue = parameter->value;
			previousValue = parameter->previousValue;
			name = "Changed " + std::string(parameter->name) + " from " + std::to_string(previousValue) + " to " + std::to_string(newValue);
		}
		virtual void execute(){
			parameter->value = newValue;
			parameter->previousValue = newValue;
			parameter->b_changed = true;
		}
		virtual void undo(){
			parameter->value = previousValue;
			parameter->previousValue = previousValue;
			parameter->b_changed = true;
		}
	};
	
	DoubleParameter(double value_, std::string name_) : value(value_), previousValue(value_), name(name_){}
	
	void gui(){
		ImGui::SetNextItemWidth(100.0);
		ImGui::InputDouble(name.c_str(), &value);
		if(ImGui::IsItemDeactivatedAfterEdit() && previousValue != value){
			
			//=========Command Invoker=========
			std::shared_ptr<EditValueCommand> action = std::make_shared<EditValueCommand>(shared_from_this());
			action->execute();
			CommandHistory::push(action);
			//=================================
			
		}
	}
	
	bool changed(){
		bool output = b_changed;
		b_changed = false;
		return output;
	}
	
	bool b_changed = false;
	double previousValue = 0.0;
	double value = 0.0;
	std::string name = "default parameter name";
	
};



class AddRemoveParameterCommand : public Command{
public:
	std::shared_ptr<DoubleParameter> parameter;
	std::vector<std::shared_ptr<DoubleParameter>>* list;
	int index;
	bool b_add;
	
	AddRemoveParameterCommand(std::shared_ptr<DoubleParameter> parameter_, std::vector<std::shared_ptr<DoubleParameter>>* list_, int index_, bool add_){
		parameter = parameter_;
		list = list_;
		b_add = add_;
		index = index_;
		if(b_add) name = "Added Parameter " + parameter->name + " at index " + std::to_string(index);
		else name = "Removed Parameter " + parameter->name + " at index " + std::to_string(index);
	}
	
	void add(){
		list->insert(list->begin() + index, parameter);
	}
	
	void remove(){
		list->erase(list->begin() + index);
	}
	
	virtual void execute(){
		if(b_add) add();
		else remove();
	}
	
	virtual void undo(){
		if(b_add) remove();
		else add();
	}
};






void testUndoHistory(){
	
	static std::vector<std::shared_ptr<DoubleParameter>> parameters = {
		std::make_shared<DoubleParameter>(1.0, "parameter 0"),
		std::make_shared<DoubleParameter>(2.0, "parameter 1"),
		std::make_shared<DoubleParameter>(3.0, "parameter 2"),
		std::make_shared<DoubleParameter>(4.0, "parameter 3"),
		std::make_shared<DoubleParameter>(5.0, "parameter 4")
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
		parameter->gui();
		if(parameter->changed()) lastModifiedParameterName = parameter->name.c_str();
		ImGui::PopID();
	}
	
	if(deletedParameterIndex >= 0){
		auto& deletedParameter = parameters[deletedParameterIndex];
		auto command = std::make_shared<AddRemoveParameterCommand>(deletedParameter, &parameters, deletedParameterIndex, false);
		command->execute();
		CommandHistory::push(command);
	}
	
	if(ImGui::Button("+")){
		std::string name = "parameter " + std::to_string(parameters.size());
		auto newParameter = std::make_shared<DoubleParameter>(0.0, name);
		auto command = std::make_shared<AddRemoveParameterCommand>(newParameter, &parameters, parameters.size(), true);
		command->execute();
		CommandHistory::push(command);
	}
	
	if(ImGui::BeginListBox("History")){
		
		int topIndex = CommandHistory::getTopIndex();
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
