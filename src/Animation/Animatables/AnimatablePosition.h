#pragma once

#include "Animation/Animatable.h"
#include "Animation/AnimationValue.h"

#include "Motion/Curve/Profile.h"


struct AnimatablePositionValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::POSITION; }
	double position;
	double velocity;
	double acceleration;
};


class AnimatablePosition : public AnimatableNumber{
public:
	
	//—————————construction & typeId——————————
	AnimatablePosition(const char* name, Unit unit) : AnimatableNumber(name, unit) { updateActualValue(AnimationValue::makePosition()); };
	static std::shared_ptr<AnimatablePosition> make(std::string name, Unit unit){ return std::make_shared<AnimatablePosition>(name.c_str(), unit); }
	virtual AnimatableType getType() override { return AnimatableType::POSITION; }
	
	//————————animation interface———————————
	virtual std::vector<InterpolationType>& getCompatibleInterpolationTypes() override;
	virtual int getCurveCount() override;
	virtual std::shared_ptr<Parameter> makeParameter() override;
	virtual void setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value) override;
	virtual void copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to) override;
	virtual std::shared_ptr<AnimationValue> parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter) override;
	virtual bool isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2) override;
	virtual std::shared_ptr<AnimationValue> getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds) override;
	virtual std::vector<double> getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value) override;
	virtual bool generateTargetAnimation(std::shared_ptr<TargetAnimation> animation) override;
	virtual bool validateAnimation(std::shared_ptr<Animation> animation) override;
	
	//—————————state——————————
	virtual bool isReadyToMove() override;
	virtual bool isReadyToStartPlaybackFromValue(std::shared_ptr<AnimationValue> animationValue) override;
	virtual bool isInRapid() override;
	virtual float getRapidProgress() override;
	
	//——————————animation values——————————
	
	virtual void updateTargetValue(double time_seconds, double deltaTime_seconds) override;
	virtual std::shared_ptr<AnimationValue> getTargetValue() override;
	std::shared_ptr<AnimatablePositionValue> targetValue;
	
	virtual void updateActualValue(std::shared_ptr<AnimationValue> newActualValue) override;
	virtual std::shared_ptr<AnimationValue> getActualValue() override;
	std::shared_ptr<AnimatablePositionValue> actualValue;
	
	virtual void updateDisabled() override;
	
	//—————————movement commands——————————
	virtual void onRapidToValue(std::shared_ptr<AnimationValue> animationValue) override;
	virtual void cancelRapid() override;
	virtual void onPlaybackStart() override;
	virtual void onPlaybackPause() override;
	virtual void onPlaybackStop() override;
	virtual void onPlaybackEnd() override;
	virtual void stopMovement() override { setVelocityTarget(0.0); }
	
	//—————————manual controls——————————
	virtual bool hasManualControls() override { return true; }
	virtual void onSetManualControlTarget(float x, float y, float z) override;
	
	
public:
	void setManualVelocityTarget(double velocityTarget);
	void setManualPositionTargetWithVelocity(double targetPosition, double targetVelocity);
	void setManualPositionTargetWithTime(double targetPosition, double targetTime);
	
private:
	void setVelocityTarget(double velocityTarget);
	void moveToPositionWithVelocity(double targetPosition, double targetVelocity);
	void moveToPositionInTime(double targetPosition, double targetTime);
	
public:
	
	bool hasPositionSetpoint();
	double getPositionSetpoint();
	double getPositionSetpointNormalized();
	
	bool hasVelocitySetpoint();
	double getVelocitySetpoint();
	double getVelocitySetpointNormalized();
	
	double getActualPosition();
	double getActualPositionNormalized();
	
	double getActualVelocity();
	double getActualVelocityNormalized();
	
	double getActualAcceleration();
	double getActualAccelerationNormalized();
	
	Motion::Profile motionProfile;
	double profileTime_seconds;
	
	double lowerPositionLimit;
	double upperPositionLimit;
	double velocityLimit;
	double accelerationLimit;
	double rapidVelocity;
	double rapidAcceleration;
	
	std::mutex mutex;
	
	enum ControlMode{
		VELOCITY_SETPOINT,	//manual velocity control
		POSITION_SETPOINT,	//go to position
		POSITION_TARGET		//planned movement to given position
	}controlMode = ControlMode::VELOCITY_SETPOINT;
	double velocitySetpoint = 0.0;
	double positionSetpoint = 0.0;
	double accelerationSetpoint = 0.0;
};


//public: lock mutex
//private: require locked mutex
//do not call public functions internally
