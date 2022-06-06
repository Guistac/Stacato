#pragma once

#include <tinyxml2.h>

#include "Animation/Animatable.h"
#include "Project/Editor/Parameter.h"
#include "Motion/Curve/Curve.h"

class AnimationComposite;
class AnimationKey;
class TargetAnimation;
class SequenceAnimation;
class Manoeuvre;

class Animation : public std::enable_shared_from_this<Animation>{
public:
	
	//———————— Construction, Saving & Loading —————————

public:
	
	static std::shared_ptr<Animation> create(std::shared_ptr<Animatable> animatable, ManoeuvreType manoeuvreType);
	static std::shared_ptr<Animation> load(tinyxml2::XMLElement* trackXML);
	static std::shared_ptr<Animation> load(tinyxml2::XMLElement* trackXML, std::shared_ptr<Animatable> animatable);
	std::shared_ptr<Animation> copy();
	
	bool save(tinyxml2::XMLElement* trackXML);
	virtual bool onSave(tinyxml2::XMLElement* trackXML) = 0;

protected:
	
	Animation(std::shared_ptr<Animatable> animatable_) {
		animatable = animatable_;
		curves.resize(animatable->getCurveCount());
	}
	
	//————————————————— General Properties ——————————————————
	
public:

	std::shared_ptr<Animatable> getAnimatable(){ return animatable; }
	std::shared_ptr<Machine> getMachine(){ return animatable->getMachine(); }
	
	bool hasManoeuvre(){ return manoeuvre != nullptr; }
	void setManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre_){ manoeuvre = manoeuvre_; }
	std::shared_ptr<Manoeuvre> getManoeuvre(){ return manoeuvre; }
	
	bool hasParentComposite(){ return parentComposite != nullptr; }
	void setParentComposite(std::shared_ptr<AnimationComposite> parentComposite_){ parentComposite = parentComposite_; }
	std::shared_ptr<AnimationComposite> getParentComposite(){ return parentComposite; }
	
	void subscribeToMachineParameter();
	void unsubscribeFromMachineParameter();
	
	void fillDefaults();
	
	void validate();
	bool isValid(){ return b_valid; }
	void setValid(bool valid){ b_valid = valid; }
	void appendValidationErrorString(std::string errorString){
		if(!validationErrorString.empty()) validationErrorString += "\n";
		validationErrorString += errorString;
	}
	
	virtual void setUnit(Unit unit){}

	std::vector<Motion::Curve>& getCurves(){ return curves; }
	
private:
	
	std::shared_ptr<AnimationComposite> parentComposite;
	std::shared_ptr<Manoeuvre> manoeuvre;
	std::shared_ptr<Animatable> animatable;
	std::vector<Motion::Curve> curves;
	bool b_valid = false;
	std::string validationErrorString = "";
	
	//———————————— Sub Class Identification & Casting ————————————————
	
public:
	
	virtual bool isComposite(){ return false; }
	std::shared_ptr<AnimationComposite> toComposite(){ return std::dynamic_pointer_cast<AnimationComposite>(shared_from_this()); }
	virtual ManoeuvreType getType() = 0;
	std::shared_ptr<AnimationKey> toKey(){ return std::dynamic_pointer_cast<AnimationKey>(shared_from_this()); }
	std::shared_ptr<TargetAnimation> toTarget(){ return std::dynamic_pointer_cast<TargetAnimation>(shared_from_this()); }
	std::shared_ptr<SequenceAnimation> toSequence(){ return std::dynamic_pointer_cast<SequenceAnimation>(shared_from_this()); }
	
	
	//————————————————————— Playback ——————————————————————
	
public:
	
	bool isMachineEnabled();
	
	virtual bool isAtStart(){ return false; }
	virtual void rapidToStart(){ return; }
	
	virtual bool isAtTarget(){ return false; }
	virtual void rapidToTarget(){ return; }
	
	virtual bool isAtPlaybackPosition(){ return false; }
	virtual void rapidToPlaybackPosition(){ return; }
	
	virtual bool isInRapid();
	virtual float getRapidProgress();
	
	virtual bool isReadyToStartPlayback(){ return false; }
	virtual bool isPlaying();
	
	virtual void startPlayback();
	virtual void interruptPlayback();
	virtual void endPlayback();
	virtual void stop();
	
	void updatePlaybackStatus();
	
	void setDuration(double seconds){ duration_seconds = seconds; }
	void setPlaybackPosition(double seconds){ playbackPosition_seconds = seconds; }
	double getPlaybackPosition(){ return playbackPosition_seconds; }
	double getDuration(){ return duration_seconds; }
	virtual void getCurvePositionRange(double& min, double& max){}
	std::shared_ptr<AnimationValue> getValueAtPlaybackTime();
	
