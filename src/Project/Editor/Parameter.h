#pragma once

#include <tinyxml2.h>
#include <imgui.h>
#include <imgui_internal.h>
#include "CommandHistory.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Fonts.h"

#include "Gui/ApplicationWindow/Gui.h"

#include "Environnement/Environnement.h"

class BaseNumberParameter;

class Parameter : public std::enable_shared_from_this<Parameter>{
public:
	
	Parameter(std::string name_, std::string saveString_){
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
	void addEditCallback(std::function<void()> callback){ editCallbacks.push_back(callback); }
	void onEdit(){
		if(editCallback) editCallback(shared_from_this());
		for(auto& editCallback : editCallbacks) editCallback();
	}
	
	virtual void gui(ImFont* font = nullptr){
		if(font){
			ImGui::PushFont(font);
			ImGui::Text("%s", getName());
			ImGui::PopFont();
		}
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
	std::shared_ptr<BaseNumberParameter> toNumber(){ return std::static_pointer_cast<BaseNumberParameter>(shared_from_this()); }
	
	std::recursive_mutex* mutex = nullptr;
	void setMutex(std::recursive_mutex* mutex_){ mutex = mutex_; }
	void lockMutex(){ if(mutex) mutex->lock(); }
	void unlockMutex(){ if(mutex) mutex->unlock(); }
	
private:
	std::function<void(std::shared_ptr<Parameter>)> editCallback;
	std::vector<std::function<void()>> editCallbacks;
	std::string name;
	std::string saveString;
	std::string imGuiID;
	std::string description;
	bool b_disabled = false;
protected:
	bool b_valid = true;
};

using Param = std::shared_ptr<Parameter>;


//===============================================================================
//=================================== NUMBERS ===================================
//===============================================================================

class ParameterGroup{
public:
	
	ParameterGroup(const char* saveName_, std::vector<Param> parameters_) : saveName(saveName_), parameters(parameters_){}
	
	const std::vector<std::shared_ptr<Parameter>>& get(){ return parameters; }
	
	bool save(tinyxml2::XMLElement* parentElement){
		using namespace tinyxml2;
		if(!parentElement) return false;
		
		XMLElement* xmlContainer = parentElement->InsertNewChildElement(saveName);
		
		bool anyError = false;
		for(Param parameter : parameters){
			if(!parameter->save(xmlContainer)) anyError = true;
		}
		return !anyError;
	}
	
	bool load(tinyxml2::XMLElement* parentElement){
		using namespace tinyxml2;
		if(!parentElement) return false;
		
		XMLElement* xmlContainer = parentElement->FirstChildElement(saveName);
		if(!xmlContainer) {
			Logger::warn("Could not load Parameter group element '{}'", saveName);
			return false;
		}
		
		bool anyError = false;
		for(Param parameter : parameters){
			if(!parameter->load(xmlContainer)) anyError = true;
		}
		return !anyError;
	}
	
private:
	std::vector<std::shared_ptr<Parameter>> parameters;
	const char* saveName;
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
	void updateFormatString(){
		const char* unitAbbreviated = unit->abbreviated;
		if(unit == Units::Fraction::Percent) unitAbbreviated = "%%";
		formatString = prefix + format + std::string(unitAbbreviated) + suffix;
	}
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
		
		//TURN DISPLAY HACK
		if(unit == Units::AngularDistance::Degree && b_useTurnAndDegrees){
			
			bool b_edited = false;
			ImGui::PushID(1);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 3.0);
			ImGui::InputScalar(getImGuiID(), ImGuiDataType_S32, &fullTurns, nullptr, nullptr, "%iRev");
			b_edited |= ImGui::IsItemDeactivatedAfterEdit();
			ImGui::PopID();
			ImGui::SameLine(0.0f, 2.0f);
			ImGui::PushID(2);
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 3.0);
			std::string format;
			if(singleTurnPosition > 0.0) format = "+%.1f°";
			else format = "%.1f°";
			ImGui::InputScalar(getImGuiID(), ImGuiDataType_Double, &singleTurnPosition, nullptr, nullptr, format.c_str());
			b_edited |= ImGui::IsItemDeactivatedAfterEdit();
			ImGui::PopID();
			if(b_edited){
				//this is for a range from -179.99 to 180.0
				singleTurnPosition = std::clamp(singleTurnPosition, -179.99, 180.0);
				double absoluteDegrees = fullTurns * 360.0 + singleTurnPosition;
				overwriteWithHistory(absoluteDegrees);
				//this is for a range from 0.0 to 360.0
				//singleTurnPosition = std::clamp(singleTurnPosition, 0.0, 359.99);
				//double absoluteDegrees = fullTurns * 360.0 + singleTurnPosition;
				//overwriteWithHistory(absoluteDegrees);
			}
			
			ImGui::SameLine();
			ImGui::Text("/");
			ImGui::SameLine();
			
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
			ImGui::InputScalar(getImGuiID(), getImGuiDataType(), &displayValue, stepSmallPtr, stepLargePtr, getFormatString(), ImGuiInputTextFlags_CharsScientific);
			if(ImGui::IsItemDeactivatedAfterEdit()){
				overwriteWithHistory(displayValue);
			}
			
		}
		else{
			ImGui::InputScalar(getImGuiID(), getImGuiDataType(), &displayValue, stepSmallPtr, stepLargePtr, getFormatString(), ImGuiInputTextFlags_CharsScientific);
			if(ImGui::IsItemDeactivatedAfterEdit()){
				overwriteWithHistory(displayValue);
			}
		}
			
