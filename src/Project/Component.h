#pragma once

#include "Serializable.h"



class Parameter : public Serializable{
public:
	
	Parameter(std::string saveString_) : Serializable(saveString_){}
	
	virtual bool serializeImpl() override {
		serializeAttribute("Value", data);
		return true;
	}
	virtual bool deserializeImpl() override {
		if(!deserializeAttribute("Value", data)) return false;
		return true;
	}
	
	std::string get(){ return data; }
	void set(std::string newData){ data = newData; }
	
private:
	std::string data;
	std::string displayName;
};








class Component : public Serializable{
public:
	
	Component(std::string saveString_) : Serializable(saveString_){}
	
	virtual void initializeImpl(){}
	virtual Component* duplicateImpl(){}
	
	virtual bool serializeImpl() override {
		serializeAttribute("UniqueID", uniqueID);
		name.serialize(this);
		return true;
	}
	virtual bool deserializeImpl() override {
		if(!deserializeAttribute("UniqueID", uniqueID)) return false;
		if(!name.deserializeFromParent(this)) return false;
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
	Parameter name = Parameter("name");
	unsigned long long uniqueID = -1;
};
