#pragma once

#include "Gui/ApplicationWindow/Window.h"

namespace Playback::Gui{

	void manoeuvrePlaybackControls(float height);

	void sequencerPlaybackControls(float height);

	class PlaybackManagerWindow : public Window{
	public:
		PlaybackManagerWindow() : Window("Playback Manager", true){}
		virtual void drawContent() override;
		SINGLETON_GET_METHOD(PlaybackManagerWindow)
	};

}
