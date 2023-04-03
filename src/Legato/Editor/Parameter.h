#pragma once

#include "Component.h"

class BaseParameter : public Legato::Component{
	
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
