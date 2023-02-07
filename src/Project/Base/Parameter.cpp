#include <pch.h>

#include "Parameter.h"

void BaseParameter::setName(std::string name){
	if(b_createNameParameter) nameParameter->setValue(name);
	else Logger::error("Can't assign name '{}' to parameter : it can't have a name", name);
}

std::string BaseParameter::getName(){
	if(b_createNameParameter) return nameParameter->getValue();
	else{
		Logger::error("Can't return parameter name : it can't have a name");
		return "[no parameter name]";
	}
}

void BaseParameter::onConstruction(){
	if(b_createNameParameter) {
		nameParameter = NewStringParameter::createInstanceWithoutName();
		nameParameter->setValue("Default Parameter Name");
		nameParameter->setSaveString("Name");
	}
}

void BaseParameter::onCopyFrom(std::shared_ptr<PrototypeBase> source){
	auto original = std::static_pointer_cast<NewStringParameter>(source);
	if(b_createNameParameter){
		nameParameter->setValue(original->nameParameter->getValue());
	}
}

bool BaseParameter::onSerialization() {
	bool success = true;
	if(b_createNameParameter){
		success &= serializeAttribute("Name", nameParameter->getValue().c_str());
	}
	return success;
}

bool BaseParameter::onDeserialization(){
	bool success = true;
	if(b_createNameParameter){
		std::string name;
		success &= deserializeAttribute("Name", name);
		nameParameter->setValue(name);
	}
	return success;
}
