#pragma once

#include "AnimatableParameterValue.h"
#include "Motion/Curve/Curve.h"
#include "Project/Editor/Parameter.h"

class Machine;
class ParameterTrack;
namespace tinyxml2 { class XMLElement; }
class AnimatableParameter;
class ParameterGroup;
class AnimatableStateParameter;
class AnimatableNumericalParameter;





class MachineParameter : public std::enable_shared_from_this<MachineParameter>{
public:
	
	MachineParameter(std::string name_) : name(name_){}
	
	//————————————————————————————————————
	//  	   General Properties
	//————————————————————————————————————
	
public:
	
	const char* getName(){ return name.c_str(); }
	
	void setMachine(std::shared_ptr<Machine> machine_){ machine = machine_; }
	std::shared_ptr<Machine> getMachine(){ return machine; }
	
	bool hasParentGroup() { return parentParameterGroup != nullptr; }
	void setParentGroup(std::shared_ptr<ParameterGroup> parent){ parentParameterGroup = parent; }
	
private:
	
	std::string name;
	std::shared_ptr<Machine> machine;
	std::shared_ptr<ParameterGroup> parentParameterGroup = nullptr;
	
	//————————————————————————————————————
	//  		 Track Creation
	//————————————————————————————————————
	
public:
	
	std::shared_ptr<ParameterTrack> createTrack(ManoeuvreType manoeuvreType);
	
	//————————————————————————————————————
	//  SubClass Identification & Casting
	//————————————————————————————————————
	
public:
	
	virtual MachineParameterType getType() = 0;
	
	virtual bool isGroup(){ return false; }
	std::shared_ptr<ParameterGroup> castToGroup(){ return std::dynamic_pointer_cast<ParameterGroup>(shared_from_this()); }
	
	virtual bool isAnimatable(){ return false; }
	std::shared_ptr<AnimatableParameter> castToAnimatable(){ return std::dynamic_pointer_cast<AnimatableParameter>(shared_from_this()); }
	
	virtual bool isNumerical(){ return false; }
	virtual bool isReal(){ return false; }
	std::shared_ptr<AnimatableNumericalParameter> castToNumerical(){ return std::dynamic_pointer_cast<AnimatableNumericalParameter>(shared_from_this()); }
	
	virtual bool isState(){ return false; }
	std::shared_ptr<AnimatableStateParameter> castToState(){ return std::dynamic_pointer_cast<AnimatableStateParameter>(shared_from_this()); }
	
	//————————————————————————————————————
	//	  Parameter Track Subscriptions
	//————————————————————————————————————
	
public:
	
	void subscribeTrack(std::shared_ptr<ParameterTrack> track){ tracks.push_back(track); }
	void unsubscribeTrack(std::shared_ptr<ParameterTrack> track){
		for(int i = 0; i < tracks.size(); i++){
			if(tracks[i] == track) {
				tracks.erase(tracks.begin() + i);
				break;
			}
		}
	}
	std::vector<std::shared_ptr<ParameterTrack>>& getTracks(){ return tracks; }

	bool hasActiveParameterTrack() { return activeParameterTrack != nullptr; }
	
	std::shared_ptr<ParameterTrack> activeParameterTrack = nullptr;
	
	void stopParameterPlayback();
	
private:
	
	std::vector<std::shared_ptr<ParameterTrack>> tracks;
};



class PlayableParameterTrack;

class AnimatableParameter : public MachineParameter{
public:
	
	AnimatableParameter(const char* name) : MachineParameter(name) {}
	virtual bool isAnimatable() override { return true; }
	
	virtual std::vector<Motion::Interpolation::Type>& getCompatibleInterpolationTypes() = 0;
	
	std::shared_ptr<AnimatableParameterValue> getActiveParameterTrackValue();
	
	std::shared_ptr<AnimatableParameterValue> getActualMachineValue();
	
	int getCurveCount();
	
	std::shared_ptr<Parameter> getEditableParameter();
	
	void setParameterValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimatableParameterValue> value);
	void copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to);
	std::shared_ptr<AnimatableParameterValue> getParameterValue(std::shared_ptr<Parameter> parameter);
	bool isParameterValueEqual(std::shared_ptr<AnimatableParameterValue> value1, std::shared_ptr<AnimatableParameterValue> value2);
	std::shared_ptr<AnimatableParameterValue> getParameterValueAtCurveTime(std::shared_ptr<PlayableParameterTrack> playableParameterTrack, double time_seconds);
	std::vector<double> getCurvePositionsFromParameterValue(std::shared_ptr<AnimatableParameterValue> value);
	
private:
	
};




//———————————————————————————————————————————————
//				NUMERICAL PARAMETER
//———————————————————————————————————————————————

class AnimatableNumericalParameter : public AnimatableParameter{
public:
	
	AnimatableNumericalParameter(const char* name, MachineParameterType type_, Unit unit_) : AnimatableParameter(name), type(type_){
		assert(type != MachineParameterType::GROUP);
		assert(type != MachineParameterType::BOOLEAN);
		assert(type != MachineParameterType::STATE);
		setUnit(unit_);
	}
	
	virtual bool isNumerical() override { return true; }
	virtual bool isReal() override {
		switch(type){
			case MachineParameterType::BOOLEAN:
			case MachineParameterType::INTEGER:
			case MachineParameterType::STATE:
			case MachineParameterType::GROUP:
				return false;
			default:
				return true;
		}
	}
	
	virtual MachineParameterType getType() override { return type; }
	virtual std::vector<Motion::Interpolation::Type>& getCompatibleInterpolationTypes() override;
	
	Unit getUnit(){ return unit; }
	void setUnit(Unit u);
	
	//std::shared_ptr<NumberParameter<int>> floatingPointDisplayPrecision = NumberParameter<int>::make(1, "Floating Point Display Precision", "Precision");
	//void setPrecision(int precision){ format = "%." + std::to_string(precision) + "f"; }
	//const char* getFormat(){ return format.c_str(); }
	
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
	
	virtual bool isState() override { return true; }
	
	virtual MachineParameterType getType() override { return MachineParameterType::STATE; }
	virtual std::vector<Motion::Interpolation::Type>& getCompatibleInterpolationTypes() override {
		static std::vector<Motion::Interpolation::Type> compatibleInterpolations = { Motion::Interpolation::Type::STEP };
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
	virtual std::vector<Motion::Interpolation::Type>& getCompatibleInterpolationTypes(){
		static std::vector<Motion::Interpolation::Type> compatibleInterpolations = { Motion::Interpolation::Type::STEP };
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
	
	virtual bool isGroup() override { return true; }
	
	virtual MachineParameterType getType() override { return MachineParameterType::GROUP; }
	
	std::vector<std::shared_ptr<MachineParameter>>& getChildren(){ return childParameters; }

private:
	std::vector<std::shared_ptr<MachineParameter>> childParameters;
};
