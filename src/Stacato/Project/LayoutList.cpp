#include <pch.h>

#include "LayoutList.h"

#include "Legato/Gui/Layout.h"
#include "Legato/Gui/Gui.h"

void LayoutList::onConstruction() {
	FileComponent::onConstruction();
	setSaveString("LayoutList");
	layouts = Legato::ListComponent<Layout>::createInstance();
	layouts->setSaveString("Layouts");
	layouts->setEntrySaveString("Layout");
	layouts->setEntryConstructor([](Serializable&) -> std::shared_ptr<Layout> { return Layout::createInstance(); });
}

void LayoutList::onCopyFrom(std::shared_ptr<Prototype> source) {
	Component::onCopyFrom(source);
}

bool LayoutList::onSerialization() {
	bool success = true;
	
	success &= FileComponent::onSerialization();
	
	if(defaultLayout != nullptr){
		Serializable defaultLayoutSerializable;
		defaultLayoutSerializable.setSaveString("DefaultLayout");
		defaultLayoutSerializable.serializeIntoParent(this);
		defaultLayoutSerializable.serializeAttribute("Name", defaultLayout->getName());
	}
	success &= layouts->serializeIntoParent(this);
	return success;
}

bool LayoutList::onDeserialization() {
	bool success = true;
	
	success &= layouts->deserializeFromParent(this);
	
	Serializable defaultLayoutSerializable;
	defaultLayoutSerializable.setSaveString("DefaultLayout");
	if(defaultLayoutSerializable.deserializeFromParent(this)){
		std::string defaultLayoutName;
		if(defaultLayoutSerializable.deserializeAttribute("Name", defaultLayoutName)){
			for(auto layout : layouts->getEntries()){
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

void LayoutList::makeCurrent(std::shared_ptr<Layout> layout){
	currentLayout = layout;
	Legato::Gui::WindowManager::applyLayout(layout);
}



void LayoutCreationPopup::onDraw(){
	ImGui::Text("Layout Name");
	nameParameter->gui();
	if(ImGui::Button("Save")){
		auto newLayout = Legato::Gui::WindowManager::captureCurentLayout();
		newLayout->setName(nameParameter->getValue());
		layoutList->add(newLayout);
		layoutList->makeCurrent(newLayout);
		close();
	}
	ImGui::SameLine();
	if(ImGui::Button("Cancel")){
		close();
	}
}


void LayoutEditPopup::onDraw(){
	ImGui::Text("Layout Name");
	nameParameter->gui();
	if(ImGui::Button("Save")){
		editedLayout->setName(nameParameter->getValue());
		close();
	}
	ImGui::SameLine();
	if(ImGui::Button("Cancel")){
		close();
	}
}

