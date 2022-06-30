#pragma once

#include "Gui/ApplicationWindow/Window.h"

namespace Playback::Gui{

	void manoeuvrePlaybackControls(float height);

	void sequencerPlaybackControls(float height);

	class ActiveAnimationsWindows : public Window{
	public:
		ActiveAnimationsWindows() : Window("Active Animations", true){}
		virtual void drawContent() override;
		SINGLETON_GET_METHOD(ActiveAnimationsWindows)
	};

}
