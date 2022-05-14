#pragma once

#include <tinyxml2.h>
#include <imgui.h>
#include "CommandHistory.h"

class Parameter{
public:
	
	bool b_changed;
	std::string name;
	std::string saveString;
	std::string imguiID;
	
	Parameter(std::string name_, std::string saveString_ = ""){
		b_changed = false;
		setName(name_);
		setSaveString(saveString_);
	}
	
	void setName(std::string name_){
		name = name_;
		imguiID = "##" + name; //this avoids rendering the parameter name in imgui input field
	}
	void setSaveString(std::string saveString_){
		saveString = saveString_;
	}
	
	virtual void gui() = 0;
	virtual bool save(tinyxml2::XMLElement* xml) = 0;
	virtual bool load(tinyxml2::XMLElement* xml) = 0;
	
	bool changed(){
		if(b_changed){
			b_changed = false;
			return true;
		}else return false;
	}
	
	typedef void (*Callback)(Parameter* thisParameter, void* userData);
	Callback editCallback = nullptr;
	void* editCallbackUserData = nullptr;
	void setEditCallback(Callback callback, void* userData){
		editCallback = callback;
		editCallbackUserData = userData;
	}
	
	/*
	Callback undoCallback = nullptr;
	Callback redoCallback = nullptr;
	void* undoCallbackUserData = nullptr;
	void* redoCallbackUserData = nullptr;
	
	void setUndoCallback(Callback callback, void* userData){
		undoCallback = callback;
		undoCallbackUserData = userData;
	}
	void setRedoCallback(Callback callback, void* userData){
		redoCallback = callback;
		redoCallbackUserData = userData;
	}
	*/
	
};



//===============================================================================
//=================================== NUMBERS ===================================
//===============================================================================

#include <iostream>

template<typename T>
class NumberParameter : public Parameter, public std::enable_shared_from_this<NumberParameter<T>>{
public:
	
	T displayValue;
	T value;
	
	T stepSmall;
	T stepLarge;
	T* stepSmallPtr;
	T* stepLargePtr;
	
	Unit unit;
	std::string format;
	bool b_hasFormat;
	
	NumberParameter(T value_, std::string name_, std::string saveString_ = "", Unit unit_ = Units::None::None, T stepSmall_ = 0, T stepLarge_ = 0, std::string format_ = "") : Parameter(name_, saveString_) {
		displayValue = value_;
		value = value_;
		unit = unit_;
		setFormat(format_);
		setStepSize(stepSmall_, stepLarge_);
	}
	
	void setStepSize(T stepSmall_, T stepLarge_){
		stepSmall = stepSmall_;
		if(stepSmall == 0) stepSmallPtr = nullptr;
		else stepSmallPtr = &stepSmall;
		stepLarge = stepLarge_;
		if(stepLarge == 0) stepLargePtr = nullptr;
		else stepLargePtr = &stepLarge;
	}
	
	void setFormat(std::string format_){
		format = format_;
		b_hasFormat = format != "";
	}
	
	void inputField();
	
	const char* getFormated(const char* defaultFormat){
		static char formatString[128];
		strcpy(formatString, b_hasFormat ? format.c_str() : defaultFormat);
		if(unit != Units::None::None) sprintf(formatString + strlen(formatString), " %s", unit->abbreviated);
		return formatString;
	}
	
	const char* getFormatedReal(){
		return getFormated("%.3f");
	}

	const char* getFormatedInteger(){
		return getFormated("%i");
	}
	
