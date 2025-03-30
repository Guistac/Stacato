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
#include "LegatoAction.h"
#include "LegatoProject.h"

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


	class BooleanParameter;
	class IntegerParameter;
	class RealParameter;
	class StringParameter;
	class OptionParameter;

	using Boolean = Ptr<BooleanParameter>;
	using Integer = Ptr<IntegerParameter>;
	using Real = Ptr<RealParameter>;
	using String = Ptr<StringParameter>;
	using Option = Ptr<OptionParameter>;


	class BooleanParameter : public Parameter{
		COMPONENT_IMPLEMENTATION(BooleanParameter)
	public:
		
		virtual void gui(bool b_drawText = true) override;
		
		void overwrite(bool newValue){
			value = newValue;
			displayValue = newValue;
		}
		void overwriteUndoable(bool newValue){
			if(newValue == value) return;
			if(parentProject) {
				auto action = std::make_shared<EditAction>(cast<BooleanParameter>(), newValue);
				parentProject->execute(action);
			}
			else overwrite(newValue);
		}
		
	private:
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
			overwrite(value);
		}
		
		bool displayValue = false;
		bool value = false;
		
		class EditAction : public Action{
		public:
			EditAction(Ptr<BooleanParameter> p, bool newValue_) : param(p) {
				oldValue = p->value;
				newValue = newValue_;
				name = "Edited " + param->getName() + " from " + std::to_string(oldValue) + " to " + std::to_string(newValue);
			}
			virtual void onExecute() override { param->overwrite(newValue); }
			virtual void onUndo() override { param->overwrite(oldValue); }
		private:
			Ptr<BooleanParameter> param;
			bool oldValue;
			bool newValue;
		};
		
		friend Boolean makeBoolean(bool val, std::string name_, std::string identifier_);
		
	};


	inline Boolean makeBoolean(bool val, std::string name_, std::string identifier_){
		auto instance = BooleanParameter::make();
		instance->value = val;
		instance->displayValue = val;
		instance->setName(name_);
		instance->setIdentifier(identifier_);
		return instance;
	}





	class IntegerParameter : public Parameter{
		COMPONENT_IMPLEMENTATION(IntegerParameter)
	public:
		virtual void gui(bool b_drawText = true) override;
		
		void overwrite(long long newValue){
			value = newValue;
			displayValue = newValue;
		}
		void overwriteUndoable(long long newValue){
			if(newValue == value) return;
			if(parentProject) {
				auto action = std::make_shared<EditAction>(cast<IntegerParameter>(), newValue);
				parentProject->execute(action);
			}
			else overwrite(newValue);
		}
		
	private:
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
			displayValue = value;
		}
	private:
		long long displayValue = false;
		long long value = false;
		
		class EditAction : public Action{
		public:
			EditAction(Ptr<IntegerParameter> p, long long newValue_) : param(p) {
				oldValue = p->value;
				newValue = newValue_;
				name = "Edited " + param->getName() + " from " + std::to_string(oldValue) + " to " + std::to_string(newValue);
			}
			virtual void onExecute() override { param->overwrite(newValue); }
			virtual void onUndo() override { param->overwrite(oldValue); }
		private:
			Ptr<IntegerParameter> param;
			long long oldValue;
			long long newValue;
		};
		
		friend Integer makeInteger(int val, std::string name_, std::string identifier_);
	};

	inline Integer makeInteger(int val, std::string name_, std::string identifier_){
		auto instance = IntegerParameter::make();
		instance->value = val;
		instance->displayValue = val;
		instance->setName(name_);
		instance->setIdentifier(identifier_);
		return instance;
	}



	typedef int RealFlags;
	enum RealFlags_{
		Real_NoFlag  = 0,
		Real_NoNegatives = 1 << 0, 		// 0b0001
		Real_TurnsAndDegrees = 1 << 1	// 0b0010
	};

class RealParameter : public Parameter{
	COMPONENT_IMPLEMENTATION(RealParameter)
public:
	virtual void gui(bool b_drawText = true) override;
	
	void overwrite(double newValue){
		value = newValue;
		displayValue = newValue;
	}
	void overwriteUndoable(double newValue){
		if(newValue == value) return;
		if(parentProject) {
			auto action = std::make_shared<EditAction>(cast<RealParameter>(), newValue);
			parentProject->execute(action);
		}
		else overwrite(newValue);
	}
	
	void setPrecision(int precision){
		formatString = "%." + std::to_string(std::clamp(precision, 1, 9)) + "f";
	}
		
	private:
		virtual void onConstruction() override{
			Parameter::onConstruction();
			formatString = "%.1f";
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
			displayValue = value;
		}
	private:
		double displayValue = false;
		double value = false;
		
		bool b_noNegatives = false;
		bool b_turnsAndDegrees = false;
		std::string formatString;
		
		class EditAction : public Action{
		public:
			EditAction(Ptr<RealParameter> p, double newValue_) : param(p) {
				oldValue = p->value;
				newValue = newValue_;
				name = "Edited " + param->getName() + " from " + std::to_string(oldValue) + " to " + std::to_string(newValue);
			}
			virtual void onExecute() override { param->overwrite(newValue); }
			virtual void onUndo() override { param->overwrite(oldValue); }
		private:
			Ptr<RealParameter> param;
			double oldValue;
			double newValue;
		};
	
