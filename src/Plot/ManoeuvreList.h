#pragma once

#include "Motion/MotionTypes.h"
#include "Legato/Editor/Component.h"
#include "Legato/Editor/ListComponent.h"

#include "Animation/NewAnimation/Manoeuvre.h"

class Plot;
namespace tinyxml2{ struct XMLElement; }

class ManoeuvreList : public Legato::Component{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(ManoeuvreList)
	
public:
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override;
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;

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
