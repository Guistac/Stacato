#pragma once

#include <tinyxml2.h>
#include <imgui.h>
#include "CommandHistory.h"
#include "Machine/AnimatableParameterValue.h"

class Parameter : public std::enable_shared_from_this<Parameter>{
public:
	
	Parameter(std::string name_, std::string saveString_ = ""){
		setName(name_);
		setSaveString(saveString_);
	}
	
	void setName(std::string name_){
		name = name_;
		imGuiID = "##" + name; //this avoids rendering the parameter name in imgui input field
	}
	void setSaveString(std::string saveString_){ saveString = saveString_; }
	
	const char* getName(){ return name.c_str(); }
	const char* getImGuiID(){ return imGuiID.c_str(); }
	const char* getSaveString(){ return saveString.c_str(); }
	
	void setEditCallback(std::function<void(std::shared_ptr<Parameter>)> cb){ editCallback = cb; }
	void onEdit(){ if(editCallback) editCallback(shared_from_this()); }
	
	virtual void gui() = 0;
	virtual bool save(tinyxml2::XMLElement* xml) = 0;
	virtual bool load(tinyxml2::XMLElement* xml) = 0;
	virtual std::shared_ptr<Parameter> makeCopy() = 0;
	
private:
	std::function<void(std::shared_ptr<Parameter>)> editCallback;
	std::string name;
	std::string saveString;
	std::string imGuiID;
};



//===============================================================================
//=================================== NUMBERS ===================================
//===============================================================================


template<typename T>
class NumberParameter : public Parameter{
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
	
	virtual void gui() override {
		inputField();
		if(ImGui::IsItemDeactivatedAfterEdit() && value != displayValue){
			//=========Command Invoker=========
			std::shared_ptr<NumberParameter<T>> thisParameter = std::dynamic_pointer_cast<NumberParameter<T>>(shared_from_this());
			std::string commandName = "Changed " + std::string(getName()) + " from " + std::to_string(value) + " to " + std::to_string(displayValue);
			CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter, commandName));
			//=================================
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml) override {
		xml->SetAttribute(getSaveString(), value);
		 return true;
	 }
	
	virtual bool load(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		 double number;
		 XMLError result = xml->QueryDoubleAttribute(getSaveString(), &number);
		 if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", getName());
		 value = number;
		 displayValue = number;
		 return true;
	 }
	
	virtual std::shared_ptr<Parameter> makeCopy() override {
		return std::make_shared<NumberParameter<T>>(value, getName(), getSaveString(), unit, stepSmall, stepLarge, format);
	}
	
	void overwrite(T newValue){
		displayValue = newValue;
		value = newValue;
	}
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<NumberParameter<T>> parameter;
		T newValue;
		T previousValue;
		EditCommand(std::shared_ptr<NumberParameter<T>> parameter_, std::string& commandName) : Command(commandName){
			parameter = parameter_;
			newValue = parameter->displayValue;
			previousValue = parameter->value;
		}
		void setNewValue(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
		}
		void setOldValue(){
			parameter->value = previousValue;
			parameter->displayValue = previousValue;
		}
		virtual void execute(){ setNewValue(); parameter->onEdit(); }
		virtual void undo(){ setOldValue(); }
		virtual void redo(){ setNewValue(); }
	};
	
};

template<>
inline void NumberParameter<float>::inputField(){
	ImGui::InputScalar(getImGuiID(), ImGuiDataType_Float, &displayValue, stepSmallPtr, stepLargePtr, getFormatedReal());
}

template<>
inline void NumberParameter<double>::inputField(){
	ImGui::InputScalar(getImGuiID(), ImGuiDataType_Double, &displayValue, stepSmallPtr, stepLargePtr, getFormatedReal());
}

