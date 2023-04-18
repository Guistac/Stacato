#include <pch.h>

#include "NamedObject.h"
#include "Parameters/StringParameter.h"

namespace Legato{

	void NamedObject::onConstruction(){
		if(b_hasNameParameter){
			nameParameter = StringParameter::createInstance("Default Object Name", "Name", "ObjectName");
			nameParameter->addEditCallback([this](){ onNameEdit(); });
		}
		else {
			nonParametricName = "Default Object Name";
		}
	}

	void NamedObject::disableNameParameter(){
		if(b_hasNameParameter && nameParameter != nullptr){
			nonParametricName = nameParameter->getValue();
			nameParameter = nullptr;
		}
		b_hasNameParameter = false;
	}

	const std::string& NamedObject::getName(){
		if(b_hasNameParameter) return nameParameter->getValue();
		else return nonParametricName;
	}

	void NamedObject::setName(std::string name){
		if(b_hasNameParameter) nameParameter->overwrite(name);
		else nonParametricName = name;
		onNameEdit();
	}

};
