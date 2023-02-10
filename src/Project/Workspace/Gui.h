#pragma once

namespace NewGui{

void setInitializationFunction(std::function<void()> fn);
void setGuiSubmitFunction(std::function<void()> fn);
void setTerminationFunction(std::function<void()> fn);

class WindowA;
class PopupA;

void openWindow(std::shared_ptr<WindowA> window);
void openPopup(std::shared_ptr<PopupA> popup);


float getScale();

};
