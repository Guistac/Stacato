#pragma once

class Window;
class Popup;

namespace Legato::Gui{

void initialize();
void drawFrame();
void terminate();

void openWindow(std::shared_ptr<Window> window);
void closeWindow(std::shared_ptr<Window> window);
void openPopup(std::shared_ptr<Popup> popup);
void closePopup(std::shared_ptr<Popup> popup);
void drawWindows();
void drawPopups();

};
