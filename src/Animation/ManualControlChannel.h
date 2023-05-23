#pragma once

#include "Project/Editor/Parameter.h"


class Animatable;
namespace tinyxml2{ class XMLElement; }

class AnimatableMapping{
public:
	std::shared_ptr<Animatable> animatable;
	NumberParam<double> multiplier = NumberParameter<double>::make(1.0, "Multiplier", "Multiplier");
	bool save(tinyxml2::XMLElement* xml);
	bool load(tinyxml2::XMLElement* xml);
};

class ChannelPreset{
public:
	void addAnimatable(std::shared_ptr<Animatable> animatable);
	bool hasAnimatable(std::shared_ptr<Animatable> animatable);
	void removeAnimatable(std::shared_ptr<Animatable> animatable);
	std::vector<std::shared_ptr<AnimatableMapping>>& getMappings(){ return animatableMappings; }
	bool save(tinyxml2::XMLElement* xml);
	bool load(tinyxml2::XMLElement* xml);
	
	std::vector<std::shared_ptr<AnimatableMapping>> animatableMappings = {};
	std::shared_ptr<StringParameter> nameParameter = StringParameter::make("Channel Mapping", "Name", "Name", 128);
};

class ManualControlChannel{
public:
	
	void createChannelPreset();
	void removeChannelPreset(std::shared_ptr<ChannelPreset> preset);
	std::vector<std::shared_ptr<ChannelPreset>>& getChannelPresets(){ return channelPresets; }
	
	std::shared_ptr<ChannelPreset> getActiveChannelPreset(){ return activeChannelPreset; }
	void setActiveChannelPreset(std::shared_ptr<ChannelPreset> preset);
	
	bool save(tinyxml2::XMLElement* xml);
	bool load(tinyxml2::XMLElement* xml);
	
	void gui();
	
	void setControlValue(float controlValue);
	
private:
	std::vector<std::shared_ptr<ChannelPreset>> channelPresets = {};
	std::shared_ptr<ChannelPreset> activeChannelPreset = nullptr;
	
	float controlSliderValue = 0.0;
	bool b_editLocked = true;
};

class ManualControlsWindow : public Window{
public:
	ManualControlsWindow() : Window("Manual Controls", true){}
	virtual void onDraw() override;
	SINGLETON_GET_METHOD(ManualControlsWindow)
};