private:
	
	double playbackPosition_seconds;
	double duration_seconds;
	
	//—————————————————— User Interface ———————————————————
	
public:
	
	void baseTrackSheetRowGui();
	virtual void trackSheetRowGui() = 0;
	void validationErrorPopup();
	
	virtual void drawCurves();
	virtual void drawCurveControls() = 0;
	static bool beginTrackSheetTable(ManoeuvreType type, ImGuiTableFlags tableFlags);
	
};
















//--------------------------------------------
//				ANIMATION KEY
//--------------------------------------------

class AnimationKey : public Animation{
	
	//———————— Construction, Saving Loading and Type Identification —————————

public:
	
	AnimationKey(std::shared_ptr<Animatable> animatable);
	
	virtual ManoeuvreType getType() override { return ManoeuvreType::KEY; }
	
	virtual bool onSave(tinyxml2::XMLElement* trackXML) override;
	static std::shared_ptr<AnimationKey> load(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable);
	std::shared_ptr<AnimationKey> copy();
	
	//————————————————— General Properties ——————————————————
	
public:
		
	virtual void setUnit(Unit unit) override;
	
	void captureTarget(){}
	std::shared_ptr<Parameter> target;
	
	virtual void getCurvePositionRange(double& min, double& max) override;
	
	//————————————————————— Playback ——————————————————————
	
public:
	
	virtual bool isAtPlaybackPosition() override;
	virtual void rapidToPlaybackPosition() override;

	//—————————————————— User Interface ———————————————————
	
	virtual void trackSheetRowGui() override;
	virtual void drawCurves() override {}
	virtual void drawCurveControls() override;
	static bool beginTrackSheetTable(ImGuiTableFlags tableFlags);
};















//--------------------------------------------
//				TARGET ANIMATION
//--------------------------------------------

class TargetAnimation : public Animation{
	
	//———————— Construction, Saving Loading and Type Identification —————————
	
public:
	
	TargetAnimation(std::shared_ptr<Animatable> animatable);
	virtual ManoeuvreType getType() override { return ManoeuvreType::TARGET; }
	
	virtual bool onSave(tinyxml2::XMLElement* trackXML) override;
	static std::shared_ptr<TargetAnimation> load(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable);
	std::shared_ptr<TargetAnimation> copy();
	
	//————————————————— General Properties ——————————————————
	
public:
	
	enum class Constraint{
		VELOCITY,
		TIME
	};
	Constraint getConstraintType(){ return constraintType->value; }
	
	virtual void setUnit(Unit unit) override;
	
	virtual void getCurvePositionRange(double& min, double& max) override;
	
	bool areCurvesGenerated();
	void clearCurves();
	
	void captureTarget(){}
	std::shared_ptr<Parameter> target;
	std::shared_ptr<EnumeratorParameter<Constraint>> constraintType = std::make_shared<EnumeratorParameter<Constraint>>(Constraint::TIME, "Constraint Type", "ConstraintType");
	std::shared_ptr<TimeParameter> timeConstraint = std::make_shared<TimeParameter>(0.0, "Movement Time", "Time");
	std::shared_ptr<NumberParameter<double>> velocityConstraint = NumberParameter<double>::make(0.0, "Movement Velocity", "Velocity");
	std::shared_ptr<EnumeratorParameter<Motion::Interpolation::Type>> interpolationType;
	std::shared_ptr<NumberParameter<double>> inAcceleration = NumberParameter<double>::make(0.0, "Start Acceleration", "StartAcceleration");
	std::shared_ptr<NumberParameter<double>> outAcceleration = NumberParameter<double>::make(0.0, "End Acceleration", "EndAcceleration");
	
	//————————————————————— Playback ——————————————————————
	
public:
	
	virtual bool isAtTarget() override;
	virtual void rapidToTarget() override;
	
	virtual bool isReadyToStartPlayback() override;
	virtual void startPlayback() override;
	virtual void endPlayback() override;
	virtual void stop() override;
	
	//—————————————————— User Interface ———————————————————
	
	virtual void trackSheetRowGui() override;
	virtual void drawCurveControls() override;
	static bool beginTrackSheetTable(ImGuiTableFlags tableFlags);
};