		ImGui::EndDisabled();
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
		 overwrite(number);
		 return true;
	 }
	
	T getValue(){ return value; }
	
	void overwrite(T newValue){
		displayValue = newValue;
		value = newValue;
		//TURN DISPLAY HACK
		//fullTurns = std::floor(newValue / 360.0);
		//singleTurnPosition = newValue - fullTurns * 360.0;
		//for -179.99 to 180.0
		fullTurns = std::floor(newValue / 360.0);
		singleTurnPosition = newValue - fullTurns * 360.0;
		if(singleTurnPosition <= -180.0){
			singleTurnPosition += 360.0;
			fullTurns--;
		}
		else if(singleTurnPosition > 180.0){
			singleTurnPosition -= 360.0;
			fullTurns++;
		}
	}
	
	void overwriteWithHistory(T newValue){
		//=========Sanitize Value==========
		if(!b_allowNegatives && newValue < 0) displayValue = 0;
		else displayValue = newValue;
		if(value == newValue) return;
		//=========Invoke Command=========
		std::shared_ptr<NumberParameter<T>> thisParameter = std::static_pointer_cast<NumberParameter<T>>(shared_from_this());
		std::string commandName = "Changed " + std::string(getName()) + " from " + std::to_string(value) + " to " + std::to_string(displayValue);
		std::make_shared<EditCommand>(thisParameter, commandName)->execute();
		//=================================
	}
	
	template<typename O>
	void copyValue(std::shared_ptr<NumberParameter<O>> otherNumberParameter){
		value = otherNumberParameter->getValue();
	}
	
	//hacky way to get rev+degree working as part of the parameter object
	int fullTurns;
	double singleTurnPosition;
	bool b_useTurnAndDegrees = false;
	
