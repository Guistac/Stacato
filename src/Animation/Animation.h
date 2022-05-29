#pragma once

class CurvePoint;
class Manoeuvre;

#include "Animation/Animatable.h"

#include <tinyxml2.h>

#include "Project/Editor/Parameter.h"

#include "Motion/Curve/Curve.h"

class AnimationComposite;
class AnimationKey;
class TargetAnimation;
class SequenceAnimation;
class Manoeuvre;

class Animation : public std::enable_shared_from_this<Animation>{
public:
	
	//———————————————————————————————————————————
	//		Construction, Saving & Loading
	//———————————————————————————————————————————

public:
	
	static std::shared_ptr<Animation> create(std::shared_ptr<Animatable> animatable, ManoeuvreType manoeuvreType);
	static std::shared_ptr<Animation> copy(std::shared_ptr<Animation> original);
	static std::shared_ptr<Animation> load(tinyxml2::XMLElement* trackXML);
	static std::shared_ptr<Animation> loadType(tinyxml2::XMLElement* trackXML, std::shared_ptr<Animatable> animatable);
	
	bool save(tinyxml2::XMLElement* trackXML);
	virtual bool onSave(tinyxml2::XMLElement* trackXML) = 0;
	
	//———————————————————————————————————————————
	//				General Properties
	//———————————————————————————————————————————
	
public:
	
	Animation(std::shared_ptr<Animatable> animatable_) {
		animatable = animatable_;
		curves.resize(animatable->getCurveCount());
	}
	std::shared_ptr<Animatable> getAnimatable(){ return animatable; }
	
	void setManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre_){ manoeuvre = manoeuvre_; }
	bool hasManoeuvre(){ return manoeuvre != nullptr; }
	std::shared_ptr<Manoeuvre> getManoeuvre(){ return manoeuvre; }
	
	bool hasParentComposite(){ return parentComposite != nullptr; }
	void setParentComposite(std::shared_ptr<AnimationComposite> parentComposite_){ parentComposite = parentComposite_; }
	std::shared_ptr<AnimationComposite> getParentComposite(){ return parentComposite; }
	
	bool isValid(){ return b_valid; }
	void setValid(bool valid){ b_valid = valid; }
	
	void subscribeToMachineParameter();
	void unsubscribeFromMachineParameter();
	
	void validate();
	void appendValidationErrorString(std::string errorString){
		if(!validationErrorString.empty()) validationErrorString += "\n";
		validationErrorString += errorString;
	}
	
	virtual void setUnit(Unit unit){}
	void setDuration(double seconds){ duration_seconds = seconds; }

private:
	
	virtual ManoeuvreType getType() = 0;
	std::shared_ptr<AnimationComposite> parentComposite;
	std::shared_ptr<Manoeuvre> manoeuvre;
	std::shared_ptr<Animatable> animatable;
	bool b_valid = false;
	std::string validationErrorString = "";
	
	//———————————————————————————————————————————
	//	   SubClass Identification & Casting
	//———————————————————————————————————————————
	
public:
	
	virtual bool isComposite(){ return false; }
	std::shared_ptr<AnimationComposite> toComposite(){ return std::dynamic_pointer_cast<AnimationComposite>(shared_from_this()); }
	std::shared_ptr<AnimationKey> toKey(){ return std::dynamic_pointer_cast<AnimationKey>(shared_from_this()); }
	std::shared_ptr<TargetAnimation> toTarget(){ return std::dynamic_pointer_cast<TargetAnimation>(shared_from_this()); }
	std::shared_ptr<SequenceAnimation> toSequence(){ return std::dynamic_pointer_cast<SequenceAnimation>(shared_from_this()); }
	
	
	//———————————————————————————————————————————
	//	   				Playback
	//———————————————————————————————————————————
	
	bool isMachineEnabled();
	virtual bool isAtStart(){}
	virtual bool isAtTarget();
	virtual bool isAtPlaybackPosition(){ return false; }
	virtual bool isReadyToStartPlayback(){ return false; }
	virtual bool isInRapid();
	virtual float getRapidProgress();
	virtual double getDuration(){ return duration_seconds; }
	
	virtual void rapidToStart(){}
	virtual void rapidToTarget();
	virtual void rapidToPlaybackPosition(){}
	virtual void startPlayback();
	virtual void pausePlayback(){}
	virtual void setPlaybackPosition(double seconds){ playbackPosition_seconds = seconds; }
	
	virtual void stop();
	virtual void interrupt();
	
	virtual void updatePlaybackStatus();
	std::shared_ptr<AnimationValue> getValueAtPlaybackTime();
	std::vector<Motion::Curve>& getCurves(){ return curves; }
	
	//called by parameter to interrupt manoeuvre
	bool isPlaying();
	
	//———————————————————————————————————————————
	//	   			User Interface
	//———————————————————————————————————————————
	
