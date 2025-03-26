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
		
		virtual void gui() = 0;
		
		void addEditCallback(std::function<void()> callback) {
			editCallbacks.push_back(callback);
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
		
		//make()
		//getValue()
		//overwrite()
		//overwriteWithHistory()
		//class EditCommand{};
		
	private:
		
		std::vector<std::function<void()>> editCallbacks;
		
	};






	class BooleanParameter : public Parameter{
		COMPONENT_IMPLEMENTATION(BooleanParameter)
	public:
		virtual void gui() override;
		virtual void onConstruction() override{
			Parameter::onConstruction();
		}
		virtual void copyFrom(Ptr<Component> source) override{
			Parameter::copyFrom(source);
			auto original = source->cast<BooleanParameter>();
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
		}
	private:
		bool displayValue = false;
		bool value = false;
	};


	class IntegerParameter : public Parameter{
		COMPONENT_IMPLEMENTATION(IntegerParameter)
	public:
		virtual void gui() override;
		virtual void onConstruction() override{
			Parameter::onConstruction();
		}
		virtual void copyFrom(Ptr<Component> source) override{
			Parameter::copyFrom(source);
			auto original = source->cast<IntegerParameter>();
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
		}
	private:
		long long displayValue = false;
		long long value = false;
	};


	class RealParameter : public Parameter{
		COMPONENT_IMPLEMENTATION(RealParameter)
	public:
		virtual void gui() override;
		virtual void onConstruction() override{
			Parameter::onConstruction();
		}
		virtual void copyFrom(Ptr<Component> source) override{
			Parameter::copyFrom(source);
			auto original = source->cast<RealParameter>();
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
		}
	private:
		double displayValue = false;
		double value = false;
	};


	class StringParameter : public Parameter{
		COMPONENT_IMPLEMENTATION(StringParameter)
	public:
		virtual void gui() override;
		virtual void onConstruction() override{
			Parameter::onConstruction();
			displayValue[0] = 0;
		}
		virtual void copyFrom(Ptr<Component> source) override{
			Parameter::copyFrom(source);
			auto original = source->cast<StringParameter>();
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
		}
	private:
		char displayValue[256];
		std::string value = "";
	};


	class OptionParameter : public Parameter{
		COMPONENT_IMPLEMENTATION(OptionParameter)
	public:
		virtual void gui() override;
	};


	class TimeParameter : public Parameter{
		COMPONENT_IMPLEMENTATION(TimeParameter)
	public:
		virtual void gui() override;
	};

}
