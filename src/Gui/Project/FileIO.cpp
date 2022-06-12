#include <pch.h>

#include "ProjectGui.h"

#include <imgui.h>
#include <GLFW/glfw3.h>
#include "Gui/Utilities/FileDialog.h"

#include "Project/Project.h"

namespace Project::Gui{

	void load(){
		FileDialog::FilePath path;
		FileDialog::FileTypeFilter filter("Stacato Project File", "stacato");
		if(FileDialog::load(path, filter)) Project::load(path.path);
	}

	bool save(){
		if(hasFilePath()) return Project::save();
		return saveAs();
	}

	bool saveAs(){
		FileDialog::FilePath path;
		FileDialog::FileTypeFilter filter("Stacato Project File", "stacato");
		if(FileDialog::save(path, filter, "project")) {
			Project::saveAs(path.path);
			return true;
		}
		return false;
	}

	void CloseProjectPopup::drawContent(){
		ImGui::Text("Do you really want to close the current project ?");
		ImGui::Text("Proceeding will stop motion and discard any unsaved changes");
		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) close();
		ImGui::SameLine();
		if (ImGui::Button("Close without saving")) {
			Project::confirmNewProjectRequest();
			close();
		}
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2, 0.5, 0.0, 1.0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2, 0.4, 0.1, 1.0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3, 0.6, 0.2, 1.0));
		if (ImGui::Button("Save and Close") || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) {
			bool b_saved = Project::Gui::save();
			if(b_saved) Project::confirmNewProjectRequest();
			close();
		}
		ImGui::PopStyleColor(3);
	}

}
