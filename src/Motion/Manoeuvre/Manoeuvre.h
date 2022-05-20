#pragma once

#include "Project/Editor/Parameter.h"
#include "Motion/MotionTypes.h"

class ParameterTrack;
class MachineParameter;
class Plot;

namespace tinyxml2 { class XMLElement; }



class Manoeuvre {
public:
	
	Manoeuvre(){
		init();
	}
	Manoeuvre(const Manoeuvre &other){
		init();
	}
	
	void init(){
		type->setEditCallback([this](std::shared_ptr<Parameter> parameter){
			Logger::warn("edited type of maneoeuvre {}", this->getName());
		});
	}
	
	ManoeuvreType getType(){ return type->value; }
	void setType(ManoeuvreType type_);
	
	const char* getName(){ return name->value.c_str(); }
	void setName(const char* name_){ name->overwrite(name_); }
	
	const char* getDescription(){ return description->value.c_str(); }
	void setDescription(const char* descr){ description->overwrite(descr); }
	
	void addTrack(std::shared_ptr<MachineParameter>& parameter);
	bool hasTrack(std::shared_ptr<MachineParameter>& parameter);
	std::vector<std::shared_ptr<ParameterTrack>>& getTracks(){ return tracks; }
	void removeTrack(std::shared_ptr<MachineParameter>& parameter);
	void removeTrack(int removedIndex);
	void moveTrack(int oldIndex, int newIndex);
	
	double getLength_seconds();
	float getPlaybackProgress();
	
	void trackSheetGui();
	void curveEditorGui();
	void spatialEditorGui();
	
	bool isPaused(){ return b_paused; }
	void setPaused(bool paused){ b_paused = paused; }
	
	void listGui();
	void miniatureGui(glm::vec2 size_arg);
	
	bool save(tinyxml2::XMLElement* manoeuvreXML);
	static std::shared_ptr<Manoeuvre> load(tinyxml2::XMLElement* manoeuvreXML);
	
private:
	
	bool b_valid = false;
	std::vector<std::shared_ptr<ParameterTrack>> tracks;
	
	std::shared_ptr<StringParameter> name = std::make_shared<StringParameter>("Default Name",
																			  "##Name",
																			  "Name",
																			  256);
	std::shared_ptr<StringParameter> description = std::make_shared<StringParameter>("Default Description",
																					 "##Description",
																					 "Description",
																					 512);
	std::shared_ptr<EnumeratorParameter<ManoeuvreType>> type = std::make_shared<EnumeratorParameter<ManoeuvreType>>(ManoeuvreType::KEY,
																													"##Type",
																													"Type");
	
	double playbackStartTime_seconds = 0.0;
	double playbackPosition_seconds = 0.0;
	bool b_paused = false;
};
