#pragma once

#include "../Parameter.h"


namespace Legato{

template<typename T>
class VectorParameter : public Parameter{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(VectorParameter<T>)
	
public:
	
	static std::shared_ptr<VectorParameter> createInstance(T defaultValue, std::string name, std::string saveString){
		auto newParameter = VectorParameter::createInstance();
		newParameter->setName(name);
		newParameter->setSaveString(saveString);
		newParameter->overwrite(defaultValue);
		return newParameter;
	}
	
	T getValue(){ return value; }
	
	void setFormat(std::string format_){
		format = format_;
	}
	
	void inputField();
	
	virtual void onGui() override;
	
	void overwrite(T newValue){
		displayValue = newValue;
		value = newValue;
	}
	
	void overwriteWithHistory(T newValue){
		overwrite(newValue);
		/*
		if(value == newValue) return;
		displayValue = newValue;
		//=========Command Invoker=========
		auto thisParameter = std::static_pointer_cast<VectorParameter<T>>(shared_from_this());
		std::string commandName = "Changed " + std::string(getName()) + " from " + glm::to_string(value) + " to " + glm::to_string(displayValue);
		std::make_shared<EditCommand>(thisParameter, commandName)->execute();
		//=================================
		 */
	}
	
	/*
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
	 */
	
private:
	
	bool save();
	bool load();
	
	virtual void onConstruction() override {
		Parameter::onConstruction();
	}
	
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override {
		Parameter::onCopyFrom(source);
		auto original = downcasted_shared_from_this<VectorParameter<T>>();
	}
	
	virtual bool onSerialization() override {
		bool success = true;
		success &= Parameter::onSerialization();
		success &= save();
		return success;
	}
	
	virtual bool onDeserialization() override {
		bool success = true;
		success &= Parameter::onDeserialization();
		success &= load();
		overwrite(value);
		return success;
	}
	
private:
	
	T displayValue;
	T value;
	std::string format;
	
};



};