private:
	
	ImGuiDataType getImGuiDataType();
	std::string getDefaultFormatString(){ return "%i"; } //template specialisation for real float and double types
	
	class EditCommand : public UndoableCommand{
	public:
		std::shared_ptr<NumberParameter<T>> parameter;
		T newValue;
		T previousValue;
		EditCommand(std::shared_ptr<NumberParameter<T>> parameter_, std::string& commandName) : UndoableCommand(commandName){
			parameter = parameter_;
			newValue = parameter->displayValue;
			previousValue = parameter->value;
		}
		void setNewValue(){
			parameter->lockMutex();
			parameter->value = newValue;
			
			//TURN DISPLAY HACK for 0.0 to 360.0
			//parameter->fullTurns = std::floor(newValue / 360.0);
			//parameter->singleTurnPosition = newValue - parameter->fullTurns * 360.0;
			
			//for -179.99 to 180.0
			int fullTurns = std::floor(newValue / 360.0);
			double degrees = newValue - fullTurns * 360.0;
			if(degrees <= -180.0){
				degrees += 360.0;
				fullTurns--;
			}
			else if(degrees > 180.0){
				degrees -= 360.0;
				fullTurns++;
			}
			parameter->fullTurns = fullTurns;
			parameter->singleTurnPosition = degrees;
			
			
			parameter->displayValue = newValue;
			parameter->unlockMutex();
		}
		void setOldValue(){
			parameter->lockMutex();
			parameter->value = previousValue;
			parameter->displayValue = previousValue;
			
			//TURN DISPLAY HACK
			parameter->fullTurns = std::floor(previousValue / 360.0);
			parameter->singleTurnPosition = previousValue - parameter->fullTurns * 360.0;
			
			parameter->unlockMutex();
		}
		virtual void onExecute(){ setNewValue(); parameter->onEdit(); }
		virtual void onUndo(){ setOldValue(); }
		virtual void onRedo(){ setNewValue(); }
	};
};

