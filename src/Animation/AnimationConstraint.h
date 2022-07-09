#pragma once

#include "Motion/MotionTypes.h"

class AnimationConstraint{
public:
	
	AnimationConstraint(std::string name_) : name(name_){}
	
	virtual AnimatableType getType() = 0;
	virtual bool isHaltConstraint(){ return false; }
	
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
	
	virtual AnimatableType getType() override { return AnimatableType::COMPOSITE; }
	virtual bool isHaltConstraint() override { return true; }
	
};
