#include <pch.h>

#include "Images.h"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Images{

	// Simple helper function to load an image into a OpenGL texture with common settings
	bool LoadTextureFromFile(const char* filename, Image& output){
		
		// Load from file
		unsigned char* image_data = stbi_load(filename, &output.width, &output.height, NULL, 4);
		if (image_data == NULL) return false;

		// Create a OpenGL texture identifier
		glGenTextures(1, &output.textureID);
		glBindTexture(GL_TEXTURE_2D, output.textureID);

		// Setup filtering parameters for display
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

		// Upload pixels into texture
	#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, output.width, output.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
		stbi_image_free(image_data);
		
		return true;
	}

	Image KeyIcon;
	Image TargetIcon;
	Image SequenceIcon;
	Image StacatoIcon;
	Image WarningIcon;

	void load(){
		LoadTextureFromFile("icons/KeyIcon.png", KeyIcon);
		LoadTextureFromFile("icons/TimeIcon.png", TargetIcon);
		LoadTextureFromFile("icons/SequenceIcon.png", SequenceIcon);
		LoadTextureFromFile("icons/StacatoIcon.png", StacatoIcon);
		LoadTextureFromFile("icons/WarningIcon.png", WarningIcon);
	}

	

}