template<typename T>
using NumberParam = std::shared_ptr<NumberParameter<T>>;

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
		if(ImGui::IsItemDeactivatedAfterEdit()){
			overwriteWithHistory(displayValue);
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
	
	void overwriteWithHistory(T newValue){
		if(value == newValue) return;
		displayValue = newValue;
		//=========Command Invoker=========
		auto thisParameter = std::static_pointer_cast<VectorParameter<T>>(shared_from_this());
		std::string commandName = "Changed " + std::string(getName()) + " from " + glm::to_string(value) + " to " + glm::to_string(displayValue);
		std::make_shared<EditCommand>(thisParameter, commandName)->execute();
		//=================================
	}
	
	class EditCommand : public UndoableCommand{
	public:
		std::shared_ptr<VectorParameter<T>> parameter;
		T newValue;
		T previousValue;
		EditCommand(std::shared_ptr<VectorParameter<T>> parameter_, std::string& commandName) : UndoableCommand(commandName){
			parameter = parameter_;
			newValue = parameter->displayValue;
			previousValue = parameter->value;
		}
		void setNewValue(){
			parameter->lockMutex();
			parameter->value = newValue;
			parameter->displayValue = newValue;
			parameter->unlockMutex();
		}
		void setOldValue(){
			parameter->lockMutex();
			parameter->value = previousValue;
			parameter->displayValue = previousValue;
			parameter->unlockMutex();
		}
		virtual void onExecute(){ setNewValue(); parameter->onEdit(); }
		virtual void onUndo(){ setOldValue(); }
		virtual void onRedo(){ setNewValue(); }
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
inline void VectorParameter<glm::ivec2>::inputField(){
    ImGui::BeginDisabled(isDisabled());
    ImGui::InputInt2(getImGuiID(), &displayValue.x);
    ImGui::EndDisabled();
}

template<>
inline bool VectorParameter<glm::ivec2>::save(tinyxml2::XMLElement* xml){
    using namespace tinyxml2;
    XMLElement* element = xml->InsertNewChildElement(getSaveString());
    element->SetAttribute("x", value.x);
    element->SetAttribute("y", value.y);
    return true;
}

template<>
inline bool VectorParameter<glm::ivec2>::load(tinyxml2::XMLElement* xml){
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
	
	static std::shared_ptr<BooleanParameter> make(bool value_, std::string name_, std::string saveString_){
		return std::make_shared<BooleanParameter>(value_, name_, saveString_);
	};
	
	virtual void gui(ImFont* font = nullptr) override {
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
		ImGui::SameLine();
		if(font){
			ImGui::PushFont(font);
			ImGui::Text("%s", getName());
			ImGui::PopFont();
		}
	}
	
	virtual void onGui() override {
		ImGui::BeginDisabled(isDisabled());
		ImGui::Checkbox(getImGuiID(), &displayValue);
		ImGui::EndDisabled();
		if(ImGui::IsItemDeactivatedAfterEdit()){
			overwriteWithHistory(displayValue);
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
	
	void overwriteWithHistory(bool newValue){
		if(value == newValue) return;
		displayValue = newValue;
		//=========Command Invoker=========
		auto thisParameter = std::static_pointer_cast<BooleanParameter>(shared_from_this());
		std::string commandName = "Inverted " + std::string(getName());
		std::make_shared<InvertCommand>(thisParameter, commandName)->execute();
		//=================================
	}
	
	class InvertCommand : public UndoableCommand{
	public:
		std::shared_ptr<BooleanParameter> parameter;
		InvertCommand(std::shared_ptr<BooleanParameter> parameter_, std::string& commandName) : UndoableCommand(commandName){
			parameter = parameter_;
		}
		void invert(){
			parameter->lockMutex();
			parameter->value = !parameter->value;
			parameter->displayValue = parameter->value;
			parameter->unlockMutex();
		}
		virtual void onExecute(){ invert(); parameter->onEdit(); }
		virtual void onUndo(){ invert(); }
		virtual void onRedo(){ invert(); }
	};
	
};


using BoolParam = std::shared_ptr<BooleanParameter>;

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
	
	static std::shared_ptr<StringParameter> make(std::string value, std::string name, std::string saveName, size_t bufferSize){
		return std::make_shared<StringParameter>(value, name, saveName, bufferSize);
	};
	
	void overwrite(const char* value_){
		strcpy(displayValue, value_);
		value = value_;
	}
	
	virtual void onGui() override {
		ImGui::BeginDisabled(isDisabled());
		ImGui::InputText(getImGuiID(), displayValue, bufferSize);
		ImGui::EndDisabled();
		if(ImGui::IsItemDeactivatedAfterEdit()){
			overwriteWithHistory(displayValue);
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
	
	void overwriteWithHistory(std::string newValue){
		if(value == newValue) return;
		value = newValue;
		//=========Command Invoker=========
		std::shared_ptr<StringParameter> thisParameter = std::static_pointer_cast<StringParameter>(shared_from_this());
		std::string commandName = "Changed " + std::string(getName()) + " from \'" + value + "\' to \'" + displayValue + "\'";
		std::make_shared<EditCommand>(thisParameter, commandName)->execute();
		//=================================
	}
	
	class EditCommand : public UndoableCommand{
	public:
		std::shared_ptr<StringParameter> parameter;
		std::string newValue;
		std::string previousValue;
		EditCommand(std::shared_ptr<StringParameter> parameter_, std::string& commandName) : UndoableCommand(commandName){
			parameter = parameter_;
			newValue = parameter->displayValue;
			previousValue = parameter->value;
		}
		void setNewValue(){
			parameter->lockMutex();
			parameter->value = newValue;
			strcpy(parameter->displayValue, newValue.c_str());
			parameter->unlockMutex();
		}
		void setOldValue(){
			parameter->lockMutex();
			parameter->value = previousValue;
			strcpy(parameter->displayValue, previousValue.c_str());
			parameter->unlockMutex();
		}
		virtual void onExecute(){ setNewValue(); parameter->onEdit(); }
		virtual void onUndo(){ setOldValue(); }
		virtual void onRedo(){ setNewValue(); }
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
	
	static std::shared_ptr<EnumeratorParameter<T>> make(T value_, std::string name, std::string saveString){
		return std::make_shared<EnumeratorParameter<T>>(value_, name, saveString);
	};
	
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
					if(ImGui::Selectable(Enumerator::getDisplayString(type), type == displayValue))  overwriteWithHistory(type);
				}
			}else{
				for(auto& type : Enumerator::getTypes<T>()){
					if(ImGui::Selectable(type.displayString, type.enumerator == displayValue)) overwriteWithHistory(type.enumerator);
				}
			}
			ImGui::EndCombo();
		}
		
		ImGui::EndDisabled();
	}
	
	virtual bool save(tinyxml2::XMLElement* xml) override {
		xml->SetAttribute(getSaveString(), Enumerator::getSaveString(value));
		return true;
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
	
	void overwriteWithHistory(T newValue){
		if(value == newValue) return;
		displayValue = newValue;
		//=========Command Invoker=========
		std::shared_ptr<EnumeratorParameter<T>> thisParameter = std::static_pointer_cast<EnumeratorParameter<T>>(shared_from_this());
		std::string commandName = "Changed " + std::string(getName()) + " from \'" + Enumerator::getDisplayString(value) + "\' to \'" + Enumerator::getDisplayString(displayValue) + "\'";
		std::make_shared<EditCommand>(thisParameter, commandName)->execute();
		//=================================
	}
	
	class EditCommand : public UndoableCommand{
	public:
		std::shared_ptr<EnumeratorParameter<T>> parameter;
		T oldValue;
		T newValue;
		EditCommand(std::shared_ptr<EnumeratorParameter<T>> parameter_, std::string& commandName) : UndoableCommand(commandName){
			parameter = parameter_;
			oldValue = parameter->value;
			newValue = parameter->displayValue;
		}
		void setNewValue(){
			parameter->lockMutex();
			parameter->value = newValue;
			parameter->displayValue = newValue;
			parameter->unlockMutex();
		}
		void setOldValue(){
			parameter->lockMutex();
			parameter->value = oldValue;
			parameter->displayValue = oldValue;
			parameter->unlockMutex();
		}
		virtual void onExecute(){ setNewValue(); parameter->onEdit(); }
		virtual void onUndo(){ setOldValue(); }
		virtual void onRedo(){ setNewValue(); }
	};
		
};

template<typename T>
using EnumParam = std::shared_ptr<EnumeratorParameter<T>>;
template<typename T>
using EnumParam = std::shared_ptr<EnumeratorParameter<T>>;


//===================================================================================
//=================================== STATE PARAMETER ===============================
//===================================================================================

#include "Animation/Animatables/AnimatableState.h"

class StateParameter : public Parameter{
public:
	
	AnimatableStateStruct* value;
	AnimatableStateStruct* displayValue;
	std::vector<AnimatableStateStruct*>* selectableStates;
	
	StateParameter(AnimatableStateStruct* value_, std::vector<AnimatableStateStruct*>& selectableStates_, std::string name, std::string saveString_) :
	Parameter(name, saveString_){
		value = value_;
		displayValue = value_;
		selectableStates = &selectableStates_;
	}
	
	virtual void onGui() override {
		ImGui::BeginDisabled(isDisabled());
		if(ImGui::BeginCombo(getImGuiID(), displayValue->displayName)){
			for(auto& state : *selectableStates){
				if(ImGui::Selectable(state->displayName, state == displayValue)){
					overwriteWithHistory(state);
				}
			}
			ImGui::EndCombo();
		}
		ImGui::EndDisabled();
	}
	
	virtual bool save(tinyxml2::XMLElement* xml) override {
		xml->SetAttribute(getSaveString(), value->saveName);
		return true;
	}
	
	virtual bool load(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		const char * stateSaveString;
		XMLError result = xml->QueryStringAttribute(getSaveString(), &stateSaveString);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", getName());
		
		for(auto& state : *selectableStates){
			if(strcmp(state->saveName, stateSaveString) == 0){
				value = state;
				displayValue = value;
				return true;
			}
		}
		
		return Logger::warn("Could not load parameter {} : could not identity state string : {}", getName(), stateSaveString);
	}
	
	std::shared_ptr<StateParameter> makeCopy() {
		return std::make_shared<StateParameter>(value, *selectableStates, getName(), getSaveString());
	}
	
	virtual std::shared_ptr<Parameter> makeBaseCopy() override { return makeCopy(); };
	
	void overwrite(AnimatableStateStruct* newValue){
		displayValue = newValue;
		value = newValue;
	}
	
	void overwriteWithHistory(AnimatableStateStruct* newValue){
		if(value == newValue) return;
		displayValue = newValue;
		//=========Command Invoker=========
		std::shared_ptr<StateParameter> thisParameter = std::static_pointer_cast<StateParameter>(shared_from_this());
		std::string commandName = "Changed " + std::string(getName()) + " from \'" + value->displayName + "\' to \'" + displayValue->displayName + "\'";
		std::make_shared<EditCommand>(thisParameter, commandName)->execute();
		//=================================
	}
	
	class EditCommand : public UndoableCommand{
	public:
		std::shared_ptr<StateParameter> parameter;
		AnimatableStateStruct* oldValue;
		AnimatableStateStruct* newValue;
		
		EditCommand(std::shared_ptr<StateParameter> parameter_, std::string& commandName) : UndoableCommand(commandName){
			parameter = parameter_;
			oldValue = parameter->value;
			newValue = parameter->displayValue;
		}
		void setNewValue(){
			parameter->lockMutex();
			parameter->value = newValue;
			parameter->displayValue = newValue;
			parameter->unlockMutex();
		}
		void setOldValue(){
			parameter->lockMutex();
			parameter->value = oldValue;
			parameter->displayValue = oldValue;
			parameter->unlockMutex();
		}
		virtual void onExecute(){ setNewValue(); parameter->onEdit(); }
		virtual void onUndo(){ setOldValue(); }
		virtual void onRedo(){ setNewValue(); }
	};
	
};


#include "Animation/Playback/TimeStringConversion.h"

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
			overwriteWithHistory(newTime);
		}
		ImGui::EndDisabled();
	}
	
	virtual bool save(tinyxml2::XMLElement* xml) override {
		xml->SetAttribute(getSaveString(), value);
		return true;
	}
	
	virtual bool load(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		XMLError result = xml->QueryDoubleAttribute(getSaveString(), &value);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", getName());
		overwrite(value);
		return true;
	}
	
	std::shared_ptr<TimeParameter> makeCopy() {
		return std::make_shared<TimeParameter>(value, getName(), getSaveString());
	}
	
	virtual std::shared_ptr<Parameter> makeBaseCopy() override { return makeCopy(); };
	
	void overwrite(double newValue){
		value = newValue;
		strcpy(displayBuffer, TimeStringConversion::secondsToTimecodeString(newValue).c_str());
	}
	
	void overwriteWithHistory(double newValue){
		if(value == newValue) {
			strcpy(displayBuffer, TimeStringConversion::secondsToTimecodeString(newValue).c_str());
			return;
		}
		std::string newTimeString = TimeStringConversion::secondsToTimecodeString(newValue);
		std::string name = "Change " + std::string(getName()) + " from " + TimeStringConversion::secondsToTimecodeString(value) + " to " + newTimeString;
		std::make_shared<EditCommand>(std::static_pointer_cast<TimeParameter>(shared_from_this()), name)->execute();
	}
	
	class EditCommand : public UndoableCommand{
	public:
		std::shared_ptr<TimeParameter> parameter;
		double oldValue;
		double newValue;
		
		EditCommand(std::shared_ptr<TimeParameter> parameter_, std::string& commandName) : UndoableCommand(commandName){
			parameter = parameter_;
			oldValue = parameter->value;
			newValue = TimeStringConversion::timecodeStringToSeconds(parameter->displayBuffer);
		}
		void setNewValue(){
			parameter->lockMutex();
			parameter->overwrite(newValue);
			parameter->unlockMutex();
		}
		void setOldValue(){
			parameter->lockMutex();
			parameter->overwrite(oldValue);
			parameter->unlockMutex();
		}
		virtual void onExecute(){ setNewValue(); parameter->onEdit(); }
		virtual void onUndo(){ setOldValue(); }
		virtual void onRedo(){ setNewValue(); }
	};
	
};







