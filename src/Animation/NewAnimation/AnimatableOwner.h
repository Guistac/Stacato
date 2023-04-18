#pragma once

#include "Legato/Editor/Component.h"

namespace AnimationSystem{

class Animatable;

	class AnimatableOwner /*: public Legato::Component*/{
		
		/*
		//DECLARE_PROTOTYPE_IMPLENTATION_METHODS(AnimatableOwner)
		
	protected:
		AnimatableOwner(){}
	private:
		std::shared_ptr<PrototypeBase> createPrototypeInstance_private() override{
			std::shared_ptr<AnimatableOwner> newPrototypeInstance = std::shared_ptr<AnimatableOwner>(new AnimatableOwner());
			newPrototypeInstance->onConstruction();
			return newPrototypeInstance;
			return nullptr;
		};
		*/
		 
	public:
		
		void addAnimatable(std::shared_ptr<Animatable> animatable){
			animatables.push_back(animatable);
		}
		
		void removeAnimatable(std::shared_ptr<Animatable> animatable){
			for(int i = (int)animatables.size(); i >= 0; i--){
				if(animatables[i] == animatable){
					animatables.erase(animatables.begin() + i);
					break;
				}
			}
		}
		
		std::vector<std::shared_ptr<Animatable>>& getAnimatables(){ return animatables; }
		
	private:
		
		std::vector<std::shared_ptr<Animatable>> animatables = {};
		
	};

};
