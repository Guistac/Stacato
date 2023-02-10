#include <pch.h>

#include "Stacato.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Images.h"
#include "Visualizer/Visualizer.h"

#include "Workspace/Gui.h"

#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 1
#define OPENGL_VERSION_STRING "#version 410 core"

namespace Stacato::Gui{

void initialize(){
	Fonts::load(NewGui::getScale());
	Images::load();
	ImGui::StyleColorsDark();
	
	Environnement::StageVisualizer::initialize(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR);
}
void terminate(){}
void preFrame(){}
void postFrame(){}

};
