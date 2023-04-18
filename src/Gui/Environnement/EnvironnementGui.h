#pragma once

#include "Legato/Gui/Window.h"

class Node;

namespace Environnement::Gui{


	//——————————————————— Editor Windows ———————————————————

	class UnlockEditorPopup : public Legato::Popup{
	public:
		UnlockEditorPopup() : Popup("Unlock Environnement Editor", true, true){}
		virtual void onDraw() override;
		SINGLETON_GET_METHOD(UnlockEditorPopup);
	};

	class NodeEditorWindow : public Legato::Window{
	public:
		NodeEditorWindow() : Window("Node Editor", true){}
		virtual void onDraw() override;
		SINGLETON_GET_METHOD(NodeEditorWindow);
	};

	std::shared_ptr<Node> nodeAdderContextMenu();
	std::shared_ptr<Node> nodeDragDropTarget();

	class NodeManagerWindow : public Legato::Window{
	public:
		NodeManagerWindow() : Window("Node Manager", true){}
		virtual void onDraw() override;
		SINGLETON_GET_METHOD(NodeManagerWindow);
	};

	class VisualizerScriptWindow : public Legato::Window{
	public:
		VisualizerScriptWindow() : Window("Visualizer Script", true){}
		virtual void onDraw() override;
		SINGLETON_GET_METHOD(VisualizerScriptWindow);
	};

	class EtherCATWindow : public Legato::Window{
	public:
		EtherCATWindow() : Window("EtherCAT Fieldbus", true){}
		virtual void onDraw() override;
		SINGLETON_GET_METHOD(EtherCATWindow);
	};

	class ScriptEditorWindow : public Legato::Window{
	public:
		ScriptEditorWindow() : Window("Environnement Script", true){}
		virtual void onDraw() override;
		SINGLETON_GET_METHOD(ScriptEditorWindow);
	};


	//——————————————————— Performance Windows ———————————————————

	class SetupWindow : public Legato::Window{
	public:
		SetupWindow() : Window("Setup", true){}
		virtual void onDraw() override;
		SINGLETON_GET_METHOD(SetupWindow);
	};

	class VisualizerWindow : public Legato::Window{
	public:
		VisualizerWindow() : Window("Visualizer", true){}
		virtual void onDraw() override;
		SINGLETON_GET_METHOD(VisualizerWindow);
	};

	class LogWindow : public Legato::Window{
	public:
		LogWindow() : Window("Log", true){}
		virtual void onDraw() override;
		SINGLETON_GET_METHOD(LogWindow);
	};


	



	inline std::vector<std::shared_ptr<Legato::Window>>& getEditorWindows(){
		static std::vector<std::shared_ptr<Legato::Window>> editorWindows = {
			NodeEditorWindow::get(),
			NodeManagerWindow::get(),
			VisualizerScriptWindow::get(),
			EtherCATWindow::get(),
			ScriptEditorWindow::get()
		};
		return editorWindows;
	}

	inline void restrictEditorWindows(){
		for(auto& editorWindow : getEditorWindows()) {
			assert(false || "ERROR");
			//editorWindow->removeFromDictionnary();
		}
	}
	inline void allowEditorWindows(){
		for(auto& editorWindow : getEditorWindows()) {
			assert(false || "ERROR");
			//editorWindow->addToDictionnary();
		}
	}

}
