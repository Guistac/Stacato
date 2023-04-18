#pragma once

#include "../Parameter.h"

namespace Legato{

class StringParameter : public Parameter{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(StringParameter)
	
public:
	
	static std::shared_ptr<StringParameter> createInstance(std::string value,
														   std::string name,
														   std::string saveString,
														   size_t bufferSize = 128){
		auto newParameter = StringParameter::createInstance();
		delete newParameter->displayValue;
		newParameter->displayValue = new char[bufferSize];
		newParameter->bufferSize = bufferSize;
		newParameter->setName(name);
		newParameter->setSaveString(saveString);
		newParameter->overwrite(value);
		return newParameter;
	}
	
public:
	
	const std::string& getValue(){ return value; }
	
	virtual void onGui() override;
	
	void overwrite(std::string newValue){
		strcpy(displayValue, newValue.c_str());
		value = newValue;
	}
	
	
	void overwriteWithHistory(std::string newValue){
		overwrite(newValue);
		/*
		if(value == newValue) return;
		value = newValue;
		//=========Command Invoker=========
		std::shared_ptr<StringParameter> thisParameter = std::static_pointer_cast<StringParameter>(shared_from_this());
		std::string commandName = "Changed " + std::string(getName()) + " from \'" + value + "\' to \'" + displayValue + "\'";
		std::make_shared<EditCommand>(thisParameter, commandName)->execute();
		//=================================
		*/
	}
	
	/*
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
	*/
	
private:
	
	virtual void onConstruction() override {
		Parameter::onConstruction();
		displayValue = new char[128];
		bufferSize = 128;
	}
	
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Parameter::onCopyFrom(source);
		auto original = std::static_pointer_cast<StringParameter>(source);
		overwrite(original->getValue());
	}
	
	virtual bool onSerialization() override {
		bool success = true;
		success &= Parameter::onSerialization();
		success &= serializeAttribute("Value", value);
		return success;
	}
	
	virtual bool onDeserialization() override {
		bool success = true;
		success &= Parameter::onDeserialization();
		success &= deserializeAttribute("Value", value);
		overwrite(value);
		return success;
	}
	
private:
	
	char* displayValue;
	size_t bufferSize;
	std::string value;
};

using StringParam = std::shared_ptr<StringParameter>;


}

