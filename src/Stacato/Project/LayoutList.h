#pragma once

#include "Legato/Gui/Layout.h"

#include "Legato/Editor/LegatoFile.h"
#include "Legato/Editor/LegatoList.h"

class LayoutList : public Legato::File{
	COMPONENT_IMPLEMENTATION(LayoutList)
public:
	
	static Ptr<LayoutList> make(std::filesystem::path path_){
		auto instance = LayoutList::make();
		instance->setPath(path_);
		return instance;
	}
	
	const std::vector<std::shared_ptr<Layout>> get(){ return layouts->getList(); }
	std::shared_ptr<Layout> getCurrent(){ return currentLayout; }
	std::shared_ptr<Layout> getDefault(){ return defaultLayout; }

	void captureNew();
	void makeCurrentDefault(){ if(currentLayout) defaultLayout = currentLayout; }
	void makeCurrent(std::shared_ptr<Layout> layout);
	
	void add(std::shared_ptr<Layout> layout){
		layouts->addEntry(layout);
	}
	
	void remove(std::shared_ptr<Layout> layout){
		layouts->removeEntry(layout);
	}
	
private:
	Legato::List<Layout> layouts = Legato::makeList<Layout>("Layouts");
	Ptr<Layout> currentLayout = nullptr;
	Ptr<Layout> defaultLayout = nullptr;
	
	virtual void onConstruction() override;
	virtual bool onSerialization() override;
	virtual bool onPostLoad() override;
	virtual void copyFrom(Ptr<Component> source) override;
	
};
