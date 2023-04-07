#include <pch.h>

#include "LayoutList.h"

#include "Legato/Gui/Layout.h"
#include "Legato/Gui/Gui.h"

bool LayoutList::onSerialization() {
	bool success = true;
	
	if(defaultLayout != nullptr){
		Serializable defaultLayoutSerializable;
		defaultLayoutSerializable.setSaveString("DefaultLayout");
		defaultLayoutSerializable.serializeIntoParent(this);
		defaultLayoutSerializable.serializeAttribute("Name", defaultLayout->getName());
	}
		
	success &= layouts.serializeIntoParent(this);
	return success;
}

bool LayoutList::onDeserialization() {
	bool success = true;
	success &= layouts.deserializeFromParent(this);
	
	Serializable defaultLayoutSerializable;
	defaultLayoutSerializable.setSaveString("DefaultLayout");
	if(defaultLayoutSerializable.deserializeFromParent(this)){
		std::string defaultLayoutName;
		if(defaultLayoutSerializable.deserializeAttribute("Name", defaultLayoutName)){
			for(auto layout : layouts){
				if(layout->getName() == defaultLayoutName){
					defaultLayout = layout;
					makeCurrent(layout);
					break;
				}
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
	auto newLayout = Legato::Gui::WindowManager::captureCurentLayout();
	newLayout->setName("New Layout");
	layouts.push_back(newLayout);
	currentLayout = newLayout;
}

void LayoutList::makeCurrent(std::shared_ptr<Layout> layout){
	currentLayout = layout;
	Legato::Gui::WindowManager::applyLayout(layout);
}
