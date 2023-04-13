#include <pch.h>

#include "Component.h"
#include "Parameters/StringParameter.h"

#include "Parameters/NumberParameter.h"
#include "Parameters/BooleanParameter.h"
#include "Parameters/VectorParameter.h"
#include "Parameters/OptionParameter.h"
#include "Parameters/TimeParameter.h"

namespace Legato{

void Component::setName(std::string name){
	if(b_hasNameParameter) nameParameter->overwrite(name);
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
	}
	return success;
}

void Component::onConstruction() {
	if(b_hasNameParameter){
		nameParameter = StringParameter::createInstanceWithoutNameParameter();
		nameParameter->overwrite("Default Component Name");
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
