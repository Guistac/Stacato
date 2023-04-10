#pragma once

#include <tinyxml2.h>

/*
DESCRIPTION
	An abstract object that can be translated to and from an xml document.
	The object type must be known before loading.

USAGE
	- Construct the object
	- Provided a save string
	- Call its deserialization method and provide a deserialization source

IMPLEMENTATION
 - mandatory virtual protected methods:
	bool onSerialization()
	bool onDeserialization()
*/


class Serializable{
public:
	
	void setSaveString(std::string saveString_){ saveString = saveString_; }
	std::string& getSaveString(){ return saveString; }
	
	//———— SERIALISATION
	
	//if a serializable needs to be serialized into another serializable
	virtual bool serializeIntoParent(Serializable& parent){
		if(parent.xmlElement == nullptr){
			Logger::warn("Failed to save element <{}>, parent element <{}> was not saved first", saveString, parent.saveString);
			return false;
		}
		if(saveString.empty()){
			Logger::critical("Failed to save element of parent element <{}> because no save string was provided", parent.saveString);
			return false;
		}
		xmlElement = parent.xmlElement->InsertNewChildElement(saveString.c_str());
		return onSerialization();
	}
	virtual bool serializeIntoParent(Serializable* parent){
		return serializeIntoParent(*parent);
	}
	virtual bool serializeIntoParent(std::shared_ptr<Serializable> parent){
		return serializeIntoParent(*parent.get());
	}
	
	
	
	//———— DESERIALIZATION
	
	//in a serializable wants to find its content inside another serializable
	virtual bool deserializeFromParent(Serializable& parent){
		if(parent.xmlElement == nullptr){
			Logger::warn("Failed to load element <{}>, parent element <{}> was not deserialized first", saveString, parent.saveString);
			return false;
		}
		if(saveString.empty()){
			Logger::warn("Failed to load element from parent element <{}> : No save string provided", parent.saveString);
			return false;
		}
		xmlElement = parent.xmlElement->FirstChildElement(saveString.c_str());
		if(xmlElement == nullptr){
			Logger::error("Failed to load element <{}> of parent element <{}> : No Attribute Found", saveString, parent.saveString);
			return false;
		}
		return onDeserialization();
	}
	virtual bool deserializeFromParent(Serializable* parent){
		return deserializeFromParent(*parent);
	}
	virtual bool deserializeFromParent(std::shared_ptr<Serializable> parent){
		deserializeFromParent(*parent.get());
	}
	
	//————— ATTRIBUTE SERIALIZATION
	
	bool serializeAttribute(std::string attributeSaveString, int data){
		if(!logAttributeSerializationError(attributeSaveString)) return false;
		xmlElement->SetAttribute(attributeSaveString.c_str(), data);
		return true;
	}
	bool serializeAttribute(std::string attributeSaveString, long long data){
		if(!logAttributeSerializationError(attributeSaveString)) return false;
		xmlElement->SetAttribute(attributeSaveString.c_str(), data);
		return true;
	}
	bool serializeAttribute(std::string attributeSaveString, unsigned long long data){
		if(!logAttributeSerializationError(attributeSaveString)) return false;
		xmlElement->SetAttribute(attributeSaveString.c_str(), data);
		return true;
	}
	bool serializeAttribute(std::string attributeSaveString, float data){
		if(!logAttributeSerializationError(attributeSaveString)) return false;
		xmlElement->SetAttribute(attributeSaveString.c_str(), data);
		return true;
	}
	bool serializeAttribute(std::string attributeSaveString, double data){
		if(!logAttributeSerializationError(attributeSaveString)) return false;
		xmlElement->SetAttribute(attributeSaveString.c_str(), data);
		return true;
	}
	bool serializeAttribute(std::string attributeSaveString, const std::string& data){
		if(!logAttributeSerializationError(attributeSaveString)) return false;
		xmlElement->SetAttribute(attributeSaveString.c_str(), data.c_str());
		return true;
	}
	
	
	//————— ATTRIBUTE DESERIALIZATION
	