template<>
inline void NumberParameter<uint8_t>::inputField(){
	ImGui::InputScalar(getImGuiID(), ImGuiDataType_U8, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}

template<>
inline void NumberParameter<int8_t>::inputField(){
	ImGui::InputScalar(getImGuiID(), ImGuiDataType_S8, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}

template<>
inline void NumberParameter<uint16_t>::inputField(){
	ImGui::InputScalar(getImGuiID(), ImGuiDataType_U16, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}

template<>
inline void NumberParameter<int16_t>::inputField(){
	ImGui::InputScalar(getImGuiID(), ImGuiDataType_S16, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}

template<>
inline void NumberParameter<uint32_t>::inputField(){
	ImGui::InputScalar(getImGuiID(), ImGuiDataType_U32, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}

template<>
inline void NumberParameter<int32_t>::inputField(){
	const char* format = getFormatedInteger();
	ImGui::InputScalar(getImGuiID(), ImGuiDataType_S32, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}

template<>
inline void NumberParameter<uint64_t>::inputField(){
	ImGui::InputScalar(getImGuiID(), ImGuiDataType_U64, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}

template<>
inline void NumberParameter<int64_t>::inputField(){
	ImGui::InputScalar(getImGuiID(), ImGuiDataType_S64, &displayValue, stepSmallPtr, stepLargePtr, getFormatedInteger());
}




//===============================================================================
//=================================== VECTORS ===================================
//===============================================================================

template<typename T>
class VectorParameter : public Parameter{
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
			auto thisParameter = std::dynamic_pointer_cast<VectorParameter<T>>(shared_from_this());
			std::string commandName = "Changed " + std::string(getName()) + " from " + glm::to_string(value) + " to " + glm::to_string(displayValue);
			CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter, commandName));
			//=================================
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);
	
	virtual std::shared_ptr<Parameter> makeCopy() override {
		return std::make_shared<VectorParameter>(value, getName(), getSaveString(), format);
	}
	
	void overwrite(T newValue){
		displayValue = newValue;
		value = newValue;
	}
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<VectorParameter<T>> parameter;
		T newValue;
		T previousValue;
		EditCommand(std::shared_ptr<VectorParameter<T>> parameter_, std::string& commandName) : Command(commandName){
			parameter = parameter_;
			newValue = parameter->displayValue;
			previousValue = parameter->value;
		}
		void setNewValue(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
		}
		void setOldValue(){
			parameter->value = previousValue;
			parameter->displayValue = previousValue;
		}
		virtual void execute(){
			setNewValue();
			parameter->onEdit();
		}
		virtual void undo(){ setOldValue(); }
		virtual void redo(){ setNewValue(); }
	};
	
};

template<>
inline void VectorParameter<glm::vec2>::inputField(){
	ImGui::InputFloat2(getImGuiID(), &displayValue.x, format);
}

template<>
inline bool VectorParameter<glm::vec2>::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->InsertNewChildElement(getSaveString());
	element->SetAttribute("x", value.x);
	element->SetAttribute("y", value.y);
	return true;
}

template<>
inline bool VectorParameter<glm::vec2>::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->FirstChildElement(getSaveString());
	if(XML_SUCCESS != element->QueryAttribute("x", &value.x) ||
	   XML_SUCCESS != element->QueryAttribute("y", &value.y)){
		return Logger::warn("Could not load parameter {}", getName());
	}
	displayValue = value;
	return true;
}




template<>
inline void VectorParameter<glm::vec3>::inputField(){
	ImGui::InputFloat3(getImGuiID(), &displayValue.x, format);
}

template<>
inline bool VectorParameter<glm::vec3>::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->InsertNewChildElement(getSaveString());
	element->SetAttribute("x", value.x);
	element->SetAttribute("y", value.y);
	element->SetAttribute("z", value.z);
	return true;
}

template<>
inline bool VectorParameter<glm::vec3>::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->FirstChildElement(getSaveString());
	if(XML_SUCCESS != element->QueryAttribute("x", &value.x) ||
	   XML_SUCCESS != element->QueryAttribute("y", &value.y) ||
	   XML_SUCCESS != element->QueryAttribute("z", &value.z)){
		return Logger::warn("Could not load parameter {}", getName());
	}
	displayValue = value;
	return true;
}



template<>
inline void VectorParameter<glm::vec4>::inputField(){
	ImGui::InputFloat4(getImGuiID(), &displayValue.x, format);
}

template<>
inline bool VectorParameter<glm::vec4>::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->InsertNewChildElement(getSaveString());
	element->SetAttribute("x", value.x);
	element->SetAttribute("y", value.y);
	element->SetAttribute("z", value.z);
	element->SetAttribute("w", value.w);
	return true;
}

template<>
inline bool VectorParameter<glm::vec4>::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->FirstChildElement(getSaveString());
	if(XML_SUCCESS != element->QueryAttribute("x", &value.x) ||
	   XML_SUCCESS != element->QueryAttribute("y", &value.y) ||
	   XML_SUCCESS != element->QueryAttribute("z", &value.z) ||
	   XML_SUCCESS != element->QueryAttribute("w", &value.w)){
		return Logger::warn("Could not load parameter {}", getName());
	}
	displayValue = value;
	return true;
}




//===============================================================================
//=================================== BOOLEANS ==================================
//===============================================================================

