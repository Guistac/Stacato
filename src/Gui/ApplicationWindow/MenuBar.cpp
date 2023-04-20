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

#include "Legato/Gui/Layout.h"

#include "SnakeGame.h"

#include "Stacato/StacatoGui.h"
#include "Stacato/Project/StacatoProject.h"
#include "Legato/Application.h"
#include "Stacato/StacatoEditor.h"

namespace Stacato::Gui {

	void menuBar() {

		auto currentProject = Stacato::Editor::getCurrentProject();
		
		ImGui::BeginMenuBar();
		if (ImGui::BeginMenu("Stacato")) {
			if (ImGui::MenuItem("About")) Stacato::Gui::AboutPopup::get()->open();
			ImGui::Separator();
			if (ImGui::MenuItem("Quit", "Cmd Q")) Application::requestQuit();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("File")) {
			
			if (ImGui::MenuItem("New Project", "Cmd N")) Stacato::Editor::createNewProject();
			if (ImGui::MenuItem("Open Project...", "Cmd O")) Stacato::Gui::load();
			
			ImGui::Separator();
			 
			
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::PushFont(Fonts::sansBold15);
			if(currentProject == nullptr) ImGui::Text("No Project Loaded.");
			else ImGui::Text("Current Project : %s", currentProject->getFilePath().c_str());
			ImGui::PopFont();
			ImGui::PopStyleColor();
			
			ImGui::BeginDisabled(currentProject == nullptr);
			
			ImGui::BeginDisabled(!currentProject->hasFilePath());
			if (ImGui::MenuItem("Save", "Cmd S")) currentProject->writeFile();
			ImGui::EndDisabled();
			
			if (ImGui::MenuItem("Save As...", "Cmd Shift S")) Stacato::Gui::saveAs();
			
			ImGui::BeginDisabled(!currentProject->hasFilePath());
			if (ImGui::MenuItem("Reload Saved", "Cmd Shift R")) {/*Project::reloadSaved();*/}
			ImGui::EndDisabled();
			
			if(ImGui::MenuItem("Close")){
				
			}
			
			ImGui::EndDisabled();
			
			ImGui::Separator();
			
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("Loaded Files :");
			ImGui::PopFont();
			ImGui::PopStyleColor();
			
			auto& files = ::Workspace::getFiles();
			auto currentProject = Stacato::Editor::getCurrentProject();
			for(int i = 0; i < files.size(); i++){
				ImGui::PushID(i);
				auto file = files[i];
				bool isCurrent = currentProject == file;
				std::string name = file->getFilePath().string();
				if(name.empty()) name = "[Unsaved Project]";
				if(ImGui::MenuItem(name.c_str(), nullptr, isCurrent)){
					if(auto project = std::dynamic_pointer_cast<StacatoProject>(file)){
						Stacato::Editor::openProject(project);
					}
				}
				ImGui::PopID();
			}
			

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			/*
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
			*/
			
			
			//ImGui::Separator();
			if(Stacato::Editor::isLocked()){
				if(ImGui::MenuItem("Show Environnement Editor", "Cmd Shift U")) Environnement::Gui::UnlockEditorPopup::get()->open();
			}
			else if(ImGui::MenuItem("Hide Environnement Editor", "Cmd Shift U")) Stacato::Editor::lock();
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("Plot")){
			
			if(!Stacato::Editor::hasCurrentProject()){
				ImGui::Text("No Project Loaded");
			}else{
				
				
				if(ImGui::MenuItem("Lock Plot Editing", nullptr, currentProject->isPlotEditLocked())){
					if(currentProject->isPlotEditLocked()) currentProject->unlockPlotEdit();
					else currentProject->lockPlotEdit();
				}
				
				/*
				
				if(!currentProject->isPlotEditLocked()){
					if(ImGui::MenuItem("Create New Plot")) PlotGui::NewPlotPopup::get()->open();
				}
				
				ImGui::Separator();
				
				auto& plots = Stacato::Editor::getCurrentProject()->getPlots();
				
				ImGui::BeginDisabled();
				ImGui::Text("Current Plot:");
				ImGui::EndDisabled();
				
				if(currentProject->isPlotEditLocked()){
					
					for(int i = 0; i < plots.size(); i++){
						auto plot = plots[i];
						ImGui::PushID(i);
						
						bool b_current = plot->isCurrent();
						
						if(b_current) ImGui::PushStyleColor(ImGuiCol_Text, Colors::yellow);
						if(ImGui::MenuItem(plot->getName(), nullptr, plot->isCurrent())) Stacato::Editor::getCurrentProject()->setCurrentPlot(plot);
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
								Stacato::Editor::getCurrentProject()->setCurrentPlot(plot);
							}
							
							if(ImGui::MenuItem("Rename")) {
								PlotGui::PlotEditorPopup::open(plot);
							}
							if(ImGui::MenuItem("Duplicate")){
								Stacato::Editor::getCurrentProject()->duplicatePlot(plot);
							}
							if(ImGui::MenuItem("Delete")) {
								PlotGui::PlotDeletePopup::open(plot);
							}
							
							ImGui::EndMenu();
						} else if(b_current) ImGui::PopStyleColor();
						ImGui::PopID();
					}
				}
				
				 */
				 
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
			}
			
			ImGui::EndMenu();
		}
		
		if(ImGui::BeginMenu("View")){
			
			if(ImGui::MenuItem("Lock Window Positions", nullptr, Legato::Gui::WindowManager::areWindowsLocked())){
				if(Legato::Gui::WindowManager::areWindowsLocked()){
					Legato::Gui::WindowManager::unlockWindows();
				}else{
					Legato::Gui::WindowManager::lockWindows();
				}
			}
			
			
			
			if(Stacato::Editor::hasCurrentProject()){
				
				auto project = Stacato::Editor::getCurrentProject();
				auto layoutList = project->getLayouts();
				auto& layouts = layoutList->getList();
				auto currentLayout = layoutList->getCurrent();
				auto defaultLayout = layoutList->getDefault();
				
				ImGui::Separator();
				if(ImGui::MenuItem("Capture New Layout")) {
					LayoutCreationPopup::open(project->getLayouts());
				}
				
				
				ImGui::Separator();
				ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
				ImGui::PushFont(Fonts::sansBold15);
				ImGui::Text("Project Layouts :");
				ImGui::PopFont();
				ImGui::PopStyleColor();
				
				for(int i = 0; i < layouts.size(); i++){
					auto& layout = layouts[i];
					bool isCurrent = layout == currentLayout;
					bool isDefault = layout == defaultLayout;
					ImGui::PushID(i);
					if(isCurrent) ImGui::PushStyleColor(ImGuiCol_Text, Colors::yellow);
					if(ImGui::MenuItem(layout->getName().c_str(), isDefault ? "Default" : "", isCurrent, !isCurrent)){
						layoutList->makeCurrent(layout);
					}
					if(isCurrent) ImGui::PopStyleColor();
					ImGui::PopID();
				}
			
				ImGui::Separator();
				ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
				ImGui::PushFont(Fonts::sansBold15);
				if(currentLayout) ImGui::Text("Current Layout : %s", currentLayout->getName().c_str());
				else ImGui::Text("No Layout Active.");
				ImGui::PopFont();
				ImGui::PopStyleColor();
			
				ImGui::BeginDisabled(layoutList->getCurrent() == nullptr);
				if(ImGui::MenuItem("Make Default Layout")) layoutList->makeCurrentDefault();
				if(ImGui::MenuItem("Overwrite Layout")) currentLayout->overwrite();
				if(ImGui::MenuItem("Reset Layout")) layoutList->makeCurrent(currentLayout);
				if(ImGui::MenuItem("Delete Layout")) layoutList->remove(currentLayout);
				if(ImGui::MenuItem("Rename Layout")) LayoutEditPopup::open(currentLayout);
				ImGui::EndDisabled();
			}
			
			
			//if(ImGui::MenuItem("New layout")) LayoutManager::capture();
			
			/*
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
			*/
			ImGui::EndMenu();
		}
		 
		if(ImGui::BeginMenu("Window")){
			
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::Text("User Windows :");
			ImGui::PopStyleColor();
			for(auto userWindow : Stacato::Editor::getUserWindows()){
				bool b_open = userWindow->isOpen();
				if(ImGui::MenuItem(userWindow->getName().c_str(), nullptr, &b_open)){
					if(userWindow->isOpen()) userWindow->close();
					else userWindow->open();
				}
			}

			if(!Stacato::Editor::isLocked()){
				ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
				ImGui::Text("Administrator Windows :");
				ImGui::PopStyleColor();
				for(auto adminWindow : Stacato::Editor::getAdministratorWindows()){
					bool b_open = adminWindow->isOpen();
					if(ImGui::MenuItem(adminWindow->getName().c_str(), nullptr, &b_open)){
						if(b_open) adminWindow->open();
						else adminWindow->close();
					}
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
		if(newProjectShortcut.isTriggered()) Stacato::Editor::createNewProject();
		
		static KeyboardShortcut openProjectShortcut(GLFW_KEY_O, KeyboardShortcut::Modifier::SUPER);
		if(openProjectShortcut.isTriggered()) Stacato::Gui::load();
		
		static KeyboardShortcut saveAsShortcut(GLFW_KEY_S, KeyboardShortcut::Modifier::SUPER, KeyboardShortcut::Modifier::SHIFT);
		if(saveAsShortcut.isTriggered()) Stacato::Gui::saveAs();
		
		static KeyboardShortcut saveShortcut(GLFW_KEY_S, KeyboardShortcut::Modifier::SUPER);
		if(saveShortcut.isTriggered()) Stacato::Gui::save();
		
		//static KeyboardShortcut reloadSavedShortcut(GLFW_KEY_R, KeyboardShortcut::Modifier::SUPER, KeyboardShortcut::Modifier::SHIFT);
		//if(reloadSavedShortcut.isTriggered()) Project::reloadSaved();
		
		static KeyboardShortcut undoShortcut(GLFW_KEY_W, KeyboardShortcut::Modifier::SUPER);
		if(undoShortcut.isTriggered()) CommandHistory::undo();

		static KeyboardShortcut redoShortcut(GLFW_KEY_W, KeyboardShortcut::Modifier::SUPER, KeyboardShortcut::Modifier::SHIFT);
		if(redoShortcut.isTriggered()) CommandHistory::redo();
		
		static KeyboardShortcut unlockEditorShortcut(GLFW_KEY_U, KeyboardShortcut::Modifier::SUPER, KeyboardShortcut::Modifier::SHIFT);
		if(unlockEditorShortcut.isTriggered()){
			if(Stacato::Editor::isLocked()) Environnement::Gui::UnlockEditorPopup::get()->open();
			else Stacato::Editor::lock();
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
