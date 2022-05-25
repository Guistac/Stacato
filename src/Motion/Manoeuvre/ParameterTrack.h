#pragma once

class CurvePoint;
class Manoeuvre;

#include "Machine/AnimatableParameter.h"

#include <tinyxml2.h>

#include "Project/Editor/Parameter.h"

#include "Motion/Curve/Curve.h"

class ParameterTrackGroup;
class AnimatedParameterTrack;
class PlayableParameterTrack;
class KeyParameterTrack;
class TargetParameterTrack;
class SequenceParameterTrack;
class Manoeuvre;

class ParameterTrack : public std::enable_shared_from_this<ParameterTrack>{
public:
	
	//———————————————————————————————————————————
	//		Construction, Saving & Loading
	//———————————————————————————————————————————

public:

	static std::shared_ptr<ParameterTrack> create(std::shared_ptr<MachineParameter> parameter, ManoeuvreType manoeuvreType);
	static std::shared_ptr<ParameterTrack> copy(const std::shared_ptr<ParameterTrack> original);
	
	static std::shared_ptr<ParameterTrack> load(tinyxml2::XMLElement* trackXML);
	bool save(tinyxml2::XMLElement* trackXML);
	virtual bool onSave(tinyxml2::XMLElement* trackXML) = 0;
	static std::shared_ptr<ParameterTrack> loadType(tinyxml2::XMLElement* trackXML, std::shared_ptr<MachineParameter> parameter);
	
	//———————————————————————————————————————————
	//				General Properties
	//———————————————————————————————————————————
	
public:
	
	ParameterTrack(std::shared_ptr<MachineParameter> parameter_) : parameter(parameter_){}
	std::shared_ptr<MachineParameter> getParameter(){ return parameter; }
	
	void setManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre_){ manoeuvre = manoeuvre_; }
	bool hasManoeuvre(){ return manoeuvre != nullptr; }
	std::shared_ptr<Manoeuvre> getManoeuvre(){ return manoeuvre; }
	
	bool hasParentGroup(){ return parent != nullptr; }
	void setParent(std::shared_ptr<ParameterTrackGroup> parent_){ parent = parent_; }
	std::shared_ptr<ParameterTrackGroup> getParent(){ return parent; }
	
	bool isValid(){ return b_valid; }
	void setValid(bool valid){ b_valid = valid; }
	
	void subscribeToMachineParameter();
	void unsubscribeFromMachineParameter();
	
	void validate();
	void appendValidationErrorString(std::string errorString){
		if(!validationErrorString.empty()) validationErrorString += "\n";
		validationErrorString += errorString;
	}

private:
	
	std::shared_ptr<ParameterTrackGroup> parent;
	std::shared_ptr<Manoeuvre> manoeuvre;
	std::shared_ptr<MachineParameter> parameter;
	bool b_valid = false;
	std::string validationErrorString = "";
	
	//———————————————————————————————————————————
	//	   SubClass Identification & Casting
	//———————————————————————————————————————————
	
public:
	
	virtual bool isGroup(){ return false; }
	std::shared_ptr<ParameterTrackGroup> castToGroup(){ return std::dynamic_pointer_cast<ParameterTrackGroup>(shared_from_this()); }
	
	virtual bool isAnimated(){ return false; }
	std::shared_ptr<AnimatedParameterTrack> castToAnimated(){ return std::dynamic_pointer_cast<AnimatedParameterTrack>(shared_from_this()); }
	
	virtual bool isPlayable(){ return false; }
	std::shared_ptr<PlayableParameterTrack> castToPlayable(){ return std::dynamic_pointer_cast<PlayableParameterTrack>(shared_from_this()); }
	
	std::shared_ptr<KeyParameterTrack> castToKey(){ return std::dynamic_pointer_cast<KeyParameterTrack>(shared_from_this()); }
	std::shared_ptr<TargetParameterTrack> castToTarget(){ return std::dynamic_pointer_cast<TargetParameterTrack>(shared_from_this()); }
	std::shared_ptr<SequenceParameterTrack> castToSequence(){ return std::dynamic_pointer_cast<SequenceParameterTrack>(shared_from_this()); }
	
	
	//———————————————————————————————————————————
	//	   				Playback
	//———————————————————————————————————————————
	
	bool isMachineEnabled();
	virtual bool isAtStart(){ return false; }
	virtual bool isAtTarget(){ return false; }
	virtual bool isAtPlaybackPosition(){ return false; }
	virtual bool isReadyToStartPlayback(){ return false; }
	virtual bool isInRapid(){ return false; }
	virtual float getRapidProgress(){ return 0.0; }
	virtual double getDuration(){ return 0.0; }
	
	virtual void rapidToStart(){}
	virtual void rapidToTarget(){}
	virtual void rapidToPlaybackPosition(){}
	virtual void startPlayback();
	virtual void pausePlayback(){}
	virtual void setPlaybackPosition(double seconds){}
	virtual void stop(){}
	
	virtual void updatePlaybackStatus(){}
	
	//called by parameter to interrupt manoeuvre
	void onStopPlayback();
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
};






