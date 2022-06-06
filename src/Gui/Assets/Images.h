#pragma once

#include <GL/glew.h>
#include <imgui.h>

struct Image{
	int width;
	int height;
	GLuint textureID;
	ImTextureID getID(){ return (void*)(intptr_t)textureID; }
	ImVec2 getSize(){ return ImVec2(width, height); }
};

namespace Images{

	void load();

	extern Image KeyIcon;
	extern Image TargetIcon;
	extern Image SequenceIcon;

}
