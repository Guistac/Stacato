#pragma once

#include "PrototypeBase.h"
#include "Serializable.h"

/*
 
DESCRIPTION
	and object that has a name, can be copied, saved & loaded
 
IMPLEMENTATION EXAMPLE
 
 class ComponentImplementation : public Component{
	DECLARE_PROTOTYPE_INTERFACE_METHODS(ComponentImplementation)
 
protected:
 
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
 
 };
 
*/

class NewStringParameter;

class Component : public PrototypeBase, public Serializable{
	
	DECLARE_PROTOTYPE_DUPLICATE_METHOD(Component)
	
public:
	
	void setName(std::string name);
	const std::string& getName();
	
protected:
	
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override;
	
	Component(){ b_hasNameParameter = true; }
	Component(bool withoutNameParameter){ b_hasNameParameter = false; }

private:
	std::shared_ptr<NewStringParameter> nameParameter;
	std::string nonParametricName;
	bool b_hasNameParameter;
};
