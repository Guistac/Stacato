#pragma once

#include "Legato/Editor/FileComponent.h"
#include "Legato/Editor/SerializableList.h"
#include "Legato/Gui/Layout.h"

class LayoutList : public Legato::FileComponent{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(LayoutList)
	
public:
	
	SerializableList<Layout> layouts;
	
	std::shared_ptr<Layout> currentLayout = nullptr;
	std::shared_ptr<Layout> defaultLayout = nullptr;

	const std::vector<std::shared_ptr<Layout>>& get(){ return layouts; }
	std::shared_ptr<Layout> getCurrent(){ return currentLayout; }
	std::shared_ptr<Layout> getDefault(){ return defaultLayout; }

	void captureNew();
	void makeCurrentDefault(){ if(currentLayout) defaultLayout = currentLayout; }
	void makeCurrent(std::shared_ptr<Layout> layout);
	
	void add(std::shared_ptr<Layout> layout){
		layouts.push_back(layout);
	}
	
	void remove(std::shared_ptr<Layout> layout){
		if(layout == currentLayout) currentLayout = nullptr;
		for(int i = 0; i < layouts.size(); i++){
			if(layouts[i] == layout){
				layouts.erase(layouts.begin() + i);
				return;
			}
		}
	}
	
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override;
	
};
