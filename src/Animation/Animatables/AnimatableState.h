#pragma once

#include "Animation/Animatable.h"
#include "Animation/AnimationValue.h"
#include "Animation/AnimationConstraint.h"

//core state structure
struct AnimatableStateStruct{
	int integerEquivalent;
	const char displayName[64];
	const char saveName[64];
};

struct AnimatableStateValue : public AnimationValue{
	virtual AnimatableType getType() override { return AnimatableType::STATE; }
	AnimatableStateStruct* value = nullptr;
};


class AnimatableState : public Animatable{
public:
	
	//construction & type identification
	AnimatableState(const char* name,
					std::vector<AnimatableStateStruct*>& allStates_,
					std::vector<AnimatableStateStruct*>& selectableStates_,
					AnimatableStateStruct* stoppedState_) :
	Animatable(name),
	allStates(&allStates_),
	selectableStates(&selectableStates_),
	stoppedState(stoppedState_){
		actualValue = AnimationValue::makeState();
		actualValue->value = stoppedState;
		targetValue = AnimationValue::makeState();
		targetValue->value = stoppedState;
	};
	static std::shared_ptr<AnimatableState> make(std::string name,
												 std::vector<AnimatableStateStruct*>& allStates_,
												 std::vector<AnimatableStateStruct*>& selectableStates_,
												 AnimatableStateStruct* stoppedState_){
		return std::make_shared<AnimatableState>(name.c_str(), allStates_, selectableStates_, stoppedState_);
	}
	virtual AnimatableType getType() override { return AnimatableType::STATE; }

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
	virtual std::vector<std::string>& getCurveNames() override;
	virtual void fillControlPointDefaults(std::shared_ptr<Motion::ControlPoint> controlpoint) override;

	virtual bool isReadyToMove() override;
	virtual bool isReadyToStartPlaybackFromValue(std::shared_ptr<AnimationValue> animationValue) override;
	virtual bool isInRapid() override;
	virtual float getRapidProgress() override;
	virtual void cancelRapid() override;
	virtual std::shared_ptr<AnimationValue> getRapidTarget() override;
	
	virtual void onRapidToValue(std::shared_ptr<AnimationValue> animationValue) override;
	virtual void onPlaybackStart(std::shared_ptr<Animation> animation) override;
	virtual void onPlaybackPause() override;
	virtual void onPlaybackStop() override;
	virtual void onPlaybackEnd() override;
	virtual void stopMovement() override;
	
	virtual bool hasManualControls() override { return true; }
	virtual void onSetManualControlTarget(float x, float y, float z) override {}
	
		
	virtual std::shared_ptr<AnimationValue> getTargetValue() override;
	std::shared_ptr<AnimatableStateValue> targetValue;
	
	virtual void updateActualValue(std::shared_ptr<AnimationValue> newActualValue) override;
	virtual std::shared_ptr<AnimationValue> getActualValue() override;
	std::shared_ptr<AnimatableStateValue> actualValue;
	
	virtual void updateTargetValue(double time_seconds, double deltaTime_seconds) override;
	virtual void followActualValue(double time_seconds, double deltaTime_seconds) override;
	
	
	
	bool b_inRapid = false;
	
	std::mutex mutex;
	
	
	virtual bool generateTargetAnimation(std::shared_ptr<TargetAnimation> animation) override;
	
	virtual bool validateAnimation(std::shared_ptr<Animation> animation) override {
		return true;
	}
	
	std::vector<AnimatableStateStruct*>* getStates() { return allStates; }
	
private:
	std::vector<AnimatableStateStruct*>* allStates;
	std::vector<AnimatableStateStruct*>* selectableStates;
	AnimatableStateStruct* stoppedState;
	
	
public:
	
	virtual std::string getTargetValueString() override{
		static char targetstring[128];
		sprintf(targetstring, "%s", getTargetValue()->toState()->value->displayName);
		return std::string(targetstring);
	}
};

