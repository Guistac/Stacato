#include <pch.h>

#include "Manoeuvre.h"
#include "Animatable.h"
#include "Animation.h"

namespace AnimationSystem{

	void Manoeuvre::addAnimation(std::shared_ptr<Animation> animation){
		animations.push_back(animation);
	}

	void Manoeuvre::removeAnimation(std::shared_ptr<Animation> animation){
		for(int i = (int)animations.size(); i >= 0; i--){
			if(animations[i] == animation){
				animations.erase(animations.begin() + i);
				break;
			}
		}
	}

};
