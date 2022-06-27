#pragma once

#include "Animation/Animatable.h"
#include "Animation/AnimationValue.h"

#include "Motion/Curve/Profile.h"

class AnimatablePosition : public AnimatableNumber{
public:
	
	//construction
	AnimatablePosition(const char* name, Unit unit) : AnimatableNumber(name, unit) {};
	static std::shared_ptr<AnimatablePosition> make(std::string name, Unit unit){ return std::make_shared<AnimatablePosition>(name.c_str(), unit); }
	virtual AnimatableType getType() override { return AnimatableType::POSITION; }
	
	virtual std::vector<InterpolationType>& getCompatibleInterpolationTypes() override;
	virtual int getCurveCount() override;
	virtual std::shared_ptr<Parameter> makeParameter() override;
	virtual void setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value) override;
	virtual void copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to) override;
	virtual std::shared_ptr<AnimationValue> parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter) override;
	virtual bool isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2) override;
	virtual std::shared_ptr<AnimationValue> getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds) override;
	virtual std::vector<double> getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value) override;
	
	
	Motion::Profile motionProfile;
	double lowerPositionLimit;
	double upperPositionLimit;
	double velocityLimit;
	double accelerationLimit;
	
	
	enum ControlMode{
		COPY,				//no target, we copy external values
		VELOCITY_SETPOINT,	//manual velocity control
		POSITION_SETPOINT,	//position curve following
		POSITION_TARGET		//planned movement to given position
	};
	
	/*
	 
	 the position animatable handles:
	 -manual velocity control
	 -manual position control
	 -control mode
	 -set point following
	 -position, velocity, acceleration limits
	 
	 */
	
};



struct AnimatablePositionValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::POSITION; }
	double position;
	double velocity;
	double acceleration;
};

