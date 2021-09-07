#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_node_editor.h>
#include <implot.h>
#include "Framework/Fonts.h"

void gui(bool closeWindowRequest);

void mainMenuBar(bool closeWindowRequest);
void quitApplicationModal(bool closeWindowRequest);
void aboutModal(bool openModal);

void etherCatSlaves();
void etherCatMetrics();
void etherCatParameters(bool resetList);
void etherCatStartModal();


namespace ImGuiNodeEditor {
	void CreateContext();
	void DestroyContext();
}
void nodeGraph();
void nodeEditor(bool alwaysShowValues);
void drawNodes(bool alwaysShowValues);
void drawLinks();
void createLink();
void deleteLink();
void deleteNode();
void DrawIcon(ImDrawList* drawList, const glm::vec2& a, const glm::vec2& b, int type, bool filled, ImU32 color, ImU32 innerColor);

void getSelectedNodes();
void nodeAdder();

void log();

void mainWindow();
void toolbar(float height);

void sequencer();