public:
	
	void baseTrackSheetRowGui();
	virtual void trackSheetRowGui() = 0;
	void validationErrorPopup();
	
	virtual void drawCurves() = 0;
	virtual void drawCurveControls() = 0;
	
	
private:
	double playbackPosition_seconds;
	double duration_seconds;
	std::vector<Motion::Curve> curves;
	
};

















































//--------------------------------------------
//				KEY PARAMETER TRACK
//--------------------------------------------

class AnimationKey : public Animation{
	
	//——————————————————————————————————————————————————————
	// Construction, Saving Loading and Type Identification
	//——————————————————————————————————————————————————————

public:
	
	AnimationKey(std::shared_ptr<Animatable> animatable) : Animation(animatable){
		target = animatable->makeParameter();
		target->setName("Target");
		target->setSaveString("Target");
		if(animatable->isNumber()) setUnit(animatable->toNumber()->getUnit());
		target->setEditCallback([this](std::shared_ptr<Parameter> thisParameter){ this->validate(); });
	}
	
	virtual ManoeuvreType getType() override { return ManoeuvreType::KEY; }
	
	virtual bool onSave(tinyxml2::XMLElement* trackXML) override;
	static std::shared_ptr<AnimationKey> load(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable);
	std::shared_ptr<AnimationKey> copy();
	
	//———————————————————————————————————————————
	//	  		  General Properties
	//———————————————————————————————————————————
	
	virtual void setUnit(Unit unit) override {
		if(getAnimatable()->isNumber()){
			target->castToNumber()->setUnit(unit);
		}
	}
	
	//———————————————————————————————————————————
	//	  		    User Interface
	//———————————————————————————————————————————
	
	virtual void trackSheetRowGui() override;
	virtual void drawCurves() override {}
	virtual void drawCurveControls() override;
	
	void captureCurrentValueAsTarget(){}
	std::shared_ptr<Parameter> target;
	
};

//rapid to end















//--------------------------------------------
//			TARGET PARAMETER TRACK
//--------------------------------------------

class TargetAnimation : public Animation{
	
	//——————————————————————————————————————————————————————
	// Construction, Saving Loading and Type Identification
	//——————————————————————————————————————————————————————
	
public:
	
