#pragma once

class NewWindow;

namespace NewGui{

void initialize();

void drawFrame();

void terminate();

void openWindow(std::shared_ptr<NewWindow> window);
void closeWindow(std::shared_ptr<NewWindow> window);

};
