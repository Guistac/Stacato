#pragma once

class CurvePoint;
class Manoeuvre;

#include "Machine/AnimatableParameter.h"

#include <tinyxml2.h>

#include "Project/Editor/Parameter.h"


static std::shared_ptr<Parameter> getParameterFromAnimatableParameter(std::shared_ptr<AnimatableParameter> machineParameter){
	switch(machineParameter->getType()){
		case MachineParameterType::BOOLEAN_PARAMETER: 	return std::make_shared<BooleanParameter>(false, "DefaultName", "DefaultSaveString");
		case MachineParameterType::INTEGER_PARAMETER: 	return std::make_shared<NumberParameter<int>>(0, "DefaultName", "DefaultSaveString");
		case MachineParameterType::STATE_PARAMETER:		return nullptr;
			
		case MachineParameterType::POSITION:
		case MachineParameterType::REAL_PARAMETER:		return std::make_shared<NumberParameter<double>>(0.0, "DefaultName", "DefaultSaveString");
			
		case MachineParameterType::POSITION_2D:
		case MachineParameterType::VECTOR_2D_PARAMETER:	return std::make_shared<VectorParameter<glm::vec2>>(glm::vec2(0.0), "DefaultName", "DefaultSaveString");
			
		case MachineParameterType::POSITION_3D:
		case MachineParameterType::VECTOR_3D_PARAMETER:	return std::make_shared<VectorParameter<glm::vec3>>(glm::vec3(0.0), "DefaultName", "DefaultSaveString");
			
		case MachineParameterType::PARAMETER_GROUP:		return nullptr;
	}
}









class ParameterTrackGroup;
class MovementParameterTrack;

//---------------------------
//	Base parameter track
//---------------------------

class ParameterTrack : public std::enable_shared_from_this<ParameterTrack>{
public:
	
	ParameterTrack(std::shared_ptr<MachineParameter> parameter_) : parameter(parameter_){}
	
	std::shared_ptr<MachineParameter> getParameter(){ return parameter; }
	
	virtual ParameterTrackType getType() = 0;
	virtual bool save(tinyxml2::XMLElement* trackXML) = 0;
	virtual bool load(tinyxml2::XMLElement* trackXML) = 0;
	
	bool hasParentGroup(){ return parent != nullptr; }
	
	void baseTrackSheetRowGui();
	virtual void trackSheetRowGui() = 0;
	
	static std::shared_ptr<ParameterTrackGroup> castToGroup(std::shared_ptr<ParameterTrack> input){
		return std::dynamic_pointer_cast<ParameterTrackGroup>(input);
	}
	
	static std::shared_ptr<MovementParameterTrack> castToMovementTrack(std::shared_ptr<ParameterTrack> input){
		return std::dynamic_pointer_cast<MovementParameterTrack>(input);
	}
	
	static std::shared_ptr<ParameterTrack> create(std::shared_ptr<MachineParameter> parameter, ManoeuvreType manoeuvreType);
	
private:
	
	std::shared_ptr<ParameterTrackGroup> parent;
	std::shared_ptr<Manoeuvre> parentManoeuvre;
	std::shared_ptr<MachineParameter> parameter;
};








//---------------------------
//	parameter group track
//---------------------------

class ParameterTrackGroup : public ParameterTrack{
public:
	
	ParameterTrackGroup(std::shared_ptr<ParameterGroup> parameterGroup, ManoeuvreType manoeuvreType) : ParameterTrack(parameterGroup){
		for(auto& childParameter : parameterGroup->getChildren()){
			auto childParameterTrack = ParameterTrack::create(childParameter, manoeuvreType);
			children.push_back(childParameterTrack);
		}
	}
	
	std::vector<std::shared_ptr<ParameterTrack>>& getChildren(){ return children; }
	
	virtual ParameterTrackType getType(){ return ParameterTrackType::GROUP; }
	
	virtual void trackSheetRowGui();
	
	virtual bool save(tinyxml2::XMLElement* trackXML){}
	virtual bool load(tinyxml2::XMLElement* trackXML){}
	
private:
	std::vector<std::shared_ptr<ParameterTrack>> children;
};








//---------------------------
//	for all tracks that can generate movement (not groups)
//---------------------------

class MovementParameterTrack : public ParameterTrack{
public:
	
	MovementParameterTrack(std::shared_ptr<MachineParameter> parameter) : ParameterTrack(parameter){}
	
	void rapidToEnd();
	void cancelRapid();
	
	float getRapidProgress();
	bool isPrimedToEnd(); //is parameter at the same position as the target
	
