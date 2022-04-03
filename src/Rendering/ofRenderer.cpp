#include <pch.h>

#include "ofRenderer.h"

#include <ofMain.h>
#include <ofGLProgrammableRenderer.h>

namespace ofRenderer{

	//this dummy window is needed for openframeworks drawing functions to be able to get to a renderer instance
	class ofDummyWindow : public ofAppBaseWindow{
	public:
		virtual void setup(const ofWindowSettings & settings) override {}
		virtual void update() override {}
		virtual void draw() override {}
		virtual ofCoreEvents & events() override {
			static ofCoreEvents events;
			return events;
		}
		virtual std::shared_ptr<ofBaseRenderer> & renderer() override { return rendererRef; }
		virtual glm::vec2 getWindowSize() override { return renderSize; }
		virtual int getWidth() override { return renderSize.x; }
		virtual int getHeight() override { return renderSize.y; }
		std::shared_ptr<ofBaseRenderer> rendererRef;
		glm::vec2 renderSize{1280, 720};
	};

	std::shared_ptr<ofDummyWindow> dummyWindow;
	std::shared_ptr<ofGLProgrammableRenderer> mainRenderer;

	void init(int openGlVersionMajor, int openglVersionMinor){
		//basic initialisation
		ofInit();
		//create a dummy window to hold the main renderer
		dummyWindow = std::make_shared<ofDummyWindow>();
		//create the main renderer and link it with the window we created
		mainRenderer = std::make_shared<ofGLProgrammableRenderer>(dummyWindow.get());
		//setup the renderer using the specified OpenGl Version
		mainRenderer->setup(openGlVersionMajor, openglVersionMinor);
		//give the main renderer reference to the dummy window
		dummyWindow->rendererRef = mainRenderer;
		//assign the window in the main openframeworks context
		ofGetMainLoop()->setCurrentWindow(dummyWindow);
	}

	void terminate(){}

	void startRender(){
		mainRenderer->startRender();
	}

	void finishRender(){
		mainRenderer->finishRender();
	}

	void setCurrentRenderSize(int width, int height){
		dummyWindow->renderSize.x = width;
		dummyWindow->renderSize.y = height;
	}

}


