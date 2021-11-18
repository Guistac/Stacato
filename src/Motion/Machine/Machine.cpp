#include <pch.h>

#include "Machine.h"

#include "Motion/AnimatableParameter.h"

void Machine::startParameterPlayback(std::shared_ptr<AnimatableParameter> parameter, std::shared_ptr<ParameterTrack> track) {
	for (auto& p : animatableParameters) {
		if (parameter == p) {
			AnimatableParameterValue value;
			getActualParameterValue(parameter, value);
			if (isParameterReadyToStartPlaybackFromValue(parameter, value)) {
				parameter->actualParameterTrack = track;
			}
		}
	}
}

void Machine::stopParameterPlayback(std::shared_ptr<AnimatableParameter> parameter) {
	for (auto& p : animatableParameters) {
		if(parameter == p) parameter->actualParameterTrack = nullptr;
	}
}