#define TargetConstraintStrings \
{TargetAnimation::Constraint::VELOCITY, 	.displayString = "Velocity", .saveString = "Velocity"},\
{TargetAnimation::Constraint::TIME, 		.displayString = "Time", 	.saveString = "Time"},\

DEFINE_ENUMERATOR(TargetAnimation::Constraint, TargetConstraintStrings)













//--------------------------------------------
//			SEQUENCE PARAMETER TRACK
//--------------------------------------------

class SequenceAnimation : public Animation{
public:
	
	//————————— Construction, Saving Loading and Type Identification ——————————
	
public:
	
	SequenceAnimation(std::shared_ptr<Animatable> animatable);
	virtual ManoeuvreType getType() override { return ManoeuvreType::SEQUENCE; }
	
	virtual bool onSave(tinyxml2::XMLElement* trackXML) override;
	static std::shared_ptr<SequenceAnimation> load(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable);
	std::shared_ptr<SequenceAnimation> copy();
	
	//————————————————— General Properties ——————————————————
	
public:
	
	virtual void setUnit(Unit unit) override;
	
	void initializeCurves();
	void updateAfterParameterEdit();
	void updateAfterCurveEdit();
	
	bool isSimple();
	bool isComplex();
	
	virtual void getCurvePositionRange(double& min, double& max) override;
	
	void captureStart();
	void captureTarget();
	std::shared_ptr<Parameter> start;
	std::shared_ptr<Parameter> target;
	std::shared_ptr<TimeParameter> duration = std::make_shared<TimeParameter>(0, "Duration", "Duration");
	std::shared_ptr<EnumeratorParameter<Motion::Interpolation::Type>> interpolationType;
	std::shared_ptr<TimeParameter> timeOffset = std::make_shared<TimeParameter>(0.0, "Time Offset", "TimeOffset");
	std::shared_ptr<NumberParameter<double>> inAcceleration = NumberParameter<double>::make(0.0, "Start Acceleration", "StartAcceleration");
	std::shared_ptr<NumberParameter<double>> outAcceleration = NumberParameter<double>::make(0.0, "End Acceleration", "EndAcceleration");
	
	
	//————————————————————— Playback —————————————————————
	
public:
		
	virtual bool isAtStart() override;
	virtual void rapidToStart() override;
	
	virtual bool isAtTarget() override;
	virtual void rapidToTarget() override;
	
	virtual bool isAtPlaybackPosition() override;
	virtual void rapidToPlaybackPosition() override;
	virtual bool isReadyToStartPlayback() override;
	
	//—————————————————— User Interface ———————————————————
	
public:
	
	virtual void trackSheetRowGui() override;
	virtual void drawCurveControls() override;
	static bool beginTrackSheetTable(ImGuiTableFlags tableFlags);
};














//————————————————————————————————————————————
//			PARAMETER TRACK GROUP
//————————————————————————————————————————————

class AnimationComposite : public Animation{
public:
	
	//called when creating a parameter track group in the track sheet editor
	AnimationComposite(std::shared_ptr<AnimatableComposite> animatableComposite, ManoeuvreType manoeuvreType) : Animation(animatableComposite){
		for(auto& childAnimatable : animatableComposite->getChildren()){
			auto childAnimation = Animation::create(childAnimatable, manoeuvreType);
			children.push_back(childAnimation);
		}
	}
	
	//called when loading a parameter track group or duplicating one
	AnimationComposite(std::shared_ptr<AnimatableComposite> animatableComposite) : Animation(animatableComposite){}
	
	virtual bool isComposite() override { return true; }
	virtual ManoeuvreType getType() override { return ManoeuvreType::KEY;} //this should never be used }
	
	std::vector<std::shared_ptr<Animation>>& getChildren(){ return children; }
	
	virtual void trackSheetRowGui() override {}
	
	virtual bool onSave(tinyxml2::XMLElement* trackXML) override;
	static std::shared_ptr<AnimationComposite> load(tinyxml2::XMLElement* xml, std::shared_ptr<AnimatableComposite> animatableComposite);
	std::shared_ptr<AnimationComposite> copy();
	
	virtual void drawCurves() override{
		for(auto& childTrack : children) childTrack->drawCurves();
	}
	virtual void drawCurveControls() override {
		for(auto& childTrack : children) childTrack->drawCurveControls();
	}
	
private:
	std::vector<std::shared_ptr<Animation>> children;
};
