#pragma once

#include <GL/glew.h>
#include <imgui.h>

namespace Images{

	void load();

	//extern unsigned int keyIcon;

	struct Image{
		int width;
		int height;
		GLuint textureID;
		ImTextureID getID(){ return (void*)(intptr_t)textureID; }
		ImVec2 getSize(){ return ImVec2(width, height); }
	};

	extern Image KeyIcon;
	extern Image TargetIcon;
	extern Image SequenceIcon;

}
