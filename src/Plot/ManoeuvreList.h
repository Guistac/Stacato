#pragma once

#include "Motion/MotionTypes.h"
#include "Legato/Editor/Component.h"
#include "Legato/Editor/ListComponent.h"

#include "Animation/NewAnimation/Manoeuvre.h"

class Plot;
namespace tinyxml2{ struct XMLElement; }

class ManoeuvreList : public Legato::Component{
public:
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(ManoeuvreList)
	
	virtual void onConstruction() override{
		Component::onConstruction();
		manoeuvres = Legato::ListComponent<AnimationSystem::Manoeuvre>::createInstance();
		manoeuvres->setSaveString("Manoeuvres");
		manoeuvres->setEntrySaveString("Manoeuvre");
		manoeuvres->setEntryConstructor([](Serializable& abstract){ return AnimationSystem::Manoeuvre::createInstance(); });
	}
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override{
		Component::onCopyFrom(source);
	}
	virtual bool onSerialization() override{
		bool success = Component::onSerialization();
		success &= manoeuvres->serializeIntoParent(this);
		return success;
	}
	virtual bool onDeserialization() override{
		bool success = Component::onSerialization();
		success &= manoeuvres->deserializeFromParent(this);
		return success;
	}

public:
	
	//ManoeuvreList(std::shared_ptr<Plot> plot_) : plot(plot_){}
	bool load(tinyxml2::XMLElement* xml);
	bool save(tinyxml2::XMLElement* xml);
	
	void setParentPlot(std::shared_ptr<Plot> plot_){ plot = plot_; }
	
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
	
	std::shared_ptr<Plot> getPlot(){ return plot; }
	
private:
	
	std::shared_ptr<Plot> plot;
	
	std::shared_ptr<Legato::ListComponent<AnimationSystem::Manoeuvre>> manoeuvres;
	
};
