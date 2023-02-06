#pragma once

#include "Serializable.h"
#include "PrototypeBase.h"

class NewStringParameter;

class BaseParameter : public PrototypeBase, public Serializable{

	DECLARE_PROTOTYPE_INTERFACE_METHODS(BaseParameter)
	
protected:
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override;
	
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	
public:
	
	void setName(std::string name);
	std::string getName();
	
	virtual void gui() = 0;
	
	void addEditCallback(std::function<void()> callback){
		editCallbacks.push_back(callback);
	}
	
	
protected:
	
	virtual void onEdit(){
		for(auto& editCallback : editCallbacks) editCallback();
	}
	
	BaseParameter(bool createNameParameter){
		b_createNameParameter = false;
	}
	
private:
	std::vector<std::function<void()>> editCallbacks;
	bool b_createNameParameter = true;
	std::shared_ptr<NewStringParameter> nameParameter;
};



class NewStringParameter : public BaseParameter{
public:
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(NewStringParameter)
	
private:
	
	virtual void onConstruction() override {
		BaseParameter::onConstruction();
		value = "Default Value";
	}
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		BaseParameter::onCopyFrom(source);
		auto original = std::static_pointer_cast<NewStringParameter>(source);
		value = original->value;
	}
	
public:
	
	const std::string get(){ return value; }
	void set(std::string newValue){ value = newValue; }
	

	virtual void gui() override {}
	
	virtual bool onDeserialization() override {
		bool success = true;
		success &= BaseParameter::onDeserialization();
		success &= deserializeAttribute("Value", value);
		return success;
	}
	virtual bool onSerialization() override {
		bool success = true;
		success &= BaseParameter::onSerialization();
		success &= serializeAttribute("Value", value.c_str());
		return success;
	}
	
private:
	std::string value;
	
	NewStringParameter(bool createNameParameter) : BaseParameter(createNameParameter){}
	
private:
	friend class BaseParameter;
	static std::shared_ptr<NewStringParameter> createInstanceWithoutName(){
		return std::shared_ptr<NewStringParameter>(new NewStringParameter(false));
	}
};
