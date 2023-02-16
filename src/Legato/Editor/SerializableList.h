#pragma once

#include "Serializable.h"

template<typename T>
class SerializableList : public Serializable{
public:
	
	//this was suggested by chatGPT to make sure we only create lists templated with serializable elements
	static_assert(std::is_base_of<Serializable, T>::value, "Templated type must be derived from Serializable since the elements in the list must be saved and loaded");
	
	void setEntrySaveString(std::string entrySaveString_){
		entrySaveString = entrySaveString_;
	}
	
	typedef std::function<std::shared_ptr<T>(std::shared_ptr<Serializable> loadedEntry)> listEntryContructor;
	
	void setEntryConstructor(listEntryContructor entryConstructor_){
		constructor = entryConstructor_;
	}
	
	std::vector<std::shared_ptr<T>>& get(){
		return content;
	}
	
private:
	
	std::vector<std::shared_ptr<T>> content;
	std::string entrySaveString;
	listEntryContructor constructor;
	
	virtual bool onSerialization() override {
		if(saveString.empty()){
			Logger::error("Could not save list : no save string provided");
			return false;
		}
		if(entrySaveString.empty()){
			Logger::error("Could not save list <{}> : no entry save string provided", saveString);
			return false;
		}
		if(xmlElement == nullptr){
			Logger::error("Could not save list, element <{}> was not serialized first", saveString);
			return false;
		}
		
		bool b_success = true;
		for(auto entry : content){
			entry->xmlElement = xmlElement->InsertNewChildElement(entrySaveString.c_str());
			b_success &= entry->onSerialization();
		}
		return b_success;
	}
	
	virtual bool onDeserialization() override {
		
		if(saveString.empty()){
			Logger::error("Could not load list : no save string provided");
			return false;
		}
		if(entrySaveString.empty()){
			Logger::error("Could not load list <{}> : no entry save string provided", saveString);
			return false;
		}
		if(xmlElement == nullptr){
			Logger::error("Could not load list, element <{}> was not deserialized first", saveString);
			return false;
		}
		
		content.clear();
		bool b_allEntriesLoaded = true;
		
		tinyxml2::XMLElement* entryXML = xmlElement->FirstChildElement(entrySaveString.c_str());
		while(entryXML != nullptr){
			
			//create an abstract Serializable to be supplied to the entry constructor function
			//this allow the user to query the abstract object for attributes
			//and gives them flexibility to decide how each entry should be constructed
			std::shared_ptr<Serializable> abstractLoadedEntry = std::make_shared<Serializable>(entryXML);
			std::shared_ptr<T> loadedEntry = constructor(abstractLoadedEntry);

			if(loadedEntry->onDeserialization()) content.push_back(loadedEntry);
			else b_allEntriesLoaded = false;
		
			entryXML = entryXML->NextSiblingElement(entrySaveString.c_str());
		}
		return b_allEntriesLoaded;
	}
};

