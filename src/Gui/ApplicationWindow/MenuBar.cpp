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




#include "Legato/Editor/ProjectComponent.h"
#include "Legato/Editor/ListComponent.h"

class InterfaceThing : public Legato::Component{
	COMPONENT_INTERFACE(InterfaceThing)
	virtual void onConstruction() override {
		Component::onConstruction();
	}
	virtual void copyFrom(Ptr<Component> source) override {
		Component::copyFrom(source);
	}
	virtual bool onSerialization() override {
		Component::onSerialization();
		return true;
	}
	virtual bool onDeserialization() override {
		Component::onDeserialization();
		return true;
	}
};

 
class ChildThing : public Legato::Component{
	COMPONENT_IMPLEMENTATION(ChildThing)
	virtual void onConstruction() override {
		Component::onConstruction();
	}
	virtual void copyFrom(Ptr<Component> source) override {
		Component::copyFrom(source);
		auto src = source->cast<ChildThing>();
		value = src->value;
	}
	virtual bool onSerialization() override {
		Component::onSerialization();
		serializeAttribute("Attrib1", 1234);
		return true;
	}
	virtual bool onDeserialization() override {
		Component::onDeserialization();
		int result;
		deserializeAttribute("Attrib1", result);
		return true;
	}
public:
	int value = 0;
};


class RealThing : public InterfaceThing{
	COMPONENT_IMPLEMENTATION(RealThing)
	
	virtual void onConstruction() override {
		InterfaceThing::onConstruction();
		childThing = ChildThing::make();
		addChild(childThing);
	}
	virtual void copyFrom(Ptr<Component> source) override {
		InterfaceThing::copyFrom(source);
	}
	virtual bool onSerialization() override {
		return InterfaceThing::onSerialization();
	}
	virtual bool onDeserialization() override {
		return InterfaceThing::onDeserialization();
	}
public:
	Ptr<ChildThing> childThing;
	
};

class ProjectThing : public Legato::Project{
	COMPONENT_IMPLEMENTATION(ProjectThing)
public:
	
};

class FileThing : public Legato::File{
	COMPONENT_IMPLEMENTATION(FileThing)
public:
};

