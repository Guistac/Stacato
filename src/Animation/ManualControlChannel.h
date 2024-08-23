#pragma once

#include "Project/Editor/Parameter.h"


class Animatable;
namespace tinyxml2{ class XMLElement; }

class ManualControlChannel : public std::enable_shared_from_this<ManualControlChannel>{
public:
	
	ManualControlChannel(std::string name_) : name(name_){}
	
	void fullGui();
	void openMappingList();
	void mappingList();
	void mappingListTooltip();
	
	void updateSubscribers();
	
	void setControlValue(float x, float y, float z);
	
	void addSubscriber(std::shared_ptr<Animatable>);
	bool removeSubscriber(std::shared_ptr<Animatable>);
	void clearSubscribers();
	
	struct ControlValues{
		float x = 0.0;
		float y = 0.0;
		float z = 0.0;
	}controlValue;

	std::string name;
	
private:
	
	std::vector<std::shared_ptr<Animatable>> subscribers = {};
	
};

class ManualControlsWindow : public Window{
public:
	ManualControlsWindow() : Window("Manual Controls", true){}
	virtual void onDraw() override;
	SINGLETON_GET_METHOD(ManualControlsWindow)
};
