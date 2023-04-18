#pragma once

#include "Legato/Gui/Window.h"

namespace Playback::Gui{

	void manoeuvrePlaybackControls(float height);

	void sequencerPlaybackControls(float height);

	class PlaybackManagerWindow : public Legato::Window{
	public:
		PlaybackManagerWindow() : Window("Playback Manager", true){}
		virtual void onDraw() override;
		SINGLETON_GET_METHOD(PlaybackManagerWindow)
	};

}