//Interface for all tracks that can generate movement (not groups)
class AnimatedParameterTrack : public ParameterTrack{
public:
	
	//———————————————————————————————————————————
	//	  Construction and Type Identification
	//———————————————————————————————————————————
	
public:
	
	AnimatedParameterTrack(std::shared_ptr<AnimatableParameter> parameter) : ParameterTrack(parameter){
		animatableParameter = parameter;
	}
	
	virtual bool isAnimated() override { return true; }
	
	virtual ManoeuvreType getType() = 0;
	
	//———————————————————————————————————————————
	//	  		  General Properties
	//———————————————————————————————————————————

public:
	
	std::shared_ptr<AnimatableParameter> getAnimatableParameter(){ return animatableParameter; }
	
	virtual void setUnit(Unit unit) = 0;
	
	void captureCurrentValueAsTarget();
	
	std::shared_ptr<Parameter> target;
	
	virtual bool isAtTarget() override;
	virtual bool isInRapid() override;
	virtual float getRapidProgress() override;
	
	virtual void rapidToTarget() override;
	virtual void stop() override;
	
private:
	
	std::shared_ptr<AnimatableParameter> animatableParameter;
	
	//———————————————————————————————————————————
	//	  		       Playback
	//———————————————————————————————————————————
	
public:
	
	//———————————————————————————————————————————
	//	  		    User Interface
	//———————————————————————————————————————————
	
};











class PlayableParameterTrack : public AnimatedParameterTrack{
public:
	
	PlayableParameterTrack(std::shared_ptr<AnimatableParameter> parameter) : AnimatedParameterTrack(parameter){
		curves.resize(getAnimatableParameter()->getCurveCount());
	}
	
	virtual bool isPlayable() override { return true; }
	
	std::vector<Motion::Curve>& getCurves(){ return curves; }
	
	//curve to animatable parameter conversion
	std::shared_ptr<AnimatableParameterValue> getParameterValueAtPlaybackTime();
	
	std::shared_ptr<EnumeratorParameter<Motion::Interpolation::Type>> interpolationType;
	std::shared_ptr<TimeParameter> timeOffset = std::make_shared<TimeParameter>(0.0, "Time Offset", "TimeOffset");
	std::shared_ptr<NumberParameter<double>> inAcceleration = NumberParameter<double>::make(0.0, "Start Acceleration", "StartAcceleration");
	std::shared_ptr<NumberParameter<double>> outAcceleration = NumberParameter<double>::make(0.0, "End Acceleration", "EndAcceleration");
	bool b_accelerationsEqual;
	
	virtual double getDuration() override { return duration_seconds; }
	double duration_seconds;
	
	virtual void setPlaybackPosition(double seconds) override { playbackPosition_seconds = seconds; }
	
	virtual void updatePlaybackStatus() override;
	
private:
	
	double playbackPosition_seconds;
	
	std::vector<Motion::Curve> curves;
	
};













//--------------------------------------------
//				KEY PARAMETER TRACK
//--------------------------------------------

class KeyParameterTrack : public AnimatedParameterTrack{
	
	//——————————————————————————————————————————————————————
	// Construction, Saving Loading and Type Identification
	//——————————————————————————————————————————————————————

public:
	
