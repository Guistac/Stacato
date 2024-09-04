#pragma once

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
	
	Animation(std::shared_ptr<Animatable> animatable_) : animatable(animatable_){}
	
	//———————————— Sub Class Identification & Casting ————————————————
	
public:
	
	virtual bool isComposite(){ return false; }
	std::shared_ptr<AnimationComposite> toComposite(){ return std::static_pointer_cast<AnimationComposite>(shared_from_this()); }
	virtual ManoeuvreType getType() = 0;
	std::shared_ptr<AnimationKey> toKey(){ return std::static_pointer_cast<AnimationKey>(shared_from_this()); }
	std::shared_ptr<TargetAnimation> toTarget(){ return std::static_pointer_cast<TargetAnimation>(shared_from_this()); }
	std::shared_ptr<SequenceAnimation> toSequence(){ return std::static_pointer_cast<SequenceAnimation>(shared_from_this()); }
	
	
	//————————————————— General Properties ——————————————————
	
public:

	std::shared_ptr<Animatable> getAnimatable(){ return animatable; }
	std::string getFullName();
	
	bool hasManoeuvre(){ return manoeuvre != nullptr; }
	std::shared_ptr<Manoeuvre> getManoeuvre(){ return manoeuvre; }
	void setManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre_){ manoeuvre = manoeuvre_; }
	
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

	std::vector<std::shared_ptr<Motion::Curve>>& getCurves(){ return curves; }
	virtual void getCurvePositionRange(double& min, double& max){}

	std::shared_ptr<AnimationValue> getValueAtPlaybackTime();
	
private:
	
	std::shared_ptr<AnimationComposite> parentComposite;
	std::shared_ptr<Manoeuvre> manoeuvre;
	std::shared_ptr<Animatable> animatable;
	std::vector<std::shared_ptr<Motion::Curve>> curves;
	bool b_valid = false;
	std::string validationErrorString = "";
	
	
	//————————————— Playback Control ————————————
	
public:
	
	bool isMachineEnabled();
	
	virtual bool canStartPlayback(){ return false; }
	virtual bool canPausePlayback(){ return false; }
	bool canRapid();
	bool canStop(){ return isActive(); }
	
	float getRapidProgress();
	float getPlaybackProgress();
	
	virtual bool isAtStart(){ return false; }
	virtual bool isAtTarget(){ return false; }
	virtual bool isAtPlaybackPosition(){ return false; }

	virtual bool canRapidToStart(){ return false; }
	virtual bool canRapidToTarget(){ return false; }
	virtual bool canRapidToPlaybackPosition(){ return false; }

	void rapidToStart();
	void rapidToTarget();
	void rapidToPlaybackPosition();
	void startPlayback();
	void pausePlayback();
	void stopPlayback();
	void stopRapid();
	void stop();
	
private:
	
	virtual bool onRapidToStart(){ return false; }
	virtual bool onRapidToTarget(){ return false; }
	virtual bool onRapidToPlaybackPosition(){ return false; }
	virtual bool onStartPlayback(){ return true; }
	
	//——————— Playback Update ———————
public:
	
	void updateDuration();
	void updatePlaybackState();
	void incrementPlaybackPosition(double timeIncrement_seconds);
	
	bool isInRapid(){ return b_isInRapid; }
	bool isPlaying(){ return b_isPlaying; }
	bool isPaused(){ return b_isPaused; }
	bool isActive(){ return b_isPlaying || b_isInRapid || b_isPaused; }
		
	void setDuration(double seconds){ duration_seconds = seconds; }
	void setPlaybackPosition(double seconds){ playbackPosition_seconds = seconds; }
	
	double getPlaybackPosition(){ return playbackPosition_seconds; }
	double getDuration(){ return duration_seconds; }
	
private:
	
	long long playbackStartTime_microseconds;
	double playbackPosition_seconds = 0.0;
	double duration_seconds = 0.0;
	bool b_isPlaying = false; 	//indicates active playback
	bool b_isPaused = false;	//indicates inactive playback but prevent removal from playback manager
	bool b_isInRapid = false;	//indicates active rapid movement
	
	
	//————————————— Curve Editing ——————————————
	
public:
	virtual void addCurvePoint(std::shared_ptr<Motion::Curve> curve, float time, float position) {}
	
	//—————————————————— User Interface ———————————————————
	
public:
	
	virtual void playbackGui() = 0;
	void baseTrackSheetRowGui();
	virtual void trackSheetRowGui() = 0;
	void validationErrorPopup();
	
	virtual void drawCurves();
	virtual void drawCurveControls() = 0;
	static bool beginTrackSheetTable(ManoeuvreType type, ImGuiTableFlags tableFlags);
	
	void requestCurveRefocus(){
		b_shouldRefocusCurves = true;
	}
	bool shouldRefocusCurves(){
		if(b_shouldRefocusCurves){
			b_shouldRefocusCurves = false;
			return true;
		}
		return false;
	}
	
	bool b_isLoop = false;
	
private:
	
	bool b_shouldRefocusCurves = false;
	
private:
	
