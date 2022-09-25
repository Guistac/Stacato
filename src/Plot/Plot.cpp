#include <pch.h>

#include "Plot.h"
#include "ManoeuvreList.h"
#include "Animation/Manoeuvre.h"
#include "Project/Project.h"

#include <tinyxml2.h>

std::shared_ptr<Plot> Plot::create(){
	auto plot = std::make_shared<Plot>();
	plot->manoeuvreList = std::make_shared<ManoeuvreList>(plot);
	return plot;
}

void Plot::selectManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre){
	selectedManoeuvre = manoeuvre;
	if(manoeuvre) selectedManoeuvre->requestCurveRefocus();
}

bool Plot::isCurrent(){
	return Project::getCurrentPlot() == shared_from_this();
}
