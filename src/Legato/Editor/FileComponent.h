#pragma once

#include "Component.h"
#include "File.h"

#include <tinyxml2.h>

/*————————————————————————————————————————————————————————————————
 
DESCRIPTION
	an abstract object for a file that can be serialized and deserialized
 
USAGE
	Supply a file name, path and save string
	call the write() method or the read() method
 
IMPLEMENTATION
 -Macro
	 DECLARE_PROTOTYPE_IMPLENTATION_METHODS()
		 Only use in a completely implemented type, no virtual classes and interfaces
		 Declares the basic methods which allow duplication and force shared_ptr instancing
 -Mandatory protected virtual methods:
	bool onSerialization()
	bool onDeserialization()
	void onConstruction()
	void onCopyFrom(std::shared_ptr<PrototypeBase> source);

IMPLEMENTATION EXAMPLE
 
	class FileComponentImplementation : public FileComponent{
	 
		DECLARE_PROTOTYPE_IMPLENTATION_METHODS(FileComponentImplementation)
 
	protected:

		virtual bool onSerialization() override {
			bool success = true;
			success &= Component::onSerialization();
			//serizalize stuff
			return success;
		}
 
		virtual bool onDeserialization() override {
			bool success = true;
			success &= Component::onDeserialization();
			//deserialize stuff
			return success;
		}
 
		 virtual void onConstruction() override {
			Component::onConstruction();
			//initialize stuff
		 }
		 
		 virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
			Component::onCopyFrom(source);
			auto original = std::static_pointer_cas<FileComponentImplementation>(source);
			 //copy stuff
		 }
	 
	 };
 
 ————————————————————————————————————————————————————————————————*/

namespace Legato{

class FileComponent : public Legato::Component, public File{
	
	DECLARE_PROTOTYPE_INTERFACE_METHODS(FileComponent)
	
public:
	
	virtual bool onWriteFile() override {
		
		using namespace tinyxml2;
		
		XMLDocument xmlDocument;
		xmlElement = xmlDocument.NewElement(saveString.c_str());
		xmlDocument.InsertEndChild(xmlElement);
		
		if(!onSerialization()) {
			Logger::warn("Failed to write document '{}' : serialization failed", filePath.string());
			return false;
		}
		
		XMLError result = xmlDocument.SaveFile(filePath.c_str());
		switch(result){
			case XMLError::XML_SUCCESS:
				Logger::trace("Successfully saved document '{}'", filePath.string());
				return true;
			case XMLError::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
				Logger::warn("Failed to write document '{}' : Invalid file path", filePath.string());
				return false;
			default:
				Logger::warn("Failed to write document '{}' : File could not be written", filePath.string());
				return false;
		}
	}
	
	
	virtual bool onReadFile() override {
		
		using namespace tinyxml2;
		
		XMLDocument xmlDocument;
		XMLError result = xmlDocument.LoadFile(filePath.c_str());
		
		switch(result){
			case XMLError::XML_SUCCESS:
				break;
			case XMLError::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
				Logger::error("Failed to read document '{}' : Invalid file path", filePath.string());
				return false;
			case XMLError::XML_ERROR_FILE_NOT_FOUND:
				Logger::error("Failed to read document '{}' : File not found", filePath.string());
				return false;
			case XMLError::XML_ERROR_FILE_READ_ERROR:
				Logger::error("Failed to read document '{}' : Error reading file", filePath.string());
				return false;
			case XMLError::XML_ERROR_EMPTY_DOCUMENT:
				Logger::error("Failed to read document '{}' : Empty document", filePath.string());
				return false;
			default:
				Logger::warn("Failed to read document '{}' : XMLError code {}", filePath.string(), result);
				return false;
		}
		
		xmlElement = xmlDocument.FirstChildElement(saveString.c_str());
		if(xmlElement == nullptr){
			Logger::warn("Failed to read document '{}' : Could not find root element <{}>", filePath.string(), saveString);
			return false;
		}
		
		if(!onDeserialization()){
			Logger::warn("Failed to read document '{}' : Deserialisation failed", filePath.string());
			return false;
		}
		
		Logger::trace("Successfully read document '{}'", filePath.string());
		return true;
	}
	
private:
	
	virtual bool serializeIntoParent(Serializable& parent) override { throwSerializationAssertion(); }
	virtual bool serializeIntoParent(Serializable* parent) override { throwSerializationAssertion(); }
	virtual bool serializeIntoParent(std::shared_ptr<Serializable> parent) override { throwSerializationAssertion(); }
	virtual bool deserializeFromParent(Serializable& parent) override { throwSerializationAssertion(); }
	virtual bool deserializeFromParent(Serializable* parent) override { throwSerializationAssertion(); }
	virtual bool deserializeFromParent(std::shared_ptr<Serializable> parent) override { throwSerializationAssertion(); }
	
	void throwSerializationAssertion(){
		assert("File Component cannot be serialized or deserialized, use readFile() and writeFile() methods");
	}
};

}
