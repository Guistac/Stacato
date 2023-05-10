#include "PositionAnimatable.h"

#include "Legato/Editor/Parameters.h"

namespace AnimationSystem{


	std::shared_ptr<Legato::Parameter> PositionAnimatable::createParameter(){
		return Legato::NumberParameter<double>::createInstance(0, "DefaultName", "DefaultSaveString");
	};

};
