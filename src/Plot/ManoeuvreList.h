#pragma once

#include "Motion/MotionTypes.h"
#include "Legato/Editor/Component.h"
#include "Legato/Editor/ListComponent.h"

#include "Animation/NewAnimation/Manoeuvre.h"

class Plot;
namespace tinyxml2{ struct XMLElement; }
namespace AnimationSystem{ class AnimatableRegistry; };

class ManoeuvreList : public Legato::Component{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(ManoeuvreList)
	
public:
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override;
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;

public:
	
	void setAnimatableRegistry(std::shared_ptr<AnimationSystem::AnimatableRegistry> registry){ animatableRegistry = registry; }
	std::shared_ptr<AnimationSystem::AnimatableRegistry> getAnimatableRegistry(){ return animatableRegistry; }
	
	std::vector<std::shared_ptr<AnimationSystem::Manoeuvre>>& getManoeuvres(){ return manoeuvres->getEntries(); }
	bool containsManoeuvre(std::shared_ptr<AnimationSystem::Manoeuvre> manoeuvre);
	int getManoeuvreIndex(std::shared_ptr<AnimationSystem::Manoeuvre> manoeuvre);
	
	void selectNextManoeuvre(){}
	void selectPreviousManoeuvre(){}
	
	void addManoeuvre();
	void deleteSelectedManoeuvre(){}
	void duplicateSelectedManoeuvre(){}
	void moveManoeuvre(std::shared_ptr<AnimationSystem::Manoeuvre> m, int newIndex){}
	
	void pasteManoeuvre(std::shared_ptr<AnimationSystem::Manoeuvre> manoeuvre);
	
private:
	
	std::shared_ptr<AnimationSystem::AnimatableRegistry> animatableRegistry = nullptr;
	std::shared_ptr<Legato::ListComponent<AnimationSystem::Manoeuvre>> manoeuvres;
	
};
