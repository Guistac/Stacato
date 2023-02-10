#pragma once

namespace NewGui{

void setInitializationFunction(std::function<void()> fn);
void setPreFrameFunction(std::function<void()> fn);
void setPostFrameFunction(std::function<void()> fn);
void setTerminationFunction(std::function<void()> fn);

void initialize();

void drawFrame();

void terminate();

class WindowA;
class PopupA;

void openWindow(std::shared_ptr<WindowA> window);
void openPopup(std::shared_ptr<PopupA> popup);

};
