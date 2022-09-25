#include <pch.h>

#include "ApplicationWindow.h"

#include <imgui.h>
#include <implot.h>
#include <GLFW/glfw3.h>

#include "Gui.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Project/ProjectGui.h"
#include "Gui/Environnement/EnvironnementGui.h"

#include "Environnement/Environnement.h"
#include "Plot/Plot.h"
#include "Project/Project.h"
#include "Project/Editor/CommandHistory.h"

#include "KeyboardShortcut.h"

#include "Layout.h"

namespace Gui {

	void menuBar() {

		ImGui::BeginMenuBar();
		if (ImGui::BeginMenu("Stacato")) {
			if (ImGui::MenuItem("About")) Gui::AboutPopup::get()->open();
			ImGui::Separator();
			if (ImGui::MenuItem("Quit", "Cmd Q")) ApplicationWindow::requestQuit();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("File")) {
			
			if (ImGui::MenuItem("New Project", "Cmd N")) Project::createNew();
			if (ImGui::MenuItem("Open Project...", "Cmd O")) Project::Gui::load();
			
			ImGui::Separator();
			 
			ImGui::BeginDisabled(!Project::hasFilePath());
			if (ImGui::MenuItem("Save", "Cmd S")) Project::save();
			ImGui::EndDisabled();
			
			if (ImGui::MenuItem("Save As...", "Cmd Shift S")) Project::Gui::saveAs();
			 
			ImGui::Separator();
			
			ImGui::BeginDisabled(!Project::hasFilePath());
			if (ImGui::MenuItem("Reload Saved", "Cmd Shift R")) Project::reloadSaved();
			ImGui::EndDisabled();

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			
			static char undoMenuString[256];
			if(CommandHistory::canUndo()) sprintf(undoMenuString, "Undo %s", CommandHistory::getUndoableCommand()->getName().c_str());
			else sprintf(undoMenuString, "Undo");
			ImGui::BeginDisabled(!CommandHistory::canUndo());
			if(ImGui::MenuItem(undoMenuString, "Cmd Z")) CommandHistory::undo();
			ImGui::EndDisabled();
			
			static char redoMenuString[256];
			if(CommandHistory::canRedo()) sprintf(redoMenuString, "Redo %s", CommandHistory::getRedoableCommand()->getName().c_str());
			else sprintf(redoMenuString, "Redo");
			ImGui::BeginDisabled(!CommandHistory::canRedo());
			if(ImGui::MenuItem(redoMenuString, "Cmd Shift Z")) CommandHistory::redo();
			ImGui::EndDisabled();
			
			ImGui::Separator();
			
			if(Environnement::isEditorLocked()) {
				if(ImGui::MenuItem("Show Environnement Editor", "Cmd Shift U")) Environnement::Gui::UnlockEditorPopup::get()->open();
			}
			else if(ImGui::MenuItem("Hide Environnement Editor", "Cmd Shift U")) Environnement::lockEditor();
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("Plot")){
			
			if(ImGui::MenuItem("New Plot")) Project::createNewPlot();
			
			ImGui::Separator();
			
			auto& plots = Project::getPlots();
			for(int i = 0; i < plots.size(); i++){
				auto plot = plots[i];
				ImGui::PushID(i);
				if(ImGui::BeginMenu(plot->getName())){
					
					if(ImGui::MenuItem("Make Current", nullptr, plot->isCurrent())) Project::setCurrentPlot(plot);
					if(ImGui::MenuItem("Rename")){}
					if(ImGui::MenuItem("Duplicate")){}
					if(ImGui::MenuItem("Delete")){}
					
					ImGui::EndMenu();
				}
				ImGui::PopID();
			}
			
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("View")){
			if(ImGui::MenuItem("New layout")) LayoutManager::capture();
            
            ImGui::MenuItem("Lock Current Layout", nullptr, &LayoutManager::b_lockLayout);
            
			ImGui::Separator();
			
			if(ImGui::MenuItem("Set factory layout")) Gui::setDefaultLayout();
			
			if(auto defaultLayout = LayoutManager::getDefaultLayout()){
				if(ImGui::MenuItem("Set default layout")) defaultLayout->makeActive();
			}
			
			if(auto activeLayout = LayoutManager::getCurrentLayout()){
				if(ImGui::MenuItem("Reset current layout")) activeLayout->makeActive();
			}
			
			if(!LayoutManager::getLayouts().empty()) ImGui::Separator();
			
			std::shared_ptr<Layout> removedLayout = nullptr;
			for(auto& layout : LayoutManager::getLayouts()){
				if(ImGui::BeginMenu(layout->name)){
					
					ImGui::BeginDisabled(layout->isActive());
					if(ImGui::MenuItem("Make Active", nullptr, layout->isActive())) layout->makeActive();
					ImGui::EndDisabled();
					
					ImGui::BeginDisabled(layout->isDefault());
					if(ImGui::MenuItem("Make Default", nullptr, layout->isDefault())) layout->makeDefault();
					ImGui::EndDisabled();
					
					if(layout->isActive()) if(ImGui::MenuItem("Overwrite")) layout->overwrite();
					if(ImGui::MenuItem("Rename")) layout->edit();
					if(ImGui::MenuItem("Remove")) removedLayout = layout;
					ImGui::EndMenu();
				}
			}
			if(removedLayout) removedLayout->remove();
			
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("Window")){
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::Text("Windows :");
			ImGui::PopStyleColor();
			for(auto& window : WindowManager::getWindowDictionnary()){
				bool b_open = window->isOpen();
				if(ImGui::MenuItem(window->name.c_str(), nullptr, &b_open)){
					if(b_open) window->open();
					else window->close();
				}
			}
			
			ImGui::EndMenu();
		}
		
		static bool imguiDemoWindowOpen = false;
		static bool imguiMetricsWindowOpen = false;
		static bool implotDemoWindowOpen = false;
		
		if (ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT) && ImGui::IsKeyDown(GLFW_KEY_LEFT_SUPER)) {
			if (ImGui::BeginMenu("Utilities")) {
				if (ImGui::MenuItem("ImGui Demo Window", nullptr, &imguiDemoWindowOpen))		imguiDemoWindowOpen = true;
				if (ImGui::MenuItem("ImGui Metrics Window", nullptr, &imguiMetricsWindowOpen))	imguiMetricsWindowOpen = true;
				if (ImGui::MenuItem("ImPlot Demo Window", nullptr, &implotDemoWindowOpen))		implotDemoWindowOpen = true;
				ImGui::EndMenu();
			}
		}
		ImGui::EndMenuBar();
		
