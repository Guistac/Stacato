#pragma once

#include "Legato/Gui/Window.h"

#include "Legato/Editor/LegatoComponent.h"
#include "Legato/Editor/LegatoParameter.h"

class Layout : public Legato::Component{
	COMPONENT_IMPLEMENTATION(Layout)
public:
	
	virtual void onConstruction() override;
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	virtual void copyFrom(std::shared_ptr<Legato::Component> source) override;
	
	void overwrite();

	std::string& getName(){ return name->getValue();  }
	
	Legato::String name = Legato::makeString("Default Name", "Name", "Name");
	
	std::string layoutString;
	std::vector<std::string> openWindowIds;
};


class RenameLayoutPopup : public Popup{
public:
	RenameLayoutPopup() : Popup("Rename Layout", true, true){}
	virtual void onDraw() override;
	
	std::shared_ptr<Layout> renamedLayout = nullptr;
	char nameBuffer[128];
	
	static void open(std::shared_ptr<Layout> layout){
		static std::shared_ptr<RenameLayoutPopup> popup = std::make_shared<RenameLayoutPopup>();
		if(layout != nullptr){
			popup->renamedLayout = layout;
			//strcpy(popup->nameBuffer, layout->getName().c_str());
			popup->Popup::open();
		}
	}
};
