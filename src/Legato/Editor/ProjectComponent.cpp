#include "ProjectComponent.h"
#include <tinyxml2.h>

bool Legato::sanitizeXmlIdentifier(const std::string input, std::string& output){
	if (input.empty()) {
		output = "DefaultIdentifier";
		Logger::warn("Provided identifier is empty, default identifier set");
		return true;
	}
	
	output.clear();
	output.reserve(input.size());
	bool b_sanitized = false;
	
	if (std::isalpha(input[0]) || input[0] == '_') output.push_back(input[0]);
	else {
		output.push_back('_');
		Logger::trace("XML Identifier '{}' first character '{}' is invalid", input, input[0]);
		b_sanitized = true;
	}
	
	for(int i = 1; i < input.size(); i++){
		char c = input[i];
		if (std::isalnum(c) || c == '-' || c == '_' || c == '.') output.push_back(c);
		else {
			output.push_back('_');
			Logger::trace("Identifier '{}' contains invalid character '{}'", input, c);
			b_sanitized = true;
		}
	}
	
	if (output.size() >= 3) {
		std::string prefix = output.substr(0, 3);
		for (char& c : prefix) c = std::tolower(c);
		if (prefix == "xml") {
			output.insert(0, "_");
			Logger::trace("Identifier '{}' cannot start with 'xml'", input);
			b_sanitized = true;
		}
	}
	
	return b_sanitized;
}


void Legato::Component::setIdentifier(std::string input){
	if(sanitizeXmlIdentifier(input, identifier)){
		Logger::warn("[{}] invalid identifier '{}' was sanitized to '{}'", getClassName(), input, identifier);
	}
}

bool Legato::Component::serialize(){
	if(identifier.empty()){
		Logger::error("Could not save element <{}> : no entry identifier provided", identifier);
		return false;
	}
	xmlElement = parentComponent->xmlElement->InsertNewChildElement(identifier.c_str());
	onSerialization();
	for(auto child : childComponents) child->serialize();
	return true;
}


bool Legato::Component::deserialize(){
	if(identifier.empty()){
		Logger::error("Could not load element <{}> : no entry identifier provided", identifier);
		return false;
	}
	xmlElement = parentComponent->xmlElement->FirstChildElement(identifier.c_str());
	if(xmlElement == nullptr){
		Logger::error("Could not load element <{}> : could not find XML element", identifier);
		return false;
	}
	//this deserializes and creates all necessary ChildComponents
	onDeserialization();
	//this deserializes all child components
	for(auto child : childComponents) child->deserialize();
	//this gets called after all children are deserialized,
	//in case the Component needs to initialize stuff after children have loaded
	onPostLoad();
	return true;
}




bool Legato::Component::checkAttributeSerializationError(std::string& ID){
	if(xmlElement == nullptr){
		Logger::error("Could not save attribute '{}', element <{}> was not serialized first", ID, identifier);
		return false;
	}
	if(ID.empty()){
		Logger::error("Coud not save attribute of element <{}> : identifier string empty", identifier);
		return false;
	}
	std::string sanitizedID;
	if(sanitizeXmlIdentifier(ID, sanitizedID)){
		Logger::warn("Invalid Attribute identifier '{}' of element <{}> was sanitized to '{}'", ID, identifier, sanitizedID);
		ID = sanitizedID;
	}
	return true;
}

bool Legato::Component::serializeAttribute(std::string ID, int data){
	if(checkAttributeSerializationError(ID)) return false;
	xmlElement->SetAttribute(ID.c_str(), data);
	return true;
}

bool Legato::Component::serializeAttribute(std::string ID, double data){
	if(checkAttributeSerializationError(ID)) return false;
	xmlElement->SetAttribute(ID.c_str(), data);
	return true;
}

bool Legato::Component::serializeAttribute(std::string ID, const std::string& data){
	if(checkAttributeSerializationError(ID)) return false;
	xmlElement->SetAttribute(ID.c_str(), data.c_str());
	return true;
}


bool Legato::Component::checkAttributeDeserializationError(std::string& ID){
	if(xmlElement == nullptr){
		Logger::error("Could not load attribute '{}', element <{}> was not deserialized first", ID, identifier);
		return true;
	}
	if(ID.empty()){
		Logger::error("Coud not load attribute of element <{}> : identifier string empty", identifier);
		return true;
	}
	std::string sanitizedID;
	if(sanitizeXmlIdentifier(ID, sanitizedID)){
		Logger::warn("Invalid Attribute identifier '{}' of element <{}> was sanitized to '{}'", ID, identifier, sanitizedID);
		ID = sanitizedID;
	}
	return false;
}

bool Legato::Component::checkAttributeDeserializationResult(int result, std::string& ID){
	switch(result){
		case tinyxml2::XML_SUCCESS:
			Logger::trace("Deserialized attribute '{}' of element <{}>", ID, identifier);
			return true;
		case tinyxml2::XML_NO_ATTRIBUTE:
			Logger::error("Failed to load attribute '{}' of element <{}> : No Attribute found", ID, identifier);
			return false;
		case tinyxml2::XML_WRONG_ATTRIBUTE_TYPE:
			Logger::error("Failed to load attribute '{}' of element <{}> : Wrong Attribute Type", ID, identifier);
			return false;
		default:
			Logger::error("Failed to load attribute '{}' of element <{}> : tinyxml::XMLError {}", ID, identifier, result);
			return false;
	}
}

