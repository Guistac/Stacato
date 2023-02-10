#pragma once

#include "PrototypeBase.h"
#include "Serializable.h"

/*
 
DESCRIPTION
	and object that has a name, can be copied, saved & loaded
 
IMPLEMENTATION EXAMPLE
 
 class ComponentImplementation : public Component{
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS()(ComponentImplementation)
 
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
class ProjectComponent;

class Component : public PrototypeBase, public Serializable{
	
	DECLARE_PROTOTYPE_INTERFACE_METHODS(Component)
	
public:
	
	void setName(std::string name);
	const std::string& getName();
	
protected:
	
	//————————— COMPONENT
	virtual void addChild(std::shared_ptr<Component> child) {
		child->parent = std::static_pointer_cast<Component>(shared_from_this());
		children.push_back(child);
	}
	const std::vector<std::shared_ptr<Component>>& getChildren(){ return children; }
	
	virtual void setParent(std::shared_ptr<Component> parent_){
		parent = parent_;
		for(auto child : children) child->setParent(parent_);
	}
	std::shared_ptr<Component> getParent(){ return parent; }
	
	virtual void setProject(std::shared_ptr<ProjectComponent> project_){
		project = project_;
		for(auto child : children) child->project = project_;
	}
	std::shared_ptr<ProjectComponent> getProject(){ return project; }
	//—————————— COMPONENT
	
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override;
	
	bool b_hasNameParameter = true;
	Component(bool withoutNameParameter){ b_hasNameParameter = false; }

	std::shared_ptr<NewStringParameter> nameParameter;
	
private:
	std::string nonParametricName;
	
	std::vector<std::shared_ptr<Component>> children;
	std::shared_ptr<Component> parent = nullptr;
	std::shared_ptr<ProjectComponent> project = nullptr;
};



//document should be a subclass of Component



class ComponentLeaf : public Component{
public:
	
	virtual void addChild(std::shared_ptr<Component> child) override {
		Logger::error("Can't add children to leaf component {}", getName());
	}
	
};
