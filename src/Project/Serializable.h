#pragma once

#include <tinyxml2.h>

class Serializable{
public:
	
	Serializable(std::string saveString_) : saveString(saveString_) {}
	
	//———— VIRTUAL METHODS
	
	virtual bool serializeImpl(){
		Logger::warn("Serializable <{}> has no specific serialisation method", saveString);
		return false;
	}
	virtual bool deserializeImpl(){
		Logger::warn("Serializable <{}> has no specific deserialisation method", saveString);
		return false;
	}
	
	//———— SERIALISATION
	
	//if a serializable needs to be serialized into another serializable
	bool serialize(Serializable& parent){
		if(parent.xmlElement){
			xmlElement = parent.xmlElement->InsertNewChildElement(saveString.c_str());
			onSerialisation();
			return true;
		}else{
			Logger::warn("Failed to save element <{}>, parent element <{}> was not saved first", saveString, parent.saveString);
			return false;
		}
	}
	bool serialize(Serializable* parent){
		return serialize(*parent);
	}
	
	
	
	//———— DESERIALIZATION
	
	//if a serializable needs to be deserialized from an already existing serializable
	bool deserializeFromSource(Serializable& source){
		if(source.xmlElement == nullptr){
			Logger::warn("Failed to load element <{}>, source was not serialized first", saveString);
			return false;
		}
		xmlElement = source.xmlElement;
		return onDeserialisation();
	}
	bool deserializeFromSource(Serializable* source){
		return deserializeFromSource(*source);
	}
	
	
	//in a serializable wants to find its content inside another serializable
	bool deserializeFromParent(Serializable& parent){
		if(parent.xmlElement == nullptr){
			Logger::warn("Failed to load element <{}>, parent element <{}> was not deserialized first", saveString, parent.saveString);
			return false;
		}
		xmlElement = parent.xmlElement->FirstChildElement(saveString.c_str());
		if(xmlElement == nullptr){
			Logger::error("Failed to load element <{}> of parent element <{}> : No Attribute Found", saveString, parent.saveString);
			return false;
		}
		return onDeserialisation();
	}
	bool deserializeFromParent(Serializable* parent){
		return deserializeFromParent(*parent);
	}
	
	
	
	
	//————— ENTRY SERIALIZATION
	
	bool serializeAttribute(std::string attributeSaveString, int data){
		if(logSerializationError(attributeSaveString)) return false;
		xmlElement->SetAttribute(attributeSaveString.c_str(), data);
		return true;
	}
	bool serializeAttribute(std::string attributeSaveString, long long data){
		if(logSerializationError(attributeSaveString)) return false;
		xmlElement->SetAttribute(attributeSaveString.c_str(), data);
		return true;
	}
	bool serializeAttribute(std::string attributeSaveString, unsigned long long data){
		if(logSerializationError(attributeSaveString)) return false;
		xmlElement->SetAttribute(attributeSaveString.c_str(), data);
		return true;
	}
	bool serializeAttribute(std::string attributeSaveString, float data){
		if(logSerializationError(attributeSaveString)) return false;
		xmlElement->SetAttribute(attributeSaveString.c_str(), data);
		return true;
	}
	bool serializeAttribute(std::string attributeSaveString, double data){
		if(logSerializationError(attributeSaveString)) return false;
		xmlElement->SetAttribute(attributeSaveString.c_str(), data);
		return true;
	}
	bool serializeAttribute(std::string attributeSaveString, const std::string& data){
		if(logSerializationError(attributeSaveString)) return false;
		xmlElement->SetAttribute(attributeSaveString.c_str(), data.c_str());
		return true;
	}
	
	bool serializeEntryList(std::vector<Serializable>& entries, std::string listName){
		if(logSerializationError(listName)) return false;
		tinyxml2::XMLElement* listXML = xmlElement->InsertNewChildElement(listName.c_str());
		for(auto& entry : entries) entry.xmlElement = listXML->InsertNewChildElement(entry.saveString.c_str());
		bool b_allSuccessfull = true;
		for(auto& entry : entries) {
			if(!entry.onSerialisation()) {
				b_allSuccessfull = false;
			}
		}
		return b_allSuccessfull;
	}
	
	
	
	
	//————— ENTRY DESERIALIZATION
	
