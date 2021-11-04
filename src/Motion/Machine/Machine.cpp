#include <pch.h>

#include "Machine.h"

#include "Motion/AnimatableParameter.h"

void Machine::stopParameterPlayback() {
	for (auto& parameter : animatableParameters) {
		parameter->actualParameterTrack = nullptr;
	}
}
