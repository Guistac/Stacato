#pragma once

#include "Project/Editor/Parameter.h"
#include "Motion/MotionTypes.h"

class Animation;
class Animatable;
class ManoeuvreList;
class Plot;
namespace tinyxml2 { class XMLElement; }


class Manoeuvre : public std::enable_shared_from_this<Manoeuvre>{

	//———————————————————————————————
	// Construction, Saving & Loading
	//———————————————————————————————
	
public:
	
	static std::shared_ptr<Manoeuvre> make(ManoeuvreType type = ManoeuvreType::KEY);
	static std::shared_ptr<Manoeuvre> load(tinyxml2::XMLElement* manoeuvreXML);
	
	std::shared_ptr<Manoeuvre> copy();
	
	bool save(tinyxml2::XMLElement* manoeuvreXML);
	
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
	
	bool isInManoeuvreList(){ return manoeuvreList != nullptr; }
	void setManoeuvreList(std::shared_ptr<ManoeuvreList> manoeuvreList_){ manoeuvreList = manoeuvreList_; }
	std::shared_ptr<ManoeuvreList> getManoeuvreList(){ return manoeuvreList; }
	
	void select();
	void deselect();
	bool isSelected();
	
	bool isValid(){ return b_valid; }
	void setValid(bool valid){ b_valid = valid; }
	
private:
	
	std::shared_ptr<StringParameter> name = std::make_shared<StringParameter>("Default Name","Manoeuvre Name","Name",256);
	std::shared_ptr<StringParameter> description = std::make_shared<StringParameter>("Default Description","Manoeuvre Description","Description",512);
	std::shared_ptr<EnumeratorParameter<ManoeuvreType>> type = std::make_shared<EnumeratorParameter<ManoeuvreType>>(ManoeuvreType::KEY,"Manoeuvre Type","Type");
	std::shared_ptr<ManoeuvreList> manoeuvreList = nullptr;
	bool b_valid = false;
	
	//—————————————— Animation Tracks —————————————————
	
public:
		
	bool hasAnimation(std::shared_ptr<Animatable> animatable);
	void addAnimation(std::shared_ptr<Animatable> animatable);
	void removeAnimation(std::shared_ptr<Animatable> animatable);
	void moveAnimation(int oldIndex, int newIndex);
	
	std::vector<std::shared_ptr<Animation>>& getAnimations(){ return animations; }
	
	void subscribeAllTracksToMachineParameter();
	void unsubscribeAllTracksFromMachineParameter();
	
	void updateTrackSummary();
	void validateAllParameterTracks();
	
private:
	
	std::vector<std::shared_ptr<Animation>> animations;
	
	//———————————————————————————————
	//			Playback
	//———————————————————————————————
	
public:
	
	bool canRapidToStart();
	bool isAtStart();
	void rapidToStart();

	bool canRapidToTarget();
	bool isAtTarget();
	void rapidToTarget();
	
	bool canRapidToPlaybackPosition();
	bool isAtPlaybackPosition();
	void rapidToPlaybackPosition();

	bool canStartPlayback();
	bool canPausePlayback();
	void startPlayback();
	void pausePlayback();

	bool canStop();
	void stop();
	
	bool canSetPlaybackPosition();
	void setPlaybackPosition(double seconds);
	
	
	void updatePlaybackState();
	bool b_hasActiveAnimations = false;
	bool hasActiveAnimations(){ return b_hasActiveAnimations; }
	std::vector<std::shared_ptr<Animation>> getActiveAnimations();
	
	double synchronizedPlaybackPosition;
	double getSychronizedPlaybackPosition(){ return synchronizedPlaybackPosition; }
	double getRemainingPlaybackTime();
	
	bool hasDuration();
	void updateDuration();
	double getDuration();
	
	void getCurveRange(double& minX, double& maxX, double& minY, double& maxY);
	
	bool areAllMachinesEnabled();
	bool areNoMachinesEnabled();
	
	void requestCurveRefocus();
	bool shouldRefocusCurves();

private:
	
	double duration_seconds = 0.0;
	
	//———————————————————————————————
	//		   User Interface
	//———————————————————————————————
	
public:
	
	void listGui();
	void miniatureGui(glm::vec2 size_arg);
	
	void sheetEditor();
	void curveEditor();
	void spatialEditor();
};
