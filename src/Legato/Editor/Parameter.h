#pragma once

#include "Component.h"

class BaseParameter : public Component{
	
	DECLARE_PROTOTYPE_INTERFACE_METHODS(BaseParameter)

public:
	
	virtual void gui() = 0;
	
	void addEditCallback(std::function<void()> callback){ editCallbacks.push_back(callback); }
	
protected:
	
	virtual void onConstruction() override {
		Component::onConstruction();
	}
	
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Component::onCopyFrom(source);
	}
	
	virtual bool onSerialization() override {
		return Component::onSerialization();
	}
	
	virtual bool onDeserialization() override {
		return Component::onDeserialization();
	}
	
	virtual void onEdit(){
		for(auto& editCallback : editCallbacks) {
			editCallback();
		}
	}
	
	BaseParameter(bool withoutNameParameter) : Component(withoutNameParameter){}
	
private:
	
	std::vector<std::function<void()>> editCallbacks;
};








#include "ProjectComponent.h"

namespace Legato{

	class Parameter : public Component{
		COMPONENT_INTERFACE(Parameter)
	
	public:
		
		virtual void gui() = 0;
		
		void addEditCallback(std::function<void()> callback) {
			editCallbacks.push_back(callback);
		}
		
	protected:
		
		virtual void onConstruction() override { Component::onConstruction(); }
		virtual void copyFrom(Ptr<Component> source) override { Component::copyFrom(source); }
		virtual bool onSerialization() override { Component::onSerialization(); }
		virtual bool onDeserialization() override { Component::onDeserialization(); }
		virtual void onPostLoad() override { Component::onPostLoad(); }
		
		virtual void onEdit(){
			for(auto& editCallback : editCallbacks) {
				editCallback();
			}
		}
		
	private:
		
		std::vector<std::function<void()>> editCallbacks;
		
	};

}