inline bool loadXMLElement(std::string elementName, tinyxml2::XMLElement* parentElement, tinyxml2::XMLElement*& output){
	tinyxml2::XMLElement* element = parentElement->FirstChildElement(elementName.c_str());
	if(element == nullptr){
		Logger::warn("could not load xml element {}", elementName);
		return false;
	}
	output = element;
	return true;
}


class OptionParameter : public Parameter{
public:

	class Option{
	public:
		Option(){}
		Option(int enumerator_, std::string displayString_, std::string saveString_, bool b_enabled_ = true){
			enumerator = enumerator_;
			displayString = displayString_;
			saveString = saveString_;
			b_enabled = b_enabled_;
		}
		void enable(){ b_enabled = true; }
		void disable(){ b_enabled = false; }
		bool isEnabled(){ return b_enabled; }
		int getInt(){ return enumerator; }
	private:
		friend class OptionParameter;
		int enumerator = 0;
		std::string displayString = "";
		std::string saveString = "";
		bool b_enabled = true;
	};
	

	
	
	int value;
	Option* displayValue;
	std::vector<Option*> optionList = {};
	
	OptionParameter(Option& value, std::vector<Option*>& options, std::string name, std::string saveString_) : Parameter(name, saveString_){
		optionList = options;
		overwrite(&value);
	}
	