	bool deserializeAttribute(std::string idString, int& data){
		if(logDeserializationError(idString)) return false;
		tinyxml2::XMLError result = xmlElement->QueryIntAttribute(idString.c_str(), &data);
		return !logAttributeDeserializationError(result, idString);
	}
	bool deserializeAttribute(std::string idString, long long& data){
		if(logDeserializationError(idString)) return false;
		tinyxml2::XMLError result = xmlElement->QueryInt64Attribute(idString.c_str(), &data);
		return !logAttributeDeserializationError(result, idString);
	}
	bool deserializeAttribute(std::string idString, unsigned long long& data){
		if(logDeserializationError(idString)) return false;
		tinyxml2::XMLError result = xmlElement->QueryUnsigned64Attribute(idString.c_str(), &data);
		return !logAttributeDeserializationError(result, idString);
	}
	bool deserializeAttribute(std::string idString, float& data){
		if(logDeserializationError(idString)) return false;
		tinyxml2::XMLError result = xmlElement->QueryFloatAttribute(idString.c_str(), &data);
		return !logAttributeDeserializationError(result, idString);
	}
	bool deserializeAttribute(std::string idString, double& data){
		if(logDeserializationError(idString)) return false;
		tinyxml2::XMLError result = xmlElement->QueryDoubleAttribute(idString.c_str(), &data);
		return !logAttributeDeserializationError(result, idString);
	}
	bool deserializeAttribute(std::string idString, std::string& data){
		if(logDeserializationError(idString)) return false;
		const char* buffer;
		tinyxml2::XMLError result = xmlElement->QueryStringAttribute(idString.c_str(), &buffer);
		if(logAttributeDeserializationError(result, idString)) return false;
		idString = buffer;
		return true;
	}
	
	bool deserializeEntryList(std::vector<Serializable>& entries, std::string listName, std::string entryName){
		if(logDeserializationError(listName)) return false;
		tinyxml2::XMLElement* listXML = xmlElement->FirstChildElement(listName.c_str());
		if(listXML == nullptr) {
			Logger::warn("Failed to load list attribute '{}' of element <{}>", listName, saveString);
			return false;
		}
		tinyxml2::XMLElement* childXML = listXML->FirstChildElement(entryName.c_str());
		while(childXML != nullptr){
			entries.push_back(Serializable(entryName));
			entries.back().xmlElement = childXML;
			childXML = childXML->NextSiblingElement(entryName.c_str());
		}
		return true;
	}
	
	
	
private:
	tinyxml2::XMLElement* xmlElement = nullptr;
	std::string saveString;
	
	bool onSerialisation() {
		return serializeImpl();
	}
	
	bool onDeserialisation() {
		return deserializeImpl();
	}
	
	bool logAttributeDeserializationError(tinyxml2::XMLError result, std::string& idString){
		switch(result){
			case tinyxml2::XML_SUCCESS:
				Logger::trace("Deserialized attribute '{}' of element <{}>", idString, saveString);
				return false;
			case tinyxml2::XML_NO_ATTRIBUTE:
				Logger::error("Failed to load attribute '{}' of element <{}> : No Attribute found", idString, saveString);
				return true;
			case tinyxml2::XML_WRONG_ATTRIBUTE_TYPE:
				Logger::error("Failed to load attribute '{}' of element <{}> : Wrong Attribute Type", idString, saveString);
				return true;
			default:
				Logger::error("Failed to load attribute '{}' of element <{}> : tinyxml::XMLError {}", idString, saveString, result);
				return true;
		}
	}
	
	bool logSerializationError(std::string& idString){
		if(xmlElement == nullptr){
			Logger::error("Could not save attribute '{}', element <{}> was not serialized first", idString, saveString);
			return true;
		}
		return false;
	}
	
	bool logDeserializationError(std::string& idString){
		if(xmlElement == nullptr){
			Logger::error("Could not save attribute '{}', element <{}> was not deserialized first", idString, saveString);
			return true;
		}
		return false;
	}
};


