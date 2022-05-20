#pragma once

class CurvePoint;
class Manoeuvre;

#include "Machine/AnimatableParameter.h"

#include <tinyxml2.h>

#include "Project/Editor/Parameter.h"

static std::shared_ptr<Parameter> getParameterFromAnimatableParameter(std::shared_ptr<AnimatableParameter> machineParameter){
	switch(machineParameter->getType()){
		case MachineParameterType::BOOLEAN: 	return std::make_shared<BooleanParameter>(false, "DefaultName", "DefaultSaveString");
		case MachineParameterType::INTEGER: 	return std::make_shared<NumberParameter<int>>(0, "DefaultName", "DefaultSaveString");
		case MachineParameterType::STATE:		return std::make_shared<StateParameter>(&MachineParameter::toState(machineParameter)->getStates().front(),
																						&MachineParameter::toState(machineParameter)->getStates(),
																						"DefaultName",
																						"DefaultSaveString");
		case MachineParameterType::POSITION:
		case MachineParameterType::VELOCITY:
		case MachineParameterType::REAL:		return std::make_shared<NumberParameter<double>>(0.0, "DefaultName", "DefaultSaveString");
			
		case MachineParameterType::POSITION_2D:
		case MachineParameterType::VELOCITY_2D:
		case MachineParameterType::VECTOR_2D:	return std::make_shared<VectorParameter<glm::vec2>>(glm::vec2(0.0), "DefaultName", "DefaultSaveString");
			
		case MachineParameterType::POSITION_3D:
		case MachineParameterType::VELOCITY_3D:
		case MachineParameterType::VECTOR_3D:	return std::make_shared<VectorParameter<glm::vec3>>(glm::vec3(0.0), "DefaultName", "DefaultSaveString");
			
		case MachineParameterType::GROUP:		return nullptr;
	}
}


class ParameterTrackGroup;
class MovementParameterTrack;


class ParameterTrack : public std::enable_shared_from_this<ParameterTrack>{
public:
	
	enum class Type{
		GROUP,
		KEY,
		TARGET,
		SEQUENCE
	};
	
	ParameterTrack(std::shared_ptr<MachineParameter> parameter_) : parameter(parameter_){}
	
	std::shared_ptr<MachineParameter> getParameter(){ return parameter; }
	
	virtual Type getType() = 0;
		
	static std::shared_ptr<ParameterTrack> load(tinyxml2::XMLElement* trackXML);
	bool save(tinyxml2::XMLElement* trackXML);
	virtual bool onSave(tinyxml2::XMLElement* trackXML) = 0;
	static std::shared_ptr<ParameterTrack> loadType(tinyxml2::XMLElement* trackXML, std::shared_ptr<MachineParameter> parameter);
	
	bool hasParentGroup(){ return parent != nullptr; }
	virtual bool isGroup() = 0;
	void setParent(std::shared_ptr<ParameterTrackGroup> parent_){ parent = parent_; }
	
	void baseTrackSheetRowGui();
	virtual void trackSheetRowGui() = 0;
	
	bool isValid(){ return b_valid; }
	void setVald(bool valid){ b_valid = valid; }
	
	static std::shared_ptr<ParameterTrackGroup> castToGroup(std::shared_ptr<ParameterTrack> input){
		return std::dynamic_pointer_cast<ParameterTrackGroup>(input);
	}
	
	static std::shared_ptr<MovementParameterTrack> castToMovementTrack(std::shared_ptr<ParameterTrack> input){
		return std::dynamic_pointer_cast<MovementParameterTrack>(input);
	}
	
	static std::shared_ptr<ParameterTrack> create(std::shared_ptr<MachineParameter> parameter, ManoeuvreType manoeuvreType);
	
private:
	bool b_valid = false;
	std::shared_ptr<ParameterTrackGroup> parent;
	std::shared_ptr<Manoeuvre> parentManoeuvre;
	std::shared_ptr<MachineParameter> parameter;
};

#define ParameterTrackTypeStrings \
{ParameterTrack::Type::GROUP, 		.displayString = "Group", 		.saveString = "Group"},\
{ParameterTrack::Type::KEY, 		.displayString = "Key", 		.saveString = "Key"},\
{ParameterTrack::Type::TARGET, 		.displayString = "Target", 		.saveString = "Target"},\
{ParameterTrack::Type::SEQUENCE, 	.displayString = "Sequence", 	.saveString = "Sequence"},\

DEFINE_ENUMERATOR(ParameterTrack::Type, ParameterTrackTypeStrings)


//Interface for all tracks that can generate movement (not groups)
class MovementParameterTrack : public ParameterTrack{
public:
	
	MovementParameterTrack(std::shared_ptr<MachineParameter> parameter) : ParameterTrack(parameter){}
	
	void rapidToEnd();
	void cancelRapid();
	
	float getRapidProgress();
	bool isPrimedToEnd(); //is parameter at the same position as the target
	
	void captureCurrentValueAsTarget();
	
	std::shared_ptr<Parameter> target;
	
	virtual bool isGroup(){ return false; }
	
private:
	bool b_priming = false;
};


//Interface for all tracks that can play a movement sequence (not keys, not groups)
class AnimatedParameterTrack : public MovementParameterTrack{
public:
	
	AnimatedParameterTrack(std::shared_ptr<MachineParameter> parameter) : MovementParameterTrack(parameter){}
	
	AnimatableParameterValue getParameterValueAtPlaybackTime();
	void setInterpolationType(Motion::Interpolation::Type t);
	
private:
	double playbackPosition_seconds;
	Motion::Interpolation::Type interpolationType;
	std::vector<std::shared_ptr<Motion::Curve>> curves;
};








