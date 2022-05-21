#pragma once

#include "Project/Editor/Parameter.h"
#include "Motion/MotionTypes.h"

class ParameterTrack;
class MachineParameter;
class Plot;

namespace tinyxml2 { class XMLElement; }


class Manoeuvre : public std::enable_shared_from_this<Manoeuvre>{
public:
	
	Manoeuvre(){ init(); }
	
	std::shared_ptr<Manoeuvre> copy();
	
	void init();
	
	ManoeuvreType getType(){ return type->value; }
	
	const char* getName(){ return name->value.c_str(); }
	void setName(const char* name_){ name->overwrite(name_); }
	
	const char* getDescription(){ return description->value.c_str(); }
	void setDescription(const char* descr){ description->overwrite(descr); }
	
	void addTrack(std::shared_ptr<MachineParameter>& parameter);
	bool hasTrack(std::shared_ptr<MachineParameter>& parameter);
	std::vector<std::shared_ptr<ParameterTrack>>& getTracks(){ return tracks; }
	void removeTrack(std::shared_ptr<MachineParameter> parameter);
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
	
	std::shared_ptr<StringParameter> name = std::make_shared<StringParameter>("Default Name",
																			  "Manoeuvre Name",
																			  "Name",
																			  256);
	std::shared_ptr<StringParameter> description = std::make_shared<StringParameter>("Default Description",
																					 "Manoeuvre Description",
																					 "Description",
																					 512);
	std::shared_ptr<EnumeratorParameter<ManoeuvreType>> type = std::make_shared<EnumeratorParameter<ManoeuvreType>>(ManoeuvreType::KEY,
																													"Manoeuvre Type",
																													"Type");
	
	void subscribeAllTracksToMachineParameter();
	void unsubscribeAllTracksFromMachineParameter();
	
private:
	
	bool b_valid = false;
	std::vector<std::shared_ptr<ParameterTrack>> tracks;
	
	double playbackStartTime_seconds = 0.0;
	double playbackPosition_seconds = 0.0;
	bool b_paused = false;
};
