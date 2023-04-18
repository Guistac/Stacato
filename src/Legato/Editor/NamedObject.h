#pragma once

namespace Legato{

class StringParameter;

class NamedObject{
public:
	
	void onConstruction();
	
	bool hasNameParameter(){ return b_hasNameParameter; }
	const std::string& getName();
	void setName(std::string name);
	
	std::shared_ptr<StringParameter> nameParameter = nullptr;
	
	void addNameEditCallback(std::function<void()> cb){
		nameEditCallbacks.push_back(cb);
	}
	
protected:
	void disableNameParameter();
	
	std::string nonParametricName;
	bool b_hasNameParameter = true;
	std::vector<std::function<void()>> nameEditCallbacks = {};
	
private:
	
	void onNameEdit(){
		for(auto callback : nameEditCallbacks){
			callback();
		}
	}
};

};
