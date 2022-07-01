#pragma once

#include "Gui/ApplicationWindow/Window.h"

class EsiReaderWindow : public Window{
public:
	EsiReaderWindow() : Window("ESI Reader", true){}
	virtual void drawContent() override;
	SINGLETON_GET_METHOD(EsiReaderWindow);
};
