#pragma once

#include "Project/Editor/Parameter.h"
#include "Motion/MotionTypes.h"

class ParameterTrack;
class MachineParameter;
class Plot;
class ManoeuvreList;
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
	
	void setManoeuvreList(std::shared_ptr<ManoeuvreList> manoeuvreList_){ manoeuvreList = manoeuvreList_; }
	bool isInManoeuvreList(){ return manoeuvreList != nullptr; }
	std::shared_ptr<ManoeuvreList> getManoeuvreList(){ return manoeuvreList; }
	
	void select();
	void deselect();
	bool isSelected();

private:
	
	std::shared_ptr<StringParameter> name = std::make_shared<StringParameter>("Default Name","Manoeuvre Name","Name",256);
	std::shared_ptr<StringParameter> description = std::make_shared<StringParameter>("Default Description","Manoeuvre Description","Description",512);
	std::shared_ptr<EnumeratorParameter<ManoeuvreType>> type = std::make_shared<EnumeratorParameter<ManoeuvreType>>(ManoeuvreType::KEY,"Manoeuvre Type","Type");
	std::shared_ptr<ManoeuvreList> manoeuvreList = nullptr;
	
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
	void validateAllParameterTracks();
	
private:
	
	std::vector<std::shared_ptr<ParameterTrack>> tracks;
	bool b_valid = false;
	
	//———————————————————————————————
	//			Playback
	//———————————————————————————————
	
public:
	
	bool areAllMachinesEnabled();
	bool areNoMachinesEnabled();
	
	bool canRapidToStart();
	bool isAtStart();

	bool canRapidToTarget();
	bool isAtTarget();
	
	bool canRapidToPlaybackPosition();
	bool isAtPlaybackPosition();
	bool canSetPlaybackPosition();

	bool canStartPlayback();
	bool canPausePlayback();
	bool isPlaybackPaused();

	float getRapidProgress();
	bool isRapidFinished();
	float getPlaybackProgress();
	bool isPlaybackFinished();
	
	void rapidToStart();
	void rapidToTarget();
	void rapidToPlaybackPosition();
	void startPlayback();
	void pausePlayback();
	void setPlaybackPosition(double seconds);
	void stop();
	
	bool b_playing = false;
	bool b_paused = false;
	bool b_inRapid = false;
	
	bool isPlaying(){ return b_playing; }
	bool isInRapid(){ return b_inRapid; }
	bool isPaused(){ return b_paused; }
	bool isFinished() { return !(b_playing || b_inRapid || b_paused); } //if any of these is true, the manoeuvre is not finished
	
	void incrementPlaybackPosition(long long deltaT_microseconds);
	void updatePlaybackStatus();
	long long playbackStartTime_microseconds = 0;
	double playbackPosition_seconds = 0.0;

private:
	
	//———————————————————————————————
	//		   User Interface
	//———————————————————————————————
	
public:
	
	void listGui();
	void miniatureGui(glm::vec2 size_arg);
	
	void trackSheetGui();
	void curveEditorGui();
	void spatialEditorGui();
};