		static KeyboardShortcut quitShortcut(GLFW_KEY_A, KeyboardShortcut::Modifier::SUPER);
		if(quitShortcut.isTriggered()) ApplicationWindow::requestQuit();
		
		static KeyboardShortcut newProjectShortcut(GLFW_KEY_N, KeyboardShortcut::Modifier::SUPER);
		if(newProjectShortcut.isTriggered()) Project::createNew();
		
		static KeyboardShortcut openProjectShortcut(GLFW_KEY_O, KeyboardShortcut::Modifier::SUPER);
		if(openProjectShortcut.isTriggered()) Project::Gui::load();
		
		static KeyboardShortcut saveAsShortcut(GLFW_KEY_S, KeyboardShortcut::Modifier::SUPER, KeyboardShortcut::Modifier::SHIFT);
		if(saveAsShortcut.isTriggered()) Project::Gui::saveAs();
		
		static KeyboardShortcut saveShortcut(GLFW_KEY_S, KeyboardShortcut::Modifier::SUPER);
		if(saveShortcut.isTriggered()) Project::Gui::save();
		
		static KeyboardShortcut reloadSavedShortcut(GLFW_KEY_R, KeyboardShortcut::Modifier::SUPER, KeyboardShortcut::Modifier::SHIFT);
		if(reloadSavedShortcut.isTriggered()) Project::reloadSaved();
		
		static KeyboardShortcut undoShortcut(GLFW_KEY_W, KeyboardShortcut::Modifier::SUPER);
		if(undoShortcut.isTriggered()) CommandHistory::undo();

		static KeyboardShortcut redoShortcut(GLFW_KEY_W, KeyboardShortcut::Modifier::SUPER, KeyboardShortcut::Modifier::SHIFT);
		if(redoShortcut.isTriggered()) CommandHistory::redo();
		
		static KeyboardShortcut unlockEditorShortcut(GLFW_KEY_U, KeyboardShortcut::Modifier::SUPER, KeyboardShortcut::Modifier::SHIFT);
		if(unlockEditorShortcut.isTriggered()){
			if(Environnement::isEditorLocked()) Environnement::Gui::UnlockEditorPopup::get()->open();
			else Environnement::lockEditor();
		}
		
		//utility windows
		if (imguiDemoWindowOpen) ImGui::ShowDemoWindow(&imguiDemoWindowOpen);
		if (imguiMetricsWindowOpen) ImGui::ShowMetricsWindow(&imguiMetricsWindowOpen);
		if (implotDemoWindowOpen) ImPlot::ShowDemoWindow(&implotDemoWindowOpen);

	}


}
