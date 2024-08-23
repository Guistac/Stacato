#include <pch.h>
#include "ManualControlChannel.h"

#include "Animatable.h"
#include "Machine/Machine.h"

#include <tinyxml2.h>

void ManualControlChannel::addSubscriber(std::shared_ptr<Animatable> newSubscriber){
	auto thisControlChannel = shared_from_this();
	if(newSubscriber->subscribedManualControlChannel == thisControlChannel) return;
	if(newSubscriber->subscribedManualControlChannel){
		newSubscriber->subscribedManualControlChannel->removeSubscriber(newSubscriber);
	}
	newSubscriber->subscribedManualControlChannel = shared_from_this();
	subscribers.push_back(newSubscriber);
}

bool ManualControlChannel::removeSubscriber(std::shared_ptr<Animatable> removedSubscriber){
	for(int i = 0; i < subscribers.size(); i++){
		if(subscribers[i] == removedSubscriber){
			removedSubscriber->subscribedManualControlChannel = nullptr;
			removedSubscriber->setManualControlTarget(0.0, 0.0, 0.0);
			subscribers.erase(subscribers.begin() + i);
			return true;
		}
	}
	return false;
}

void ManualControlChannel::clearSubscribers(){
	for(auto subscriber : subscribers){
		subscriber->subscribedManualControlChannel = nullptr;
		subscriber->setManualControlTarget(0.0, 0.0, 0.0);
	}
	subscribers.clear();
}

void ManualControlChannel::updateSubscribers(){
	for(auto subscriber : subscribers){
		subscriber->setManualControlTarget(controlValue.x, controlValue.y, controlValue.z);
	}
}

void ManualControlChannel::setControlValue(float x, float y, float z){
	controlValue.x = x;
	controlValue.y = y;
	controlValue.z = z;
	updateSubscribers();
}
