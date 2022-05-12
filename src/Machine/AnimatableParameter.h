#pragma once

#include "AnimatableParameterValue.h"
#include "Motion/Curve/Curve.h"

class Machine;
class ParameterTrack;
namespace tinyxml2 { class XMLElement; }


class AnimatableParameter;
class ParameterGroup;
class AnimatableStateParameter;

class MachineParameter : public std::enable_shared_from_this<MachineParameter>{
public:
	MachineParameter(const char* name_){ strcpy(name, name_); }
	
	virtual MachineParameterType getType() = 0;
	bool hasParentGroup() { return parentParameterGroup != nullptr; }
	void setParentGroup(std::shared_ptr<ParameterGroup> parent){ parentParameterGroup = parent; }
	
	const char* getName(){ return name; }
	void setMachine(std::shared_ptr<Machine> machine_){ machine = machine_; }
	std::shared_ptr<Machine> getMachine(){ return machine; }
	
	static std::shared_ptr<ParameterGroup> castToGroup(std::shared_ptr<MachineParameter> input){
		return std::dynamic_pointer_cast<ParameterGroup>(input);
	}
	
	static std::shared_ptr<AnimatableParameter> castToAnimatable(std::shared_ptr<MachineParameter> input){
		return std::dynamic_pointer_cast<AnimatableParameter>(input);
	}
	
	static std::shared_ptr<AnimatableStateParameter> toState(std::shared_ptr<MachineParameter> input){
		return std::dynamic_pointer_cast<AnimatableStateParameter>(input);
	}

private:
	char name[256];
	std::shared_ptr<Machine> machine;
	std::shared_ptr<ParameterGroup> parentParameterGroup = nullptr;
};



class AnimatableParameter : public MachineParameter{
public:
	
	AnimatableParameter(const char* name) : MachineParameter(name) {}
	
	virtual std::vector<Motion::InterpolationType>& getCompatibleInterpolationTypes() = 0;

	bool hasParameterTrack() { return actualParameterTrack != nullptr; }
	std::shared_ptr<AnimatableParameterValue> getActiveTrackParameterValue(){
		//return actualParameterTrack->getParameterValueAtPlaybackTime();
		return nullptr;
	}
	
private:
	std::shared_ptr<ParameterTrack> actualParameterTrack = nullptr;
};




//———————————————————————————————————————————————
//				NUMERICAL PARAMETER
//———————————————————————————————————————————————

class AnimatableNumericalParameter : public AnimatableParameter{
public:
	
	AnimatableNumericalParameter(const char* name, MachineParameterType type_, Unit unit_) : AnimatableParameter(name), unit(unit_), type(type_){
		assert(type != MachineParameterType::GROUP);
		assert(type != MachineParameterType::BOOLEAN);
		assert(type != MachineParameterType::STATE);
	}
	
	virtual MachineParameterType getType(){ return type; }
	virtual std::vector<Motion::InterpolationType>& getCompatibleInterpolationTypes();
	
	Unit getUnit(){ return unit; }
	void setUnit(Unit u){ unit = u; }
	
private:
	Unit unit;
	MachineParameterType type;
};



//———————————————————————————————————————————————
//				STATE PARAMETER
//———————————————————————————————————————————————

class AnimatableStateParameter : public AnimatableParameter{
public:
	
	AnimatableStateParameter(const char* name, std::vector<AnimatableParameterState>* stateValues) : AnimatableParameter(name), states(stateValues){};
	
	virtual MachineParameterType getType(){ return MachineParameterType::STATE; }
	virtual std::vector<Motion::InterpolationType>& getCompatibleInterpolationTypes(){
		static std::vector<Motion::InterpolationType> compatibleInterpolations = { Motion::InterpolationType::STEP };
		return compatibleInterpolations;
	}
	
	std::vector<AnimatableParameterState>& getStates() { return *states; }
	
private:
	std::vector<AnimatableParameterState>* states;
};



//———————————————————————————————————————————————
//				BOOLEAN PARAMETER
//———————————————————————————————————————————————

class AnimatableBooleanParameter : public AnimatableParameter{
public:
	
	AnimatableBooleanParameter(const char* name) : AnimatableParameter(name){}
	
	virtual MachineParameterType getType(){ return MachineParameterType::BOOLEAN; }
	virtual std::vector<Motion::InterpolationType>& getCompatibleInterpolationTypes(){
		static std::vector<Motion::InterpolationType> compatibleInterpolations = { Motion::InterpolationType::STEP };
		return compatibleInterpolations;
	}
};



//———————————————————————————————————————————————
//				PARAMETER GROUP
//———————————————————————————————————————————————

class ParameterGroup : public MachineParameter{
public:
	
	ParameterGroup(const char* name, std::vector<std::shared_ptr<MachineParameter>> children) : MachineParameter(name), childParameters(children){
		for(auto& childParameter : childParameters){
			auto thisGroup = std::dynamic_pointer_cast<ParameterGroup>(shared_from_this());
			childParameter->setParentGroup(thisGroup);
		}
	}
	
	virtual MachineParameterType getType(){ return MachineParameterType::GROUP; }
	
	std::vector<std::shared_ptr<MachineParameter>>& getChildren(){ return childParameters; }

private:
	std::vector<std::shared_ptr<MachineParameter>> childParameters;
};




















