void drawChildren(Ptr<Legato::Component> parent){
	std::string displayString;
	std::string completePath;
	bool b_file = false;

	if(auto project = parent->cast<Legato::Project>()){
		displayString = "[Project:" + project->getClassName() + "] dir=" + project->getDirectoryName().string();
		completePath = project->getPath();
		b_file = true;
	}
	else if(auto directory = parent->cast<Legato::Directory>()){
		displayString = "[Directory] " + directory->getDirectoryName().string() + "/";
	}
	else if(auto file = parent->cast<Legato::File>()){
		displayString = "[File:" + file->getClassName() + "] " + file->getFileName().string() + " <" + file->getIdentifier() + ">";
		completePath = file->getPath();
		b_file = true;
	}
	else{
		displayString = "[" + parent->getClassName() + "] <" + parent->getIdentifier() + ">";
	}
	
	if(parent->hasChildren()){
		if(ImGui::TreeNode(displayString.c_str())){
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			if(b_file) ImGui::Text("Complete Path : %s", completePath.c_str());
			ImGui::PopStyleColor();
			auto& children = parent->getChildren();
			for(int i = 0; i < children.size(); i++){
				ImGui::PushID(i);
				drawChildren(children[i]);
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
	}
	else {
		ImGui::TreePush("Tree");
		ImGui::Text("%s", displayString.c_str());
		ImGui::TreePop();
	}
}

namespace Stacato::Gui {

	void menuBar() {
		
		
		auto currentProject = Stacato::Editor::getCurrentProject();
		

		static KeyboardShortcut quitShortcut(ImGuiKey_Q);
		if(quitShortcut.isTriggered()) Application::requestQuit();
		
		static KeyboardShortcut newProjectShortcut(ImGuiKey_N);
		if(newProjectShortcut.isTriggered()) Stacato::Editor::createNewProject();
		
		static KeyboardShortcut openProjectShortcut(ImGuiKey_O);
		if(openProjectShortcut.isTriggered()) Stacato::Gui::load();
		
		static KeyboardShortcut saveAsShortcut(ImGuiKey_S, KeyboardShortcut::Modifier::CTRL_CMD, KeyboardShortcut::Modifier::SHIFT);
		if(saveAsShortcut.isTriggered()) Stacato::Gui::saveAs();
		
		static KeyboardShortcut saveShortcut(ImGuiKey_S);
		if(saveShortcut.isTriggered()) Stacato::Gui::save();
		
		//static KeyboardShortcut reloadSavedShortcut(GLFW_KEY_R, KeyboardShortcut::Modifier::SUPER, KeyboardShortcut::Modifier::SHIFT);
		//if(reloadSavedShortcut.isTriggered()) Project::reloadSaved();
		
		static KeyboardShortcut undoShortcut(ImGuiKey_W);
		if(undoShortcut.isTriggered()) CommandHistory::undo();

		static KeyboardShortcut redoShortcut(ImGuiKey_W, KeyboardShortcut::Modifier::CTRL_CMD, KeyboardShortcut::Modifier::SHIFT);
		if(redoShortcut.isTriggered()) CommandHistory::redo();
		
		static KeyboardShortcut unlockEditorShortcut(ImGuiKey_U, KeyboardShortcut::Modifier::CTRL_CMD, KeyboardShortcut::Modifier::SHIFT);
		if(unlockEditorShortcut.isTriggered()){
			if(Stacato::Editor::isLocked()) Environnement::Gui::UnlockEditorPopup::get()->open();
			else Stacato::Editor::lock();
		}




		ImGui::BeginMenuBar();
		if (ImGui::BeginMenu("Stacato")) {
			if (ImGui::MenuItem("About")) Stacato::Gui::AboutPopup::get()->open();
			ImGui::Separator();
			if (ImGui::MenuItem("Quit", quitShortcut.getString())) Application::requestQuit();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("File")) {
			
			if (ImGui::MenuItem("New Project", newProjectShortcut.getString())) Stacato::Editor::createNewProject();
			if (ImGui::MenuItem("Open Project...", openProjectShortcut.getString())) Stacato::Gui::load();
			
			ImGui::Separator();
			 
			
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::PushFont(Fonts::sansBold15);
			if(currentProject == nullptr) ImGui::Text("No Project Loaded.");
			else ImGui::Text("Current Project : %s", currentProject->getFilePath().c_str());
			ImGui::PopFont();
			ImGui::PopStyleColor();
			
			ImGui::BeginDisabled(currentProject == nullptr);
			
			ImGui::BeginDisabled(!currentProject->hasFilePath());
			if (ImGui::MenuItem("Save", saveShortcut.getString())) currentProject->writeFile();
			ImGui::EndDisabled();
			
			if (ImGui::MenuItem("Save As...", saveAsShortcut.getString())) Stacato::Gui::saveAs();
			
			//ImGui::BeginDisabled(!currentProject->hasFilePath());
			//if (ImGui::MenuItem("Reload Saved", "Cmd Shift R")) {/*Project::reloadSaved();*/}
			//ImGui::EndDisabled();
			
			//if(ImGui::MenuItem("Close")){}
			
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
				if(ImGui::MenuItem("Show Environnement Editor", unlockEditorShortcut.getString())) Environnement::Gui::UnlockEditorPopup::get()->open();
			}
			else if(ImGui::MenuItem("Hide Environnement Editor", unlockEditorShortcut.getString())) Stacato::Editor::lock();
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
			
			if(ImGui::MenuItem("Fullscreen")){
				Legato::Gui::makeMainWindowFullscreen();
			}

/*
			if(ImGui::MenuItem("Lock Window Positions", nullptr, Legato::Gui::WindowManager::areWindowsLocked())){
				if(Legato::Gui::WindowManager::areWindowsLocked()){
					Legato::Gui::WindowManager::unlockWindows();
				}else{
					Legato::Gui::WindowManager::lockWindows();
				}
			}
*/			
			
			
			if(Stacato::Editor::hasCurrentProject()){
				
				auto project = Stacato::Editor::getCurrentProject();
				auto layoutList = project->getLayouts();
				auto& layouts = layoutList->get();
				auto currentLayout = layoutList->getCurrent();
				auto defaultLayout = layoutList->getDefault();
				
				ImGui::Separator();
				if(ImGui::MenuItem("Capture New Layout")) layoutList->captureNew();
				
				
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
				if(ImGui::MenuItem("Delete Layout")) layoutList->remove(currentLayout);
				if(ImGui::MenuItem("Rename Layout")) RenameLayoutPopup::open(currentLayout);
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
		
		if (ImGui::IsKeyDown(ImGuiKey_LeftAlt)) {
			if (ImGui::BeginMenu("Utilities")) {
				if (ImGui::MenuItem("ImGui Demo Window", nullptr, &imguiDemoWindowOpen))		imguiDemoWindowOpen = true;
				if (ImGui::MenuItem("ImGui Metrics Window", nullptr, &imguiMetricsWindowOpen))	imguiMetricsWindowOpen = true;
				if (ImGui::MenuItem("ImPlot Demo Window", nullptr, &implotDemoWindowOpen))		implotDemoWindowOpen = true;
				ImGui::EndMenu();
			}
		}
		
		
		
		
		ImGui::SameLine();
		if(ImGui::Button("Test")) ImGui::OpenPopup("Testing");
		if(ImGui::BeginPopup("Testing")){
			static bool b_testInit = false;
			static Ptr<ProjectThing> proj = ProjectThing::make();
			if(!b_testInit){
				b_testInit = true;
				proj = ProjectThing::make();
				proj->setPath("LegatoTestProject.stacato");
				
				auto dir1 = Legato::Directory::make("folder1");
				proj->addChild(dir1);
				auto file1 = FileThing::make();
				file1->setFileName("test.file");
				dir1->addChild(file1);
				auto rt = RealThing::make();
				file1->addChild(rt);
				
				auto dir2 = Legato::Directory::make("folder2");
				proj->addChild(dir2);
				auto file2 = FileThing::make();
				file2->setFileName("ListFile.test");
				dir2->addChild(file2);
				auto realList = Legato::List<RealThing>::make("ThingList", "ReaaaalThinnnnng");
				file2->addChild(realList);
				realList->addEntry(RealThing::make());
				realList->addEntry(RealThing::make());
				realList->addEntry(RealThing::make(),1);
				
			}
			drawChildren(proj);
			
			if(ImGui::Button("Serialize")) {
				proj->serialize();
			}
			ImGui::SameLine();
			if(ImGui::Button("Deserialize")) {
				proj->deserialize();
			}
			
			ImGui::EndPopup();
		}
		
		
		ImGui::EndMenuBar();
		
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
