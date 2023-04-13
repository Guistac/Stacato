#pragma once

#include "../Parameter.h"

namespace Legato{

class BaseNumberParameter : public Parameter{
public:
	std::shared_ptr<BaseNumberParameter> duplicate(){
		return std::static_pointer_cast<BaseNumberParameter>(duplicatePrototype());
	}
protected:
	BaseNumberParameter() : Parameter(false) {}
	
public:
	
	virtual void setStepSize(double small, double large) = 0;
	
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
	
private:
	
	virtual void onConstruction() override {
		Parameter::onConstruction();
	}
	
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Parameter::onCopyFrom(source);
	}
	
	virtual bool onSerialization() override {
		bool success = true;
		success &= Parameter::onSerialization();
		return success;
	}
	
	virtual bool onDeserialization() override {
		bool success = true;
		success &= Parameter::onDeserialization();
		return success;
	}
	
private:
	
	std::string prefix;
	std::string format;
	Unit unit;
	std::string suffix;
	std::string formatString;
	
};




template<typename T>
class NumberParameter : public BaseNumberParameter{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(NumberParameter)
	
public:
	
	static std::shared_ptr<NumberParameter> createInstance(T defaultValue, std::string name, std::string saveString){
		auto newParameter = NumberParameter::createInstance();
		newParameter->setName(name);
		newParameter->setSaveString(saveString);
		newParameter->overwrite(defaultValue);
		newParameter->setFormat(newParameter->getDefaultFormatString());
	}

	virtual void setStepSize(double small, double large) override {
		stepSmall = small;
		if(stepSmall == 0) stepSmallPtr = nullptr;
		else stepSmallPtr = &stepSmall;
		stepLarge = large;
		if(stepLarge == 0) stepLargePtr = nullptr;
		else stepLargePtr = &stepLarge;
	}
	
	virtual void onGui() override;
	
	T getValue(){ return value; }
	
	void overwrite(T newValue){
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
	
	ImGuiDataType getImGuiDataType();
	std::string getDefaultFormatString(){ return "%i"; } //template specialisation for real float and double types
	
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
};

};
