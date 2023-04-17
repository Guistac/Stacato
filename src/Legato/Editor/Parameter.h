#pragma once

#include "Component.h"

#include <imgui.h>

/*
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
*/



namespace Legato{

	class Parameter : public Component{

	public:
		std::shared_ptr<Parameter> duplicate(){
			return std::static_pointer_cast<Parameter>(duplicatePrototype());
		}
	protected:
		Parameter() : Component(true){}
		
	public:
		
		virtual void onConstruction() override;
		virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override { Component::onCopyFrom(source); }
		virtual bool onSerialization() override;
		virtual bool onDeserialization() override;
		
		void setDisabled(bool disabled){ b_disabled = disabled; }
		bool isDisabled(){ return b_disabled; }
		
		void setValid(bool valid){ b_valid = valid; }
		bool isValid(){ return b_valid; }
		
		void addEditCallback(std::function<void()> callback){ editCallbacks.push_back(callback); }
		void onEdit(){ for(auto& editCallback : editCallbacks) editCallback(); }
		
		virtual void gui(){
			bool drawInvalid = !b_valid;
			if(drawInvalid) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 255));
			onGui();
			if(drawInvalid) ImGui::PopStyleColor();
		}
		virtual void onGui() = 0;
		
		void lockMutex(){ mutex.lock(); }
		void unlockMutex(){ mutex.unlock(); }
		
		const char* getImGuiID(){ return imGuiID.c_str(); }
		
		virtual void setName(std::string name) override;
		
	private:
		std::vector<std::function<void()>> editCallbacks;
		std::string imGuiID;
		std::string description;
		bool b_disabled = false;
		bool b_valid = true;
		std::mutex mutex;
	};




	class ParameterGroup : public Component{
		DECLARE_PROTOTYPE_IMPLENTATION_METHODS(ParameterGroup)
	public:
		
		static std::shared_ptr<ParameterGroup> createInstance(std::string saveString, std::vector<std::shared_ptr<Parameter>> parameters){
			auto newGroup = ParameterGroup::createInstance();
			newGroup->parameters = parameters;
			newGroup->setSaveString(saveString);
			return newGroup;
		}
		
		virtual void onConstruction() override {
			Component::onConstruction();
		};
		virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
			Component::onCopyFrom(source);
		}
		virtual bool onSerialization() override {
			bool success = true;
			success &= Component::onSerialization();
			for(auto parameter : parameters){
				success &= parameter->serializeIntoParent(this);
			}
			return success;
		}
		virtual bool onDeserialization() override {
			bool success = true;
			success &= Component::onDeserialization();
			for(auto parameter : parameters){
				success &= parameter->deserializeFromParent(this);
			}
			return success;
		}
		
		const std::vector<std::shared_ptr<Parameter>>& getParameters(){ return parameters; }
		
		void gui();
		
	private:
		std::vector<std::shared_ptr<Parameter>> parameters;
	};

	using ParamGroup = std::shared_ptr<ParameterGroup>;


}
