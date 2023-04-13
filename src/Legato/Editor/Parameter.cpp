#include <pch.h>

#include "Parameter.h"
#include "Parameters/StringParameter.h"

namespace Legato{

void Parameter::onConstruction() {
	Component::onConstruction();
	addEditCallback([this](){
		imGuiID = "##" + getName();
	});
}

}
