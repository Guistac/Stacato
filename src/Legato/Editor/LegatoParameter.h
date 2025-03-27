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








#include "LegatoComponent.h"

namespace Legato{

	class Parameter : public Component{
		COMPONENT_INTERFACE(Parameter)
		
	public:
		
		virtual void gui(bool b_drawText = true) = 0;
		
		void addEditCallback(std::function<void()> callback) {
			editCallbacks.push_back(callback);
		}
		
		virtual void setName(std::string newName) override {
			name = newName;
			imguiID = "##" + newName;
		}
		
	protected:
		
		virtual void onConstruction() override {
			Component::onConstruction();
		}
		virtual void copyFrom(Ptr<Component> source) override {
			Component::copyFrom(source);
		}
		virtual bool onSerialization() override {
			Component::onSerialization();
		}
		virtual bool onDeserialization() override {
			Component::onDeserialization();
		}
		virtual void onPostLoad() override {
			Component::onPostLoad();
		}
		virtual void onEdit(){
			for(auto& editCallback : editCallbacks) {
				editCallback();
			}
		}
		
		std::string imguiID;
		
		//overwrite()
		//overwriteWithHistory()
		//class EditCommand{};
		
	private:
		std::vector<std::function<void()>> editCallbacks;
	};






	class BoolParam : public Parameter{
		COMPONENT_IMPLEMENTATION(BoolParam)
	public:
		static Ptr<BoolParam> make(bool val, std::string name_, std::string identifier_){
			auto instance = make();
			instance->value = val;
			instance->displayValue = val;
			instance->setName(name_);
			instance->setIdentifier(identifier_);
			return instance;
		}
		
		virtual void gui(bool b_drawText = true) override;
	private:
		virtual void onConstruction() override{
			Parameter::onConstruction();
		}
		virtual void copyFrom(Ptr<Component> source) override{
			Parameter::copyFrom(source);
			auto original = source->cast<BoolParam>();
			displayValue = original->displayValue;
			value = original->displayValue;
		}
		virtual bool onSerialization() override{
			Parameter::onSerialization();
			serializeBoolAttribute("value", value);
		}
		virtual bool onDeserialization() override{
			Parameter::onDeserialization();
			deserializeBoolAttribute("value", value);
			displayValue = value;
		}
	private:
		bool displayValue = false;
		bool value = false;
	};


	class IntParam : public Parameter{
		COMPONENT_IMPLEMENTATION(IntParam)
	public:
		static Ptr<IntParam> make(int val, std::string name_, std::string identifier_){
			auto instance = make();
			instance->value = val;
			instance->displayValue = val;
			instance->setName(name_);
			instance->setIdentifier(identifier_);
			return instance;
		}
		virtual void gui(bool b_drawText = true) override;
	private:
		virtual void onConstruction() override{
			Parameter::onConstruction();
		}
		virtual void copyFrom(Ptr<Component> source) override{
			Parameter::copyFrom(source);
			auto original = source->cast<IntParam>();
			displayValue = original->displayValue;
			value = original->displayValue;
		}
		virtual bool onSerialization() override{
			Parameter::onSerialization();
			serializeLongAttribute("value", value);
		}
		virtual bool onDeserialization() override{
			Parameter::onDeserialization();
			deserializeLongAttribute("value", value);
			displayValue = value;
		}
	private:
		long long displayValue = false;
		long long value = false;
	};


	class RealParam : public Parameter{
		COMPONENT_IMPLEMENTATION(RealParam)
	public:
		static Ptr<RealParam> make(int val, std::string name_, std::string identifier_){
			auto instance = make();
			instance->value = val;
			instance->displayValue = val;
			instance->setName(name_);
			instance->setIdentifier(identifier_);
			return instance;
		}
		virtual void gui(bool b_drawText = true) override;
	private:
		virtual void onConstruction() override{
			Parameter::onConstruction();
		}
		virtual void copyFrom(Ptr<Component> source) override{
			Parameter::copyFrom(source);
			auto original = source->cast<RealParam>();
			displayValue = original->displayValue;
			value = original->displayValue;
		}
		virtual bool onSerialization() override{
			Parameter::onSerialization();
			serializeDoubleAttribute("value", value);
		}
		virtual bool onDeserialization() override{
			Parameter::onDeserialization();
			deserializeDoubleAttribute("value", value);
			displayValue = value;
		}
	private:
		double displayValue = false;
		double value = false;
	};


	class StrParam : public Parameter{
		COMPONENT_IMPLEMENTATION(StrParam)
	public:
		static Ptr<StrParam> make(std::string val, std::string name_, std::string identifier_){
			auto instance = make();
			instance->value = val;
			std::strcpy(instance->displayValue, val.c_str());
			instance->setName(name_);
			instance->setIdentifier(identifier_);
			return instance;
		}
		virtual void gui(bool b_drawName = true) override;
		virtual void onConstruction() override{
			Parameter::onConstruction();
			displayValue[0] = 0;
		}
		virtual void copyFrom(Ptr<Component> source) override{
			Parameter::copyFrom(source);
			auto original = source->cast<StrParam>();
			strcpy(displayValue, original->displayValue);
			value = original->value;
		}
		virtual bool onSerialization() override{
			Parameter::onSerialization();
			serializeStringAttribute("value", value);
		}
		virtual bool onDeserialization() override{
			Parameter::onDeserialization();
			deserializeStringAttribute("value", value);
			strcpy(displayValue, value.c_str());
		}
	private:
		char displayValue[256];
		std::string value = "";
	};


	class Option{
	public:
		Option(int enumerator_, std::string name_, bool b_enabled_ = true){
			enumerator = enumerator_;
			name = name_;
			b_enabled = b_enabled_;
		}
		void enable(){ b_enabled = true; }
		void disable(){ b_enabled = false; }
		bool isEnabled(){ return b_enabled; }
		int getInt(){ return enumerator; }
	private:
		friend class OptParam;
		int enumerator = 0;
		std::string name = "";
		bool b_enabled = true;
	};


	class OptParam : public Parameter{
		COMPONENT_IMPLEMENTATION(OptParam)
	public:
		static Ptr<OptParam> make(int value, std::string name_, std::string identifier_, std::vector<Option> options) {
			auto instance = make();
			instance->options = options;
			instance->overwrite(value);
			instance->setName(name_);
			instance->setIdentifier(identifier_);
			return instance;
		}
		virtual void gui(bool b_drawName = true) override;
	private:
		virtual void onConstruction() override{
			Parameter::onConstruction();
		}
		virtual void copyFrom(Ptr<Component> source) override{
			Parameter::copyFrom(source);
			auto original = source->cast<OptParam>();
			options = original->options;
			overwrite(original->value);
		}
		virtual bool onSerialization() override{
			Parameter::onSerialization();
			serializeIntAttribute("value", value);
		}
		virtual bool onDeserialization() override{
			Parameter::onDeserialization();
			deserializeIntAttribute("value", value);
			overwrite(value);
		}
		void overwrite(int enumValue){
			value = enumValue;
			for(auto& option : options){
				if(option.enumerator == value){
					displayValue = &option;
					return;
				}
			}
			displayValue = nullptr;
		}
		int value;
		Option* displayValue = nullptr;
		std::vector<Option> options;
	};


	class TimeParam : public Parameter{
		COMPONENT_IMPLEMENTATION(TimeParam)
	public:
		virtual void gui(bool b_drawName = true) override;
	};

}