	TargetAnimation(std::shared_ptr<Animatable> animatable) : Animation(animatable){
		target = animatable->makeParameter();
		target->setSaveString("Target");
		target->setName("Target");
		velocityConstraint->denyNegatives();
		inAcceleration->denyNegatives();
		outAcceleration->denyNegatives();
		if(animatable->isNumber()){
			velocityConstraint->setPrefix("Velocity: ");
			velocityConstraint->setSuffix("/s");
			inAcceleration->setPrefix("In: ");
			inAcceleration->setSuffix("/s\xC2\xB2");
			outAcceleration->setPrefix("Out: ");
			outAcceleration->setSuffix("/s\xC2\xB2");
			Unit unit = animatable->toNumber()->getUnit();
			setUnit(unit);
		}
		if(!animatable->isNumber() || !animatable->toNumber()->isReal()){
			constraintType->setDisabled(true);
			timeConstraint->setDisabled(true);
			velocityConstraint->setDisabled(true);
			inAcceleration->setDisabled(true);
			outAcceleration->setDisabled(true);
		}
		
		Motion::Interpolation::Type defaultInterpolation = animatable->getCompatibleInterpolationTypes().front();
		interpolationType = std::make_shared<EnumeratorParameter<Motion::Interpolation::Type>>(defaultInterpolation, "Interpolation Type", "interpolationType");
		
		auto editCallback = [this](std::shared_ptr<Parameter> thisParameter){ validate(); };
		target->setEditCallback(editCallback);
		interpolationType->setEditCallback(editCallback);
		velocityConstraint->setEditCallback(editCallback);
		inAcceleration->setEditCallback(editCallback);
		outAcceleration->setEditCallback(editCallback);
		constraintType->setEditCallback(editCallback);
		timeConstraint->setEditCallback(editCallback);
	}
	
	
	virtual ManoeuvreType getType() override { return ManoeuvreType::TARGET; }
	
	virtual bool onSave(tinyxml2::XMLElement* trackXML) override;
	static std::shared_ptr<TargetAnimation> load(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable);
	std::shared_ptr<TargetAnimation> copy();
	
	//———————————————————————————————————————————
	//	  		  General Properties
	//———————————————————————————————————————————
	
public:
	
	enum class Constraint{
		VELOCITY,
		TIME
	};
	
	Constraint getConstraintType(){ return constraintType->value; }
	
	virtual void setUnit(Unit unit) override {
		if(getAnimatable()->isNumber()){
			target->castToNumber()->setUnit(unit);
			velocityConstraint->setUnit(unit);
			inAcceleration->setUnit(unit);
			outAcceleration->setUnit(unit);
		}
	}
	
	
	virtual bool isAtPlaybackPosition() override;
	virtual bool isReadyToStartPlayback() override;
	virtual void startPlayback() override;

	std::shared_ptr<Parameter> target;
	std::shared_ptr<EnumeratorParameter<Constraint>> constraintType = std::make_shared<EnumeratorParameter<Constraint>>(Constraint::TIME, "Constraint Type", "ConstraintType");
	std::shared_ptr<TimeParameter> timeConstraint = std::make_shared<TimeParameter>(0.0, "Movement Time", "Time");
	std::shared_ptr<NumberParameter<double>> velocityConstraint = NumberParameter<double>::make(0.0, "Movement Velocity", "Velocity");
	std::shared_ptr<EnumeratorParameter<Motion::Interpolation::Type>> interpolationType;
	std::shared_ptr<TimeParameter> timeOffset = std::make_shared<TimeParameter>(0.0, "Time Offset", "TimeOffset");
	std::shared_ptr<NumberParameter<double>> inAcceleration = NumberParameter<double>::make(0.0, "Start Acceleration", "StartAcceleration");
	std::shared_ptr<NumberParameter<double>> outAcceleration = NumberParameter<double>::make(0.0, "End Acceleration", "EndAcceleration");
	
	//———————————————————————————————————————————
	//	  		    User Interface
	//———————————————————————————————————————————
	
	virtual void trackSheetRowGui() override;
	virtual void drawCurves() override;
	virtual void drawCurveControls() override;
	
	//void captureCurrentValueAsTarget();
	//bool b_accelerationsEqual;
};

#define TargetConstraintStrings \
{TargetAnimation::Constraint::VELOCITY, 	.displayString = "Velocity", .saveString = "Velocity"},\
{TargetAnimation::Constraint::TIME, 		.displayString = "Time", 	.saveString = "Time"},\

DEFINE_ENUMERATOR(TargetAnimation::Constraint, TargetConstraintStrings)


//rapid to end
//start
//pause
//resume













//--------------------------------------------
//			SEQUENCE PARAMETER TRACK
//--------------------------------------------

class SequenceAnimation : public Animation{
public:
	
	//——————————————————————————————————————————————————————
	// Construction, Saving Loading and Type Identification
	//——————————————————————————————————————————————————————
	
public:
	