	virtual void gui(){
		inputField();
		if(ImGui::IsItemDeactivatedAfterEdit() && value != displayValue){
			//=========Command Invoker=========
			std::shared_ptr<NumberParameter<T>> thisParameter = this->shared_from_this();
			CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter));
			//=================================
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml){
		xml->SetAttribute(saveString.c_str(), value);
		 return true;
	 }
	
	virtual bool load(tinyxml2::XMLElement* xml){
		using namespace tinyxml2;
		 double number;
		 XMLError result = xml->QueryDoubleAttribute(saveString.c_str(), &number);
		 if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", name);
		 value = number;
		 displayValue = number;
		 return true;
	 }
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<NumberParameter<T>> parameter;
		T newValue;
		T previousValue;
		EditCommand(std::shared_ptr<NumberParameter<T>> parameter_){
			parameter = parameter_;
			newValue = parameter->displayValue;
			previousValue = parameter->value;
			name = "Changed \'" + std::string(parameter->name) + "\' from " + std::to_string(previousValue) + " to " + std::to_string(newValue);
		}
		virtual void execute(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
			parameter->b_changed = true;
			if(parameter->editCallback) parameter->editCallback(parameter.get(), parameter->editCallbackUserData);
		}
		virtual void undo(){
			parameter->value = previousValue;
			parameter->displayValue = previousValue;
			parameter->b_changed = true;
			//if(parameter->undoCallback) parameter->undoCallback(parameter.get(), parameter->undoCallbackUserData);
		}
		virtual void redo(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
			parameter->b_changed = true;
			//if(parameter->redoCallback) parameter->redoCallback(parameter.get(), parameter->redoCallbackUserData);
		}
	};
	
};

template<>
inline void NumberParameter<float>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_Float, &displayValue, stepSmallPtr, stepLargePtr, getFormatedReal());
}

template<>
inline void NumberParameter<double>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_Double, &displayValue, stepSmallPtr, stepLargePtr, getFormatedReal());
}

template<>
inline void NumberParameter<uint8_t>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_U8, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}

template<>
inline void NumberParameter<int8_t>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_S8, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}

template<>
inline void NumberParameter<uint16_t>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_U16, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}

template<>
inline void NumberParameter<int16_t>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_S16, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}

template<>
inline void NumberParameter<uint32_t>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_U32, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}

template<>
inline void NumberParameter<int32_t>::inputField(){
	const char* format = getFormatedInteger();
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_S32, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}

template<>
inline void NumberParameter<uint64_t>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_U64, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}

template<>
inline void NumberParameter<int64_t>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_S64, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}




//===============================================================================
//=================================== VECTORS ===================================
//===============================================================================

template<typename T>
class VectorParameter : public Parameter, public std::enable_shared_from_this<VectorParameter<T>>{
public:
	
	T displayValue;
	T value;
	const char* format = nullptr;
	
	VectorParameter(T value_, std::string name_, std::string saveString_, const char* format_ = nullptr) : Parameter(name_, saveString_) {
		displayValue = value_;
		value = value_;
		format = format_;
	}
	
	void setFormat(const char* format_){
		format = format_;
	}
	
	void inputField();
	
	virtual void gui(){
		inputField();
		if(ImGui::IsItemDeactivatedAfterEdit() && value != displayValue){
			//=========Command Invoker=========
			auto thisParameter = this->shared_from_this();
			CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter));
			//=================================
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<VectorParameter<T>> parameter;
		T newValue;
		T previousValue;
		EditCommand(std::shared_ptr<VectorParameter<T>> parameter_){
			parameter = parameter_;
			newValue = parameter->displayValue;
			previousValue = parameter->value;
			name = "Changed \'" + std::string(parameter->name) + "\' from " + glm::to_string(previousValue) + " to " + glm::to_string(newValue);
		}
		virtual void execute(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
			parameter->b_changed = true;
			if(parameter->editCallback) parameter->editCallback(parameter.get(), parameter->editCallbackUserData);
		}
		virtual void undo(){
			parameter->value = previousValue;
			parameter->displayValue = previousValue;
			parameter->b_changed = true;
			//if(parameter->undoCallback) parameter->undoCallback(parameter.get(), parameter->undoCallbackUserData);
		}
		virtual void redo(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
			parameter->b_changed = true;
			//if(parameter->redoCallback) parameter->redoCallback(parameter.get(), parameter->redoCallbackUserData);
		}
	};
	
};

template<>
inline void VectorParameter<glm::vec2>::inputField(){
	ImGui::InputFloat2(imguiID.c_str(), &displayValue.x, format);
}

template<>
inline bool VectorParameter<glm::vec2>::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->InsertNewChildElement(saveString.c_str());
	element->SetAttribute("x", value.x);
	element->SetAttribute("y", value.y);
	return true;
}

