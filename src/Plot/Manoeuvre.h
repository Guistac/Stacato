#pragma once

class AnimatableParameter;

class Manoeuvre{
public:

	const char name[64];
	const char description[256];

	std::vector<std::shared_ptr<AnimatableParameter>> parameters;

};