//————————————————————————————————————————————
//			PARAMETER TRACK GROUP
//————————————————————————————————————————————

class ParameterTrackGroup : public ParameterTrack{
public:
	
	virtual Type getType() override { return ParameterTrack::Type::GROUP; }
	
	ParameterTrackGroup(std::shared_ptr<ParameterGroup> parameterGroup, ManoeuvreType manoeuvreType) : ParameterTrack(parameterGroup){
		for(auto& childParameter : parameterGroup->getChildren()){
			auto childParameterTrack = ParameterTrack::create(childParameter, manoeuvreType);
			children.push_back(childParameterTrack);
		}
	}
	
	ParameterTrackGroup(std::shared_ptr<ParameterGroup> parameterGroup) : ParameterTrack(parameterGroup){}
	
	std::vector<std::shared_ptr<ParameterTrack>>& getChildren(){ return children; }
	
	virtual void trackSheetRowGui() override;
	
	virtual bool isGroup() override { return true; }
	
	virtual bool onSave(tinyxml2::XMLElement* trackXML) override;
	static std::shared_ptr<ParameterTrackGroup> load(tinyxml2::XMLElement* xml, std::shared_ptr<ParameterGroup> parameter);
	
private:
	std::vector<std::shared_ptr<ParameterTrack>> children;
};





//————————————————————————————————————————————
//				KEY PARAMETER TRACK
//————————————————————————————————————————————

class KeyParameterTrack : public MovementParameterTrack{
public:
	
	virtual Type getType() override { return ParameterTrack::Type::KEY; }
	
	KeyParameterTrack(std::shared_ptr<MachineParameter> parameter) : MovementParameterTrack(parameter){
		auto animatableParameter = MachineParameter::castToAnimatable(parameter);
		target = getParameterFromAnimatableParameter(animatableParameter);
		target->setName("Target");
		target->setSaveString("Target");
	}
	
	virtual void trackSheetRowGui() override;
	
	virtual bool onSave(tinyxml2::XMLElement* trackXML) override;
	static std::shared_ptr<KeyParameterTrack> load(tinyxml2::XMLElement* xml, std::shared_ptr<MachineParameter> parameter);
	
};

//rapid to end




//————————————————————————————————————————————
//			TARGET PARAMETER TRACK
//————————————————————————————————————————————

class TargetParameterTrack : public AnimatedParameterTrack{
public:
	
	virtual Type getType() override { return ParameterTrack::Type::TARGET; }
	
	enum class Constraint{
		VELOCITY,
		TIME
	};
	
	TargetParameterTrack(std::shared_ptr<MachineParameter> parameter) : AnimatedParameterTrack(parameter){
		auto animatableParameter = MachineParameter::castToAnimatable(parameter);
		target = getParameterFromAnimatableParameter(animatableParameter);
	}
	
	virtual void trackSheetRowGui() override;
		
	virtual bool onSave(tinyxml2::XMLElement* trackXML) override;
	static std::shared_ptr<TargetParameterTrack> load(tinyxml2::XMLElement* xml, std::shared_ptr<MachineParameter> parameter);
	
private:
	std::shared_ptr<Parameter> constraint = std::make_shared<NumberParameter<double>>(0.0, "Constraint", "Constraint");
	std::shared_ptr<Parameter> inAcceleration = std::make_shared<NumberParameter<double>>(0.0, "Start Acceleration", "StartAcceleration");
	std::shared_ptr<Parameter> outAcceleration = std::make_shared<NumberParameter<double>>(0.0, "End Acceleration", "EndAcceleration");
	std::shared_ptr<Parameter> timeOffset = std::make_shared<NumberParameter<double>>(0.0, "Time Offset", "TimeOffset");
	std::shared_ptr<Parameter> constraintType = std::make_shared<EnumeratorParameter<Constraint>>(Constraint::TIME, "Constraint Type", "ConstraintType");
	bool b_accelerationsEqual;
};

#define TargetConstraintStrings \
{TargetParameterTrack::Constraint::VELOCITY, 	.displayString = "Velocity", .saveString = "Velocity"},\
{TargetParameterTrack::Constraint::TIME, 		.displayString = "Time", 	.saveString = "Time"},\

DEFINE_ENUMERATOR(TargetParameterTrack::Constraint, TargetConstraintStrings)


//rapid to end
//start
//pause
//resume




//————————————————————————————————————————————
//			SEQUENCE PARAMETER TRACK
//————————————————————————————————————————————

class SequenceParameterTrack : public AnimatedParameterTrack{
public:
	
	virtual Type getType() override { return ParameterTrack::Type::SEQUENCE; }
	
	SequenceParameterTrack(std::shared_ptr<MachineParameter> parameter) : AnimatedParameterTrack(parameter){
		auto animatableParameter = MachineParameter::castToAnimatable(parameter);
		target = getParameterFromAnimatableParameter(animatableParameter);
		target->setName("End");
		target->setSaveString("End");
		start = getParameterFromAnimatableParameter(animatableParameter);
		start->setName("Start");
		start->setSaveString("Start");
	}
	
	virtual void trackSheetRowGui() override;
	
	double getLength_seconds();
	
	void rapidToStart();
	void rapidToPlaybackPosition();
	
	bool isPrimedToStart(); //is axis at the same location as the playback position
	bool isPrimedToPlaybackPosition();
	
	virtual bool onSave(tinyxml2::XMLElement* trackXML) override;
	static std::shared_ptr<SequenceParameterTrack> load(tinyxml2::XMLElement* xml, std::shared_ptr<MachineParameter> parameter);
	
	std::shared_ptr<Parameter> start;
};

//rapid to start
//rapid to end
//rapid to playback position
//start
//pause
//resume
