#pragma once

#include <tinyxml2.h>
#include <imgui.h>
#include "CommandHistory.h"
#include "Machine/AnimatableParameterValue.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Fonts.h"

class BaseNumberParameter;

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
	
	void setDisabled(bool disabled){ b_disabled = disabled; }
	bool isDisabled(){ return b_disabled; }
	
	void setValid(bool valid){ b_valid = valid; }
	bool isValid(){ return b_valid; }
	
	const char* getName(){ return name.c_str(); }
	const char* getImGuiID(){ return imGuiID.c_str(); }
	const char* getSaveString(){ return saveString.c_str(); }
	
	void setEditCallback(std::function<void(std::shared_ptr<Parameter>)> cb){ editCallback = cb; }
	void onEdit(){ if(editCallback) editCallback(shared_from_this()); }
	
	void gui(){
		bool drawInvalid = !b_valid;
		if(drawInvalid) {
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
			ImGui::PushFont(Fonts::sansBold15);
		}
		onGui();
		if(drawInvalid) {
			ImGui::PopStyleColor();
			ImGui::PopFont();
		}
	}
	virtual void onGui() = 0;
	virtual bool save(tinyxml2::XMLElement* xml) = 0;
	virtual bool load(tinyxml2::XMLElement* xml) = 0;
	virtual std::shared_ptr<Parameter> makeBaseCopy() = 0;
	
	virtual bool isNumber(){ return false; }
	std::shared_ptr<BaseNumberParameter> castToNumber(){ return std::dynamic_pointer_cast<BaseNumberParameter>(shared_from_this()); }
	
private:
	std::function<void(std::shared_ptr<Parameter>)> editCallback;
	std::string name;
	std::string saveString;
	std::string imGuiID;
	bool b_disabled = false;
	bool b_valid = true;
};



//===============================================================================
//=================================== NUMBERS ===================================
//===============================================================================

class BaseNumberParameter : public Parameter{
public:
	
	BaseNumberParameter(std::string& name_, std::string& saveString_) : Parameter(name_, saveString_){}
	virtual bool isNumber() override { return true; }
	
	virtual void setStepSize(double small, double large) = 0;
	
	void setAllowNegatives(bool allowNegatives){ b_allowNegatives = allowNegatives; }
	void allowNegatives(){ b_allowNegatives = true; }
	void denyNegatives(){ b_allowNegatives = false; }
	
	void setPrefix(std::string p) { prefix = p; updateFormatString(); }
	void setFormat(std::string f) { format = f; updateFormatString();  }
	void setUnit(Unit u){ unit = u; updateFormatString(); }
	void setSuffix(std::string s) { suffix = s; updateFormatString();  }
	void updateFormatString(){ formatString = prefix + format + std::string(unit->abbreviated) + suffix; }
	const char* getFormatString(){ return formatString.c_str(); }
	
	virtual bool validateRange(double rangeMin, double rangeMax, bool withMin = true, bool withMax = true) = 0;
	
	virtual std::shared_ptr<BaseNumberParameter> makeCopy() = 0;
	
	virtual double getReal() = 0;
	virtual int getInteger() = 0;
	
	std::string prefix;
	std::string format;
	Unit unit;
	std::string suffix;
	std::string formatString;
	bool b_allowNegatives = true;
};

template<typename T>
class NumberParameter : public BaseNumberParameter{
public:
	
	T displayValue;
	T value;
	
	T stepSmall;
	T stepLarge;
	T* stepSmallPtr;
	T* stepLargePtr;
	
	NumberParameter(T value_,
					std::string name_,
					std::string saveString_ = "",
					std::string format_ = "",
					Unit unit_ = Units::None::None,
					bool allowNegatives = true,
					T stepSmall_ = 0,
					T stepLarge_ = 0,
					std::string prefix_ = "",
					std::string suffix_ = "") :
	BaseNumberParameter(name_, saveString_) {
		displayValue = value_;
		value = value_;
		unit = unit_;
		if(format_.empty()) format = getDefaultFormatString();
		else format = format_;
		prefix = prefix_;
		suffix = suffix_;
		updateFormatString();
		setStepSize(stepSmall_, stepLarge_);
		setAllowNegatives(allowNegatives);
	}
	
	virtual double getReal() override { return value; }
	virtual int getInteger() override { return value; }
	