template<>
inline bool VectorParameter<glm::vec2>::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->FirstChildElement(saveString.c_str());
	if(XML_SUCCESS != element->QueryAttribute("x", &value.x) ||
	   XML_SUCCESS != element->QueryAttribute("y", &value.y)){
		return Logger::warn("Could not load parameter {}", name);
	}
	displayValue = value;
	return true;
}




template<>
inline void VectorParameter<glm::vec3>::inputField(){
	ImGui::InputFloat3(imguiID.c_str(), &displayValue.x, format);
}

template<>
inline bool VectorParameter<glm::vec3>::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->InsertNewChildElement(saveString.c_str());
	element->SetAttribute("x", value.x);
	element->SetAttribute("y", value.y);
	element->SetAttribute("z", value.z);
	return true;
}

template<>
inline bool VectorParameter<glm::vec3>::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->FirstChildElement(saveString.c_str());
	if(XML_SUCCESS != element->QueryAttribute("x", &value.x) ||
	   XML_SUCCESS != element->QueryAttribute("y", &value.y) ||
	   XML_SUCCESS != element->QueryAttribute("z", &value.z)){
		return Logger::warn("Could not load parameter {}", name);
	}
	displayValue = value;
	return true;
}



template<>
inline void VectorParameter<glm::vec4>::inputField(){
	ImGui::InputFloat4(imguiID.c_str(), &displayValue.x, format);
}

template<>
inline bool VectorParameter<glm::vec4>::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->InsertNewChildElement(saveString.c_str());
	element->SetAttribute("x", value.x);
	element->SetAttribute("y", value.y);
	element->SetAttribute("z", value.z);
	element->SetAttribute("w", value.w);
	return true;
}

template<>
inline bool VectorParameter<glm::vec4>::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->FirstChildElement(saveString.c_str());
	if(XML_SUCCESS != element->QueryAttribute("x", &value.x) ||
	   XML_SUCCESS != element->QueryAttribute("y", &value.y) ||
	   XML_SUCCESS != element->QueryAttribute("z", &value.z) ||
	   XML_SUCCESS != element->QueryAttribute("w", &value.w)){
		return Logger::warn("Could not load parameter {}", name);
	}
	displayValue = value;
	return true;
}




//===============================================================================
//=================================== BOOLEANS ==================================
//===============================================================================

class BooleanParameter : public Parameter, public std::enable_shared_from_this<BooleanParameter>{
public:
	
	bool displayValue;
	bool value;
	
	BooleanParameter(bool value_, std::string name_, std::string saveString_) : Parameter(name_, saveString_) {
		displayValue = value_;
		value = value_;
	}
	
	virtual void gui(){
		ImGui::Checkbox(imguiID.c_str(), &displayValue);
		if(ImGui::IsItemDeactivatedAfterEdit() && value != displayValue){
			//=========Command Invoker=========
			std::shared_ptr<BooleanParameter> thisParameter = shared_from_this();
			CommandHistory::pushAndExecute(std::make_shared<InvertCommand>(thisParameter));
			//=================================
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml){
		xml->SetAttribute(saveString.c_str(), value);
		return true;
	}
	
	virtual bool load(tinyxml2::XMLElement* xml){
		using namespace tinyxml2;
		XMLError result = xml->QueryBoolAttribute(saveString.c_str(), &value);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", name);
		displayValue = value;
		return true;
	}
	
	class InvertCommand : public Command{
	public:
		std::shared_ptr<BooleanParameter> parameter;
		InvertCommand(std::shared_ptr<BooleanParameter> parameter_){
			parameter = parameter_;
			name = "Inverted \'" + std::string(parameter->name) + "\'";
		}
		void invert(){
			parameter->value = !parameter->value;
			parameter->displayValue = parameter->value;
			parameter->b_changed = true;
		}
		virtual void execute(){
			invert();
			if(parameter->editCallback) parameter->editCallback(parameter.get(), parameter->editCallbackUserData);
		}
		virtual void undo(){
			invert();
			//if(parameter->undoCallback) parameter->undoCallback(parameter.get(), parameter->undoCallbackUserData);
		}
		virtual void redo(){
			invert();
			//if(parameter->redoCallback) parameter->redoCallback(parameter.get(), parameter->redoCallbackUserData);
		}
	};
	
};




//===============================================================================
//=================================== STRINGS ===================================
//===============================================================================

class StringParameter : public Parameter, public std::enable_shared_from_this<StringParameter>{
public:
	
