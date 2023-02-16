#include <pch.h>
#include "Window.h"

#include "Gui_Private.h"

void NewWindow::open(){
	NewGui::openWindow(shared_from_this());
}

void NewWindow::close(){
	NewGui::closeWindow(shared_from_this());
}

void NewWindow::focus(){
	
}

void NewWindow::draw(){
	//open window
	onDraw();
	//close window
}

bool NewWindow::isOpen(){
	return b_isOpen;
}

bool NewWindow::isFocused(){
	
}

