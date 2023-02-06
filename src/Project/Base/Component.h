#pragma once

#include "Serializable.h"

class Component : public Serializable{
public:
	
	Component(std::string saveString_) {}
	
	virtual void initializeImpl() = 0;
	virtual Component* duplicateImpl() = 0;
	
	virtual bool onSerialization() override {
		serializeAttribute("UniqueID", uniqueID);
		//name.serialize(this);
		return true;
	}
	virtual bool onDeserialization() override {
		if(!deserializeAttribute("UniqueID", uniqueID)) return false;
		//if(!name.deserializeFromParent(this)) return false;
		return true;
	}
	
	void initialize(){
		initializeImpl();
	}
	
	Component* duplicate(){
		Component* copy = duplicateImpl();
		//copy name parameter value
		//generate new uniqueID or do this later ?
		return copy;
	}
	
	
private:
	//Parameter name;
	std::vector<Serializable*> attributes;
	unsigned long long uniqueID = -1;
};
