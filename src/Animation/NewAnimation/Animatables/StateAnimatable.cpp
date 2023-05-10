#include "StateAnimatable.h"

#include "Legato/Editor/Parameters.h"

namespace AnimationSystem{

	std::shared_ptr<Legato::Parameter> StateAnimatable::createParameter() {
		return Legato::OptionParameter::createInstance(defaultOption, {&defaultOption}, "DefaultName", "DefaultSaveString");
	};

};
