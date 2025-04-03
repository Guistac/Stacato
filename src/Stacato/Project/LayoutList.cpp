#include <pch.h>

#include "LayoutList.h"

#include "Legato/Gui/Layout.h"
#include "Legato/Gui/Gui.h"

void LayoutList::onConstruction(){
	Legato::File::onConstruction();
	addChild(layouts);
}

bool LayoutList::onSerialization() {
	Legato::File::onSerialization();
	std::string defaultLayoutName = "";
	if(defaultLayout) defaultLayoutName = defaultLayout->getName();
	serializeStringAttribute("DefaultLayout", defaultLayoutName);
	return true;
}

bool LayoutList::onPostLoad(){
	Legato::File::onPostLoad();
	std::string defaultLayoutName;
	if(!deserializeStringAttribute("DefaultLayout", defaultLayoutName)) return false;
	for(auto layout : layouts->getList()){
		if(layout->getName() == defaultLayoutName){
			defaultLayout = layout;
			makeCurrent(layout);
			break;
		}
	}
	return true;
}

void LayoutList::copyFrom(std::shared_ptr<Legato::Component> source) {
	Legato::File::copyFrom(source);
	Component::copyFrom(source);
}



void LayoutList::captureNew(){
	auto newLayout = Legato::Gui::WindowManager::captureCurentLayout();
	newLayout->name->overwrite("New Layout");
	layouts->addEntry(newLayout);
	currentLayout = newLayout;
}

void LayoutList::makeCurrent(std::shared_ptr<Layout> layout){
	currentLayout = layout;
	Legato::Gui::WindowManager::applyLayout(layout);
}
