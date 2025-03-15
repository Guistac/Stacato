#pragma once

#include "ProjectComponent.h"
#include <tinyxml2.h>

namespace Legato{

	template<typename T>
	class ListComponent : public Component{
		
		COMPONENT_IMPLEMENTATION(ListComponent)
		
	public:
		static std::shared_ptr<ListComponent> make(std::string ID, std::string entryID){
			auto newInstance = make();
			newInstance->setIdentifier(ID);
			newInstance->setEntryIdentifier(entryID);
			return newInstance;
		}
		
		void addChild(Ptr<Component> child) override{
			Logger::critical("<{}> Cannot add children to listComponent, use addEntry()", identifier);
		}
		
		//this was suggested by chatGPT to make sure we only create lists templated with serializable elements
		static_assert(std::is_base_of<Component, T>::value, "Templated type must be derived from Component since the elements in the list must be saved and loaded");
		
		void setEntryIdentifier(std::string entryID){
			if(sanitizeXmlIdentifier(entryID, entryIdentifier)){
				Logger::error("[{}] invalid list entry identifier '{}' was sanitized to '{}'", getClassName(), entryID, entryIdentifier);
			}
		}
		void setEntryConstructor(std::function<std::shared_ptr<T>(void)> constructor){ entryConstructor = constructor; }
		
		int size(){ return 0; }
		bool hasEntry(Ptr<T> queriedEntry){ return false; }
		void addEntry(Ptr<T> newEntry){}
		void removeEntry(Ptr<T> removedEntry){}
		void moveEntry(Ptr<T> movedEntry, int newIndex){}
		Ptr<T> getEntry(int index){
			if(index < 0) return nullptr;
			else if(index > childComponents.size() - 1) return nullptr;
			Ptr<Component> componentEntry = childComponents[index];
			return componentEntry->cast<T>();
		}
		
		//also handle selection ?
		
	private:
		
		std::string entryIdentifier = "";
		std::function<std::shared_ptr<T>(void)> entryConstructor;
		
		
		
		bool onSerialization() override {
			if(entryIdentifier.empty()){
				Logger::error("Could not save list <{}> : no entry identifier provided", identifier);
				return false;
			}
			if(xmlElement == nullptr){
				Logger::error("Could not save list, element <{}> was not serialized first", identifier);
				return false;
			}
			for(auto entry : childComponents){
				entry->xmlElement = xmlElement->InsertNewChildElement(entryIdentifier.c_str());
			}
			return true;
		}
		
		
		
		bool onDeserialization() override {
			if(entryIdentifier.empty()){
				Logger::error("Could not load list <{}> : no entry identifier provided", identifier);
				return false;
			}
			if(xmlElement == nullptr){
				Logger::error("Could not load list, element <{}> was not deserialized first", identifier);
				return false;
			}
			
			childComponents.clear();
			bool b_allEntriesLoaded = true;
			
			tinyxml2::XMLElement* entryXML = xmlElement->FirstChildElement(entryIdentifier.c_str());
			while(entryXML != nullptr){
				
				std::shared_ptr<T> loadedEntry = entryConstructor();
				loadedEntry->xmlElement = entryXML;
				childComponents.push_back(loadedEntry);
				
				entryXML = entryXML->NextSiblingElement(entryIdentifier.c_str());
			}
			
			return true;
		}
		
	};

}