		friend Real makeReal(int val, std::string name_, std::string identifier_, RealFlags flags);
	
	};

	inline Real makeReal(int val, std::string name_, std::string identifier_, RealFlags flags = Real_NoFlag){
		auto instance = RealParameter::make();
		instance->value = val;
		instance->displayValue = val;
		instance->setName(name_);
		instance->setIdentifier(identifier_);
		instance->b_turnsAndDegrees = flags & Real_TurnsAndDegrees;
		instance->b_noNegatives = flags & Real_NoNegatives;
		return instance;
	}



	typedef int StringFlags;
	enum StringFlags_{
		String_NoFlag  = 0,
		String_NoSpaces = 1 << 0,  			// 0b0001
		String_FilePathSafe = 1 << 1  		// 0b0010
	};


	class StringParameter : public Parameter{
		COMPONENT_IMPLEMENTATION(StringParameter)
	public:
		virtual void gui(bool b_drawName = true) override;
		
		void overwrite(std::string newValue){
			value = newValue;
			snprintf(displayValue, 256, "%s", newValue.c_str());
		}
		void overwriteUndoable(std::string newValue){
			if(newValue == value) return;
			if(parentProject) {
				auto action = std::make_shared<EditAction>(cast<StringParameter>(), newValue);
				parentProject->execute(action);
			}
			else overwrite(newValue);
		}
		
	private:
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
			strcpy(displayValue, value.c_str());
		}
	private:
		char displayValue[256];
		std::string value = "";
		
		class EditAction : public Action{
		public:
			EditAction(Ptr<StringParameter> p, std::string newValue_) : param(p) {
				oldValue = p->value;
				newValue = newValue_;
				name = "Edited " + param->getName() + " from \"" + oldValue + "\" to \"" + newValue + "\"";
			}
			virtual void onExecute() override { param->overwrite(newValue); }
			virtual void onUndo() override { param->overwrite(oldValue); }
		private:
			Ptr<StringParameter> param;
			std::string oldValue;
			std::string newValue;
		};
		
		friend String makeString(std::string val, std::string name_, std::string identifier_, StringFlags flags);
	};

	inline String makeString(std::string val, std::string name_, std::string identifier_, StringFlags flags = String_NoFlag){
		auto instance = StringParameter::make();
		instance->value = val;
		std::strcpy(instance->displayValue, val.c_str());
		instance->setName(name_);
		instance->setIdentifier(identifier_);
		return instance;
	}




	class Opt{
	public:
		Opt(int enumerator_, std::string name_, bool b_enabled_ = true){
			enumerator = enumerator_;
			name = name_;
			b_enabled = b_enabled_;
		}
		void enable(){ b_enabled = true; }
		void disable(){ b_enabled = false; }
		bool isEnabled(){ return b_enabled; }
		int getInt(){ return enumerator; }
	private:
		friend class OptionParameter;
		int enumerator = 0;
		std::string name = "";
		bool b_enabled = true;
	};


	class OptionParameter : public Parameter{
		COMPONENT_IMPLEMENTATION(OptionParameter)
	public:
		virtual void gui(bool b_drawName = true) override;
		
		void overwrite(int newValue){
			value = newValue;
			displayValue = findOption(newValue);
		}
		void overwriteUndoable(int newValue){
			if(newValue == value) return;
			if(parentProject) {
				auto action = std::make_shared<EditAction>(cast<OptionParameter>(), newValue);
				parentProject->execute(action);
			}
			else overwrite(newValue);
		}
		Opt* findOption(int input){
			for(auto& option : options){
				if(option.enumerator == input){
					return &option;
				}
			}
			return nullptr;
		}
		
	private:
		virtual void onConstruction() override{
			Parameter::onConstruction();
		}
		virtual void copyFrom(Ptr<Component> source) override{
			Parameter::copyFrom(source);
			auto original = source->cast<OptionParameter>();
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
		int value;
		Opt* displayValue = nullptr;
		std::vector<Opt> options;
		
		class EditAction : public Action{
		public:
			EditAction(Ptr<OptionParameter> p, int newValue_) : param(p) {
				oldValue = p->value;
				newValue = newValue_;
				Opt* oldOption = param->findOption(oldValue);
				Opt* newOption = param->findOption(newValue);
				name = "Edited " + param->getName() + " from "
					+ (oldOption == nullptr ? std::to_string(oldValue) : oldOption->name) + " to \""
					+ (newOption == nullptr ? std::to_string(newValue) : newOption->name) + "\"";
			}
			virtual void onExecute() override { param->overwrite(newValue); }
			virtual void onUndo() override { param->overwrite(oldValue); }
		private:
			Ptr<OptionParameter> param;
			int oldValue;
			int newValue;
		};
		
		friend Option makeOption(int value, std::string name_, std::string identifier_, std::vector<Opt> options);
	};

	inline Option makeOption(int value, std::string name_, std::string identifier_, std::vector<Opt> options){
		auto instance = OptionParameter::make();
		instance->options = options;
		instance->overwrite(value);
		instance->setName(name_);
		instance->setIdentifier(identifier_);
		return instance;
	}



	class TimeParam : public Parameter{
		COMPONENT_IMPLEMENTATION(TimeParam)
	public:
		virtual void gui(bool b_drawName = true) override;
	};




	class ParameterGroupComponent : public Component{
		COMPONENT_IMPLEMENTATION(ParameterGroupComponent)
	};

	using ParameterGroup = Ptr<ParameterGroupComponent>;

	inline ParameterGroup makeParameterGroup(std::string name, std::string identifier, std::vector<Ptr<Parameter>> parameters){
		auto instance = ParameterGroupComponent::make();
		   instance->setName(name);
		   instance->setIdentifier(identifier);
		   for(auto parameter : parameters){
			   instance->addChild(parameter);
		   }
		   return instance;
	   }


}
