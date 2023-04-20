#pragma once

#include "Legato/Editor/FileComponent.h"
#include "Legato/Editor/ListComponent.h"
#include "ManoeuvreList.h"

namespace AnimationSystem{
	class Manoeuvre;
}

class Plot : public Legato::FileComponent{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(Plot)
	
public:
	
	virtual void onConstruction() override {
		manoeuvreLists = Legato::ListComponent<ManoeuvreList>::createInstance();
		manoeuvreLists->setSaveString("ManoeuvreLists");
		manoeuvreLists->setEntrySaveString("ManoeuvreList");
		manoeuvreLists->setEntryConstructor([](Serializable& abstract){ return ManoeuvreList::createInstance(); });
		auto defaultManoeuvreList = ManoeuvreList::createInstance();
		defaultManoeuvreList->setName("Default Manoeuvre List");
		addManoeuvreList(defaultManoeuvreList);
	}
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override;
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	
public:
	
	void addManoeuvreList(std::shared_ptr<ManoeuvreList> manoeuvreList){
		manoeuvreList->setParentPlot(downcasted_shared_from_this<Plot>());
		manoeuvreLists->addEntry(manoeuvreList);
		selectedManoeuvreList = manoeuvreList;
	}
	
	void removeManoeuvreList(std::shared_ptr<ManoeuvreList> manoeuvreList){
		manoeuvreList->setParentPlot(nullptr);
		manoeuvreLists->removeEntry(manoeuvreList);
		if(manoeuvreList == selectedManoeuvreList) selectedManoeuvreList = nullptr;
	}
	
	std::vector<std::shared_ptr<ManoeuvreList>>& getManoeuvreLists(){ return manoeuvreLists->getEntries(); }
	std::shared_ptr<ManoeuvreList> getSelectedManoeuvreList(){ return selectedManoeuvreList; }
	std::shared_ptr<AnimationSystem::Manoeuvre> getSelectedManoeuvre(){ return selectedManoeuvre; }
	
	void selectManoeuvreList(std::shared_ptr<ManoeuvreList> manoeuvreList);
	void selectManoeuvre(std::shared_ptr<AnimationSystem::Manoeuvre> manoeuvre);
	
private:
	
	std::shared_ptr<Legato::ListComponent<ManoeuvreList>> manoeuvreLists = nullptr;
	std::shared_ptr<ManoeuvreList> selectedManoeuvreList = nullptr;
	std::shared_ptr<AnimationSystem::Manoeuvre> selectedManoeuvre = nullptr;

	bool b_scrollToSelectedManoeuvre = false;
	
};
