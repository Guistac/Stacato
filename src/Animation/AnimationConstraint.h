#pragma once

#include "Motion/MotionTypes.h"

class AnimationConstraint{
public:

	enum class Type{
		HALT,
		KEEPIN,
		KEEPOUT
	};
	
	AnimationConstraint(std::string name_) : name(name_){}
	
	virtual AnimatableType getAnimatableType() = 0;
	virtual Type getType() = 0;
	
	std::string& getName(){ return name; }
	bool isEnabled() { return b_enabled; }
	void enable() { b_enabled = true; }
	void disable() { b_enabled = false; }

private:
	bool b_enabled = false;
	std::string name;
};

class HaltConstraint : public AnimationConstraint{
public:
	
	HaltConstraint(std::string name) : AnimationConstraint(name){}
	
	virtual Type getType() override { return AnimationConstraint::Type::HALT; }
	
	virtual AnimatableType getAnimatableType() override { return AnimatableType::COMPOSITE; }
	
};