	bool deserializeAttribute(std::string idString, int& data){
		if(!logAttributeDeserializationError(idString)) return false;
		tinyxml2::XMLError result = xmlElement->QueryIntAttribute(idString.c_str(), &data);
		return logAttributeDeserializationResult(result, idString);
	}
	bool deserializeAttribute(std::string idString, long long& data){
		if(!logAttributeDeserializationError(idString)) return false;
		tinyxml2::XMLError result = xmlElement->QueryInt64Attribute(idString.c_str(), &data);
		return logAttributeDeserializationResult(result, idString);
	}
	bool deserializeAttribute(std::string idString, unsigned long long& data){
		if(!logAttributeDeserializationError(idString)) return false;
		tinyxml2::XMLError result = xmlElement->QueryUnsigned64Attribute(idString.c_str(), &data);
		return logAttributeDeserializationResult(result, idString);
	}
	bool deserializeAttribute(std::string idString, float& data){
		if(!logAttributeDeserializationError(idString)) return false;
		tinyxml2::XMLError result = xmlElement->QueryFloatAttribute(idString.c_str(), &data);
		return logAttributeDeserializationResult(result, idString);
	}
	bool deserializeAttribute(std::string idString, double& data){
		if(!logAttributeDeserializationError(idString)) return false;
		tinyxml2::XMLError result = xmlElement->QueryDoubleAttribute(idString.c_str(), &data);
		return logAttributeDeserializationResult(result, idString);
	}
	bool deserializeAttribute(std::string idString, std::string& data){
		if(!logAttributeDeserializationError(idString)) return false;
		const char* buffer;
		tinyxml2::XMLError result = xmlElement->QueryStringAttribute(idString.c_str(), &buffer);
		if(!logAttributeDeserializationResult(result, idString)) return false;
		data = buffer;
		return true;
	}

	
public:
	
	tinyxml2::XMLElement* xmlElement = nullptr;
	virtual bool onSerialization() { return true; }
	virtual bool onDeserialization() { return true; }
	
protected:
	
	std::string saveString;
	
	void setXmlElement(tinyxml2::XMLElement* xmlElement_){
		xmlElement = xmlElement_;
	}
	
	bool logAttributeDeserializationResult(tinyxml2::XMLError result, std::string& idString){
		switch(result){
			case tinyxml2::XML_SUCCESS:
				Logger::trace("Deserialized attribute '{}' of element <{}>", idString, saveString);
				return true;
			case tinyxml2::XML_NO_ATTRIBUTE:
				Logger::error("Failed to load attribute '{}' of element <{}> : No Attribute found", idString, saveString);
				return false;
			case tinyxml2::XML_WRONG_ATTRIBUTE_TYPE:
				Logger::error("Failed to load attribute '{}' of element <{}> : Wrong Attribute Type", idString, saveString);
				return false;
			default:
				Logger::error("Failed to load attribute '{}' of element <{}> : tinyxml::XMLError {}", idString, saveString, result);
				return false;
		}
	}
	
	bool logAttributeSerializationError(std::string& idString){
		if(xmlElement == nullptr){
			Logger::error("Could not save attribute '{}', element <{}> was not serialized first", idString, saveString);
			return false;
		}
		if(idString.empty()){
			Logger::error("Coud not save attribute of element <{}> : no save string provided", saveString);
			return false;
		}
		return true;
	}
	
	bool logAttributeDeserializationError(std::string& idString){
		if(xmlElement == nullptr){
			Logger::error("Could not load attribute '{}', element <{}> was not deserialized first", idString, saveString);
			return false;
		}
		if(idString.empty()){
			Logger::error("Coud not load attribute of element <{}> : no save string provided", saveString);
			return false;
		}
		return true;
	}
};