	void captureCurrentValueAsTarget();
	
	bool isValid(){ return b_valid; }
	
	std::shared_ptr<Parameter> target;
	
private:
	bool b_valid = false;
	bool b_priming = false;
};


class KeyParameterTrack : public MovementParameterTrack{
public:
	
	KeyParameterTrack(std::shared_ptr<MachineParameter> parameter) : MovementParameterTrack(parameter){
		auto animatableParameter = MachineParameter::castToAnimatable(parameter);
		target = getParameterFromAnimatableParameter(animatableParameter);
		target->setName("Target");
		target->setSaveString("Target");
	}
	
	virtual ParameterTrackType getType(){ return ParameterTrackType::KEY; }
	
	virtual void trackSheetRowGui();
	
	virtual bool save(tinyxml2::XMLElement* trackXML){}
	virtual bool load(tinyxml2::XMLElement* trackXML){}
	
};










//---------------------------
//	for all tracks that can play a movement sequence (not keys, not groups)
//---------------------------

class AnimatedParameterTrack : public MovementParameterTrack{
public:
	
	AnimatedParameterTrack(std::shared_ptr<MachineParameter> parameter) : MovementParameterTrack(parameter){}
	
	AnimatableParameterValue getParameterValueAtPlaybackTime();
	void setInterpolationType(Motion::InterpolationType t);
	virtual ParameterTrackType getType() = 0;
	
private:
	double playbackPosition_seconds;
	Motion::InterpolationType interpolationType;
	std::vector<std::shared_ptr<Motion::Curve>> curves;
};


class TargetParameterTrack : public AnimatedParameterTrack{
public:
	
	enum class Constraint{
		VELOCITY,
		TIME
	};
	
	TargetParameterTrack(std::shared_ptr<MachineParameter> parameter) : AnimatedParameterTrack(parameter){
		auto animatableParameter = MachineParameter::castToAnimatable(parameter);
		target = getParameterFromAnimatableParameter(animatableParameter);
		type = ParameterTrackType::FIXED_TIME;
	}
	
	virtual ParameterTrackType getType(){ return type; }
	
	void setTimeConstraintType(){ type = ParameterTrackType::FIXED_TIME; }
	void setVelocityConstraintType(){ type = ParameterTrackType::FIXED_VELOCITY; }
	
	virtual void trackSheetRowGui();
		
	virtual bool save(tinyxml2::XMLElement* trackXML){}
	virtual bool load(tinyxml2::XMLElement* trackXML){}
	
private:
	ParameterTrackType type;
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



class SequenceParameterTrack : public AnimatedParameterTrack{
public:
	
	SequenceParameterTrack(std::shared_ptr<MachineParameter> parameter) : AnimatedParameterTrack(parameter){
		auto animatableParameter = MachineParameter::castToAnimatable(parameter);
		target = getParameterFromAnimatableParameter(animatableParameter);
		target->setName("End");
		target->setSaveString("End");
		start = getParameterFromAnimatableParameter(animatableParameter);
		start->setName("Start");
		start->setSaveString("Start");
	}
	
	virtual ParameterTrackType getType(){ return ParameterTrackType::ANIMATED_SEQUENCE; }
	
	virtual void trackSheetRowGui();
	
	double getLength_seconds();
	
	void rapidToStart();
	void rapidToPlaybackPosition();
	
	bool isPrimedToStart(); //is axis at the same location as the playback position
	bool isPrimedToPlaybackPosition();
	
	virtual bool save(tinyxml2::XMLElement* trackXML){}
	virtual bool load(tinyxml2::XMLElement* trackXML){}
	
	std::shared_ptr<Parameter> start;
	
private:
	
	
		
};




inline std::shared_ptr<ParameterTrack> ParameterTrack::create(std::shared_ptr<MachineParameter> parameter, ManoeuvreType manoeuvreType){
	
	if(parameter->getType() == MachineParameterType::PARAMETER_GROUP){
		auto parameterGroup = MachineParameter::castToGroup(parameter);
		return std::make_shared<ParameterTrackGroup>(parameterGroup, manoeuvreType);
	}
	 else{
		switch(manoeuvreType){
			case ManoeuvreType::KEY:
				return std::make_shared<KeyParameterTrack>(parameter);
			case ManoeuvreType::TARGET:
				return std::make_shared<TargetParameterTrack>(parameter);
			case ManoeuvreType::SEQUENCE:
				return std::make_shared<SequenceParameterTrack>(parameter);
		}
	}
}
