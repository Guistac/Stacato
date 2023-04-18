#pragma once

#include "Legato/Editor/FileComponent.h"
#include "Legato/Editor/ListComponent.h"
#include "Legato/Gui/Layout.h"
#include "Legato/Editor/Parameters.h"

class LayoutList : public Legato::FileComponent{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(LayoutList)
	
public:

	const std::vector<std::shared_ptr<Layout>>& getList(){ return layouts->getEntries(); }
	std::shared_ptr<Layout> getCurrent(){ return currentLayout; }
	std::shared_ptr<Layout> getDefault(){ return defaultLayout; }

	void makeCurrentDefault(){ if(currentLayout) defaultLayout = currentLayout; }
	void makeCurrent(std::shared_ptr<Layout> layout);
	
	void add(std::shared_ptr<Layout> layout){
		layouts->addEntry(layout);
	}
	
	void remove(std::shared_ptr<Layout> layout){
		if(layout == currentLayout) currentLayout = nullptr;
		layouts->removeEntry(layout);
	}
	
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override;
	
private:
	
	std::shared_ptr<Layout> currentLayout = nullptr;
	std::shared_ptr<Layout> defaultLayout = nullptr;
	std::shared_ptr<Legato::ListComponent<Layout>> layouts;
	
};

class LayoutCreationPopup : public Legato::Popup{
public:
	
	static void open(std::shared_ptr<LayoutList> list){
		static std::shared_ptr<LayoutCreationPopup> popup = std::make_shared<LayoutCreationPopup>();
		popup->layoutList = list;
		popup->nameParameter->overwrite("New Layout");
		popup->Legato::Popup::open();
	}
	
	LayoutCreationPopup() : Popup("New Layout", true, false){
		nameParameter = Legato::StringParameter::createInstance("New Layout", "Layout Name", "");
	}
	
	virtual void onDraw() override;
	
	std::shared_ptr<LayoutList> layoutList = nullptr;
	Legato::StringParam nameParameter;
	
};

class LayoutEditPopup : public Legato::Popup{
public:
	
	static void open(std::shared_ptr<Layout> layout){
		static std::shared_ptr<LayoutEditPopup> popup = std::make_shared<LayoutEditPopup>();
		popup->editedLayout = layout;
		popup->nameParameter->overwrite(layout->getName());
		popup->Legato::Popup::open();
	}
	
	LayoutEditPopup() : Popup("Edit Layout", true, false){
		nameParameter = Legato::StringParameter::createInstance("", "Layout Name", "");
	}
	
	virtual void onDraw() override;
	
	std::shared_ptr<Layout> editedLayout = nullptr;
	Legato::StringParam nameParameter;
	
};
