#pragma once

#include "LegatoComponent.h"
#include <tinyxml2.h>

namespace Legato{

	template<typename T>
	class ListComponent : public Component{
		COMPONENT_IMPLEMENTATION_NOADDCHILD(ListComponent)
		
	public:
		//this was suggested by chatGPT to make sure we only create lists templated with Component elements
		static_assert(std::is_base_of<Component, T>::value, "[Legato::ListComponent] Templated type must be derived from Legato::Component since the elements in the list must be saved and loaded");
		
		void setEntryIdentifier(std::string entryID){
			if(sanitizeXmlIdentifier(entryID, entryIdentifier)){
				Logger::error("[List:{}] invalid list entry identifier '{}' was sanitized to '{}'", T::getStaticClassName(), entryID, entryIdentifier);
				return;
			}
			entryIdentifier = entryID;
			for(auto child : childComponents) child->setIdentifier(entryIdentifier);
		}
		
		void setEntryConstructor(std::function<std::shared_ptr<T>(void)> constructor){
			if(constructor == nullptr){
				Logger::error("[List:{}] invalid entry contrusctor was null", T::getStaticClassName());
				return;
			}
			entryConstructor = constructor;
		}
		
		int size(){ childComponents.size(); }
		
		bool hasEntry(Ptr<T> queriedEntry){
			for(auto child : childComponents){
				if(child == queriedEntry) return true;
			}
			return false;
		}
		
		Ptr<T> getEntry(int index){
			if(index < 0) return nullptr;
			else if(index > childComponents.size() - 1) return nullptr;
			Ptr<Component> componentEntry = childComponents[index];
			return componentEntry->cast<T>();
		}
		
		void addEntry(Ptr<T> newEntry, int index = -1){
			if(newEntry == nullptr) return;
			if(hasChild(newEntry)) {
				Logger::warn("[{}:{}] cannot add duplicate component [{}:{}]", getClassName(), getName(), newEntry->getClassName(), newEntry->getName());
				return;
			}
			if(index == -1) {
				Component::addChild(newEntry);
				newEntry->setIdentifier(entryIdentifier);
			}
			else{
				if(index > childComponents.size()) index = int(childComponents.size());
				else if(index < 0) index = 0;
				childComponents.insert(childComponents.begin() + index, newEntry);
				addChildDependencies(newEntry);
				newEntry->setIdentifier(entryIdentifier);
			}
		}
		
		void removeEntry(Ptr<T> removedEntry){
			for(int i = 0; i < childComponents.size(); i++){
				if(i == removedEntry){
					childComponents.erase(childComponents.begin() + i);
					return;
				}
			}
		}
		
		int getEntryIndex(Ptr<T> entry){
			for(int i = 0; i < childComponents.size(); i++){
				if(childComponents[i] == entry) return i;
			}
			return -1;
		}
		
		void moveEntry(Ptr<T> movedEntry, int newIndex){
			int oldIndex = getEntryIndex(movedEntry);
			if(oldIndex == -1 || newIndex < 0 || newIndex >= childComponents.size() || oldIndex == newIndex) return;
			moveEntry(oldIndex, newIndex);
		}
		
		void moveEntry(int oldIndex, int newIndex){
			if(oldIndex < 0 || oldIndex >= childComponents.size() || newIndex < 0 || newIndex >= childComponents.size() || oldIndex == newIndex) return;
			Ptr<T> temp = childComponents[oldIndex];
			childComponents.erase(childComponents.begin() + oldIndex);
			childComponents.insert(childComponents.begin() + newIndex, temp);
		}
		
		//also handle selection ?
		
	private:
		
		std::string entryIdentifier = "";
		std::function<std::shared_ptr<T>(void)> entryConstructor;
		
		void onConstruction() override {
			Component::onConstruction();
			//Add the default entry constructor so the user doesn't have to add it
			entryConstructor = []() -> Ptr<T> { return T::make(); };
			//Also add the default entry constructor
			setEntryIdentifier(T::getStaticClassName());
		};
		
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
				entry->setIdentifier(entryIdentifier);
			}
			return true;
		}
		
		virtual bool deserialize() override {

			if(identifier.empty()){
				 Logger::error("[List] Could not load element <{}> : no entry identifier provided", identifier);
				 return false;
			}
			if(entryIdentifier.empty()){
				Logger::error("[List] Could not load list <{}> : no entry identifier provided", identifier);
				return false;
			}
			xmlElement = parentComponent->xmlElement->FirstChildElement(identifier.c_str());
				if(xmlElement == nullptr){
				Logger::error("[Load Failure] {} : could not find XML element", getIdentifierPath());
				return false;
			}
			
			childComponents.clear();

			tinyxml2::XMLElement* entryXML = xmlElement->FirstChildElement(entryIdentifier.c_str());
			while(entryXML != nullptr){
				std::shared_ptr<T> loadedEntry = entryConstructor();
				loadedEntry->xmlElement = entryXML;
				Ptr<Component> loadedComponent = std::static_pointer_cast<Component>(loadedEntry);
				loadedComponent->setIdentifier(entryIdentifier);
				addChild(loadedEntry);
				entryXML = entryXML->NextSiblingElement(entryIdentifier.c_str());
			}
			
			for(auto child : childComponents){
				child->onDeserialization();
				for(auto childChild : child->childComponents) childChild->deserialize();
				child->onPostLoad();
			}
			
			onPostLoad();
			
			return true;
			
		}
		
		friend std::shared_ptr<ListComponent> makeList(std::string ID, std::string entryID);
		
	};

	template<typename T>
	using List = Ptr<ListComponent<T>>;

	template<typename T>
	inline List<T> makeList(std::string ID, std::string entryID = ""){
		auto newInstance = ListComponent<T>::make();
		newInstance->setIdentifier(ID);
		//entry ID is not mandatory, since the default entry identifier is T:getStaticClassName()
		if(!entryID.empty()) newInstance->setEntryIdentifier(entryID);
		return newInstance;
	}

}
