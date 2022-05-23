#include <pch.h>

#include "Plot.h"
#include "ManoeuvreList.h"

#include <tinyxml2.h>

std::shared_ptr<Plot> Plot::create(){
	auto plot = std::make_shared<Plot>();
	plot->manoeuvreList = std::make_shared<ManoeuvreList>(plot);
	return plot;
}