	static std::shared_ptr<OptionParameter> make(Option& value, std::vector<Option*>& options, std::string name, std::string saveString) {
		return std::make_shared<OptionParameter>(value, options, name, saveString);
	}
	
	static std::shared_ptr<OptionParameter> make2(Option& value, std::vector<Option*> options, std::string name, std::string saveString) {
		return std::make_shared<OptionParameter>(value, options, name, saveString);
	}
	
	virtual void onGui() override {
		ImGui::BeginDisabled(isDisabled());
		const char* previewString = "";
		if(displayValue) previewString = displayValue->displayString.c_str();
		if(ImGui::BeginCombo(getImGuiID(), previewString)){
			for(auto& option : optionList){
				ImGui::BeginDisabled(!option->b_enabled);
				if(ImGui::Selectable(option->displayString.c_str(), value == option->enumerator)){
					overwriteWithHistory(option);
				}
				ImGui::EndDisabled();
			}
			ImGui::EndCombo();
		}
		ImGui::EndDisabled();
	}
	
	virtual bool save(tinyxml2::XMLElement* xml) override {
		xml->SetAttribute(getSaveString(), value);
		return true;
	}
	
	virtual bool load(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		XMLError result = xml->QueryIntAttribute(getSaveString(), &value);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", getName());
		return overwrite(value);
	}
	
