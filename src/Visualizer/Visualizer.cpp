#include <pch.h>
#include "Visualizer.h"

#include "Scripting/Script.h"
#include "ofRenderer.h"
#include <ofMain.h>

#include "Scripting/CanvasLibrary.h"
#include "Scripting/EnvironnementLibrary.h"

// declare the wrapped modules
extern "C" {
	int luaopen_of(lua_State* L);
	int luaopen_glm(lua_State* L);
}

namespace Environnement::StageVisualizer{



	void initialize(int openGlVersionMajor, int openGlVersionMinor){
		ofRenderer::init(openGlVersionMajor, openGlVersionMinor);
	}

	void terminate(){
		ofRenderer::terminate();
	}


 
	LuaScript script("Stage Visualizer Script");
	ofFbo framebuffer;

	void compile(){
		script.stop();
		script.compile();
		script.stop();
	}

	bool b_shouldStart = false;
	bool b_shouldStop = false;

	void start(){ b_shouldStart = true; }
	void stop(){ b_shouldStop = true; }
	
	void onStart(){
		script.stop();
		script.setLoadLibrairiesCallback([](lua_State* L){
			luaopen_of(L);
			luaopen_glm(L);
			Scripting::CanvasLibrary::openLib(L);
			Scripting::EnvironnementLibrary::openlib(L);
		});
		script.compileAndRun();
		ofRenderer::startRender();
		if(script.checkHasFunction("setup")) script.callFunction("setup");
		ofRenderer::finishRender();
	}

	void onStop(){
		ofRenderer::startRender();
		if(script.checkHasFunction("exit")) script.callFunction("exit");
		ofRenderer::finishRender();
		script.stop();
	}

	void saveScript(const char* filePath){
		script.save(filePath);
	}

	void loadScript(const char* filePath){
		script.stop();
		script.load(filePath);
	}

	void editor(glm::vec2 size_arg){
		ImGui::BeginChild("StageVisualizerScriptEditor", size_arg);
		if(ImGui::Button("Compile")) compile();
		ImGui::SameLine();
		if(ImGui::Button("Start")) start();
		ImGui::SameLine();
		if(ImGui::Button("Stop")) stop();
		script.editor(ImGui::GetContentRegionAvail());
		ImGui::EndChild();
	}

	int frameBufferWidth = 0;
	int frameBufferHeight = 0;
	int mousePosX = 0;
	int mousePosY = 0;
	bool b_mousePressed = false;

	void resizeFrameBuffer(int width, int height){
		frameBufferWidth = width;
		frameBufferHeight = height;
		ofFboSettings settings;
		settings.textureTarget = GL_TEXTURE_2D;
		settings.internalformat = GL_RGBA;
		settings.width = frameBufferWidth;
		settings.height = frameBufferHeight;
		settings.numSamples = 4;
		framebuffer.allocate(settings);
	}

