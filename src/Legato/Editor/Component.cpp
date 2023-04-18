#include <pch.h>

#include "Component.h"
#include "Parameters/StringParameter.h"

namespace Legato{

bool Component::onSerialization() {
	bool success = true;
	if(hasNameParameter()){
		success &= serializeAttribute("Name", nameParameter->getValue());
	}
	return success;
}

bool Component::onDeserialization() {
	bool success = true;
	if(hasNameParameter()) {
		std::string name;
		success &= deserializeAttribute("Name", name);
		setName(name);
	}
	return success;
}

void Component::onConstruction() {
	NamedObject::onConstruction();
}

void Component::onCopyFrom(std::shared_ptr<PrototypeBase> source) {
	auto original = std::static_pointer_cast<Component>(source);
	setName(original->getName() + " copy");
}

}
