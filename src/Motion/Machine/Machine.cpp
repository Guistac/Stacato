#include <pch.h>

#include "Machine.h"

#include "Motion/AnimatableParameter.h"

void Machine::addAnimatableParameter(std::shared_ptr<AnimatableParameter> parameter) {
	parameter->machine = std::dynamic_pointer_cast<Machine>(shared_from_this());
	if (!parameter->childParameters.empty()) {
		for (auto& childParameter : parameter->childParameters) {
			childParameter->machine = parameter->machine;
			childParameter->parentParameter = parameter;
			animatableParameters.push_back(childParameter);
		}
	}
	animatableParameters.push_back(parameter);
}

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