#include <pch.h>

#include "Parameter.h"
#include "Parameters/StringParameter.h"

namespace Legato{

void Parameter::onConstruction() {
	Component::onConstruction();
	//All parameters own a string parameter to hold their component name
	//this way each parameter can be renamed easily
	//this string parameter owned by parameters does not in turn have a parametric name
	//instead it just has a a string as a parameter that reads "Parameter Name"
	//When a parameter gets its name set, we also update its imguiID to comply with the gui library
	if(b_hasNameParameter){
		nameParameter->addEditCallback([this](){
			std::string parameterName = getName();
			imGuiID = "##" + parameterName;
		});
	}
}

}
