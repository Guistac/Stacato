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
	//void machineManager();
	//void deviceManager();
	void nodeEditor();
	void stageEditor();
	void log();


	class EnvironnementEditorWindow : public Window{
	public:
		EnvironnementEditorWindow() : Window("Environnement", true){}
		virtual void drawContent() override { gui(); };
		static std::shared_ptr<EnvironnementEditorWindow> get();
	};

	class SetupWindow : public Window{
	public:
		SetupWindow() : Window("Setup", true){}
		virtual void drawContent() override{ homingAndSetup(); };
		static std::shared_ptr<SetupWindow> get();
	};

	class UnlockEditorPopup : public Popup{
	public:
		UnlockEditorPopup() : Popup("Unlock Environnement Editor", true, true){}
		virtual void drawContent() override;
		static std::shared_ptr<UnlockEditorPopup> get(){
			static auto popup = std::make_shared<UnlockEditorPopup>();
			return popup;
		}
	};

}
