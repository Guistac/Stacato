#pragma once

class CurvePoint;
class Manoeuvre;

#include "Machine/AnimatableParameter.h"

#include <tinyxml2.h>

#include "Project/Editor/Parameter.h"

#include "Motion/Curve/Curve.h"

class ParameterTrackGroup;
class AnimatedParameterTrack;
class KeyParameterTrack;
class TargetParameterTrack;
class SequenceParameterTrack;

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
	
	bool hasParentGroup(){ return parent != nullptr; }
	void setParent(std::shared_ptr<ParameterTrackGroup> parent_){ parent = parent_; }
	std::shared_ptr<ParameterTrackGroup> getParent(){ return parent; }
	
	bool isValid(){ return b_valid; }
	void setValid(bool valid){ b_valid = valid; }
	
	void subscribeToMachineParameter();
	void unsubscribeFromMachineParameter();
	
	void validate();
	void appendValidationErrorString(std::string errorString){
		if(!validationErrorString.empty()) validationErrorString += "\n",
		validationErrorString += errorString;
	}

private:
	
	std::shared_ptr<ParameterTrackGroup> parent;
	std::shared_ptr<Manoeuvre> parentManoeuvre;
	std::shared_ptr<MachineParameter> parameter;
	bool b_valid = false;
	std::string validationErrorString = "";
	
	//———————————————————————————————————————————
	//	   SubClass Identification & Casting
	//———————————————————————————————————————————
	
public:
	
	enum class Type{
		GROUP,
		KEY,
		TARGET,
		SEQUENCE
	};
	
	virtual Type getType() = 0;
	
	virtual bool isGroup(){ return false; }
	std::shared_ptr<ParameterTrackGroup> castToGroup(){ return std::dynamic_pointer_cast<ParameterTrackGroup>(shared_from_this()); }
	
	virtual bool isAnimated(){ return false; }
	std::shared_ptr<AnimatedParameterTrack> castToAnimated(){ return std::dynamic_pointer_cast<AnimatedParameterTrack>(shared_from_this()); }
	
	std::shared_ptr<KeyParameterTrack> castToKey(){ return std::dynamic_pointer_cast<KeyParameterTrack>(shared_from_this()); }
	std::shared_ptr<TargetParameterTrack> castToTarget(){ return std::dynamic_pointer_cast<TargetParameterTrack>(shared_from_this()); }
	std::shared_ptr<SequenceParameterTrack> castToSequence(){ return std::dynamic_pointer_cast<SequenceParameterTrack>(shared_from_this()); }
	
	//———————————————————————————————————————————
	//	   			User Interface
	//———————————————————————————————————————————
	
public:
	
	void baseTrackSheetRowGui();
	virtual void trackSheetRowGui() = 0;
};


#define ParameterTrackTypeStrings \
{ParameterTrack::Type::GROUP, 		.displayString = "Group", 		.saveString = "Group"},\
{ParameterTrack::Type::KEY, 		.displayString = "Key", 		.saveString = "Key"},\
{ParameterTrack::Type::TARGET, 		.displayString = "Target", 		.saveString = "Target"},\
{ParameterTrack::Type::SEQUENCE, 	.displayString = "Sequence", 	.saveString = "Sequence"},\

DEFINE_ENUMERATOR(ParameterTrack::Type, ParameterTrackTypeStrings)



//Interface for all tracks that can generate movement (not groups)
class AnimatedParameterTrack : public ParameterTrack{
public:
	
	//———————————————————————————————————————————
	//	  Construction and Type Identification
	//———————————————————————————————————————————
	
public:
	
	AnimatedParameterTrack(std::shared_ptr<AnimatableParameter> parameter) : ParameterTrack(parameter){
		animatableParameter = parameter;
		Motion::Interpolation::Type defaultInterpolation = getAnimatableParameter()->getCompatibleInterpolationTypes().front();
		interpolationType = std::make_shared<EnumeratorParameter<Motion::Interpolation::Type>>(defaultInterpolation, "Interpolation", "Interpolation");
		interpolationType->setEditCallback([this](std::shared_ptr<Parameter> thisParameter){ validate(); });
		curves.resize(animatableParameter->getCurveCount());
	}
	
	virtual bool isAnimated(){ return true; }
	
	//———————————————————————————————————————————
	//	  		  General Properties
	//———————————————————————————————————————————

public:
	
	std::shared_ptr<AnimatableParameter> getAnimatableParameter(){ return animatableParameter; }
	std::vector<Motion::Curve>& getCurves(){ return curves; }
	
	virtual void setUnit(Unit unit) = 0;
	void setInterpolationType(Motion::Interpolation::Type t);
	void captureCurrentValueAsTarget();
	
	void refreshAfterCurveEdit(){}
	
	std::shared_ptr<Parameter> target;
	std::shared_ptr<EnumeratorParameter<Motion::Interpolation::Type>> interpolationType;
	
private:
	
	std::shared_ptr<AnimatableParameter> animatableParameter;
	
	std::vector<Motion::Curve> curves;
	
	//———————————————————————————————————————————
	//	  		       Playback
	//———————————————————————————————————————————
	
public:
	
	void rapidToEnd();
	void cancelRapid();
	
	float getRapidProgress();
	bool isPrimedToEnd(); //is parameter at the same position as the target
	
	AnimatableParameterValue getParameterValueAtPlaybackTime();
	
private:
	
	bool b_priming = false;
	double playbackPosition_seconds;
	
	//———————————————————————————————————————————
	//	  		    User Interface
	//———————————————————————————————————————————
	
public:
	
