#pragma once

#include "../Parameter.h"


namespace Legato{

class TimeParameter : public Parameter{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(TimeParameter)
	
public:
	
	static std::shared_ptr<TimeParameter> createInstance(double defaultValue, std::string name, std::string saveString){
		auto newParameter = TimeParameter::createInstance();
		newParameter->setName(name);
		newParameter->setSaveString(saveString);
		newParameter->overwrite(defaultValue);
	}
	
	
	virtual void onGui() override;
	
	void overwrite(double newValue){
		value = newValue;
		//strcpy(displayBuffer, TimeStringConversion::secondsToTimecodeString(newValue).c_str());
	}
	
	void overwriteWithHistory(double newValue){
		overwrite(newValue);
		/*
		if(value == newValue) {
			strcpy(displayBuffer, TimeStringConversion::secondsToTimecodeString(newValue).c_str());
			return;
		}
		std::string newTimeString = TimeStringConversion::secondsToTimecodeString(newValue);
		std::string name = "Change " + std::string(getName()) + " from " + TimeStringConversion::secondsToTimecodeString(value) + " to " + newTimeString;
		std::make_shared<EditCommand>(std::static_pointer_cast<TimeParameter>(shared_from_this()), name)->execute();
		*/
	}
	
	/*
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
	
	double value;
	char displayBuffer[32];
	
};

};
