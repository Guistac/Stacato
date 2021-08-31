#include <pch.h>

#include "GUI.h"

#include "Gui/Guiwindow.h"

#include <imgui.h>
#include <imgui_node_editor.h>

void nodeGraph() {
	ImGui::Text("Node Graph Editor");

	ax::NodeEditor::SetCurrentEditor(GuiWindow::nodeEditorContext);

	ax::NodeEditor::Begin("Node Editor");




	ax::NodeEditor::End();
}