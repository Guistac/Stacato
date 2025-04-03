#pragma once

#include "LegatoComponent.h"
#include "LegatoAction.h"
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
		
		Ptr<T>& operator[](size_t index) {
			if(index < 0) return nullptr;
			else if(index > childComponents.size() - 1) return nullptr;
			Ptr<Component> componentEntry = childComponents[index];
			return componentEntry->cast<T>();
		}
		
		
		
		size_t size(){ return childComponents.size(); }
		
		bool hasEntry(Ptr<T> queriedEntry){
			for(auto child : childComponents){
				if(child == queriedEntry) return true;
			}
			return false;
		}
		
		int getEntryIndex(Ptr<T> entry){
			for(int i = 0; i < childComponents.size(); i++){
				if(childComponents[i] == entry) return i;
			}
			return -1;
		}
		
		std::vector<Ptr<T>> getList(){
			size_t count = size();
			std::vector<Ptr<T>> output(size());
			for(int i = 0; i < size(); i++) {
				output[i] = childComponents[i]->template cast<T>();
			}
			return output;
		}
		
		void addEntry(Ptr<T> newEntry, int index = -1){
			if(newEntry == nullptr) return;
			if(hasChild(newEntry)) {
				Logger::warn("[{}:{}] cannot add duplicate component [{}:{}]", getClassName(), getIdentifier(), newEntry->getClassName(), newEntry->getIdentifier());
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
				if(childComponents[i] == removedEntry){
					childComponents.erase(childComponents.begin() + i);
					return;
				}
			}
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
		
		void addEntry_undoable(Ptr<T> newEntry, int index = -1){
			if(newEntry == nullptr) return;
			if(parentProject){
				auto command = std::make_shared<AddEntryAction>(cast<ListComponent>(), newEntry, index);
				parentProject->execute(command);
			}
			else addEntry(newEntry, index);
		}

		void removeEntry_undoable(Ptr<T> removedEntry){
			if(removedEntry == nullptr) return;
			if(!hasEntry(removedEntry)) return;
			if(parentProject){
				auto command = std::make_shared<RemoveEntryAction>(cast<ListComponent>(), removedEntry);
				parentProject->execute(command);
			}
			else removeEntry(removedEntry);
		}

		void moveEntry_undoable(int oldIndex, int newIndex){
			if(oldIndex < 0 || oldIndex >= childComponents.size() || newIndex < 0 || newIndex >= childComponents.size() || oldIndex == newIndex) return;
			if(parentProject){
				auto command = std::make_shared<MoveEntryAction>(cast<ListComponent>(), oldIndex, newIndex);
				parentProject->execute(command);
			}
			else moveEntry(oldIndex, newIndex);
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
			
			for(auto entry : childComponents) entry->setIdentifier(entryIdentifier);
			
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
			
			bool b_success = true;
			for(auto child : childComponents){
				b_success &= child->onDeserialization();
				for(auto childChild : child->childComponents) {
					b_success &= childChild->deserialize();
				}
				b_success &= child->onPostLoad();
			}
			
			b_success &= onPostLoad();
			
			return b_success;
			
		}
		
		class AddEntryAction : public Action{
			AddEntryAction(Ptr<ListComponent> list_, Ptr<T> entry_, int index_) : list(list_), entry(entry_), index(index_){
				name = "Add entry to " + list->getIdentifier();
			}
			virtual void onExecute() override {
				list->addEntry(entry, index);
			}
			virtual void onUndo() override {
				list->removeEntry(entry);
			}
		private:
			Ptr<ListComponent> list;
			Ptr<T> entry;
			int index;
		};
		
		class RemoveEntryAction : public Action{
			RemoveEntryAction(Ptr<ListComponent> list_, Ptr<T> entry_) : list(list_), entry(entry_){
				name = "Remove Entry from " + list->getIdentifier();
				index = list->getEntryIndex(entry);
			}
			virtual void onExecute() override {
				list->removeEntry(entry);
			}
			virtual void onUndo() override {
				list->addEntry(entry, index);
			}
		private:
			Ptr<ListComponent> list;
			Ptr<T> entry;
			int index;
		};
		
		class MoveEntryAction : public Action{
			MoveEntryAction(Ptr<ListComponent> list_, int oldIndex_, int newIndex_) : list(list_), oldIndex(oldIndex_), newIndex(newIndex_){
				name = "Move entry in " + list->getIdentifier();
			}
			virtual void onExecute() override {
				list->moveEntry(oldIndex, newIndex);
			}
			virtual void onUndo() override {
				list->moveEntry(newIndex, oldIndex);
			}
		private:
			Ptr<ListComponent> list;
			int oldIndex;
			int newIndex;
		};
		
		template<typename U>
		friend Ptr<ListComponent<U>> makeList(std::string ID, std::string entryID);
		
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
