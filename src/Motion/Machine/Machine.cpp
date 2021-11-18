#include <pch.h>

#include "Machine.h"

#include "Motion/AnimatableParameter.h"

void Machine::startParameterPlayback(std::shared_ptr<AnimatableParameter> parameter, std::shared_ptr<ParameterTrack> track) {
	for (auto& p : animatableParameters) {
		if (parameter == p) {
			parameter->actualParameterTrack = track;
			onParameterPlaybackStart(parameter);
		}
	}
}

void Machine::stopParameterPlayback(std::shared_ptr<AnimatableParameter> parameter) {
	for (auto& p : animatableParameters) {
		if (parameter == p) {
			parameter->actualParameterTrack = nullptr;
			onParameterPlaybackStop(parameter);
		}
	}
}