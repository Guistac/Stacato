#include <pch.h>

#include "Plot.h"
#include "ManoeuvreList.h"
#include "Animation/Manoeuvre.h"

#include "Stacato/StacatoWorkspace.h"
#include "Stacato/StacatoProject.h"

#include <tinyxml2.h>

std::shared_ptr<Plot> Plot::create(){
	auto plot = std::make_shared<Plot>();
	plot->manoeuvreList = std::make_shared<ManoeuvreList>(plot);
	return plot;
}

std::shared_ptr<Plot> Plot::duplicate(){
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
}

void Plot::selectManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre){
	selectedManoeuvre = manoeuvre;
	if(manoeuvre) selectedManoeuvre->requestCurveRefocus();
}

bool Plot::isCurrent(){
	return Stacato::Workspace::getCurrentProject()->getCurrentPlot() == shared_from_this();
}
