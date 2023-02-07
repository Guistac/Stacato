#pragma once

#include "PrototypeBase.h"
#include "Serializable.h"
#include "Parameter.h"

class Component : public PrototypeBase, public Serializable{
public:
	
	DECLARE_PROTOTYPE_INTERFACE_METHODS(Component)
	
	virtual bool onSerialization() override {
		bool success = true;
		success &= serializeAttribute("UniqueID", uniqueID);
		success &= nameParameter->serializeIntoParent(this);
		return success;
	}
	
	virtual bool onDeserialization() override {
		bool success = true;
		success &= deserializeAttribute("UniqueID", uniqueID);
		success &= nameParameter->deserializeFromParent(this);
		return success;
	}
	
	virtual void onConstruction() override {
		nameParameter = NewStringParameter::createInstanceWithoutName();
		nameParameter->setValue("Default Component Name");
		nameParameter->setName("Component Name");
		nameParameter->setSaveString("Name");
	}
	
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		auto original = std::static_pointer_cast<Component>(source);
		nameParameter->setValue(original->nameParameter->getValue());
		//don't duplicate the unique id
	}

private:
	std::shared_ptr<NewStringParameter> nameParameter;
	unsigned long long uniqueID = -1;
};