	char* displayValue;
	std::string value;
	size_t bufferSize;
	
	StringParameter(std::string value_, std::string name_, std::string saveString_, size_t bufferSize_) : Parameter(name_, saveString_){
		value = value_;
		bufferSize = bufferSize_;
		displayValue = new char[bufferSize];
		strcpy(displayValue, value_.c_str());
	}
	
	void overwrite(const char* value_){
		strcpy(displayValue, value_);
		value = value_;
	}
	
	virtual void gui(){
		ImGui::InputText(imguiID.c_str(), displayValue, bufferSize);
		if(ImGui::IsItemDeactivatedAfterEdit() && strcmp(displayValue, value.c_str()) != 0){
			//=========Command Invoker=========
			std::shared_ptr<StringParameter> thisParameter = shared_from_this();
			CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter));
			//=================================
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml){
		xml->SetAttribute(saveString.c_str(), value.c_str());
		return true;
	}
	
	virtual bool load(tinyxml2::XMLElement* xml){
		using namespace tinyxml2;
		XMLError result = xml->QueryStringAttribute(saveString.c_str(), (const char**)&displayValue);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", name);
		strcpy(displayValue, value.c_str());
		return true;
	}
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<StringParameter> parameter;
		std::string newValue;
		std::string previousValue;
		EditCommand(std::shared_ptr<StringParameter> parameter_){
			parameter = parameter_;
			newValue = parameter->displayValue;
			previousValue = parameter->value;
			name = "Changed \'" + std::string(parameter->name) + "\' from \'" + previousValue + "\' to \'" + newValue + "\'";
		}
		virtual void execute(){
			parameter->value = newValue;
			strcpy(parameter->displayValue, newValue.c_str());
			parameter->b_changed = true;
			if(parameter->editCallback) parameter->editCallback(parameter.get(), parameter->editCallbackUserData);
		}
		virtual void undo(){
			parameter->value = previousValue;
			strcpy(parameter->displayValue, previousValue.c_str());
			parameter->b_changed = true;
			//if(parameter->undoCallback) parameter->undoCallback(parameter.get(), parameter->undoCallbackUserData);
		}
		virtual void redo(){
			parameter->value = newValue;
			strcpy(parameter->displayValue, newValue.c_str());
			parameter->b_changed = true;
			//if(parameter->redoCallback) parameter->redoCallback(parameter.get(), parameter->redoCallbackUserData);
		}
	};
	
};



//===============================================================================
//================================= ENUMERATORS =================================
//===============================================================================

template <typename T>
class EnumeratorParameter : public Parameter, public std::enable_shared_from_this<EnumeratorParameter<T>>{
public:
	T value;
	T displayValue;
	
	EnumeratorParameter(T value_, std::string name, std::string saveString_) : Parameter(name, saveString_){
		value = value_;
		displayValue = value_;
	}
	
