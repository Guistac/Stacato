#pragma once

#include "Gui/ApplicationWindow/Window.h"

namespace Environnement::Gui{

	//Performance
	void plot();
	void machineList();
	void dashboards();
	void homingAndSetup();
	void stageVisualizer();


	//Editing & Managing
	void gui();
	void editor();
	void nodeManager();
	void nodeEditor();
	void stageEditor();
	void log();


	class EnvironnementEditorWindow : public Window{
	public:
		EnvironnementEditorWindow() : Window("Environnement", true){}
		virtual void drawContent() override { gui(); };
		SINGLETON_GET_METHOD(EnvironnementEditorWindow);
	};

	class SetupWindow : public Window{
	public:
		SetupWindow() : Window("Setup", true){}
		virtual void drawContent() override{ homingAndSetup(); };
		SINGLETON_GET_METHOD(SetupWindow);
	};

	class UnlockEditorPopup : public Popup{
	public:
		UnlockEditorPopup() : Popup("Unlock Environnement Editor", true, true){}
		virtual void drawContent() override;
		SINGLETON_GET_METHOD(UnlockEditorPopup);
	};

}
