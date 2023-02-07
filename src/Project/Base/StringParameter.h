#pragma once

#include "Parameter.h"

class NewStringParameter : public BaseParameter{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(NewStringParameter)
	
public:
	
	static std::shared_ptr<NewStringParameter> createInstanceWithoutNameParameter(){
		std::shared_ptr<NewStringParameter> instance = std::shared_ptr<NewStringParameter>(new NewStringParameter(false));
		instance->onConstruction();
		return instance;
	}
	
	const std::string& getValue(){ return value; }
	
	void setValue(std::string newValue){ value = newValue; }
	
	virtual void gui() override {}
	
private:
	
	virtual void onConstruction() override {
		BaseParameter::onConstruction();
		value = "Default Value";
	}
	
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		BaseParameter::onCopyFrom(source);
		auto original = std::static_pointer_cast<NewStringParameter>(source);
		value = original->value;
	}
	
	virtual bool onDeserialization() override {
		bool success = true;
		success &= BaseParameter::onDeserialization();
		success &= deserializeAttribute("Value", value);
		return success;
	}
	
	virtual bool onSerialization() override {
		bool success = true;
		success &= BaseParameter::onSerialization();
		success &= serializeAttribute("Value", value.c_str());
		return success;
	}
	
private:
	
	std::string value;
	NewStringParameter(bool withoutNameParameter) : BaseParameter(withoutNameParameter){}
};


/*
class ParameterNameParameter{
	public:
};
*/
