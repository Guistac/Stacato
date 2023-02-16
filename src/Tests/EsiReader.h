#pragma once

#include "Legato/Gui/Window.h"

class EsiReaderWindow : public Window{
public:
	EsiReaderWindow() : Window("ESI Reader", true){}
	virtual void onDraw() override;
	SINGLETON_GET_METHOD(EsiReaderWindow);
};