	std::shared_ptr<TimeParameter> makeCopy() {
		return std::make_shared<TimeParameter>(value, getName(), getSaveString());
	}
	
	virtual std::shared_ptr<Parameter> makeBaseCopy() override { return makeCopy(); };
	
	Option* findOption(int enumerator){
		if(optionList.empty()) return nullptr;
		for(auto option : optionList){
			if(option->enumerator == enumerator) return option;
		}
		Logger::error("Parameter {} : option with enumerator {} not found", getName(), enumerator);
		return nullptr;
	}
	
	void overwrite(Option* newValue){
		value = newValue->enumerator;
		displayValue = newValue;
	}
	
	bool overwrite(int newEnumerator){
		value = newEnumerator;
		displayValue = findOption(newEnumerator);
		return displayValue != nullptr;
	}
	
	void overwriteWithHistory(Option* newValue){
		if(value == newValue->enumerator) return;
		displayValue = newValue;
		std::string previousValueString = "";
		if(displayValue) previousValueString = displayValue->displayString;
		std::string name = "Change " + std::string(getName()) + " from " + previousValueString + " to " + newValue->displayString;
		std::make_shared<EditCommand>(std::static_pointer_cast<OptionParameter>(shared_from_this()), name)->execute();
	}
	
	class EditCommand : public UndoableCommand{
	public:
		std::shared_ptr<OptionParameter> parameter;
		int oldValue;
		int newValue;
		
		EditCommand(std::shared_ptr<OptionParameter> parameter_, std::string& commandName) : UndoableCommand(commandName){
			parameter = parameter_;
			oldValue = parameter->value;
			newValue = parameter->displayValue->enumerator;
		}
		void setNewValue(){
			parameter->lockMutex();
			parameter->overwrite(newValue);
			parameter->unlockMutex();
		}
		void setOldValue(){
			parameter->lockMutex();
			parameter->overwrite(oldValue);
			parameter->unlockMutex();
		}
		virtual void onExecute(){ setNewValue(); parameter->onEdit(); }
		virtual void onUndo(){ setOldValue(); }
		virtual void onRedo(){ setNewValue(); }
	};
	
};

using OptionParam = std::shared_ptr<OptionParameter>;
using Option = OptionParameter::Option;