class BooleanParameter : public Parameter{
public:
	
	bool displayValue;
	bool value;
	
	BooleanParameter(bool value_, std::string name_, std::string saveString_) : Parameter(name_, saveString_) {
		displayValue = value_;
		value = value_;
	}
	
	virtual void gui(){
		ImGui::Checkbox(getImGuiID(), &displayValue);
		if(ImGui::IsItemDeactivatedAfterEdit() && value != displayValue){
			//=========Command Invoker=========
			auto thisParameter = std::dynamic_pointer_cast<BooleanParameter>(shared_from_this());
			std::string commandName = "Inverted " + std::string(getName());
			CommandHistory::pushAndExecute(std::make_shared<InvertCommand>(thisParameter, commandName));
			//=================================
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml){
		xml->SetAttribute(getSaveString(), value);
		return true;
	}
	
	virtual bool load(tinyxml2::XMLElement* xml){
		using namespace tinyxml2;
		XMLError result = xml->QueryBoolAttribute(getSaveString(), &value);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", getName());
		displayValue = value;
		return true;
	}
	
	virtual std::shared_ptr<Parameter> makeCopy() override {
		return std::make_shared<BooleanParameter>(value, getName(), getSaveString());
	}
	
	void overwrite(bool newValue){
		displayValue = newValue;
		value = newValue;
	}
	
	class InvertCommand : public Command{
	public:
		std::shared_ptr<BooleanParameter> parameter;
		InvertCommand(std::shared_ptr<BooleanParameter> parameter_, std::string& commandName) : Command(commandName){
			parameter = parameter_;
		}
		void invert(){
			parameter->value = !parameter->value;
			parameter->displayValue = parameter->value;
		}
		virtual void execute(){ invert(); parameter->onEdit(); }
		virtual void undo(){ invert(); }
		virtual void redo(){ invert(); }
	};
	
};




//===============================================================================
//=================================== STRINGS ===================================
//===============================================================================

class StringParameter : public Parameter{
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
		ImGui::InputText(getImGuiID(), displayValue, bufferSize);
		if(ImGui::IsItemDeactivatedAfterEdit() && strcmp(displayValue, value.c_str()) != 0){
			//=========Command Invoker=========
			std::shared_ptr<StringParameter> thisParameter = std::dynamic_pointer_cast<StringParameter>(shared_from_this());
			std::string commandName = "Changed " + std::string(getName()) + " from \'" + value + "\' to \'" + displayValue + "\'";
			CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter, commandName));
			//=================================
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml){
		xml->SetAttribute(getSaveString(), value.c_str());
		return true;
	}
	
	virtual bool load(tinyxml2::XMLElement* xml){
		using namespace tinyxml2;
		const char* loadedString;
		XMLError result = xml->QueryStringAttribute(getSaveString(), &loadedString);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", getName());
		strcpy(displayValue, loadedString);
		value = loadedString;
		return true;
	}
	
	virtual std::shared_ptr<Parameter> makeCopy() override {
		return std::make_shared<StringParameter>(value, getName(), getSaveString(), bufferSize);
	}
	
	void overwrite(std::string newValue){
		strcpy(displayValue, newValue.c_str());
		value = newValue;
	}
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<StringParameter> parameter;
		std::string newValue;
		std::string previousValue;
		EditCommand(std::shared_ptr<StringParameter> parameter_, std::string& commandName) : Command(commandName){
			parameter = parameter_;
			newValue = parameter->displayValue;
			previousValue = parameter->value;
		}
		void setNewValue(){
			parameter->value = newValue;
			strcpy(parameter->displayValue, newValue.c_str());
		}
		void setOldValue(){
			parameter->value = previousValue;
			strcpy(parameter->displayValue, previousValue.c_str());
		}
		virtual void execute(){ setNewValue(); parameter->onEdit(); }
		virtual void undo(){ setOldValue(); }
		virtual void redo(){ setNewValue(); }
	};
	
};



//===============================================================================
//================================= ENUMERATORS =================================
//===============================================================================

template <typename T>
class EnumeratorParameter : public Parameter{
public:
	T value;
	T displayValue;
	
	EnumeratorParameter(T value_, std::string name, std::string saveString_) : Parameter(name, saveString_){
		value = value_;
		displayValue = value_;
	}
	