	virtual bool validateRange(double rangeMin, double rangeMax, bool withMin, bool withMax) override {
		bool valid = true;
		if(withMin && value < rangeMin) valid = false;
		else if(!withMin && value <= rangeMin) valid = false;
		else if(withMax && value > rangeMax) valid = false;
		else if(!withMax && value >= rangeMax) valid = false;
		setValid(valid);
		return valid;
	};
	
	static std::shared_ptr<NumberParameter<T>> make(T value_,
													std::string name_,
													std::string saveString_ = "",
													std::string format_ = "",
													Unit unit_ = Units::None::None,
													bool allowNegatives = true,
													T stepSmall_ = 0,
													T stepLarge_ = 0,
													std::string prefix_ = "",
													std::string suffix_ = ""){
		return std::make_shared<NumberParameter<T>>(value_, name_, saveString_, format_, unit_, allowNegatives, stepSmall_, stepLarge_, prefix_, suffix_);
	}
	
	virtual std::shared_ptr<BaseNumberParameter> makeCopy() override{
		return make(value, getName(), getSaveString(), format, unit, b_allowNegatives, stepSmall, stepLarge, prefix, suffix);
	};
	
	virtual std::shared_ptr<Parameter> makeBaseCopy() override { return makeCopy(); };
	
	virtual void setStepSize(double small, double large) override {
		stepSmall = small;
		if(stepSmall == 0) stepSmallPtr = nullptr;
		else stepSmallPtr = &stepSmall;
		stepLarge = large;
		if(stepLarge == 0) stepLargePtr = nullptr;
		else stepLargePtr = &stepLarge;
	}
	
