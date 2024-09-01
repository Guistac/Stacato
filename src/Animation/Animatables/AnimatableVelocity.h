#pragma once

#include "Animation/Animatable.h"
#include "Animation/AnimationValue.h"
#include "Animation/AnimationConstraint.h"

#include "Motion/Curve/Profile.h"

#include "Project/Editor/Parameter.h"

struct AnimatableVelocityValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::VELOCITY; }
	double velocity;
	double acceleration;
};


class AnimatableVelocity : public AnimatableNumber{
public:
	
	//—————————construction & typeId——————————
	AnimatableVelocity(const char* name, Unit unit) : AnimatableNumber(name, unit) {
		actualValue = AnimationValue::makeVelocity();
		targetValue = AnimationValue::makeVelocity();
	};
	static std::shared_ptr<AnimatableVelocity> make(std::string name, Unit unit){ return std::make_shared<AnimatableVelocity>(name.c_str(), unit); }
	virtual AnimatableType getType() override { return AnimatableType::VELOCITY; }
	
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
	virtual bool isControlledManuallyOrByAnimation() override;
	virtual std::vector<std::string>& getCurveNames() override;
	virtual void fillControlPointDefaults(std::shared_ptr<Motion::ControlPoint> controlpoint) override;
	
	//—————————state——————————
	virtual bool isReadyToMove() override;
	virtual bool isReadyToStartPlaybackFromValue(std::shared_ptr<AnimationValue> animationValue) override;
	virtual bool isInRapid() override;
	virtual float getRapidProgress() override;
	virtual std::shared_ptr<AnimationValue> getRapidTarget() override;
	
	//——————————animation values——————————
	
	virtual void updateTargetValue(double time_seconds, double deltaTime_seconds) override;
	virtual std::shared_ptr<AnimationValue> getTargetValue() override;
	std::shared_ptr<AnimatableVelocityValue> targetValue;
	
	virtual void simulateTargetValue(double time_seconds, double deltaTime_seconds);
	
	virtual void updateActualValue(std::shared_ptr<AnimationValue> newActualValue) override;
	virtual std::shared_ptr<AnimationValue> getActualValue() override;
	std::shared_ptr<AnimatableVelocityValue> actualValue;
	
	virtual void followActualValue(double time_seconds, double deltaTime_seconds) override;
		
	//—————————movement commands——————————
	virtual void onRapidToValue(std::shared_ptr<AnimationValue> animationValue) override;
	virtual void cancelRapid() override;
	virtual void onPlaybackStart(std::shared_ptr<Animation> animation) override;
	virtual void onPlaybackPause() override;
	virtual void onPlaybackStop() override;
	virtual void onPlaybackEnd() override;
	virtual void stopMovement() override { setVelocityTarget(0.0); }
	
	//—————————manual controls——————————
	virtual bool hasManualControls() override { return true; }
	virtual void onSetManualControlTarget(float x, float y, float z) override;
	
	virtual bool isMoving() override;
	
public:
	void setManualVelocityTarget(double velocityTarget);
	void forceVelocityTarget(double velocity, double acceleration);
	
	void overrideVelocityTarget(double positionTarget);
	
private:
	void setVelocityTarget(double velocityTarget);
	void copyMotionProfilerValueToTargetValue();
	
public:
	
	bool hasVelocitySetpoint();
	double getVelocitySetpoint();
	double getVelocitySetpointNormalized();
	
	double normalizeVelocity(double velocity);
	
	double getActualVelocity();
	double getActualVelocityNormalized();
	
	double getActualAcceleration();
	double getActualAccelerationNormalized();
	
	Motion::Profile motionProfile;
	double profileTime_seconds;
	
	double velocityLimit;
	double accelerationLimit;
	
	enum ControlMode{
		VELOCITY_SETPOINT,	//manual velocity control
		FORCED_VELOCITY_SETPOINT	//forced control ???? what mean
	}controlMode = ControlMode::VELOCITY_SETPOINT;
	double velocitySetpoint = 0.0;
	double accelerationSetpoint = 0.0;
	
	double velocitySliderDisplayValue = 0.0;
	
	void manualControlsVerticalGui(float sliderHeight, const char* customName = nullptr, bool invert = false);
	
	virtual std::string getTargetValueString() override{
		static char targetstring[128];
		getTargetValue()->toVelocity();
		//snprintf(targetstring, 128, "%.3f%s/s", getTargetValue()->toVelocity()->velocity, getUnit()->abbreviated);
		return std::string(targetstring);
	}
	
	virtual void settingsGui() override;
	virtual bool save(tinyxml2::XMLElement*) override;
	virtual bool load(tinyxml2::XMLElement*) override;
	
	enum class ManualControlAxis{
		X = 0,
		Y = 1,
		Z = 2
	};
	OptionParameter::Option controlChannelAxisX_option = OptionParameter::Option(int(ManualControlAxis::X), "X Axis", "X");
	OptionParameter::Option controlChannelAxisY_option = OptionParameter::Option(int(ManualControlAxis::Y), "Y Axis", "Y");
	OptionParameter::Option controlChannelAxisZ_option = OptionParameter::Option(int(ManualControlAxis::Z), "Z Axis", "Z");
	std::vector<OptionParameter::Option*> controlChannelAxis_options = {
		&controlChannelAxisX_option,
		&controlChannelAxisY_option,
		&controlChannelAxisZ_option
	};
	OptionParam controlChannelAxis_parameter = OptionParameter::make2(controlChannelAxisX_option, controlChannelAxis_options, "Manual Control Axis", "ManualControlChannelAxis");
	BoolParam controlChannelInvert_parameter = BooleanParameter::make(false, "Invert Manual Control", "InvertManualControlAxis");
	
};


//public: lock mutex
//private: require locked mutex
//do not call public functions internally
