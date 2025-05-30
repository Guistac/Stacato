#pragma once

#include "Animation/Animatable.h"
#include "Animation/AnimationValue.h"
#include "Animation/AnimationConstraint.h"

#include "Motion/Curve/Profile.h"

#include "Project/Editor/Parameter.h"

struct AnimatablePositionValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::POSITION; }
	double position;
	double velocity;
	double acceleration;
};

class AnimatablePosition_KeepoutConstraint : public AnimationConstraint{
public:
	
	AnimatablePosition_KeepoutConstraint(std::string name, double min, double max) : AnimationConstraint(name){
		adjust(min, max);
	}
	
	void adjust(double min, double max){
		if(min > max) {
			keepOutMinPosition = max;
			keepOutMaxPosition = min;
		}else{
			keepOutMinPosition = min;
			keepOutMaxPosition = max;
		}
	}
	
	virtual AnimatableType getAnimatableType() override { return AnimatableType::POSITION; };
	
	virtual Type getType() override { return Type::KEEPOUT; }
	
	double keepOutMinPosition;
	double keepOutMaxPosition;
};


class AnimatablePosition : public AnimatableNumber{
public:
	
	//—————————construction & typeId——————————
	AnimatablePosition(const char* name, Unit unit) : AnimatableNumber(name, unit) {
		actualValue = AnimationValue::makePosition();
		targetValue = AnimationValue::makePosition();
	};
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
	std::shared_ptr<AnimatablePositionValue> targetValue;
	
    virtual void simulateTargetValue(double time_seconds, double deltaTime_seconds);
    
	virtual void updateActualValue(std::shared_ptr<AnimationValue> newActualValue) override;
	virtual std::shared_ptr<AnimationValue> getActualValue() override;
	std::shared_ptr<AnimatablePositionValue> actualValue;
	
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
	void setManualPositionTargetWithVelocity(double targetPosition, double targetVelocity);
	void setManualPositionTargetWithTime(double targetPosition, double targetTime);
	void forcePositionTarget(double position, double velocity, double acceleration);
	void forceVelocityTarget(double velocity, double acceleration);
	double getBrakingPosition();
	
	void overridePositionTarget(double positionTarget);
	
	void setVelocityTarget(double velocityTarget);
	void moveToPositionWithVelocity(double targetPosition, double targetVelocity);
	void moveToPositionInTime(double targetPosition, double targetTime);
	
private:
	void copyMotionProfilerValueToTargetValue();
	
public:
	
	void getConstraintPositionLimits(double& min, double& max);
	
	bool hasPositionSetpoint();
	double getPositionSetpoint();
	double getPositionSetpointNormalized();
	
	bool hasVelocitySetpoint();
	double getVelocitySetpoint();
	double getVelocitySetpointNormalized();
	
	bool hasAccelerationSetpoint();
	double getAccelerationSetpoint();
	double getAccelerationSetpointNormalized();
	
	double getActualPosition();
	double getActualPositionNormalized();
	
	double normalizePosition(double position);
	
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
	
	enum ControlMode{
		VELOCITY_SETPOINT,	//manual velocity control
		POSITION_SETPOINT,	//go to position
		FORCED_POSITION_SETPOINT,
		FORCED_VELOCITY_SETPOINT
	}controlMode = ControlMode::VELOCITY_SETPOINT;
	double velocitySetpoint = 0.0;
	double positionSetpoint = 0.0;
	double accelerationSetpoint = 0.0;
	
	double velocitySliderDisplayValue = 0.0;
	double rapidTargetPositionDisplayValue = 0.0;
	
	void manualControlsVerticalGui(float sliderHeight, const char* customName = nullptr, bool invert = false);
	void manualControlsHorizontalGui(float sliderWidth, const char* customName = nullptr);
	
	
	virtual std::string getTargetValueString() override{
		static char targetstring[128];
		sprintf(targetstring, "%.3f%s", getTargetValue()->toPosition()->position, getUnit()->abbreviated);
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
	BoolParam canHaveMasterAnimatable = BooleanParameter::make(false, "Can have master animatable", "CanHaveMasterAnimatable");

};


//public: lock mutex
//private: require locked mutex
//do not call public functions internally