public:
	std::shared_ptr<Animation> masterAnimation = nullptr;
	
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
	
	void captureTarget(){
        auto animatable = getAnimatable();
        animatable->setParameterValueFromAnimationValue(target, animatable->getTargetValue());
	}
	std::shared_ptr<Parameter> target;
	
	virtual void getCurvePositionRange(double& min, double& max) override;
	
	//————————————————————— Playback ——————————————————————
	
public:
	
	virtual bool isAtPlaybackPosition() override;
	virtual bool canRapidToPlaybackPosition() override;
	
private:
	
	virtual bool onRapidToPlaybackPosition() override;
	
	//—————————————————— User Interface ———————————————————
public:
	
	virtual void playbackGui() override;
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
	
	void captureTarget(){
        auto animatable = getAnimatable();
        animatable->setParameterValueFromAnimationValue(target, animatable->getTargetValue());
	}
	std::shared_ptr<Parameter> target;
	std::shared_ptr<EnumeratorParameter<Constraint>> constraintType = std::make_shared<EnumeratorParameter<Constraint>>(Constraint::TIME, "Constraint Type", "ConstraintType");
	std::shared_ptr<TimeParameter> timeConstraint = std::make_shared<TimeParameter>(0.0, "Movement Time", "Time");
	std::shared_ptr<NumberParameter<double>> velocityConstraint = NumberParameter<double>::make(0.0, "Movement Velocity", "Velocity");
	std::shared_ptr<EnumeratorParameter<InterpolationType>> interpolationType;
	std::shared_ptr<NumberParameter<double>> inAcceleration = NumberParameter<double>::make(0.0, "Start Acceleration", "StartAcceleration");
	std::shared_ptr<NumberParameter<double>> outAcceleration = NumberParameter<double>::make(0.0, "End Acceleration", "EndAcceleration");
	
	//————————————————————— Playback ——————————————————————
	
public:
	
	virtual bool canRapidToTarget() override;
	virtual bool isAtTarget() override;
	virtual bool canStartPlayback() override;
	
private:
	
	virtual bool onRapidToTarget() override;
	virtual bool onStartPlayback() override;
	
	//—————————————————— User Interface ———————————————————
	
public:
	
	virtual void playbackGui() override;
	virtual void trackSheetRowGui() override;
	virtual void drawCurveControls() override;
	static bool beginTrackSheetTable(ImGuiTableFlags tableFlags);
};


#define TargetConstraintStrings \
{TargetAnimation::Constraint::VELOCITY, 	.displayString = "Velocity", .saveString = "Velocity"},\
{TargetAnimation::Constraint::TIME, 		.displayString = "Time", 	.saveString = "Time"},\

DEFINE_ENUMERATOR(TargetAnimation::Constraint, TargetConstraintStrings)













//--------------------------------------------
//				SEQUENCE ANIMATION
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
	std::shared_ptr<EnumeratorParameter<InterpolationType>> interpolationType;
	std::shared_ptr<TimeParameter> timeOffset = std::make_shared<TimeParameter>(0.0, "Time Offset", "TimeOffset");
	std::shared_ptr<NumberParameter<double>> inAcceleration = NumberParameter<double>::make(0.0, "Start Acceleration", "StartAcceleration");
	std::shared_ptr<NumberParameter<double>> outAcceleration = NumberParameter<double>::make(0.0, "End Acceleration", "EndAcceleration");
	
	void updateTheoreticalShortestDuration();
	double theoreticalShortestDuration = 0.0;
	
public:
	virtual void addCurvePoint(std::shared_ptr<Motion::Curve> curve, float time, float position) override;
	
	void changeGlobalSpeed(double factor);
	void subtractTime(double from, double amount);
	
	//————————————————————— Playback —————————————————————
	
public:
	
	virtual bool canRapidToStart() override;
	virtual bool canRapidToTarget() override;
	virtual bool canRapidToPlaybackPosition() override;
	
	virtual bool canStartPlayback() override;
	virtual bool canPausePlayback() override;
	
	virtual bool isAtStart() override;
	virtual bool isAtTarget() override;
	virtual bool isAtPlaybackPosition() override;
	
private:
	
	virtual bool onRapidToStart() override;
	virtual bool onRapidToTarget() override;
	virtual bool onRapidToPlaybackPosition() override;
	
	//—————————————————— User Interface ———————————————————
	
public:
	
	virtual void playbackGui() override;
	virtual void trackSheetRowGui() override;
	virtual void drawCurveControls() override;
	static bool beginTrackSheetTable(ImGuiTableFlags tableFlags);
	
};














//————————————————————————————————————————————
//				COMPOSITE ANIMATION
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
	virtual bool onSave(tinyxml2::XMLElement* trackXML) override;
	
	static std::shared_ptr<AnimationComposite> load(tinyxml2::XMLElement* xml, std::shared_ptr<AnimatableComposite> animatableComposite);
	std::shared_ptr<AnimationComposite> copy();
	
	//—————————————— General —————————————
	
	std::vector<std::shared_ptr<Animation>>& getChildren(){ return children; }
	
	virtual void trackSheetRowGui() override {}
	
	virtual void drawCurves() override{
		for(auto& childTrack : children) childTrack->drawCurves();
	}
	virtual void drawCurveControls() override {
		for(auto& childTrack : children) childTrack->drawCurveControls();
	}
	
	virtual void playbackGui() override {}
	
private:
	std::vector<std::shared_ptr<Animation>> children;
};
