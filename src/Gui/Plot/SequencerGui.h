#pragma once

#include "Gui/ApplicationWindow/Window.h"

namespace Sequencer::Gui{
	
	void editor();

	void transportControls(float height);

	class SequencerWindow : public Window{
	public:
		SequencerWindow() : Window("Sequencer", false){}
		virtual void drawContent() override { Sequencer::Gui::editor(); };
		static std::shared_ptr<SequencerWindow> get();
	};


};
