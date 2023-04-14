#pragma once

#include "../Parameter.h"

namespace Legato{

template<typename T>
class NumberParameter : public Parameter{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(NumberParameter)
	
public:
	
	static std::shared_ptr<NumberParameter> createInstance(T defaultValue, std::string name, std::string saveString){
		auto newParameter = NumberParameter::createInstance();
		newParameter->setName(name);
		newParameter->setSaveString(saveString);
		newParameter->overwrite(defaultValue);
		newParameter->setFormat(newParameter->getDefaultFormatString());
		return newParameter;
	}
	
	void setPrefix(std::string p) { prefix = p; updateFormatString(); }
	void setFormat(std::string f) { format = f; updateFormatString();  }
	void setUnit(Unit u){ unit = u; updateFormatString(); }
	void setSuffix(std::string s) { suffix = s; updateFormatString();  }
	void allowNegatives(bool allowNegatives){ b_allowNegatives = allowNegatives; }
	void setStepSize(double small, double large) {
		stepSmall = small;
		if(stepSmall == 0) stepSmallPtr = nullptr;
		else stepSmallPtr = &stepSmall;
		stepLarge = large;
		if(stepLarge == 0) stepLargePtr = nullptr;
		else stepLargePtr = &stepLarge;
	}
	
	bool validateRange(double rangeMin, double rangeMax, bool withMin, bool withMax) {
		bool valid = true;
		if(withMin && value < rangeMin) valid = false;
		else if(!withMin && value <= rangeMin) valid = false;
		else if(withMax && value > rangeMax) valid = false;
		else if(!withMax && value >= rangeMax) valid = false;
		setValid(valid);
		return valid;
	};
	
	virtual void onGui() override;
	
	T getValue(){ return value; }
	
	void overwrite(T newValue){
		if(!b_allowNegatives && newValue < 0) newValue = 0;
		displayValue = newValue;
		value = newValue;
	}
	
	void overwriteWithHistory(T newValue){
		overwrite(newValue);
		/*
		 //=========Sanitize Value==========
		 if(!b_allowNegatives && newValue < 0) displayValue = 0;
		 else displayValue = newValue;
		 if(value == newValue) return;
		 //=========Invoke Command=========
		 std::shared_ptr<NumberParameter<T>> thisParameter = std::static_pointer_cast<NumberParameter<T>>(shared_from_this());
		 std::string commandName = "Changed " + std::string(getName()) + " from " + std::to_string(value) + " to " + std::to_string(displayValue);
		 std::make_shared<EditCommand>(thisParameter, commandName)->execute();
		 //=================================
		 */
	}
	
private:
	
	void updateFormatString(){
		const char* unitAbbreviated = unit->abbreviated;
		if(unit == Units::Fraction::Percent) unitAbbreviated = "%%";
		formatString = prefix + format + std::string(unitAbbreviated) + suffix;
	}
	ImGuiDataType getImGuiDataType();
	std::string getDefaultFormatString();
	
	/*
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
	 */
	
private:
	
	virtual void onConstruction() override {
		Parameter::onConstruction();
	}
	
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Parameter::onCopyFrom(source);
		auto original = std::static_pointer_cast<StringParameter>(source);
	}
	
	virtual bool onSerialization() override {
		bool success = true;
		success &= Parameter::onSerialization();
		success &= serializeAttribute("Value", double(value));
		return success;
	}
	
	virtual bool onDeserialization() override {
		bool success = true;
		success &= Parameter::onDeserialization();
		double loadedValue;
		success &= deserializeAttribute("Value", loadedValue);
		overwrite(loadedValue);
		return success;
	}
	
private:
	
	T displayValue;
	T value;
	
	T stepSmall;
	T stepLarge;
	T* stepSmallPtr;
	T* stepLargePtr;
	
	std::string prefix;
	std::string format;
	Unit unit = Units::None::None;
	std::string suffix;
	std::string formatString;
	
	bool b_allowNegatives = true;
};

template<typename T>
using NumberParam = std::shared_ptr<NumberParameter<T>>;

template<typename T>
void NumberParameter<T>::onGui() {
	ImGui::BeginDisabled(isDisabled());
	ImGui::InputScalar(getImGuiID(), getImGuiDataType(), &displayValue, stepSmallPtr, stepLargePtr, formatString.c_str(), ImGuiInputTextFlags_CharsScientific);
	ImGui::EndDisabled();
	if(ImGui::IsItemDeactivatedAfterEdit()){
		overwriteWithHistory(displayValue);
	}
}

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
inline std::string NumberParameter<uint8_t>::getDefaultFormatString(){ return "%i"; }
template<>
inline std::string NumberParameter<int8_t>::getDefaultFormatString(){ return "%i"; }
template<>
inline std::string NumberParameter<uint16_t>::getDefaultFormatString(){ return "%i"; }
template<>
inline std::string NumberParameter<int16_t>::getDefaultFormatString(){ return "%i"; }
template<>
inline std::string NumberParameter<uint32_t>::getDefaultFormatString(){ return "%i"; }
template<>
inline std::string NumberParameter<int32_t>::getDefaultFormatString(){ return "%i"; }
template<>
inline std::string NumberParameter<uint64_t>::getDefaultFormatString(){ return "%i"; }
template<>
inline std::string NumberParameter<int64_t>::getDefaultFormatString(){ return "%i"; }
template<>
inline std::string NumberParameter<float>::getDefaultFormatString(){ return "%.3f"; }
template<>
inline std::string NumberParameter<double>::getDefaultFormatString(){ return "%.3f"; }

};
