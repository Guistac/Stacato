#pragma once

#include "Project/Editor/Parameter.h"
#include "Motion/MotionTypes.h"

class ParameterTrack;
class MachineParameter;
class Plot;
namespace tinyxml2 { class XMLElement; }


class Manoeuvre : public std::enable_shared_from_this<Manoeuvre>{

	//———————————————————————————————
	// Construction, Saving & Loading
	//———————————————————————————————
	
public:
	
	Manoeuvre();
	std::shared_ptr<Manoeuvre> copy();
	bool save(tinyxml2::XMLElement* manoeuvreXML);
	static std::shared_ptr<Manoeuvre> load(tinyxml2::XMLElement* manoeuvreXML);
	
	//———————————————————————————————
	//		General Properties
	//———————————————————————————————

public:

	ManoeuvreType getType(){ return type->value; }
	void overwriteType(ManoeuvreType type_){ type->overwrite(type_); }
	
	const char* getName(){ return name->value.c_str(); }
	void setName(std::string name_){ name->overwrite(name_); }
	
	const char* getDescription(){ return description->value.c_str(); }
	void setDescription(const char* descr){ description->overwrite(descr); }

private:
	
	std::shared_ptr<StringParameter> name = std::make_shared<StringParameter>("Default Name","Manoeuvre Name","Name",256);
	std::shared_ptr<StringParameter> description = std::make_shared<StringParameter>("Default Description","Manoeuvre Description","Description",512);
	std::shared_ptr<EnumeratorParameter<ManoeuvreType>> type = std::make_shared<EnumeratorParameter<ManoeuvreType>>(ManoeuvreType::KEY,"Manoeuvre Type","Type");

	//———————————————————————————————
	//		  Parameter Tracks
	//———————————————————————————————
	
public:
	
	bool hasTrack(std::shared_ptr<MachineParameter>& parameter);
	std::vector<std::shared_ptr<ParameterTrack>>& getTracks(){ return tracks; }
	
	void addTrack(std::shared_ptr<MachineParameter>& parameter);
	void removeTrack(std::shared_ptr<MachineParameter> parameter);
	void moveTrack(int oldIndex, int newIndex);
	
	void subscribeAllTracksToMachineParameter();
	void unsubscribeAllTracksFromMachineParameter();
	
	void updateValidation();
	
private:
	
	std::vector<std::shared_ptr<ParameterTrack>> tracks;
	bool b_valid = false;
	
	//———————————————————————————————
	//			Playback
	//———————————————————————————————
	
public:
	
	double getLength_seconds();
	float getPlaybackProgress();
	
	void setPaused(bool paused){ b_paused = paused; }
	
	void trackSheetGui();
	void curveEditorGui();
	void spatialEditorGui();
	
	bool canRapidToStart();
	bool isAtStart();
	
	bool canRapidToEnd();
	bool isAtEnd();
	
	bool canRapidToPlaybackPosition();
	bool isAtPlaybackPosition();
	bool canStartPlayback();
	bool canStopPlayback();
	bool isPlaying();
	bool isPaused(){ return false; }
	bool isStopped();
	
	//Playback Button States:
	

private:
	
	double playbackStartTime_seconds = 0.0;
	double playbackPosition_seconds = 0.0;
	bool b_paused = false;
	
	//———————————————————————————————
	//		   User Interface
	//———————————————————————————————
	
public:
	
	void listGui();
	void miniatureGui(glm::vec2 size_arg);
};
