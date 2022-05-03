#include "CommandZ.h"

#include <imgui.h>

namespace ActionHistory{

	std::vector<std::shared_ptr<Action>> history;
	size_t undoableActionCount = 0;

	void push(std::shared_ptr<Action> action){
		if(undoableActionCount < history.size()) {
			history.erase(history.begin() + undoableActionCount, history.end());
		}
		history.push_back(action);
		undoableActionCount++;
	}

	void undo(){
		undoableActionCount--;
		history[undoableActionCount]->undo();
	}

	void redo(){
		history[undoableActionCount]->redo();
		undoableActionCount++;
	}


	bool canUndo(){
		return undoableActionCount > 0;
	}

	bool canRedo(){
		return undoableActionCount < history.size();
	}

	std::vector<std::shared_ptr<Action>>& get(){
		return history;
	}

	size_t getTopIndex(){
		return undoableActionCount;
	}
	
};



class DoubleParameter : public std::enable_shared_from_this<DoubleParameter>{
public:
	
	class EditAction : public Action{
	public:
		
		std::shared_ptr<DoubleParameter> parameter;
		double value, previousValue;
		
		void set(std::shared_ptr<DoubleParameter> parameter_){
			parameter = parameter_;
			value = parameter->value;
			previousValue = parameter->previousValue;
			name = "Changed " + std::string(parameter->name) + " from " + std::to_string(previousValue) + " to " + std::to_string(value);
		}
		
		virtual void undo(){
			parameter->value = previousValue;
			parameter->previousValue = previousValue;
			parameter->b_changed = true;
		}
		virtual void redo(){
			parameter->value = value;
			parameter->previousValue = value;
			parameter->b_changed = true;
		}
	};
	
	DoubleParameter(double value_, const char* name_) : value(value_), previousValue(value_), name(name_){}
	
	void gui(){
		ImGui::SetNextItemWidth(200.0);
		ImGui::InputDouble(name, &value);
		if(ImGui::IsItemDeactivatedAfterEdit() && previousValue != value){
			Logger::warn("{} Modified from {} to {}", name, previousValue, value);
			
			std::shared_ptr<EditAction> action = std::make_shared<EditAction>();
			std::shared_ptr<DoubleParameter> thisParameter = shared_from_this();
			action->set(thisParameter);
			ActionHistory::push(action);
			
			previousValue = value;
			b_changed = true;
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
	const char* name = "default parameter name";
	
};

void testUndoHistory(){
	
	static std::shared_ptr<DoubleParameter> param1 = std::make_shared<DoubleParameter>(1.0, "parameter 1");
	static std::shared_ptr<DoubleParameter> param2 = std::make_shared<DoubleParameter>(2.0, "parameter 2");
	static std::shared_ptr<DoubleParameter> param3 = std::make_shared<DoubleParameter>(3.0, "parameter 3");
	static std::shared_ptr<DoubleParameter> param4 = std::make_shared<DoubleParameter>(4.0, "parameter 4");
	static std::shared_ptr<DoubleParameter> param5 = std::make_shared<DoubleParameter>(5.0, "parameter 5");
	
	static const char* lastModifiedParameterName = "";
	
	ImGui::BeginDisabled(!ActionHistory::canUndo());
	if(ImGui::Button("Undo")) ActionHistory::undo();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!ActionHistory::canRedo());
	if(ImGui::Button("Redo")) ActionHistory::redo();
	ImGui::EndDisabled();
	
	param1->gui();
	if(param1->changed()) lastModifiedParameterName = param1->name;
	param2->gui();
	if(param2->changed()) lastModifiedParameterName = param2->name;
	param3->gui();
	if(param3->changed()) lastModifiedParameterName = param3->name;
	param4->gui();
	if(param4->changed()) lastModifiedParameterName = param4->name;
	param5->gui();
	if(param5->changed()) lastModifiedParameterName = param5->name;
	
	if(ImGui::BeginListBox("History")){
		
		int topIndex = ActionHistory::getTopIndex();
		std::vector<std::shared_ptr<Action>>& history = ActionHistory::get();
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
