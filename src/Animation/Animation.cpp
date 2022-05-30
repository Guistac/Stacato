#include <pch.h>

#include "Animation.h"

#include "Machine/Machine.h"
#include "Animation/Animatable.h"
#include "Motion/Curve/Curve.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Environnement/Environnement.h"
#include "Animation/Manoeuvre.h"
#include "Plot/Plot.h"

#include "Project/Editor/Parameter.h"


std::shared_ptr<Animation> Animation::create(std::shared_ptr<Animatable> animatable, ManoeuvreType manoeuvreType){
	if(animatable->isComposite()) return std::make_shared<AnimationComposite>(animatable->toComposite(), manoeuvreType);
	switch(manoeuvreType){
		case ManoeuvreType::KEY:
			return std::make_shared<AnimationKey>(animatable);
		case ManoeuvreType::TARGET:
			return std::make_shared<TargetAnimation>(animatable);
		case ManoeuvreType::SEQUENCE:
			return std::make_shared<SequenceAnimation>(animatable);
	}
}

void Animation::subscribeToMachineParameter(){
	animatable->subscribeAnimation(shared_from_this());
}

void Animation::unsubscribeFromMachineParameter(){
	animatable->unsubscribeTrack(shared_from_this());
}


std::shared_ptr<Animation> Animation::copy(){
	std::shared_ptr<Animation> copy;
	if(isComposite()) copy = toComposite()->copy();
	else{
		switch(getType()){
			case ManoeuvreType::KEY:	copy = toKey()->copy(); break;
			case ManoeuvreType::TARGET: copy = toTarget()->copy(); break;
			case ManoeuvreType::SEQUENCE: copy = toSequence()->copy(); break;
		}
	}
	copy->subscribeToMachineParameter();
	return copy;
}

void Animation::validate(){
	validationErrorString = "";
	b_valid = getAnimatable()->getMachine()->validateAnimation(shared_from_this());
	if(hasManoeuvre()) manoeuvre->updateTrackSummary();
}


void Animation::updatePlaybackStatus(){
	if(playbackPosition_seconds >= duration_seconds){
		auto animatable = getAnimatable();
		if(animatable->hasAnimation()) animatable->getMachine()->endAnimationPlayback(animatable);
	}
}


std::shared_ptr<AnimationValue> Animation::getValueAtPlaybackTime(){
	return animatable->getValueAtAnimationTime(shared_from_this(), playbackPosition_seconds);
}


bool Animation::isMachineEnabled(){ return animatable->getMachine()->isEnabled(); }


bool Animation::isInRapid(){
	return getRapidProgress() < 1.0;
}

float Animation::getRapidProgress(){
	return animatable->getMachine()->getAnimatableRapidProgress(animatable);
}


bool Animation::isPlaying(){
	return getAnimatable()->getAnimation() == shared_from_this();
}

void Animation::startPlayback(){
	if(!isReadyToStartPlayback()) return;
	animatable->getMachine()->startAnimationPlayback(shared_from_this());
}

void Animation::interruptPlayback(){
	if(isPlaying()) animatable->getMachine()->interruptAnimationPlayback(animatable);
	animatable->getMachine()->cancelAnimatableRapid(animatable);
}

void Animation::stopPlayback(){
	if(isPlaying()) animatable->getMachine()->endAnimationPlayback(animatable);
	animatable->getMachine()->cancelAnimatableRapid(animatable);
	setPlaybackPosition(0.0);
	if(hasManoeuvre()) getManoeuvre()->onTrackPlaybackStop();
}
