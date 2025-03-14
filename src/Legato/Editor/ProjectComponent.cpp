#include "ProjectComponent.h"
#include <tinyxml2.h>


bool Legato::Component::setIdentifier(std::string input){
	if (input.empty()) {
		identifier = "DefaultIdentifier";
		Logger::warn("Provided identifier is empty, default identifier set");
		b_hasValidIdentifier = true;
		return false;
	}
	
	identifier.clear();
	identifier.reserve(input.size());
	
	bool b_anyCharacterValid = false;
	bool b_sanitized = false;
	
	if (std::isalpha(input[0]) || input[0] == '_') {
		identifier.push_back(input[0]);
		b_anyCharacterValid = true;
	}
	else {
		identifier.push_back('_');
		Logger::trace("Identifier '{}' first character '{}' is invalid", input, input[0]);
		b_sanitized = true;
	}
	
	for(int i = 1; i < input.size(); i++){
		char c = input[i];
		if (std::isalnum(c) || c == '-' || c == '_' || c == '.') {
			identifier.push_back(c);
			b_anyCharacterValid = true;
		}
		else {
			identifier.push_back('_');
			Logger::trace("Identifier '{}' contains invalid character '{}'", input, c);
			b_sanitized = true;
		}
	}
	
	if (identifier.size() >= 3) {
		std::string prefix = identifier.substr(0, 3);
		for (char& c : prefix) c = std::tolower(c);
		if (prefix == "xml") {
			identifier.insert(0, "_");
			Logger::trace("Identifier '{}' cannot start with 'xml'", input);
			b_sanitized = true;
		}
	}
	
	if(!b_anyCharacterValid){
		b_hasValidIdentifier = false;
		Logger::trace("Provided identifier '{}' does not contain any valid character", input);
		b_sanitized = true;
	}
	
	if(b_sanitized) Logger::warn("Invalid identifier '{}' updated to '{}'", input, identifier);
	
	return b_anyCharacterValid;
}


bool Legato::Component::serialize(){
	xmlElement = parentComponent->xmlElement->InsertNewChildElement(identifier.c_str());
	onSerialization();
	for(auto child : childComponents) child->serialize();
	return true;
}


bool Legato::Component::deserialize(){
	xmlElement = parentComponent->xmlElement->FirstChildElement(identifier.c_str());
	//this deserializes and creates all necessary ChildComponents
	onDeserialization();
	//this deserializes all child components
	for(auto child : childComponents) child->deserialize();
	//this gets called after all children are deserialized,
	//in case the Component needs to initialize stuff after children have loaded
	onPostLoad();
	return true;
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
