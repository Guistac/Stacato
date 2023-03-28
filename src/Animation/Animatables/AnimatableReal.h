#pragma once

/*
#include "Animation/Animatable.h"
#include "Animation/AnimationValue.h"

struct AnimatableRealValue : public AnimationValue{
	virtual AnimatableType getType() override { return AnimatableType::REAL; }
	double value = 0.0;
};


class AnimatableReal : public AnimatableNumber{
public:
	virtual std::string getTargetValueString() override;
	
	//———————————— Contruction & Type Identification ————————————
	
	AnimatableReal(std::string name_, Unit unit, std::string suffix_) : AnimatableNumber(name_.c_str(), unit), suffix(suffix_){
		actualValue = AnimationValue::makeReal();
		targetValue = AnimationValue::makeReal();
	}
	
	static std::shared_ptr<AnimatableReal> make(std::string name, Unit unit, std::string suffix){
		return std::make_shared<AnimatableReal>(name, unit, suffix);
	}
	
	virtual AnimatableType getType() override { return AnimatableType::REAL; }
	
	virtual std::vector<std::string>& getCurveNames() override;
	virtual void fillControlPointDefaults(std::shared_ptr<Motion::ControlPoint> controlpoint) override;
	
	virtual void updateActualValue(std::shared_ptr<AnimationValue> newActualValue) override;
	virtual void updateTargetValue(double time_seconds, double deltaTime_seconds) override;
	virtual void followActualValue(double time_seconds, double deltaTime_seconds) override;
	
	virtual std::shared_ptr<AnimationValue> getActualValue() override;
	virtual std::shared_ptr<AnimationValue> getTargetValue() override;
		
	//—————————————— Commands ———————————————
	
public:
	virtual bool generateTargetAnimation(std::shared_ptr<TargetAnimation> animation) override;
	virtual bool validateAnimation(std::shared_ptr<Animation> animation) override;
	
	virtual bool isReadyToMove() override;
	virtual bool isReadyToStartPlaybackFromValue(std::shared_ptr<AnimationValue> animationValue) override;
	
	virtual bool isInRapid() override;
	virtual float getRapidProgress() override;
	virtual void cancelRapid() override;
	virtual std::shared_ptr<AnimationValue> getRapidTarget() override;
	
	virtual void stopMovement() override;
	
private:
	virtual void onRapidToValue(std::shared_ptr<AnimationValue> animationValue) override;
	virtual void onPlaybackStart(std::shared_ptr<Animation> animation) override;
	virtual void onPlaybackPause() override;
	virtual void onPlaybackStop() override;
	virtual void onPlaybackEnd() override;
	
	//———————————— Manual Controls —————————————
	
public:
	virtual bool hasManualControls() override;
	virtual void onSetManualControlTarget(float x, float y, float z) override;
	
public:
	//implemented by each animatable type
	virtual std::vector<InterpolationType>& getCompatibleInterpolationTypes() override;
	virtual int getCurveCount() override;
	virtual std::shared_ptr<Parameter> makeParameter() override;
	virtual void setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value) override;
	virtual void copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to) override;
	virtual std::shared_ptr<AnimationValue> parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter) override;
	virtual bool isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2) override;
	virtual std::shared_ptr<AnimationValue> getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds) override;
	virtual std::vector<double> getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value) override;
	virtual bool isControlledManuallyOrByAnimation() override;
	
	virtual bool isMoving() override;
	
private:
	
	std::shared_ptr<AnimatableRealValue> targetValue;
	std::shared_ptr<AnimatableRealValue> actualValue;
	std::string suffix;
};
*/
