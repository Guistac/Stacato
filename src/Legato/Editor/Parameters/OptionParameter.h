#pragma once

#include "../Parameter.h"


namespace Legato{

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
	const std::string& getSaveString(){ return saveString; }
private:
	friend class OptionParameter;
	int enumerator = 0;
	std::string displayString = "";
	std::string saveString = "";
	bool b_enabled = true;
};



class OptionParameter : public Parameter{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(OptionParameter)
	
public:
	
	static std::shared_ptr<OptionParameter> createInstance(Option& defaultValue, std::vector<Option*> options, std::string name, std::string saveString){
		auto newParameter = OptionParameter::createInstance();
		newParameter->setName(name);
		newParameter->setSaveString(saveString);
		newParameter->overwrite(defaultValue);
		newParameter->optionList = options;
		return newParameter;
	}
	
	virtual void onGui() override;
	
	int getValue(){ return value; }
	Option* getOptionValue(){ return findOption(value); }
	
	Option* findOption(int enumerator){
		if(optionList.empty()) return nullptr;
		for(auto option : optionList){
			if(option->enumerator == enumerator) return option;
		}
		Logger::error("Parameter {} : option with enumerator {} not found", getName(), enumerator);
		return nullptr;
	}
	
	Option* findOption(std::string saveString){
		if(optionList.empty()) return nullptr;
		for(auto option : optionList){
			if(option->saveString == saveString) return option;
		}
		Logger::error("Parmaeter {} : option with savestring {} not found", getName(), saveString);
		return nullptr;
	}
	
	void overwrite(Option& newValue){
		value = newValue.enumerator;
		displayValue = &newValue;
	}
	
	bool overwrite(int newEnumerator){
		value = newEnumerator;
		displayValue = findOption(newEnumerator);
		return displayValue != nullptr;
	}
	
	void overwriteWithHistory(Option& newValue){
		overwrite(newValue);
		/*
		if(value == newValue->enumerator) return;
		displayValue = newValue;
		std::string previousValueString = "";
		if(displayValue) previousValueString = displayValue->displayString;
		std::string name = "Change " + std::string(getName()) + " from " + previousValueString + " to " + newValue->displayString;
		std::make_shared<EditCommand>(std::static_pointer_cast<OptionParameter>(shared_from_this()), name)->execute();
		 */
	}
	
	/*
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
	*/
	
private:
	
	virtual void onConstruction() override {
		Parameter::onConstruction();
	}
	
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override {
		Parameter::onCopyFrom(source);
		auto original = source->downcasted_shared_from_this<OptionParameter>();
	}
	
	virtual bool onSerialization() override {
		bool success = true;
		success &= Parameter::onSerialization();
		if(Option* option = findOption(value)){
			success &= serializeAttribute("Value", option->saveString);
		}
		else success &= serializeAttribute("Value", 0);
		return success;
	}
	
	virtual bool onDeserialization() override {
		bool success = true;
		success &= Parameter::onDeserialization();
		std::string valueSaveString;
		success &= deserializeAttribute("Value", valueSaveString);
		if(Option* option = findOption(valueSaveString)){
			overwrite(*option);
		}
		else success &= false;
		return success;
	}
	
private:
	
	int value;
	Option* displayValue;
	std::vector<Option*> optionList = {};
	
};

using OptionParam = std::shared_ptr<OptionParameter>;

};