	virtual void gui(){
		if(ImGui::BeginCombo(getImGuiID(), Enumerator::getDisplayString(displayValue))){
			for(auto& type : Enumerator::getTypes<T>()){
				if(ImGui::Selectable(type.displayString, type.enumerator == displayValue)){
					displayValue = type.enumerator;
					//=========Command Invoker=========
					std::shared_ptr<EnumeratorParameter<T>> thisParameter = std::dynamic_pointer_cast<EnumeratorParameter<T>>(shared_from_this());
					std::string commandName = "Changed " + std::string(getName()) + " from \'" + Enumerator::getDisplayString(value) + "\' to \'" + Enumerator::getDisplayString(displayValue) + "\'";
					CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter, commandName));
					//=================================
				}
			}
			ImGui::EndCombo();
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml){
		xml->SetAttribute(getSaveString(), Enumerator::getSaveString(value));
	}
	
	virtual bool load(tinyxml2::XMLElement* xml){
		using namespace tinyxml2;
		const char * enumeratorSaveString;
		XMLError result = xml->QueryStringAttribute(getSaveString(), &enumeratorSaveString);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", getName());
		if(!Enumerator::isValidSaveName<T>(enumeratorSaveString)) return Logger::warn("Could not load parameter {} : Invalid Enumerator Save String \'{}\'", getName(), enumeratorSaveString);
		value = Enumerator::getEnumeratorFromSaveString<T>(enumeratorSaveString);
		displayValue = value;
		return true;
	}
	
	virtual std::shared_ptr<Parameter> makeCopy() override {
		return std::make_shared<EnumeratorParameter<T>>(value, getName(), getSaveString());
	}
	
	void overwrite(T newValue){
		displayValue = newValue;
		value = newValue;
	}
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<EnumeratorParameter<T>> parameter;
		T oldValue;
		T newValue;
		EditCommand(std::shared_ptr<EnumeratorParameter<T>> parameter_, std::string& commandName) : Command(commandName){
			parameter = parameter_;
			oldValue = parameter->value;
			newValue = parameter->displayValue;
		}
		void setNewValue(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
		}
		void setOldValue(){
			parameter->value = oldValue;
			parameter->displayValue = oldValue;
		}
		virtual void execute(){ setNewValue(); parameter->onEdit(); }
		virtual void undo(){ setOldValue(); }
		virtual void redo(){ setNewValue(); }
	};
	
};




//===================================================================================
//=================================== STATE PARAMETER ===============================
//===================================================================================

class StateParameter : public Parameter{
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
		if(ImGui::BeginCombo(getImGuiID(), displayValue->displayName)){
			for(auto& entry : *values){
				if(ImGui::Selectable(entry.displayName, &entry == displayValue)){
					displayValue = &entry;
					//=========Command Invoker=========
					std::shared_ptr<StateParameter> thisParameter = std::dynamic_pointer_cast<StateParameter>(shared_from_this());
					std::string commandName = "Changed " + std::string(getName()) + " from \'" + value->displayName + "\' to \'" + displayValue->displayName + "\'";
					CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter, commandName));
					//=================================
				}
			}
			ImGui::EndCombo();
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml){
		xml->SetAttribute(getSaveString(), value->saveName);
	}
	
	virtual bool load(tinyxml2::XMLElement* xml){
		using namespace tinyxml2;
		const char * stateSaveString;
		XMLError result = xml->QueryStringAttribute(getSaveString(), &stateSaveString);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", getName());
		
		for(auto& entry : *values){
			if(strcmp(entry.saveName, stateSaveString) == 0){
				value = &entry;
				displayValue = value;
				return true;
			}
		}
		
		return Logger::warn("Could not load parameter {} : could not identity state string : {}", getName(), stateSaveString);
	}
	
	virtual std::shared_ptr<Parameter> makeCopy() override {
		return std::make_shared<StateParameter>(value, values, getName(), getSaveString());
	}
	
	void overwrite(AnimatableParameterState* newValue){
		displayValue = newValue;
		value = newValue;
	}
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<StateParameter> parameter;
		AnimatableParameterState* oldValue;
		AnimatableParameterState* newValue;
		
		EditCommand(std::shared_ptr<StateParameter> parameter_, std::string& commandName) : Command(commandName){
			parameter = parameter_;
			oldValue = parameter->value;
			newValue = parameter->displayValue;
		}
		void setNewValue(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
		}
		void setOldValue(){
			parameter->value = oldValue;
			parameter->displayValue = oldValue;
		}
		virtual void execute(){ setNewValue(); parameter->onEdit(); }
		virtual void undo(){ setOldValue(); }
		virtual void redo(){ setNewValue(); }
	};
	
};