	virtual void gui(){
		if(ImGui::BeginCombo(imguiID.c_str(), Enumerator::getDisplayString(displayValue))){
			for(auto& type : Enumerator::getTypes<T>()){
				if(ImGui::Selectable(type.displayString, type.enumerator == displayValue)){
					displayValue = type.enumerator;
					//=========Command Invoker=========
					std::shared_ptr<EnumeratorParameter<T>> thisParameter = this->shared_from_this();
					CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter));
					//=================================
				}
			}
			ImGui::EndCombo();
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml){
		xml->SetAttribute(saveString.c_str(), Enumerator::getSaveString(value));
	}
	
	virtual bool load(tinyxml2::XMLElement* xml){
		using namespace tinyxml2;
		const char * enumeratorSaveString;
		XMLError result = xml->QueryStringAttribute(saveString.c_str(), &enumeratorSaveString);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", name);
		if(!Enumerator::isValidSaveName<T>(enumeratorSaveString)) return Logger::warn("Could not load parameter {} : Invalid Enumerator Save String \'{}\'", name, enumeratorSaveString);
		value = Enumerator::getEnumeratorFromSaveString<T>(enumeratorSaveString);
		displayValue = value;
		return true;
	}
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<EnumeratorParameter<T>> parameter;
		T oldValue;
		T newValue;
		EditCommand(std::shared_ptr<EnumeratorParameter<T>> parameter_){
			parameter = parameter_;
			oldValue = parameter->value;
			newValue = parameter->displayValue;
			name = "Edited \'" + std::string(parameter->name)
			+ "\' from \'" + std::string(Enumerator::getDisplayString(oldValue))
			+ "\' to \'" + std::string(Enumerator::getDisplayString(newValue)) + "\'";
		}
		virtual void execute(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
			parameter->b_changed = true;
			//if(parameter->editCallback) parameter->editCallback(parameter.get(), parameter->editCallbackUserData);
		}
		virtual void undo(){
			parameter->value = oldValue;
			parameter->displayValue = oldValue;
			parameter->b_changed = true;
			//if(parameter->undoCallback) parameter->undoCallback(parameter.get(), parameter->undoCallbackUserData);
		}
		virtual void redo(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
			parameter->b_changed = true;
			//if(parameter->redoCallback) parameter->redoCallback(parameter.get(), parameter->redoCallbackUserData);
		}
	};
	
};




//===================================================================================
//=================================== STATE PARAMETER ===============================
//===================================================================================

#include "Machine/AnimatableParameterValue.h"

class StateParameter : public Parameter, public std::enable_shared_from_this<StateParameter>{
public:
	
	AnimatableParameterState* value;
	AnimatableParameterState* displayValue;
	std::vector<AnimatableParameterState>* values;
	
	StateParameter(AnimatableParameterState* value_, std::vector<AnimatableParameterState>* values_, std::string name, std::string saveString_) : Parameter(name, saveString_){
		value = value_;
		displayValue = value_;
		values = values_;
	}
	
	virtual void gui(){
		if(ImGui::BeginCombo(imguiID.c_str(), displayValue->displayName)){
			for(auto& entry : *values){
				if(ImGui::Selectable(entry.displayName, &entry == displayValue)){
					displayValue = &entry;
					//=========Command Invoker=========
					std::shared_ptr<StateParameter> thisParameter = this->shared_from_this();
					CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter));
					//=================================
				}
			}
			ImGui::EndCombo();
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml){
		xml->SetAttribute(saveString.c_str(), value->saveName);
	}
	
	virtual bool load(tinyxml2::XMLElement* xml){
		using namespace tinyxml2;
		const char * stateSaveString;
		XMLError result = xml->QueryStringAttribute(saveString.c_str(), &stateSaveString);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", name);
		
		for(auto& entry : *values){
			if(strcmp(entry.saveName, stateSaveString) == 0){
				value = &entry;
				displayValue = value;
				return true;
			}
		}
		
		return Logger::warn("Could not load parameter {} : could not identity state string : {}", name, stateSaveString);
	}
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<StateParameter> parameter;
		AnimatableParameterState* oldValue;
		AnimatableParameterState* newValue;
		
		EditCommand(std::shared_ptr<StateParameter> parameter_){
			parameter = parameter_;
			oldValue = parameter->value;
			newValue = parameter->displayValue;
			name = "Edited \'" + std::string(parameter->name)
			+ "\' from \'" + std::string(oldValue->displayName)
			+ "\' to \'" + std::string(newValue->displayName) + "\'";
		}
		virtual void execute(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
			parameter->b_changed = true;
			if(parameter->editCallback) parameter->editCallback(parameter.get(), parameter->editCallbackUserData);
		}
		virtual void undo(){
			parameter->value = oldValue;
			parameter->displayValue = oldValue;
			parameter->b_changed = true;
			//if(parameter->undoCallback) parameter->undoCallback(parameter.get(), parameter->undoCallbackUserData);
		}
		virtual void redo(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
			parameter->b_changed = true;
			//if(parameter->redoCallback) parameter->redoCallback(parameter.get(), parameter->redoCallbackUserData);
		}
	};
	
};
