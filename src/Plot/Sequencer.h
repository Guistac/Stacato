#pragma once

#include "Gui/Plot/SequencerLibrary.h"

class Animatable;

class Track{
public:
	
	Track(std::shared_ptr<Animatable> animatable_) : animatable(animatable_){}
	
	std::shared_ptr<Animatable> animatable;
	
};

class Sequence{
public:
	
	long long playbackPosition_micros;
	
	std::vector<std::shared_ptr<Track>> tracks;
	std::vector<std::shared_ptr<Track>>& getTracks(){ return tracks; }
	bool hasTrack(std::shared_ptr<Animatable> animatable);
	
	void addTrack(std::shared_ptr<Animatable> animatable);
	
	SequencerLibrary::Context* guiContext = SequencerLibrary::createContext();
	
};


namespace Sequencer{

	std::shared_ptr<Sequence> getSequence();

};
