#include "LegatoFile.h"

#include <tinyxml2.h>
#include "LegatoDirectory.h"

void Legato::File::onConstruction(){
	Component::onConstruction();
	setIdentifier(getClassName());
}

void Legato::File::setFileName(std::filesystem::path input){
	if(!input.has_filename()) {
		Logger::error("[File] '{}' does not contain a filename", input.string());
		return;
	}
	fileName = input;
}

std::filesystem::path Legato::File::getPath(){
	std::filesystem::path completePath = fileName;
	Ptr<Directory> nextDir = parentDirectory;
	while(nextDir){
		completePath = nextDir->getDirectoryName() / completePath;
		nextDir = nextDir->parentDirectory;
	}
	return completePath;
}

bool Legato::File::serialize(){
	
	if(!fileName.has_filename()) {
		Logger::error("[File] Cannot serialize, "" is not a filename", fileName.string());
		return;
	}
	
	tinyxml2::XMLDocument xmlDocument;
	xmlElement = xmlDocument.NewElement(identifier.c_str());
	xmlDocument.InsertEndChild(xmlElement);
	
	onSerialization();
	for(auto child : childComponents) child->serialize();
	
	std::filesystem::path completePath = getPath();
	tinyxml2::XMLError result = xmlDocument.SaveFile(completePath.c_str());
	switch(result){
		case tinyxml2::XMLError::XML_SUCCESS:
			Logger::debug("[File] Successfully saved file '{}'", completePath.c_str());
			return true;
		case tinyxml2::XMLError::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
			Logger::error("[File] Failed to write file '{}' : Invalid file path", completePath.c_str());
			return false;
		default:
			Logger::error("[File] Failed to write file '{}' : File could not be written", completePath.c_str());
			return false;
	}
	
	return true;
}

bool Legato::File::deserialize(){
	
	if(!fileName.has_filename()) {
		Logger::error("[File] Cannot deserialize, "" is not a filename", fileName.string());
		return;
	}
	
	std::filesystem::path completePath = getPath();
	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLError result = xmlDocument.LoadFile(completePath.c_str());
	switch(result){
		case tinyxml2::XMLError::XML_SUCCESS:
			break;
		case tinyxml2::XMLError::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
			Logger::error("[File] Failed to read file '{}' : Invalid file path", completePath.string());
			return false;
		case tinyxml2::XMLError::XML_ERROR_FILE_NOT_FOUND:
			Logger::error("[File] Failed to read file '{}' : File not found", completePath.string());
			return false;
		case tinyxml2::XMLError::XML_ERROR_FILE_READ_ERROR:
			Logger::error("[File] Failed to read file '{}' : Error reading file", completePath.string());
			return false;
		case tinyxml2::XMLError::XML_ERROR_EMPTY_DOCUMENT:
			Logger::error("[File] Failed to read file '{}' : Empty document", completePath.string());
			return false;
		default:
			Logger::warn("[File] Failed to read file '{}' : XMLError code {}", completePath.string(), result);
			return false;
	}
	
	xmlElement = xmlDocument.FirstChildElement(identifier.c_str());
	if(xmlElement == nullptr){
		Logger::warn("[File] Failed to read file '{}' : Could not find root element <{}>", completePath.string(), identifier);
		return false;
	}
	
	onDeserialization();
	for(auto child : childComponents) child->deserialize();
	onPostLoad();
		
	Logger::debug("Successfully read file '{}'", completePath.string());
	return true;
}
 









