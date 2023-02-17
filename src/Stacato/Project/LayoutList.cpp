#include <pch.h>

#include "LayoutList.h"

#include "Legato/Gui/Layout.h"

bool LayoutList::onSerialization() {
	bool success = true;
	if(defaultLayout != nullptr) serializeAttribute("DefaultLayout", defaultLayout->getName());
	success &= layouts.serializeIntoParent(this);
	return success;
}

bool LayoutList::onDeserialization() {
	bool success = true;
	success &= layouts.deserializeFromParent(this);
	std::string defaultLayoutName;
	if(deserializeAttribute("DefaultLayout", defaultLayoutName)){
		for(auto layout : layouts.get()){
			if(layout->getName() == defaultLayoutName){
				defaultLayout = layout;
				makeCurrent(layout);
				break;
			}
		}
	}
	return success;
}

void LayoutList::onConstruction() {
	Component::onConstruction();
	setSaveString("LayoutList");
	layouts.setSaveString("Layouts");
	layouts.setEntrySaveString("Layout");
	layouts.setEntryConstructor(Layout::createInstance);
}

void LayoutList::onCopyFrom(std::shared_ptr<PrototypeBase> source) {
	Component::onCopyFrom(source);
}

void LayoutList::captureNew(){
	auto newLayout = Legato::Gui::LayoutManager::captureCurentLayout();
	newLayout->setName("New Layout");
	layouts.get().push_back(newLayout);
	currentLayout = newLayout;
}

void LayoutList::makeCurrent(std::shared_ptr<Layout> layout){
	currentLayout = layout;
	Legato::Gui::LayoutManager::applyLayout(layout);
}
