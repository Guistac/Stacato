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
	std::vector<std::shared_ptr<AnimatableMapping>> getMappings();
	bool save(tinyxml2::XMLElement* xml);
	bool load(tinyxml2::XMLElement* xml);
	
	std::vector<std::shared_ptr<AnimatableMapping>> animatableMappings = {};
	std::shared_ptr<StringParameter> nameParameter = StringParameter::make("Channel Mapping", "Name", "Name", 128);
	
	std::mutex mutex;
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














class ManualControlAxis{
public:
	
	ManualControlAxis(std::string name, std::string saveName){
		displayName = name;
		saveString = saveName;
	}
	
	std::string& getName(){ return displayName; }
	std::string& getSaveString(){ return saveString; }
	
private:
	
	std::string displayName;
	std::string saveString;
};



class ManualControlDevice{
public:
	
	ManualControlDevice(std::string name, std::string saveName){
		displayName = name;
		saveString = saveName;
	}
	
	enum class Type{
		JOYSTICK_1AXIS,
		JOYSTICK_2AXIS,
		JOYSTICK_3AXIS,
		JOYSTICK_6AXIS
	};
	
	std::vector<std::shared_ptr<ManualControlAxis>>& getAxes(){ return axes; }
	size_t getAxisCount(){ return axes.size(); }
	
	virtual Type getType() = 0;
	
	std::string& getName(){ return displayName; }
	std::string& getSaveString(){ return saveString; }
	
protected:
	
	std::vector<std::shared_ptr<ManualControlAxis>> axes = {};
	
	std::string displayName;
	std::string saveString;
	
};


class Joystick1Axis : public ManualControlDevice{
public:
	
	Joystick1Axis(std::string name, std::string saveName) : ManualControlDevice(name, saveName){
		auto axis = std::make_shared<ManualControlAxis>("Joystick Axis", "JoystickAxis");
		axes.push_back(axis);
	}
	
	virtual Type getType() override { return ManualControlDevice::Type::JOYSTICK_1AXIS; }
	
};


class Joystick2Axis : public ManualControlDevice{
public:
	
	Joystick2Axis(std::string name, std::string saveName) : ManualControlDevice(name, saveName){
		auto xAxis = std::make_shared<ManualControlAxis>("X-Axis", "X-Axis");
		auto yAxis = std::make_shared<ManualControlAxis>("Y-Axis", "Y-Axis");
		axes.push_back(xAxis);
		axes.push_back(yAxis);
	}
	
	virtual Type getType() override { return ManualControlDevice::Type::JOYSTICK_2AXIS; }
	
};


class Joystick3Axis : public ManualControlDevice{
public:
	
	Joystick3Axis(std::string name, std::string saveName) : ManualControlDevice(name, saveName){
		auto xAxis = std::make_shared<ManualControlAxis>("X-Axis", "X-Axis");
		auto yAxis = std::make_shared<ManualControlAxis>("Y-Axis", "Y-Axis");
		auto zAxis = std::make_shared<ManualControlAxis>("Z-Axis", "Z-Axis");
		axes.push_back(xAxis);
		axes.push_back(yAxis);
		axes.push_back(zAxis);
	}
	
	virtual Type getType() override { return ManualControlDevice::Type::JOYSTICK_3AXIS; }
	
};


class Joystick6Axis : public ManualControlDevice{
public:
	
	Joystick6Axis(std::string name, std::string saveName) : ManualControlDevice(name, saveName){
		auto xAxis = std::make_shared<ManualControlAxis>("X-Axis", "X-Axis");
		auto yAxis = std::make_shared<ManualControlAxis>("Y-Axis", "Y-Axis");
		auto zAxis = std::make_shared<ManualControlAxis>("Z-Axis", "Z-Axis");
		auto aAxis = std::make_shared<ManualControlAxis>("A-Axis", "A-Axis");
		auto bAxis = std::make_shared<ManualControlAxis>("B-Axis", "B-Axis");
		auto cAxis = std::make_shared<ManualControlAxis>("C-Axis", "C-Axis");
		axes.push_back(xAxis);
		axes.push_back(yAxis);
		axes.push_back(zAxis);
		axes.push_back(aAxis);
		axes.push_back(bAxis);
		axes.push_back(cAxis);
	}
	
	virtual Type getType() override { return ManualControlDevice::Type::JOYSTICK_6AXIS; }
	
};
