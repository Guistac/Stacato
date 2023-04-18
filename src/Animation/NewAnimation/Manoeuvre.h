#pragma once

#include "Legato/Editor/Component.h"

namespace AnimationSystem{

class Animation;
class Animatable;

class Manoeuvre : public Legato::Component{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(Manoeuvre)
	
public:
	
	virtual void onConstruction() override{}
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override{}
	virtual bool onSerialization() override{}
	virtual bool onDeserialization() override{}
	
	void editorGui();
	
	void addAnimation(std::shared_ptr<Animation> animation);
	void removeAnimation(std::shared_ptr<Animation> animation);
	
private:
	
	std::vector<std::shared_ptr<Animation>> animations = {};
	
};

};
