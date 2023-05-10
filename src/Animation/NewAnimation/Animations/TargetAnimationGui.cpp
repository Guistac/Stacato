#include <pch.h>

#include "TargetAnimation.h"

namespace AnimationSystem{

	void TargetAnimation::parameterGui(){
		curveTargetParameter->gui();
		rampInParameter->gui();
		rampOutParameter->gui();
	}

};