	KeyParameterTrack(std::shared_ptr<AnimatableParameter> parameter) : AnimatedParameterTrack(parameter){
		target = parameter->getEditableParameter();
		target->setName("Target");
		target->setSaveString("Target");
		if(parameter->isNumerical()){
			Unit unit = parameter->castToNumerical()->getUnit();
			setUnit(unit);
		}
		target->setEditCallback([this](std::shared_ptr<Parameter> thisParameter){ this->validate(); });
	}
	
	virtual ManoeuvreType getType() override { return ManoeuvreType::KEY; }
	
	virtual bool onSave(tinyxml2::XMLElement* trackXML) override;
	static std::shared_ptr<KeyParameterTrack> load(tinyxml2::XMLElement* xml, std::shared_ptr<AnimatableParameter> parameter);
	std::shared_ptr<KeyParameterTrack> copy();
	
	//———————————————————————————————————————————
	//	  		  General Properties
	//———————————————————————————————————————————
	
	virtual void setUnit(Unit unit) override {
		if(getParameter()->isNumerical()){
			target->castToNumber()->setUnit(unit);
		}
	}
	
	//———————————————————————————————————————————
	//	  		    User Interface
	//———————————————————————————————————————————
	
	virtual void trackSheetRowGui() override;
	virtual void drawCurves() override {}
	virtual void drawCurveControls() override;
	
};

//rapid to end















//--------------------------------------------
//			TARGET PARAMETER TRACK
//--------------------------------------------

class TargetParameterTrack : public PlayableParameterTrack{
	
	//——————————————————————————————————————————————————————
	// Construction, Saving Loading and Type Identification
	//——————————————————————————————————————————————————————
	
public:
	
	TargetParameterTrack(std::shared_ptr<AnimatableParameter> parameter) : PlayableParameterTrack(parameter){
		target = parameter->getEditableParameter();
		target->setSaveString("Target");
		target->setName("Target");
		velocityConstraint->denyNegatives();
		inAcceleration->denyNegatives();
		outAcceleration->denyNegatives();
		if(parameter->isNumerical()){
			velocityConstraint->setPrefix("Velocity: ");
			velocityConstraint->setSuffix("/s");
			inAcceleration->setPrefix("In: ");
			inAcceleration->setSuffix("/s\xC2\xB2");
			outAcceleration->setPrefix("Out: ");
			outAcceleration->setSuffix("/s\xC2\xB2");
			Unit unit = parameter->castToNumerical()->getUnit();
			setUnit(unit);
		}
		if(!parameter->isReal()){
			constraintType->setDisabled(true);
			timeConstraint->setDisabled(true);
			velocityConstraint->setDisabled(true);
			inAcceleration->setDisabled(true);
			outAcceleration->setDisabled(true);
		}
		
		Motion::Interpolation::Type defaultInterpolation = getAnimatableParameter()->getCompatibleInterpolationTypes().front();
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
	static std::shared_ptr<TargetParameterTrack> load(tinyxml2::XMLElement* xml, std::shared_ptr<AnimatableParameter> parameter);
	std::shared_ptr<TargetParameterTrack> copy();
	
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
		if(getParameter()->isNumerical()){
			target->castToNumber()->setUnit(unit);
			velocityConstraint->setUnit(unit);
			inAcceleration->setUnit(unit);
			outAcceleration->setUnit(unit);
		}
	}
	
	
	virtual bool isAtPlaybackPosition() override;
	virtual bool isReadyToStartPlayback() override;
	virtual void startPlayback() override;

	std::shared_ptr<EnumeratorParameter<Constraint>> constraintType = std::make_shared<EnumeratorParameter<Constraint>>(Constraint::TIME, "Constraint Type", "ConstraintType");
	std::shared_ptr<TimeParameter> timeConstraint = std::make_shared<TimeParameter>(0.0, "Movement Time", "Time");
	std::shared_ptr<NumberParameter<double>> velocityConstraint = NumberParameter<double>::make(0.0, "Movement Velocity", "Velocity");
	
	//———————————————————————————————————————————
	//	  		    User Interface
	//———————————————————————————————————————————
	
	virtual void trackSheetRowGui() override;
	virtual void drawCurves() override;
	virtual void drawCurveControls() override;
};

#define TargetConstraintStrings \
{TargetParameterTrack::Constraint::VELOCITY, 	.displayString = "Velocity", .saveString = "Velocity"},\
{TargetParameterTrack::Constraint::TIME, 		.displayString = "Time", 	.saveString = "Time"},\

