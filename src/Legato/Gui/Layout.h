#pragma once

#include <tinyxml2.h>
#include <imgui.h>
#include <GLFW/glfw3.h>

#include "Legato/Gui/Window.h"

#include "Legato/Editor/Component.h"
#include "Legato/Editor/SerializableList.h"

class Layout : public Component{

	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(Layout)
	
public:
	
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override;
	
	void makeActive();
	bool isActive();
	
	void makeDefault();
	bool isDefault();
	
	void edit();
	void remove();
	
	void overwrite();
	
	std::string layoutString;
	std::vector<std::string> openWindowIds;
	
	char name[256];
	
	void nameEditField(){
		ImGui::InputText("##LayoutName", name, 256);
	}
	
};



namespace Legato::Gui::LayoutManager{

	void registerWindow(std::shared_ptr<Window> window);
	void unregisterWindow(std::shared_ptr<Window> window);

	std::shared_ptr<Layout> captureCurentLayout();

	bool isLayoutLocked();
	bool unlockLayout();
	bool lockLayout();

	void applyLayout(std::shared_ptr<Layout> layout);

};


/*
 class LayoutEditorPopup : public Popup{
 public:
	 LayoutEditorPopup() : Popup("Edit Layout", true, true){}
	 virtual void onDraw() override;
	 SINGLETON_GET_METHOD(LayoutEditorPopup);
 };
 */
