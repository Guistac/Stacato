#pragma once

#include "Legato/Editor/FileComponent.h"
#include "Legato/Editor/ListComponent.h"
#include "ManoeuvreList.h"

namespace AnimationSystem{
	class Manoeuvre;
	class AnimatableRegistry;
}

class StacatoProject;

class Plot : public Legato::FileComponent{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(Plot)
	
public:
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override;
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	
	void addManoeuvreList(std::shared_ptr<ManoeuvreList> manoeuvreList){
		manoeuvreList->setAnimatableRegistry(animatableRegistry);
		manoeuvreLists->addEntry(manoeuvreList);
		selectedManoeuvreList = manoeuvreList;
	}
	
	void removeManoeuvreList(std::shared_ptr<ManoeuvreList> manoeuvreList){
		manoeuvreList->setAnimatableRegistry(animatableRegistry);
		manoeuvreLists->removeEntry(manoeuvreList);
		if(manoeuvreList == selectedManoeuvreList) selectedManoeuvreList = nullptr;
	}
	
	std::vector<std::shared_ptr<ManoeuvreList>>& getManoeuvreLists(){ return manoeuvreLists->getEntries(); }
	std::shared_ptr<ManoeuvreList> getSelectedManoeuvreList(){ return selectedManoeuvreList; }
	std::shared_ptr<AnimationSystem::Manoeuvre> getSelectedManoeuvre(){ return selectedManoeuvre; }
	
	void selectManoeuvreList(std::shared_ptr<ManoeuvreList> manoeuvreList);
	void selectManoeuvre(std::shared_ptr<AnimationSystem::Manoeuvre> manoeuvre);
	
	void setAnimatableRegistry(std::shared_ptr<AnimationSystem::AnimatableRegistry> registry){ animatableRegistry = registry; }
	
	
private:
	
	std::shared_ptr<Legato::ListComponent<ManoeuvreList>> manoeuvreLists = nullptr;
	std::shared_ptr<ManoeuvreList> selectedManoeuvreList = nullptr;
	std::shared_ptr<AnimationSystem::Manoeuvre> selectedManoeuvre = nullptr;

	bool b_scrollToSelectedManoeuvre = false;
	
	std::shared_ptr<AnimationSystem::AnimatableRegistry> animatableRegistry = nullptr;
	
};
