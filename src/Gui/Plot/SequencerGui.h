#pragma once

#include "Legato/Gui/Window.h"

namespace Sequencer::Gui{
	
	void editor();

	void transportControls(float height);

	class SequencerWindow : public Legato::Window{
	public:
		SequencerWindow() : Window("Sequencer", false){}
		virtual void onDraw() override { Sequencer::Gui::editor(); };
		SINGLETON_GET_METHOD(SequencerWindow);
	};


};
