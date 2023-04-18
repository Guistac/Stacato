#pragma once

#include "PrototypeBase.h"
#include "Serializable.h"
#include "NamedObject.h"

/*
 
DESCRIPTION
	and object that has a name, can be copied, saved & loaded
 
IMPLEMENTATION EXAMPLE
 
 class ComponentImplementation : public Component{
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(ComponentImplementation)
 
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
		 Component::onConstruction();
	 }
	 
	 virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		 Component::onCopyFrom(source);
	 }
	 
 };
 
*/


//Components:
//-Nodes
//-Parameters
//-Documents
//-Layouts



class ProjectComponent;

namespace Legato{

	class Component : public PrototypeBase, public Serializable, public virtual NamedObject{
		
		DECLARE_PROTOTYPE_INTERFACE_METHODS(Component)
		
	public:
		
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
		
		
		//—————————— SERIALIZABLE
		
		virtual bool onSerialization() override;
		virtual bool onDeserialization() override;
		
		//—————————— PROTOTYPE
		
		virtual void onConstruction() override;
		virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override;
		
	protected:
		
		std::vector<std::shared_ptr<Component>> children;
		std::shared_ptr<Component> parent = nullptr;
		std::shared_ptr<ProjectComponent> project = nullptr;
	};



	//document should be a subclass of Component


}
