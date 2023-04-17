#include <pch.h>

#include "Component.h"
#include "Parameters/StringParameter.h"

namespace Legato{

void Component::setName(std::string name){
	if(b_hasNameParameter) {
		nameParameter->overwrite(name);
		nameParameter->onEdit();
	}
	else nonParametricName = name;
}

const std::string& Component::getName(){
	if(b_hasNameParameter) return nameParameter->getValue();
	return nonParametricName;
}

bool Component::onSerialization() {
	bool success = true;
	if(b_hasNameParameter) success &= serializeAttribute("Name", nameParameter->getValue());
	return success;
}

bool Component::onDeserialization() {
	bool success = true;
	if(b_hasNameParameter) {
		std::string componentName;
		success &= deserializeAttribute("Name", componentName);
		nameParameter->overwrite(componentName);
		nameParameter->onEdit();
	}
	return success;
}

void Component::onConstruction() {
	if(b_hasNameParameter){
		nameParameter = StringParameter::createInstance("Default Component Name", "Name", "Name");
	}else{
		nonParametricName = "Name";
	}
}

void Component::onCopyFrom(std::shared_ptr<PrototypeBase> source) {
	auto original = std::static_pointer_cast<Component>(source);
	if(b_hasNameParameter){
		nameParameter->overwrite(original->nameParameter->getValue() + " copy");
	}else{
		nonParametricName = original->nameParameter->getValue();
	}
}

}