	SequenceAnimation(std::shared_ptr<Animatable> animatable) : Animation(animatable){
		target = animatable->makeParameter();
		target->setName("End");
		target->setSaveString("End");
		start = animatable->makeParameter();
		start->setName("Start");
		start->setSaveString("Start");
		inAcceleration->denyNegatives();
		outAcceleration->denyNegatives();
		if(animatable->isNumber()){
			inAcceleration->setPrefix("In: ");
			inAcceleration->setSuffix("/s\xC2\xB2");
			outAcceleration->setPrefix("Out: ");
			outAcceleration->setSuffix("/s\xC2\xB2");
			Unit unit = animatable->toNumber()->getUnit();
			setUnit(unit);
		}
		
		if(!animatable->isNumber() || !animatable->toNumber()->isReal()){
			inAcceleration->setDisabled(true);
			outAcceleration->setDisabled(true);
		}
		
		Motion::Interpolation::Type defaultInterpolation = animatable->getCompatibleInterpolationTypes().front();
		interpolationType = std::make_shared<EnumeratorParameter<Motion::Interpolation::Type>>(defaultInterpolation, "Interpolation Type", "interpolationType");
		
		auto editCallback = [this](std::shared_ptr<Parameter> thisParameter){ updateAfterParameterEdit(); };
		interpolationType->setEditCallback(editCallback);
		target->setEditCallback(editCallback);
		start->setEditCallback(editCallback);
		duration->setEditCallback(editCallback);
		timeOffset->setEditCallback(editCallback);
		inAcceleration->setEditCallback(editCallback);
		outAcceleration->setEditCallback(editCallback);
		initializeCurves();
	}
	
	void initializeCurves();
	
	virtual ManoeuvreType getType() override { return ManoeuvreType::SEQUENCE; }
	
	virtual bool onSave(tinyxml2::XMLElement* trackXML) override;
	static std::shared_ptr<SequenceAnimation> load(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable);
	std::shared_ptr<SequenceAnimation> copy();
	
	//———————————————————————————————————————————
	//	  		  General Properties
	//———————————————————————————————————————————
	
public:
	
	void updateAfterParameterEdit();
	void updateAfterCurveEdit();
	
	virtual void setUnit(Unit unit) override {
		if(getAnimatable()->isNumber()){
			target->castToNumber()->setUnit(unit);
			start->castToNumber()->setUnit(unit);
		}
	}
	
	virtual void rapidToPlaybackPosition() override;
	
	void captureCurrentValueAsStart(){}
	void captureCurrentValueAsTarget(){}
	
	virtual bool isAtStart() override;
	virtual void rapidToStart() override;
	
	virtual bool isAtPlaybackPosition() override;
	virtual bool isReadyToStartPlayback() override;
	
private:
	
	std::shared_ptr<Parameter> start;
	std::shared_ptr<Parameter> target;
	std::shared_ptr<TimeParameter> duration = std::make_shared<TimeParameter>(0, "Duration", "Duration");
	std::shared_ptr<EnumeratorParameter<Motion::Interpolation::Type>> interpolationType;
	std::shared_ptr<TimeParameter> timeOffset = std::make_shared<TimeParameter>(0.0, "Time Offset", "TimeOffset");
	std::shared_ptr<NumberParameter<double>> inAcceleration = NumberParameter<double>::make(0.0, "Start Acceleration", "StartAcceleration");
	std::shared_ptr<NumberParameter<double>> outAcceleration = NumberParameter<double>::make(0.0, "End Acceleration", "EndAcceleration");
	
	//———————————————————————————————————————————
	//	  		        Playback
	//———————————————————————————————————————————
	
public:
	
	//———————————————————————————————————————————
	//	  		     User Interface
	//———————————————————————————————————————————
	
public:
	
	virtual void trackSheetRowGui() override;
	virtual void drawCurves() override;
	virtual void drawCurveControls() override;
};

//rapid to start
//rapid to end
//rapid to playback position
//start
//pause
//resume














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
