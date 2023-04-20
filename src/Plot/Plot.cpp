#include <pch.h>

#include "Plot.h"
#include "ManoeuvreList.h"
#include "Animation/Manoeuvre.h"

#include "Stacato/StacatoEditor.h"
#include "Stacato/Project/StacatoProject.h"

#include <tinyxml2.h>


void Plot::onCopyFrom(std::shared_ptr<Prototype> source){
	Component::onCopyFrom(source);
	/*
	auto copy = create();
	copy->setName("copy of " + std::string(getName()));
	
	auto& originalManoeuvres = manoeuvreList->getManoeuvres();
	auto& copiedManoeuvres = copy->getManoeuvreList()->getManoeuvres();
	
	for(auto originalManoeuvre : originalManoeuvres){
		auto manoeuvreCopy = originalManoeuvre->copy();
		manoeuvreCopy->setManoeuvreList(copy->getManoeuvreList());
		manoeuvreCopy->name->overwrite(originalManoeuvre->getName());
		copiedManoeuvres.push_back(manoeuvreCopy);
		manoeuvreCopy->validateAllAnimations();
	}
	
	return copy;
	 */
}

bool Plot::onSerialization(){
	bool success = manoeuvreLists->serializeIntoParent(this);
	return success;
}

bool Plot::onDeserialization(){
	bool success = manoeuvreLists->deserializeFromParent(this);
	selectedManoeuvreList = manoeuvreLists->getEntries().back();
	return success;
}

void Plot::selectManoeuvre(std::shared_ptr<AnimationSystem::Manoeuvre> manoeuvre){
	selectedManoeuvre = manoeuvre;
	//if(manoeuvre) selectedManoeuvre->requestCurveRefocus();
}

void Plot::selectManoeuvreList(std::shared_ptr<ManoeuvreList> manoeuvreList){
	selectedManoeuvreList = manoeuvreList;
	selectedManoeuvre = nullptr;
}
