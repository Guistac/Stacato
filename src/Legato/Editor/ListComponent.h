#pragma once

#include "Component.h"

namespace Legato{

template<typename T, typename = std::enable_if_t<std::is_base_of_v<Serializable, T>>>
class ListComponent : public Component, public std::vector<std::shared_ptr<T>>{
	
	//DECLARE_PROTOTYPE_IMPLENTATION_METHODS(ListComponent<T>)
	

	DECLARE_SHARED_CONTRUCTION(ListComponent<T>)
public:
	std::shared_ptr<ListComponent<T>> duplicate(){
		auto prototypeCopy = duplicatePrototype();
		auto downcastedCopy = prototypeCopy->template downcasted_shared_from_this();
		//auto downcastedCopy = prototypeCopy->downcasted_shared_from_this<ListComponent<T>>();
		return downcastedCopy;
		/*return std::static_pointer_cast<Typename>(duplicatePrototype());*/
	}
private:
	std::shared_ptr<PrototypeBase> createPrototypeInstance_private() override{
		std::shared_ptr<ListComponent> newPrototypeInstance = std::shared_ptr<ListComponent>(new ListComponent());
		newPrototypeInstance->onConstruction();
		return newPrototypeInstance;
	};
	
	
public:
	void setEntrySaveString(std::string entrySaveString_){
		entrySaveString = entrySaveString_;
	}
	
	void setEntryConstructor(std::function<std::shared_ptr<T>(Serializable&)> entryConstructor_){
		constructor = entryConstructor_;
	}
	
	std::vector<std::shared_ptr<T>>& getEntries(){ return *this; }
	
	void addEntry(std::shared_ptr<T> entry) { this->push_back(entry); }
	void removeEntry(std::shared_ptr<T> entry){
		for(int i = (int)this->size() - 1; i >= 0; i--){
			if(entry == (*this)[i]){
				this->erase(this->begin() + i);
				break;
			}
		}
	}
	
private:
	
	std::string entrySaveString;
	std::function<std::shared_ptr<T>(Serializable& abstractEntry)> constructor;
	
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
			
			Serializable abstractEntry;
			abstractEntry.setSaveString(entrySaveString);
			abstractEntry.xmlElement = entryXML;
			
			
			
			if(std::shared_ptr<T> loadedEntry = constructor(abstractEntry)){
				loadedEntry->xmlElement = entryXML;
				loadedEntry->setSaveString(entrySaveString);
				
				if(loadedEntry->onDeserialization()) std::vector<std::shared_ptr<T>>::push_back(loadedEntry);
				else b_allEntriesLoaded = false;
			}else{
				Logger::error("Failed to construct list element <{}>", entrySaveString);
			}
			
			entryXML = entryXML->NextSiblingElement(entrySaveString.c_str());
		}
		return b_allEntriesLoaded;
	}
	
	virtual void onConstruction() override {
		Component::onConstruction();
	}
	
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Component::onCopyFrom(source);
		auto listComponent = source->downcasted_shared_from_this<ListComponent<T>>();
		//auto listComponent = std::static_pointer_cast<ListComponent<T>>(source);
		for(auto originalEntry : *listComponent.get()){ //get the raw pointer and dereference it to get the vector
			auto copy = originalEntry->duplicate();
			std::vector<std::shared_ptr<T>>::push_back(copy); //we need to specify the push_back method of the specific vector subclass
		}
	}
	
};

}