	void interpolationTypeGui();
	
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
		target->setEditCallback([this](std::shared_ptr<Parameter> thisParameter){ validate(); });
	}
	
	virtual Type getType() override { return ParameterTrack::Type::KEY; }
	
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
	
};

//rapid to end















//--------------------------------------------
//			TARGET PARAMETER TRACK
//--------------------------------------------

class TargetParameterTrack : public AnimatedParameterTrack{
	
	//——————————————————————————————————————————————————————
	// Construction, Saving Loading and Type Identification
	//——————————————————————————————————————————————————————
	
public:
	
	TargetParameterTrack(std::shared_ptr<AnimatableParameter> parameter) : AnimatedParameterTrack(parameter){
		target = parameter->getEditableParameter();
		target->setSaveString("Target");
		target->setName("Target");
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
		target->setEditCallback([this](std::shared_ptr<Parameter> thisParameter){ validate(); });
		velocityConstraint->setEditCallback([this](std::shared_ptr<Parameter> thisParameter){ validate(); });
		velocityConstraint->denyNegatives();
		inAcceleration->setEditCallback([this](std::shared_ptr<Parameter> thisParameter){ validate(); });
		inAcceleration->denyNegatives();
		outAcceleration->setEditCallback([this](std::shared_ptr<Parameter> thisParameter){ validate(); });
		outAcceleration->denyNegatives();
		constraintType->setEditCallback([this](std::shared_ptr<Parameter> thisParameter){ validate(); });
		timeConstraint->setEditCallback([this](std::shared_ptr<Parameter> thisParameter){ validate(); });
	}
	
	virtual Type getType() override { return ParameterTrack::Type::TARGET; }
	
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
		
	std::shared_ptr<TimeParameter> timeOffset = std::make_shared<TimeParameter>(0.0, "Time Offset", "TimeOffset");
	std::shared_ptr<EnumeratorParameter<Constraint>> constraintType = std::make_shared<EnumeratorParameter<Constraint>>(Constraint::TIME, "Constraint Type", "ConstraintType");
	std::shared_ptr<TimeParameter> timeConstraint = std::make_shared<TimeParameter>(0.0, "Movement Time", "Time");
	std::shared_ptr<BaseNumberParameter> velocityConstraint = NumberParameter<double>::make(0.0, "Movement Velocity", "Velocity");
	std::shared_ptr<BaseNumberParameter> inAcceleration = NumberParameter<double>::make(0.0, "Start Acceleration", "StartAcceleration");
	std::shared_ptr<BaseNumberParameter> outAcceleration = NumberParameter<double>::make(0.0, "End Acceleration", "EndAcceleration");
	bool b_accelerationsEqual;
	
	//———————————————————————————————————————————
	//	  		    User Interface
	//———————————————————————————————————————————
	
	virtual void trackSheetRowGui() override;
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

class SequenceParameterTrack : public AnimatedParameterTrack{
public:
	
	//——————————————————————————————————————————————————————
	// Construction, Saving Loading and Type Identification
	//——————————————————————————————————————————————————————
	
public:
	
	SequenceParameterTrack(std::shared_ptr<AnimatableParameter> parameter) : AnimatedParameterTrack(parameter){
		target = parameter->getEditableParameter();
		target->setName("End");
		target->setSaveString("End");
		start = parameter->getEditableParameter();
		start->setName("Start");
		start->setSaveString("Start");
		if(parameter->isNumerical()){
			Unit unit = parameter->castToNumerical()->getUnit();
			setUnit(unit);
		}
		target->setEditCallback([this](std::shared_ptr<Parameter> thisParameter){ validate(); });
		start->setEditCallback([this](std::shared_ptr<Parameter> thisParameter){ validate(); });
		duration->setEditCallback([this](std::shared_ptr<Parameter> thisParameter){ validate(); });
		timeOffset->setEditCallback([this](std::shared_ptr<Parameter> thisParameter){ validate(); });
	}
	
	virtual Type getType() override { return ParameterTrack::Type::SEQUENCE; }
	
	virtual bool onSave(tinyxml2::XMLElement* trackXML) override;
	static std::shared_ptr<SequenceParameterTrack> load(tinyxml2::XMLElement* xml, std::shared_ptr<AnimatableParameter> parameter);
	std::shared_ptr<SequenceParameterTrack> copy();
	
	//———————————————————————————————————————————
	//	  		  General Properties
	//———————————————————————————————————————————
	
public:
	
	virtual void setUnit(Unit unit) override {
		if(getParameter()->isNumerical()){
			target->castToNumber()->setUnit(unit);
			start->castToNumber()->setUnit(unit);
		}
	}
	
	std::shared_ptr<Parameter> start;
	
private:
	
	std::shared_ptr<TimeParameter> duration = std::make_shared<TimeParameter>(0, "Duration", "Duration");
	std::shared_ptr<TimeParameter> timeOffset = std::make_shared<TimeParameter>(0, "Time Offset", "Offset");
	
	//———————————————————————————————————————————
	//	  		        Playback
	//———————————————————————————————————————————
	
public:
	
	double getLength_seconds();
	
	void rapidToStart();
	void rapidToPlaybackPosition();
	
	bool isPrimedToStart(); //is axis at the same location as the playback position
	bool isPrimedToPlaybackPosition();
	
	//———————————————————————————————————————————
	//	  		     User Interface
	//———————————————————————————————————————————
	
public:
	
	virtual void trackSheetRowGui() override;
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
	
	virtual Type getType() override { return ParameterTrack::Type::GROUP; }
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
	
private:
	std::vector<std::shared_ptr<ParameterTrack>> children;
};
