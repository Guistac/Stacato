#pragma once

namespace AnimationSystem{

class Animation;
class Animatable;

class Manoeuvre{
public:
	
	Manoeuvre(){}
	
	void editorGui();
	
	void addAnimation(std::shared_ptr<Animation> animation);
	void removeAnimation(std::shared_ptr<Animation> animation);
	
private:
	
	std::vector<std::shared_ptr<Animation>> animations = {};
	
};

};
