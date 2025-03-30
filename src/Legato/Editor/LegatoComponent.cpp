#include "LegatoComponent.h"
#include <tinyxml2.h>

#include "LegatoFile.h"
#include "LegatoDirectory.h"
#include "LegatoProject.h"


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
	else identifier = input;
}

std::string Legato::Component::getIdentifierPath(){
	std::string identifierPath = identifier;
	if(parentComponent == nullptr) return identifier;
	else if(parentComponent == parentFile) return parentFile->getPath().string() + "/<" + identifier + ">";
	else return parentComponent->getIdentifierPath() + "/<" + identifier + ">";
}

void Legato::Component::onConstruction(){
	setIdentifier(getClassName());
}

Ptr<Legato::Component> Legato::Component::addChild(Ptr<Component> child){
	if(child == nullptr) return nullptr;
	if(hasChild(child)) {
		Logger::warn("[{}:{}] cannot add duplicate component [{}:{}]", getClassName(), getIdentifier(), child->getClassName(), child->getIdentifier());
		return nullptr;
	}
	addChildDependencies(child);
	childComponents.push_back(child);
	return child;
}

bool Legato::Component::hasChild(Ptr<Component> input){
	for(auto child : childComponents){
		if(child == input) return true;
	}
	return false;
}

void Legato::Component::addChildDependencies(Ptr<Component> child){
	child->parentComponent = shared_from_this();
	if(auto thisFile = cast<File>()){
		child->parentFile = thisFile;
	}
	else child->parentFile = parentFile;
	
	if(auto thisProject = cast<Project>()){
		child->parentProject = thisProject;
	}
	else child->parentProject = parentProject;
	
	//in case a component already has children when adding it to a parent
	//we need the parents dependencies to be propagated down the hierarchy
	//maybe this is an inefficient way to do it?
	for(auto childrensChild : child->childComponents){
		child->addChildDependencies(childrensChild);
	}
}




bool Legato::Component::serialize(){
	if(identifier.empty()){
		Logger::error("Could not save element <{}> : no entry identifier provided", identifier);
		return false;
	}
	xmlElement = parentComponent->xmlElement->InsertNewChildElement(identifier.c_str());
	bool b_success = true;
	b_success &= onSerialization();
	for(auto child : childComponents) {
		b_success &= child->serialize();
	}
	return b_success;
}


bool Legato::Component::deserialize(){
	if(identifier.empty()){
		Logger::error("Could not load element <{}> : no entry identifier provided", identifier);
		return false;
	}
	xmlElement = parentComponent->xmlElement->FirstChildElement(identifier.c_str());
	if(xmlElement == nullptr){
		Logger::error("[Load Failure] {} : could not find XML element", getIdentifierPath());
		return false;
	}
	bool b_success = true;
	//this deserializes and creates all necessary ChildComponents
	b_success &= onDeserialization();
	//this deserializes all child components
	for(auto child : childComponents) {
		b_success &= child->deserialize();
	}
	//this gets called after all children are deserialized,
	//in case the Component needs to initialize stuff after children have loaded
	b_success &= onPostLoad();
	return b_success;
}




bool Legato::Component::checkAttributeSerializationError(std::string& ID){
	if(xmlElement == nullptr){
		Logger::error("Could not save attribute '{}', element <{}> was not serialized first", ID, identifier);
		return true;
	}
	if(ID.empty()){
		Logger::error("Coud not save attribute of element <{}> : identifier string empty", identifier);
		return true;
	}
	std::string sanitizedID;
	if(sanitizeXmlIdentifier(ID, sanitizedID)){
		Logger::warn("Invalid Attribute identifier '{}' of element <{}> was sanitized to '{}'", ID, identifier, sanitizedID);
		ID = sanitizedID;
		return true;
	}
	return false;
}


bool Legato::Component::serializeBoolAttribute(std::string ID, bool data){
	if(checkAttributeSerializationError(ID)) return false;
	xmlElement->SetAttribute(ID.c_str(), data);
	return true;
}

bool Legato::Component::serializeIntAttribute(std::string ID, int data){
	if(checkAttributeSerializationError(ID)) return false;
	xmlElement->SetAttribute(ID.c_str(), data);
	return true;
}

bool Legato::Component::serializeLongAttribute(std::string ID, long long data){
	if(checkAttributeSerializationError(ID)) return false;
	xmlElement->SetAttribute(ID.c_str(), data);
	return true;
}

bool Legato::Component::serializeDoubleAttribute(std::string ID, double data){
	if(checkAttributeSerializationError(ID)) return false;
	xmlElement->SetAttribute(ID.c_str(), data);
	return true;
}

bool Legato::Component::serializeStringAttribute(std::string ID, const std::string& data){
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
			//Logger::trace("Deserialized attribute '{}' of element <{}>", ID, identifier);
			return true;
		case tinyxml2::XML_NO_ATTRIBUTE:
			Logger::error("[Load Failure] {}/{} : No Attribute found", getIdentifierPath(), ID);
			return false;
		case tinyxml2::XML_WRONG_ATTRIBUTE_TYPE:
			Logger::error("[Load Error] {}/{} : Wrong Attribute Type", getIdentifierPath(), ID);
			return false;
		default:
			Logger::error("[Load Error] {}/{} : tinyxml::XMLError {}", getIdentifierPath(), ID, result);
			return false;
	}
}


bool Legato::Component::deserializeBoolAttribute(std::string ID, bool& data){
	if(checkAttributeDeserializationError(ID)) return false;
	tinyxml2::XMLError result = xmlElement->QueryBoolAttribute(ID.c_str(), &data);
	return checkAttributeDeserializationResult(result, ID);
}

bool Legato::Component::deserializeIntAttribute(std::string ID, int& data){
	if(checkAttributeDeserializationError(ID)) return false;
	tinyxml2::XMLError result = xmlElement->QueryIntAttribute(ID.c_str(), &data);
	return checkAttributeDeserializationResult(result, ID);
}

bool Legato::Component::deserializeLongAttribute(std::string ID, long long& data){
	if(checkAttributeDeserializationError(ID)) return false;
	tinyxml2::XMLError result = xmlElement->QueryInt64Attribute(ID.c_str(), &data);
	return checkAttributeDeserializationResult(result, ID);
}

bool Legato::Component::deserializeDoubleAttribute(std::string ID, double& data){
	if(checkAttributeDeserializationError(ID)) return false;
	tinyxml2::XMLError result = xmlElement->QueryDoubleAttribute(ID.c_str(), &data);
	return checkAttributeDeserializationResult(result, ID);
}

bool Legato::Component::deserializeStringAttribute(std::string ID, std::string& data){
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


