#pragma once

namespace AnimationSystem{

class Animation;

class Manoeuvre{
public:
	
	
	Manoeuvre(){}
	
	void editorGui();
	
	
private:
	
	std::vector<std::shared_ptr<Animation>> animations = {};
	
};

};
