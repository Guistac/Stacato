#pragma once

#include "Component.h"

namespace Legato{

template<typename T, typename = std::enable_if_t<std::is_base_of_v<Serializable, T>>>
class ListComponent : public Component, public std::vector<std::shared_ptr<T>>{
	
public:
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(ListComponent<T>)
	
	void setEntrySaveString(std::string entrySaveString_){
		entrySaveString = entrySaveString_;
	}
	
	void setEntryConstructor(std::function<std::shared_ptr<T>(void)> entryConstructor_){
		constructor = entryConstructor_;
	}
	
private:
	
	std::string entrySaveString;
	std::function<std::shared_ptr<T>(void)> constructor;
	
	virtual bool onSerialization() override {
		if(entrySaveString.empty()){
			Logger::error("Could not save list <{}> : no entry save string provided", saveString);
			return false;
		}
		if(xmlElement == nullptr){
			Logger::error("Could not save list, element <{}> was not serialized first", saveString);
			return false;
		}
		
		bool b_success = true;
		for(auto& entry : *this){
			entry->xmlElement = xmlElement->InsertNewChildElement(entrySaveString.c_str());
			b_success &= entry->onSerialization();
		}
		return b_success;
	}
	
	virtual bool onDeserialization() override {
		if(entrySaveString.empty()){
			Logger::error("Could not load list <{}> : no entry save string provided", saveString);
			return false;
		}
		if(xmlElement == nullptr){
			Logger::error("Could not load list, element <{}> was not deserialized first", saveString);
			return false;
		}
		
		std::vector<std::shared_ptr<T>>::clear();
		bool b_allEntriesLoaded = true;
		
		tinyxml2::XMLElement* entryXML = xmlElement->FirstChildElement(entrySaveString.c_str());
		while(entryXML != nullptr){
			
			std::shared_ptr<T> loadedEntry = constructor();
			loadedEntry->xmlElement = entryXML;
			
			if(loadedEntry->onDeserialization()) std::vector<std::shared_ptr<T>>::push_back(loadedEntry);
			else b_allEntriesLoaded = false;
			
			entryXML = entryXML->NextSiblingElement(entrySaveString.c_str());
		}
		return b_allEntriesLoaded;
	}
	
	virtual void onConstruction() override {
		Component::onConstruction();
	}
	
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Component::onCopyFrom(source);
		
		auto listComponent = std::static_pointer_cast<ListComponent<T>>(source);
		for(auto originalEntry : *listComponent.get()){ //get the raw pointer and dereference it to get the vector
			auto copy = originalEntry->duplicate();
			std::vector<std::shared_ptr<T>>::push_back(copy); //we need to specify the push_back method of the specific vector subclass
		}
	}
	
};

}