DEFINE_ENUMERATOR(TargetParameterTrack::Constraint, TargetConstraintStrings)


//rapid to end
//start
//pause
//resume













//--------------------------------------------
//			SEQUENCE PARAMETER TRACK
//--------------------------------------------

class SequenceParameterTrack : public PlayableParameterTrack{
public:
	
	//——————————————————————————————————————————————————————
	// Construction, Saving Loading and Type Identification
	//——————————————————————————————————————————————————————
	
public:
	
	SequenceParameterTrack(std::shared_ptr<AnimatableParameter> parameter) : PlayableParameterTrack(parameter){
		target = parameter->getEditableParameter();
		target->setName("End");
		target->setSaveString("End");
		start = parameter->getEditableParameter();
		start->setName("Start");
		start->setSaveString("Start");
		inAcceleration->denyNegatives();
		outAcceleration->denyNegatives();
		if(parameter->isNumerical()){
			inAcceleration->setPrefix("In: ");
			inAcceleration->setSuffix("/s\xC2\xB2");
			outAcceleration->setPrefix("Out: ");
			outAcceleration->setSuffix("/s\xC2\xB2");
			Unit unit = parameter->castToNumerical()->getUnit();
			setUnit(unit);
		}
		if(!parameter->isReal()){
			inAcceleration->setDisabled(true);
			outAcceleration->setDisabled(true);
		}
		Motion::Interpolation::Type defaultInterpolation = getAnimatableParameter()->getCompatibleInterpolationTypes().front();
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
	static std::shared_ptr<SequenceParameterTrack> load(tinyxml2::XMLElement* xml, std::shared_ptr<AnimatableParameter> parameter);
	std::shared_ptr<SequenceParameterTrack> copy();
	
	//———————————————————————————————————————————
	//	  		  General Properties
	//———————————————————————————————————————————
	
public:
	
	void updateAfterParameterEdit();
	void updateAfterCurveEdit();
	
	virtual void setUnit(Unit unit) override {
		if(getParameter()->isNumerical()){
			target->castToNumber()->setUnit(unit);
			start->castToNumber()->setUnit(unit);
		}
	}
	
	virtual void rapidToPlaybackPosition() override;
	
	std::shared_ptr<Parameter> start;
	
	void captureCurrentValueAsStart();
	
	virtual bool isAtStart() override;
	virtual void rapidToStart() override;
	
	virtual bool isAtPlaybackPosition() override;
	virtual bool isReadyToStartPlayback() override;
	
private:
	
	std::shared_ptr<TimeParameter> duration = std::make_shared<TimeParameter>(0, "Duration", "Duration");
	
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

class ParameterTrackGroup : public ParameterTrack{
public:
	
	virtual bool isGroup() override { return true; }
	
	//called when creating a parameter track group in the track sheet editor
	ParameterTrackGroup(std::shared_ptr<ParameterGroup> parameterGroup, ManoeuvreType manoeuvreType) : ParameterTrack(parameterGroup){
		for(auto& childParameter : parameterGroup->getChildren()){
			auto childParameterTrack = ParameterTrack::create(childParameter, manoeuvreType);
			children.push_back(childParameterTrack);
		}
	}
	
	//called when loading a parameter track group or duplicating one
	ParameterTrackGroup(std::shared_ptr<ParameterGroup> parameterGroup) : ParameterTrack(parameterGroup){}
	
	std::vector<std::shared_ptr<ParameterTrack>>& getChildren(){ return children; }
	
	virtual void trackSheetRowGui() override;
	
	virtual bool onSave(tinyxml2::XMLElement* trackXML) override;
	static std::shared_ptr<ParameterTrackGroup> load(tinyxml2::XMLElement* xml, std::shared_ptr<ParameterGroup> parameter);
	std::shared_ptr<ParameterTrackGroup> copy();
	
	virtual void drawCurves() override{
		for(auto& childTrack : children) childTrack->drawCurves();
	}
	virtual void drawCurveControls() override {
		for(auto& childTrack : children) childTrack->drawCurveControls();
	}
	
private:
	std::vector<std::shared_ptr<ParameterTrack>> children;
};
