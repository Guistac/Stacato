#pragma once

class Window;

namespace NewGui{

void initialize();

void drawFrame();

void terminate();

void openWindow(std::shared_ptr<Window> window);
void closeWindow(std::shared_ptr<Window> window);

};