	virtual void onGui() override {
		ImGui::BeginDisabled(isDisabled());
		ImGui::InputScalar(getImGuiID(), getImGuiDataType(), &displayValue, stepSmallPtr, stepLargePtr, getFormatString());
		ImGui::EndDisabled();
		if(ImGui::IsItemDeactivatedAfterEdit() && value != displayValue){
			if(!b_allowNegatives && displayValue < 0) {
				displayValue = 0;
				if(value == displayValue) return;
			}
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
	
	void overwrite(T newValue){
		displayValue = newValue;
		value = newValue;
	}
	
private:
	
	ImGuiDataType getImGuiDataType();
	std::string getDefaultFormatString(){ return "%i"; } //template specialisation for real float and double types
	
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
inline ImGuiDataType NumberParameter<float>::getImGuiDataType(){ return ImGuiDataType_Float; }
template<>
inline ImGuiDataType NumberParameter<double>::getImGuiDataType(){ return ImGuiDataType_Double; }
template<>
inline ImGuiDataType NumberParameter<uint8_t>::getImGuiDataType(){ return ImGuiDataType_U8; }
template<>
inline ImGuiDataType NumberParameter<int8_t>::getImGuiDataType(){ return ImGuiDataType_S8; }
template<>
inline ImGuiDataType NumberParameter<uint16_t>::getImGuiDataType(){ return ImGuiDataType_U16; }
template<>
inline ImGuiDataType NumberParameter<int16_t>::getImGuiDataType(){ return ImGuiDataType_S16; }
template<>
inline ImGuiDataType NumberParameter<uint32_t>::getImGuiDataType(){ return ImGuiDataType_U32; }
template<>
inline ImGuiDataType NumberParameter<int32_t>::getImGuiDataType(){ return ImGuiDataType_S32; }
template<>
inline ImGuiDataType NumberParameter<uint64_t>::getImGuiDataType(){ return ImGuiDataType_U64; }
template<>
inline ImGuiDataType NumberParameter<int64_t>::getImGuiDataType(){ return ImGuiDataType_S64; }

template<>
inline std::string NumberParameter<float>::getDefaultFormatString(){ return "%.3f"; }
template<>
inline std::string NumberParameter<double>::getDefaultFormatString(){ return "%.3f"; }

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
	
	virtual void onGui() override {
		inputField();
		if(ImGui::IsItemDeactivatedAfterEdit() && value != displayValue){
			//=========Command Invoker=========
			auto thisParameter = std::dynamic_pointer_cast<VectorParameter<T>>(shared_from_this());
			std::string commandName = "Changed " + std::string(getName()) + " from " + glm::to_string(value) + " to " + glm::to_string(displayValue);
			CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter, commandName));
			//=================================
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml) override;
	virtual bool load(tinyxml2::XMLElement* xml) override;
	
	std::shared_ptr<VectorParameter> makeCopy() {
		return std::make_shared<VectorParameter>(value, getName(), getSaveString(), format);
	}
	
	virtual std::shared_ptr<Parameter> makeBaseCopy() override { return makeCopy(); };
	
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
	ImGui::BeginDisabled(isDisabled());
	ImGui::InputFloat2(getImGuiID(), &displayValue.x, format);
	ImGui::EndDisabled();
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
	ImGui::BeginDisabled(isDisabled());
	ImGui::InputFloat3(getImGuiID(), &displayValue.x, format);
	ImGui::EndDisabled();
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
	ImGui::BeginDisabled(isDisabled());
	ImGui::InputFloat4(getImGuiID(), &displayValue.x, format);
	ImGui::EndDisabled();
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
	
	virtual void onGui() override {
		ImGui::BeginDisabled(isDisabled());
		ImGui::Checkbox(getImGuiID(), &displayValue);
		ImGui::EndDisabled();
		if(ImGui::IsItemDeactivatedAfterEdit() && value != displayValue){
			//=========Command Invoker=========
			auto thisParameter = std::dynamic_pointer_cast<BooleanParameter>(shared_from_this());
			std::string commandName = "Inverted " + std::string(getName());
			CommandHistory::pushAndExecute(std::make_shared<InvertCommand>(thisParameter, commandName));
			//=================================
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml) override {
		xml->SetAttribute(getSaveString(), value);
		return true;
	}
	
	virtual bool load(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		XMLError result = xml->QueryBoolAttribute(getSaveString(), &value);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", getName());
		displayValue = value;
		return true;
	}
	
	std::shared_ptr<BooleanParameter> makeCopy() {
		return std::make_shared<BooleanParameter>(value, getName(), getSaveString());
	}
	
	virtual std::shared_ptr<Parameter> makeBaseCopy() override { return makeCopy(); };
	
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
	
	virtual void onGui() override {
		ImGui::BeginDisabled(isDisabled());
		ImGui::InputText(getImGuiID(), displayValue, bufferSize);
		ImGui::EndDisabled();
		if(ImGui::IsItemDeactivatedAfterEdit() && strcmp(displayValue, value.c_str()) != 0){
			//=========Command Invoker=========
			std::shared_ptr<StringParameter> thisParameter = std::dynamic_pointer_cast<StringParameter>(shared_from_this());
			std::string commandName = "Changed " + std::string(getName()) + " from \'" + value + "\' to \'" + displayValue + "\'";
			CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter, commandName));
			//=================================
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml) override {
		xml->SetAttribute(getSaveString(), value.c_str());
		return true;
	}
	
	virtual bool load(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		const char* loadedString;
		XMLError result = xml->QueryStringAttribute(getSaveString(), &loadedString);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", getName());
		strcpy(displayValue, loadedString);
		value = loadedString;
		return true;
	}
	
	std::shared_ptr<StringParameter> makeCopy() {
		return std::make_shared<StringParameter>(value, getName(), getSaveString(), bufferSize);
	}
	
	virtual std::shared_ptr<Parameter> makeBaseCopy() override { return makeCopy(); };
	
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
	
	virtual void onGui() override {
		ImGui::BeginDisabled(isDisabled());
		combo();
		ImGui::EndDisabled();
	}
	
	virtual void combo(T* availableValues = nullptr, size_t availableValueCount = 0){
		
		ImGui::BeginDisabled(availableValues != nullptr && availableValueCount == 1);
		
		if(ImGui::BeginCombo(getImGuiID(), Enumerator::getDisplayString(displayValue))){
			if(availableValues != nullptr && availableValueCount > 0){
				for(int i = 0; i < availableValueCount; i++){
					T type = availableValues[i];
					if(ImGui::Selectable(Enumerator::getDisplayString(type), type == displayValue)) onChange(type);
				}
			}else{
				for(auto& type : Enumerator::getTypes<T>()){
					if(ImGui::Selectable(type.displayString, type.enumerator == displayValue)) onChange(type.enumerator);
				}
			}
			ImGui::EndCombo();
		}
		
		ImGui::EndDisabled();
	}
	
	virtual bool save(tinyxml2::XMLElement* xml) override {
		xml->SetAttribute(getSaveString(), Enumerator::getSaveString(value));
	}
	
	virtual bool load(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		const char * enumeratorSaveString;
		XMLError result = xml->QueryStringAttribute(getSaveString(), &enumeratorSaveString);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", getName());
		if(!Enumerator::isValidSaveName<T>(enumeratorSaveString)) return Logger::warn("Could not load parameter {} : Invalid Enumerator Save String \'{}\'", getName(), enumeratorSaveString);
		value = Enumerator::getEnumeratorFromSaveString<T>(enumeratorSaveString);
		displayValue = value;
		return true;
	}
	
	std::shared_ptr<EnumeratorParameter<T>> makeCopy() {
		return std::make_shared<EnumeratorParameter<T>>(value, getName(), getSaveString());
	}
	
	virtual std::shared_ptr<Parameter> makeBaseCopy() override { return makeCopy(); };
	
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
	
	private:
	
	void onChange(T newValue){
		displayValue = newValue;
		//=========Command Invoker=========
		std::shared_ptr<EnumeratorParameter<T>> thisParameter = std::dynamic_pointer_cast<EnumeratorParameter<T>>(shared_from_this());
		std::string commandName = "Changed " + std::string(getName()) + " from \'" + Enumerator::getDisplayString(value) + "\' to \'" + Enumerator::getDisplayString(displayValue) + "\'";
		CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter, commandName));
		//=================================
	}
	
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
	
	virtual void onGui() override {
		ImGui::BeginDisabled(isDisabled());
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
		ImGui::EndDisabled();
	}
	
	virtual bool save(tinyxml2::XMLElement* xml) override {
		xml->SetAttribute(getSaveString(), value->saveName);
	}
	
	virtual bool load(tinyxml2::XMLElement* xml) override {
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
	
	std::shared_ptr<StateParameter> makeCopy() {
		return std::make_shared<StateParameter>(value, values, getName(), getSaveString());
	}
	
	virtual std::shared_ptr<Parameter> makeBaseCopy() override { return makeCopy(); };
	
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


#include "Motion/Playback/TimeStringConversion.h"

//===================================================================================
//=================================== TIME PARAMETER ================================
//===================================================================================

class TimeParameter : public Parameter{
public:
	
	double value;
	char displayBuffer[32];
	
	TimeParameter(double value_, std::string name, std::string saveString_) : Parameter(name, saveString_){
		overwrite(value_);
	}
	
	virtual void onGui() override {
		ImGui::BeginDisabled(isDisabled());
		ImGui::InputText(getImGuiID(), displayBuffer, 32, ImGuiInputTextFlags_AutoSelectAll);
		if(ImGui::IsItemDeactivatedAfterEdit()){
			double newTime = TimeStringConversion::timecodeStringToSeconds(displayBuffer);
			std::string newTimeString = TimeStringConversion::secondsToTimecodeString(newTime);
			std::string name = "Change " + std::string(getName()) + " from " + TimeStringConversion::secondsToTimecodeString(value) + " to " + newTimeString;
			auto command = std::make_shared<EditCommand>(std::dynamic_pointer_cast<TimeParameter>(shared_from_this()), name);
			CommandHistory::pushAndExecute(command);
		}
		ImGui::EndDisabled();
	}
	
	virtual bool save(tinyxml2::XMLElement* xml) override {
		xml->SetAttribute(getSaveString(), value);
	}
	
	virtual bool load(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		XMLError result = xml->QueryDoubleAttribute(getSaveString(), &value);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", getName());
		overwrite(value);
	}
	
	std::shared_ptr<TimeParameter> makeCopy() {
		return std::make_shared<TimeParameter>(value, getName(), getSaveString());
	}
	
	virtual std::shared_ptr<Parameter> makeBaseCopy() override { return makeCopy(); };
	
	void overwrite(double newValue){
		value = newValue;
		strcpy(displayBuffer, TimeStringConversion::secondsToTimecodeString(newValue).c_str());
	}
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<TimeParameter> parameter;
		double oldValue;
		double newValue;
		
		EditCommand(std::shared_ptr<TimeParameter> parameter_, std::string& commandName) : Command(commandName){
			parameter = parameter_;
			oldValue = parameter->value;
			newValue = TimeStringConversion::timecodeStringToSeconds(parameter->displayBuffer);
		}
		void setNewValue(){
			parameter->overwrite(newValue);
		}
		void setOldValue(){
			parameter->overwrite(oldValue);
		}
		virtual void execute(){ setNewValue(); parameter->onEdit(); }
		virtual void undo(){ setOldValue(); }
		virtual void redo(){ setNewValue(); }
	};
	
};
