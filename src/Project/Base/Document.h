#pragma once

#include "Serializable.h"
#include "PrototypeBase.h"

#include <tinyxml2.h>


/*————————————————————————————————————————————————————————————————
 
DESCRIPTION
	an abstract object for a file that can be loaded and saved
 
USAGE
	Supply a file name, path and save string
	call the write() method or the read() method
 
IMPLEMENTATION
 -Mandatory protected virtual methods:
	 bool onSerialization()
	 bool onDeserialization()
 
IMPLEMENTATION EXAMPLE
 
	class DocumentImplementation : public Document{
	 
	protected:

		virtual bool onSerialization() override {
			bool success = true;
			success &= ParentClass::onSerialization();
			success &= serializeAttribute("Field", field);
			return success;
		}
 
		virtual bool onDeserialization() override {
			bool success = true;
			success &= ParentClass::onDeserialization();
			success &= deserializeAttribute("Field", field);
			return success;
		}
	 
	 private:
		float field = 0;
	 };
 
 ————————————————————————————————————————————————————————————————*/

class Document : public Serializable {
public:
	
	void setFileName(std::string fileName_){
		fileName = fileName_;
		updateFullFilePath();
	}
	
	void setFilePath(std::filesystem::path filePath_){
		filePath = filePath_;
		updateFullFilePath();
	}
	
	bool writeFile(){
		
		if(fileName.empty()){
			Logger::error("Failed to write document : No file name provided");
			return false;
		}
		if(filePath.empty()){
			Logger::error("Failed to write document '{}' : No file path provided", fileName);
			return false;
		}
		
		using namespace tinyxml2;
		
		XMLDocument xmlDocument;
		xmlElement = xmlDocument.NewElement(saveString.c_str());
		
		if(!onSerialization()) {
			Logger::warn("Failed to write document '{}' : serialization failed", fullFilePath);
			return false;
		}
		
		XMLError result = xmlDocument.SaveFile(fullFilePath.c_str());
		switch(result){
			case XMLError::XML_SUCCESS:
				Logger::trace("Successfully saved document '{}'", fullFilePath);
				return true;
			case XMLError::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
				Logger::warn("Failed to write document '{}' : Invalid file path", fullFilePath);
				return false;
			default:
				Logger::warn("Failed to write document '{}' : File could not be written", fullFilePath);
				return false;
		}
	}
	
	
	bool readFile(){
		
		if(fileName.empty()){
			Logger::error("Failed to read document : No file name provided");
			return false;
		}
		if(filePath.empty()){
			Logger::error("Failed to read document '{}' : No file path provided", fileName);
			return false;
		}
		
		using namespace tinyxml2;
		
		XMLDocument xmlDocument;
		XMLError result = xmlDocument.LoadFile(fullFilePath.c_str());
		
		switch(result){
			case XMLError::XML_SUCCESS:
				break;
			case XMLError::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
				Logger::error("Failed to read document '{}' : Invalid file path", fullFilePath);
				return false;
			case XMLError::XML_ERROR_FILE_NOT_FOUND:
				Logger::error("Failed to read document '{}' : File not found", fullFilePath);
				return false;
			case XMLError::XML_ERROR_FILE_READ_ERROR:
				Logger::error("Failed to read document '{}' : Error reading file", fullFilePath);
				return false;
			case XMLError::XML_ERROR_EMPTY_DOCUMENT:
				Logger::error("Failed to read document '{}' : Empty document", fullFilePath);
				return false;
			default:
				Logger::warn("Failed to read document '{}' : XMLError code {}", fullFilePath, result);
				return false;
		}
		
		xmlElement = xmlDocument.FirstChildElement(saveString.c_str());
		if(xmlElement == nullptr){
			Logger::warn("Failed to read document '{}' : Could not find root element <{}>", fullFilePath, saveString);
			return false;
		}
		
		if(!onDeserialization()){
			Logger::warn("Failed to read document '{}' : Deserialisation failed", fullFilePath);
			return false;
		}
		
		Logger::trace("Successfully read document '{}'", fullFilePath);
		return true;
	}
	
private:

	std::string fileName;
	std::filesystem::path filePath;
	std::string fullFilePath;
	
	virtual bool serializeIntoParent(Serializable& parent) override {}
	virtual bool serializeIntoParent(Serializable* parent) override {}
	virtual bool serializeIntoParent(std::shared_ptr<Serializable> parent) override {}
	virtual bool deserializeFromParent(Serializable& parent) override {}
	virtual bool deserializeFromParent(Serializable* parent) override {}
	virtual bool deserializeFromParent(std::shared_ptr<Serializable> parent) override {}
	
	void updateFullFilePath(){
		fullFilePath = filePath.string() + fileName;
	}
};