	void canvas(glm::vec2 size_arg, float border, float rounding){
		if(size_arg.x == 0.0 && size_arg.y == 0.0) size_arg = ImGui::GetContentRegionAvail();
		else if(size_arg.x <= 0 || size_arg.y <= 0) return;
		int width = size_arg.x;
		int height = size_arg.y;
		
		//handles intializing and resizing the canvas framebuffer
		if(frameBufferWidth != width || frameBufferHeight != height){
			resizeFrameBuffer(width, height);
		}
		
		//ImGui element for layout and event handling
		ImGui::InvisibleButton("Applet", ImVec2(width, height));
		
		//Update input variables
		ImVec2 mousePos = ImGui::GetMousePos();
		ImVec2 topLeft = ImGui::GetItemRectMin();
		mousePosX = mousePos.x - topLeft.x;
		mousePosY = mousePos.y - topLeft.y;
		mousePosX = std::min(mousePosX, width);
		mousePosY = std::min(mousePosY, height);
		mousePosX = std::max(mousePosX, 0);
		mousePosY = std::max(mousePosY, 0);
		b_mousePressed = ImGui::IsMouseDown(ImGuiMouseButton_Left);
		
		//this allows ofWidth() and ofHeight() to work
		ofRenderer::setCurrentRenderSize(frameBufferWidth, frameBufferHeight);
		
		//actual script rendering
		ofRenderer::startRender();
		framebuffer.begin();
		
		if(b_shouldStart) {
			b_shouldStart = false;
			onStart();
		}
		if(script.isRunning()) script.callFunction("update");
		else ofBackground(0, 0, 0, 255);
		if(b_shouldStop){
			b_shouldStop = false;
			onStop();
		}
			 
		framebuffer.end();
		ofRenderer::finishRender();
		
		bool b_drawBorder = border > 0.0;
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		min -= glm::vec2(border, border);
		max += glm::vec2(border, border);
		
		//draw the framebuffer to the imgui canvas
		ImTextureID textureID = (ImTextureID)(uintptr_t)framebuffer.getTexture().texData.textureID;
		if(rounding <= 0.0) {
			if(b_drawBorder) ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::GetWindowDrawList()->AddImage(textureID,
																 ImGui::GetItemRectMin(),
																 ImGui::GetItemRectMax());
		}
		else {
			if(b_drawBorder) ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(1.0f, 1.0f, 1.0f, 1.0f), rounding + border);
			ImGui::GetWindowDrawList()->AddImageRounded(textureID,
														 ImGui::GetItemRectMin(),
														 ImGui::GetItemRectMax(),
														 ImVec2(0.0f,0.0f),
														 ImVec2(1.0f,1.0f),
														 ImColor(1.0f, 1.0f, 1.0f, 1.0f),
														 rounding);
		}
		
		if(!script.isRunning()){
			glm::vec2 textPos = ImGui::GetItemRectMin() + glm::vec2(5.0, 5.0);
			ImGui::GetWindowDrawList()->AddText(textPos, ImColor(1.0f, 1.0f, 1.0f, 1.0f), "Stage Visualizer script is not running.");
		}
	}

	void reset(){
		std::string defaultScript =
			"\n"
			"---------------------------------------\n"
			"----Default Stage Visualizer Script----\n"
			"---------------------------------------\n\n\n"
			"--Initialize and Load stuff here :\n\n"
			"function setup()\n"
			"	Logger:info(\"Starting Stage Visualizer Script at\", of.getElapsedTimef(), \"seconds.\")\n"
			"	of.setCircleResolution(128)\n"
			"end\n\n\n"
			"--Display and Animate stuff here :\n\n"
			"function update()\n"
			"	time = of.getElapsedTimef()\n"
			"	brightness = (math.sin(time) + 1.0) * 64\n"
			"	diameter = (math.sin(time) + 1.0) * 100\n"
			"\n"
			"	of.background(brightness)\n"
			"\n"
			"	size = glm.vec2(Canvas.getSize())\n"
			"	middle = glm.vec2(size.x / 2.0, size.y / 2.0)\n"
			"	of.drawCircle(middle, diameter)\n"
			"\n"
			"	of.drawBitmapStringHighlight(\"Default Stage Visualizer Script\", 20, 30)\n"
			"\n"
			"	mouse = glm.vec2(Canvas.getMousePosition())\n"
			"	if Canvas.isPressed() then of.setColor(255, 0, 0, 255)\n"
			"	else of.setColor(0, 0, 255, 255) end\n"
			"	of.drawCircle(mouse, 10)\n"
			"\n"
			"	lStart = glm.vec2(100, 100)\n"
			"	lEnd = glm.vec2(size.x - lStart.x, size.y - lStart.y)\n"
			"	of.setColor(255, 0, 0, 255)\n"
			"	of.drawLine(lStart, lEnd)\n"
			"	lerp = (math.sin(of.getElapsedTimef() * 0.456) + 1.0) / 2.0\n"
			"	rPos = glm.vec2(of.lerp(lStart.x, lEnd.x, lerp), of.lerp(lStart.y, lEnd.y, lerp))\n"
			"	of.drawCircle(rPos, 50)\n"
			"end\n\n\n"
			"--Unload and Clean Up stuff here :\n\n"
			"function exit()\n"
			"	Logger:info(\"Exiting Stage Visualizer Script at\", of.getElapsedTimef(), \"seconds.\")\n"
			"end\n";
		script.load(defaultScript);
	}

};