bool Legato::Component::deserializeAttribute(std::string ID, int& data){
	if(checkAttributeDeserializationError(ID)) return false;
	tinyxml2::XMLError result = xmlElement->QueryIntAttribute(ID.c_str(), &data);
	return checkAttributeDeserializationResult(result, ID);
}

bool Legato::Component::deserializeAttribute(std::string ID, double& data){
	if(checkAttributeDeserializationError(ID)) return false;
	tinyxml2::XMLError result = xmlElement->QueryDoubleAttribute(ID.c_str(), &data);
	return checkAttributeDeserializationResult(result, ID);
}

bool Legato::Component::deserializeAttribute(std::string ID, std::string& data){
	if(checkAttributeDeserializationError(ID)) return false;
	const char* buffer;
	tinyxml2::XMLError result = xmlElement->QueryStringAttribute(ID.c_str(), &buffer);
	if(checkAttributeDeserializationResult(result, ID)){
		data = buffer;
		return true;
	}
	return false;
}


Ptr<Legato::Component> Legato::Component::duplicateComponent(){
	//this makes a new instance of the component, with data at defaults
	Ptr<Component> copy = instanciateComponent();
	//this makes sure the copy has all the same ChildComponents as the original
	//this also copies fields that are not ChildComponents
	copy->copyFrom(shared_from_this());
	//now we make all ChildComponents copy the data of the original ChildComponents
	for(auto originalChild : childComponents){
		for(auto copyChild : copy->childComponents){
			//match original childComponents to copies using their identifier and classname
			if(originalChild->identifier == copyChild->identifier && originalChild->getClassName() == copyChild->getClassName()){
				copyChild->copyFrom(originalChild);
			}
		}
	}
	//this gets called after all children are duplicated,
	//in case the Component needs to initialize stuff after children have finished setting up
	onPostLoad();
	//in theory we now have a deep copy of the original
	return copy;
}




/*
void Legato::FileComponent::setFileName(std::string input){
	std::filesystem::path inputPath = input;
	if(!inputPath.has_filename()) {
		Logger::error("[File] '{}' does not contain a filename", input);
		return;
	}
	
	if(inputPath.has_parent_path()){
		Logger::warn("[File] '{}' is not a file name, removing parent path", input);
		fileName = inputPath.filename().string();
	}
	else fileName = input;

	if(filePath.has_filename()) filePath = filePath.parent_path() / fileName;
	else filePath = filePath / fileName;
}
void Legato::FileComponent::setFileLocation(std::filesystem::path input){
	if(!input.has_parent_path()){
		Logger::error("[File] '{}' does not contain a path", input.string());
		return;
	}
	if(input.has_filename()){
		Logger::warn("[File] '{}' is not a directory, removing file name", input.string());
		filePath = input.parent_path() / fileName;
	}
	filePath = filePath / fileName;
}
void Legato::FileComponent::setFileLocationAndName(std::filesystem::path input){
	if(!input.has_parent_path()){
		Logger::error("[File] '{}' does not have a parent path", input.string());
		return;
	}
	if(!input.has_filename()){
		Logger::error("[File] '{}' does not have a file name", input.string());
		return;
	}
	filePath = input;
}
 

bool Legato::FileComponent::serialize(){
	
	tinyxml2::XMLDocument xmlDocument;
	xmlElement = xmlDocument.NewElement(identifier.c_str());
	xmlDocument.InsertEndChild(xmlElement);
	
	onSerialization();
	for(auto child : childComponents) child->serialize();
	
	tinyxml2::XMLError result = xmlDocument.SaveFile(filePath.c_str());
	switch(result){
		case tinyxml2::XMLError::XML_SUCCESS:
			Logger::debug("[File] Successfully saved file '{}'", filePath.string());
			return true;
		case tinyxml2::XMLError::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
			Logger::error("[File] Failed to write file '{}' : Invalid file path", filePath.string());
			return false;
		default:
			Logger::error("[File] Failed to write file '{}' : File could not be written", filePath.string());
			return false;
	}
	
	return true;
}

bool Legato::FileComponent::deserialize(){
	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLError result = xmlDocument.LoadFile(filePath.c_str());
	switch(result){
		case tinyxml2::XMLError::XML_SUCCESS:
			break;
		case tinyxml2::XMLError::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
			Logger::error("[File] Failed to read file '{}' : Invalid file path", filePath.string());
			return false;
		case tinyxml2::XMLError::XML_ERROR_FILE_NOT_FOUND:
			Logger::error("[File] Failed to read file '{}' : File not found", filePath.string());
			return false;
		case tinyxml2::XMLError::XML_ERROR_FILE_READ_ERROR:
			Logger::error("[File] Failed to read file '{}' : Error reading file", filePath.string());
			return false;
		case tinyxml2::XMLError::XML_ERROR_EMPTY_DOCUMENT:
			Logger::error("[File] Failed to read file '{}' : Empty document", filePath.string());
			return false;
		default:
			Logger::warn("[File] Failed to read file '{}' : XMLError code {}", filePath.string(), result);
			return false;
	}
	
	xmlElement = xmlDocument.FirstChildElement(identifier.c_str());
	if(xmlElement == nullptr){
		Logger::warn("[File] Failed to read file '{}' : Could not find root element <{}>", filePath.string(), identifier);
		return false;
	}
	
	onDeserialization();
	for(auto child : childComponents) child->deserialize();
	onPostLoad();
		
	Logger::debug("Successfully read file '{}'", filePath.string());
	return true;
}
*/
 

bool Legato::Project::serialize(){
	
}

bool Legato::Project::deserialize(){
	
}
