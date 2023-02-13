#include <pch.h>

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
#include "Gui/Plot/PlotGui.h"
#include "Project/Editor/CommandHistory.h"

#include "Animation/Manoeuvre.h"
#include "Plot/ManoeuvreList.h"

#include "KeyboardShortcut.h"

#include "Layout.h"

#include "SnakeGame.h"

#include "Project/Stacato.h"
#include "Project/StacatoProject.h"
#include "Project/Workspace/Application.h"

namespace Gui {

	void menuBar() {

		auto currentProject = Stacato::Workspace::getCurrentProject();
		
		ImGui::BeginMenuBar();
		if (ImGui::BeginMenu("Stacato")) {
			if (ImGui::MenuItem("About")) Gui::AboutPopup::get()->open();
			ImGui::Separator();
			if (ImGui::MenuItem("Quit", "Cmd Q")) Application::requestQuit();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("File")) {
			
			if (ImGui::MenuItem("New Project", "Cmd N")) Stacato::Workspace::createNewProject();
			if (ImGui::MenuItem("Open Project...", "Cmd O")) Project::Gui::load();
			
			ImGui::Separator();
			 
			
			ImGui::BeginDisabled(currentProject == nullptr || !currentProject->hasFilePath());
			if (ImGui::MenuItem("Save", "Cmd S")) currentProject->writeFile();
			ImGui::EndDisabled();
			
			if (ImGui::MenuItem("Save As...", "Cmd Shift S")) Project::Gui::saveAs();
			 
			ImGui::Separator();
			
			ImGui::BeginDisabled(currentProject == nullptr || !currentProject->hasFilePath());
			if (ImGui::MenuItem("Reload Saved", "Cmd Shift R")) {/*Project::reloadSaved();*/}
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
			
			if(ImGui::MenuItem("Lock Plot Editing", nullptr, currentProject->isPlotEditLocked())){
				if(currentProject->isPlotEditLocked()) currentProject->unlockPlotEdit();
				else currentProject->lockPlotEdit();
			}
			
            if(!currentProject->isPlotEditLocked()){
                if(ImGui::MenuItem("Create New Plot")) PlotGui::NewPlotPopup::get()->open();
            }
			
			ImGui::Separator();
			
			auto& plots = Stacato::Workspace::getCurrentProject()->getPlots();
			
			ImGui::BeginDisabled();
			ImGui::Text("Current Plot:");
			ImGui::EndDisabled();
			
			if(currentProject->isPlotEditLocked()){
				
				for(int i = 0; i < plots.size(); i++){
					auto plot = plots[i];
					ImGui::PushID(i);
					
					bool b_current = plot->isCurrent();
					
					if(b_current) ImGui::PushStyleColor(ImGuiCol_Text, Colors::yellow);
					if(ImGui::MenuItem(plot->getName(), nullptr, plot->isCurrent())) Stacato::Workspace::getCurrentProject()->setCurrentPlot(plot);
					if(b_current) ImGui::PopStyleColor();
					
					ImGui::PopID();
				}
				
			}else{
			
				for(int i = 0; i < plots.size(); i++){
					auto plot = plots[i];
					ImGui::PushID(i);
					
					bool b_current = plot->isCurrent();
					
					if(b_current) ImGui::PushStyleColor(ImGuiCol_Text, Colors::yellow);
					if(ImGui::BeginMenu(plot->getName())){
						if(b_current) ImGui::PopStyleColor();
						
						if(ImGui::MenuItem("Make Current", nullptr, plot->isCurrent())) {
							Stacato::Workspace::getCurrentProject()->setCurrentPlot(plot);
						}
						
						if(ImGui::MenuItem("Rename")) {
							PlotGui::PlotEditorPopup::open(plot);
						}
						if(ImGui::MenuItem("Duplicate")){
							Stacato::Workspace::getCurrentProject()->duplicatePlot(plot);
						}
						if(ImGui::MenuItem("Delete")) {
							PlotGui::PlotDeletePopup::open(plot);
						}
						
						ImGui::EndMenu();
					} else if(b_current) ImGui::PopStyleColor();
					ImGui::PopID();
				}
			}
			
			/*
            if(!currentProject->isPlotEditLocked()){
            
                ImGui::Separator();
                
                ImGui::PushFont(Fonts::sansBold15);
                if(Project::getClipboardManeouvre() == nullptr) {
                    ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
                    ImGui::Text("Clipboard empty.");
                }else{
                    ImGui::PushStyleColor(ImGuiCol_Text, Colors::yellow);
                    ImGui::Text("Clipboard: %s", Project::getClipboardManeouvre()->getName());
                }
                ImGui::PopStyleColor();
                ImGui::PopFont();
                
				auto currentPlot = StacatoEditor::getCurrentProject()->getCurrentPlot();
                
                ImGui::BeginDisabled(currentPlot->getSelectedManoeuvre() == nullptr);
                if(ImGui::MenuItem("Copy Manoeuvre", "Cmd+C")) Project::pushManoeuvreToClipboard(currentPlot->getSelectedManoeuvre());
                ImGui::EndDisabled();
                
                ImGui::BeginDisabled(Project::getClipboardManeouvre() == nullptr);
                if(ImGui::MenuItem("Paste Manoeuvre", "Cmd+V")) {
                    auto manoeuvreList = StacatoEditor::getCurrentProject()->getCurrentPlot()->getManoeuvreList();
                    manoeuvreList->pasteManoeuvre(Project::getClipboardManeouvre());
                }
                ImGui::EndDisabled();
                
            }
			 */
			
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("View")){
			if(ImGui::MenuItem("New layout")) LayoutManager::capture();
            
            ImGui::MenuItem("Lock Current Layout", nullptr, &LayoutManager::b_lockLayout);
            
			ImGui::Separator();
			
			if(auto defaultLayout = LayoutManager::getDefaultLayout()){
				if(ImGui::MenuItem("Set default layout")) defaultLayout->makeActive();
			}
			
			if(auto activeLayout = LayoutManager::getCurrentLayout()){
				if(ImGui::MenuItem("Reset current layout")) activeLayout->makeActive();
			}
			
			if(!LayoutManager::getLayouts().empty()) ImGui::Separator();
			
			std::shared_ptr<Layout> removedLayout = nullptr;
			for(auto& layout : LayoutManager::getLayouts()){
				
				bool b_active = layout->isActive();
				
				if(b_active) ImGui::PushStyleColor(ImGuiCol_Text, Colors::yellow);
				if(ImGui::BeginMenu(layout->name)){
					if(b_active) ImGui::PopStyleColor();
					
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
				}else if(b_active) ImGui::PopStyleColor();
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
		if(quitShortcut.isTriggered()) Application::requestQuit();
		
		static KeyboardShortcut newProjectShortcut(GLFW_KEY_N, KeyboardShortcut::Modifier::SUPER);
		if(newProjectShortcut.isTriggered()) Stacato::Workspace::createNewProject();
		
		static KeyboardShortcut openProjectShortcut(GLFW_KEY_O, KeyboardShortcut::Modifier::SUPER);
		if(openProjectShortcut.isTriggered()) Project::Gui::load();
		
		static KeyboardShortcut saveAsShortcut(GLFW_KEY_S, KeyboardShortcut::Modifier::SUPER, KeyboardShortcut::Modifier::SHIFT);
		if(saveAsShortcut.isTriggered()) Project::Gui::saveAs();
		
		static KeyboardShortcut saveShortcut(GLFW_KEY_S, KeyboardShortcut::Modifier::SUPER);
		if(saveShortcut.isTriggered()) Project::Gui::save();
		
		//static KeyboardShortcut reloadSavedShortcut(GLFW_KEY_R, KeyboardShortcut::Modifier::SUPER, KeyboardShortcut::Modifier::SHIFT);
		//if(reloadSavedShortcut.isTriggered()) Project::reloadSaved();
		
		static KeyboardShortcut undoShortcut(GLFW_KEY_W, KeyboardShortcut::Modifier::SUPER);
		if(undoShortcut.isTriggered()) CommandHistory::undo();

		static KeyboardShortcut redoShortcut(GLFW_KEY_W, KeyboardShortcut::Modifier::SUPER, KeyboardShortcut::Modifier::SHIFT);
		if(redoShortcut.isTriggered()) CommandHistory::redo();
		
		static KeyboardShortcut unlockEditorShortcut(GLFW_KEY_U, KeyboardShortcut::Modifier::SUPER, KeyboardShortcut::Modifier::SHIFT);
		if(unlockEditorShortcut.isTriggered()){
			if(Environnement::isEditorLocked()) Environnement::Gui::UnlockEditorPopup::get()->open();
			else Environnement::lockEditor();
		}
		
		if(ImGui::IsKeyDown(ImGuiKey_UpArrow) &&
		   ImGui::IsKeyDown(ImGuiKey_DownArrow) &&
		   ImGui::IsKeyDown(ImGuiKey_LeftArrow) &&
		   ImGui::IsKeyDown(ImGuiKey_RightArrow)){
			SnakeGameWindow::get()->open();
		}
		
		//utility windows
		if (imguiDemoWindowOpen) ImGui::ShowDemoWindow(&imguiDemoWindowOpen);
		if (imguiMetricsWindowOpen) ImGui::ShowMetricsWindow(&imguiMetricsWindowOpen);
		if (implotDemoWindowOpen) ImPlot::ShowDemoWindow(&implotDemoWindowOpen);

	}


}
