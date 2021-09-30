#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_node_editor.h>
#include <implot.h>

#include "Framework/Fonts.h"
#include "NodeEditor/NodeEditorGui.h"

extern std::mutex GuiMutex;

void gui(bool closeWindowRequest);

void mainMenuBar(bool closeWindowRequest);
void quitApplicationModal(bool closeWindowRequest);

void aboutModal(bool openModal);

void log();

void mainWindow();
void toolbar(float height);

void sequencer();

