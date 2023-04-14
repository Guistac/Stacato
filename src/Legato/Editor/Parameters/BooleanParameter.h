#pragma once

#include "../Parameter.h"

namespace Legato{


class BooleanParameter : public Parameter{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(BooleanParameter)
	
public:
	
	static std::shared_ptr<BooleanParameter> createInstance(bool defaultValue, std::string name, std::string saveString){
		auto newParameter = BooleanParameter::createInstance();
		newParameter->setName(name);
		newParameter->setSaveString(saveString);
		newParameter->overwrite(defaultValue);
		return newParameter;
	}
		
	virtual void onGui() override;
	
	bool getValue(){ return value; }
	
	void overwrite(bool newValue){
		displayValue = newValue;
		value = newValue;
	}
	
	void overwriteWithHistory(bool newValue){
		overwrite(newValue);
		/*
		if(value == newValue) return;
		displayValue = newValue;
		//=========Command Invoker=========
		auto thisParameter = std::static_pointer_cast<BooleanParameter>(shared_from_this());
		std::string commandName = "Inverted " + std::string(getName());
		std::make_shared<InvertCommand>(thisParameter, commandName)->execute();
		//=================================
		 */
	}
	
	/*
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
	
	bool displayValue;
	bool value;
	
};

using BoolParam = std::shared_ptr<BooleanParameter>;

};
