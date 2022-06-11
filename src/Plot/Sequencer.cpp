#include "Sequencer.h"

namespace Sequencer{

	std::shared_ptr<Sequence> sequence = std::make_shared<Sequence>();

	std::shared_ptr<Sequence> getSequence(){ return sequence; }


};


bool Sequence::hasTrack(std::shared_ptr<Animatable> animatable){
	for(auto& track : tracks){
		if(track->animatable == animatable) return true;
	}
	return false;
}

void Sequence::addTrack(std::shared_ptr<Animatable> animatable){
	auto newTrack = std::make_shared<Track>(animatable);
	tracks.push_back(newTrack);
}
