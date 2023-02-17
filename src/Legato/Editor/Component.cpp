#include <pch.h>

#include "Component.h"
#include "StringParameter.h"

void Component::setName(std::string name){
	if(b_hasNameParameter) nameParameter->setValue(name);
	else nonParametricName = name;
}

const std::string& Component::getName(){
	if(b_hasNameParameter) return nameParameter->getValue();
	return nonParametricName;
}

bool Component::onSerialization() {
	bool success = true;
	if(b_hasNameParameter) success &= nameParameter->serializeIntoParent(this);
	return success;
}

bool Component::onDeserialization() {
	bool success = true;
	if(b_hasNameParameter) success &= nameParameter->deserializeFromParent(this);
	return success;
}

void Component::onConstruction() {
	if(b_hasNameParameter){
		nameParameter = NewStringParameter::createInstanceWithoutNameParameter();
		nameParameter->setValue("Default Component Name");
		nameParameter->setSaveString("ComponentName");
	}else{
		nonParametricName = "Name";
	}
}

void Component::onCopyFrom(std::shared_ptr<PrototypeBase> source) {
	auto original = std::static_pointer_cast<Component>(source);
	if(b_hasNameParameter){
		nameParameter->setValue(original->nameParameter->getValue() + " copy");
	}else{
		nonParametricName = original->nameParameter->getValue();
	}
}
