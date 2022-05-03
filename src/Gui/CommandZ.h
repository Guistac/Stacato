#pragma once

#include <imgui.h>

class Command{
public:
	virtual void execute() = 0;
	virtual void undo() = 0;
	std::string name;
};

namespace CommandHistory{

	inline std::vector<std::shared_ptr<Command>>& get(){
		static std::vector<std::shared_ptr<Command>> history;
		return history;
	}

	inline size_t& getUndoableCommandCount(){
		static size_t undoableCommandCount = 0;
		return undoableCommandCount;
	}

	inline void push(std::shared_ptr<Command> command){
		if(getUndoableCommandCount() < get().size()) get().erase(get().begin() + getUndoableCommandCount(), get().end());
		get().push_back(command);
		getUndoableCommandCount()++;
	}

	inline void undo(){
		getUndoableCommandCount()--;
		get()[getUndoableCommandCount()]->undo();
	}

	inline void redo(){
		get()[getUndoableCommandCount()]->execute();
		getUndoableCommandCount()++;
	}

	inline bool canUndo(){
		return getUndoableCommandCount() > 0;
	}

	inline bool canRedo(){
		return getUndoableCommandCount() < get().size();
	}

};





template<typename T>
class Parameter : public std::enable_shared_from_this<Parameter<T>>{
public:
	
	T displayValue;
	T value;
	std::string name;
	bool b_changed;
	
	Parameter(T value_, std::string name_) {
		displayValue = value_;
		value = value_;
		name = name_;
		b_changed = false;
	}
	
	
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<Parameter<T>> parameter;
		T newValue;
		T previousValue;
		EditCommand(std::shared_ptr<Parameter<T>> parameter_){
			parameter = parameter_;
			newValue = parameter->displayValue;
			previousValue = parameter->value;
			name = "Changed " + std::string(parameter->name) + " from " + std::to_string(previousValue) + " to " + std::to_string(newValue);
		}
		virtual void execute(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
			parameter->b_changed = true;
		}
		virtual void undo(){
			parameter->value = previousValue;
			parameter->displayValue = previousValue;
			parameter->b_changed = true;
		}
	};
	
	void gui(float width = 0.0){
		if(width > 0.0) ImGui::SetNextItemWidth(width);
		inputField();
		if(ImGui::IsItemDeactivatedAfterEdit() && value != displayValue){
			//=========Command Invoker=========
			std::shared_ptr<Parameter<T>> thisParameter = this->shared_from_this();
			std::shared_ptr<EditCommand> action = std::make_shared<EditCommand>(thisParameter);
			action->execute();
			CommandHistory::push(action);
			//=================================
		}
	}
	
	void inputField();
	
	bool changed(){
		bool output = b_changed;
		b_changed = false;
		return output;
	}
	
};

template<>
inline void Parameter<float>::inputField(){
	ImGui::InputFloat(name.c_str(), &displayValue);
}

template<>
inline void Parameter<double>::inputField(){
	ImGui::InputDouble(name.c_str(), &displayValue);
}

template<>
inline void Parameter<int>::inputField(){
	ImGui::InputInt(name.c_str(), &displayValue);
}

template<>
inline void Parameter<bool>::inputField(){
	ImGui::Checkbox(name.c_str(), &displayValue);
}

template<>
inline void Parameter<uint8_t>::inputField(){
	ImGui::InputScalar(name.c_str(), ImGuiDataType_U8, &displayValue);
}

template<>
inline void Parameter<int8_t>::inputField(){
	ImGui::InputScalar(name.c_str(), ImGuiDataType_S8, &displayValue);
}

template<>
inline void Parameter<uint16_t>::inputField(){
	ImGui::InputScalar(name.c_str(), ImGuiDataType_U16, &displayValue);
}

template<>
inline void Parameter<int16_t>::inputField(){
	ImGui::InputScalar(name.c_str(), ImGuiDataType_S16, &displayValue);
}


template<>
inline void Parameter<uint32_t>::inputField(){
	ImGui::InputScalar(name.c_str(), ImGuiDataType_U32, &displayValue);
}

/*
template<>
inline void NumericalParameter<int32_t>::inputField(){
	ImGui::InputScalar(name.c_str(), ImGuiDataType_S32, &displayValue);
}
*/

template<>
inline void Parameter<uint64_t>::inputField(){
	ImGui::InputScalar(name.c_str(), ImGuiDataType_U64, &displayValue);
}

template<>
inline void Parameter<int64_t>::inputField(){
	ImGui::InputScalar(name.c_str(), ImGuiDataType_S64, &displayValue);
}


template<typename T>
class AddParameterToListCommand : public Command{
public:
	std::shared_ptr<Parameter<T>> parameter;
	std::vector<std::shared_ptr<Parameter<T>>>* list;
	int index;
	
	AddParameterToListCommand(std::shared_ptr<Parameter<T>> parameter_, std::vector<std::shared_ptr<Parameter<T>>>* list_, int index_){
		parameter = parameter_;
		list = list_;
		index = index_;
		name = "Added Parameter " + parameter->name + " at index " + std::to_string(index);
	}
	
	virtual void execute(){
		list->insert(list->begin() + index, parameter);
	}
	
	virtual void undo(){
		list->erase(list->begin() + index);
	}
};

template<typename T>
class RemoveParameterFromListCommand : public Command{
public:
	std::shared_ptr<Parameter<T>> parameter;
	std::vector<std::shared_ptr<Parameter<T>>>* list;
	int index;
	
	RemoveParameterFromListCommand(std::shared_ptr<Parameter<T>> parameter_, std::vector<std::shared_ptr<Parameter<T>>>* list_, int index_){
		parameter = parameter_;
		list = list_;
		index = index_;
		name = "Removed Parameter " + parameter->name + " at index " + std::to_string(index);
	}
	
	virtual void execute(){
		list->erase(list->begin() + index);
	}
	
	virtual void undo(){
		list->insert(list->begin() + index, parameter);
	}
};














void testUndoHistory();
