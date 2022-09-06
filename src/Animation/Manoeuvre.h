#pragma once

#include "Project/Editor/Parameter.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curve/Curve.h"

class Animation;
class Animatable;
class ManoeuvreList;
class Plot;
namespace tinyxml2 { class XMLElement; }


class Manoeuvre : public std::enable_shared_from_this<Manoeuvre>{

	//———————————— Construction, Saving & Loading ————————————
	
public:
	
	static std::shared_ptr<Manoeuvre> make(ManoeuvreType type = ManoeuvreType::KEY);
	static std::shared_ptr<Manoeuvre> load(tinyxml2::XMLElement* manoeuvreXML);
	std::shared_ptr<Manoeuvre> copy();
	bool save(tinyxml2::XMLElement* manoeuvreXML);
	
	//—————————————— General —————————————————
	
public:

	ManoeuvreType getType(){ return type->value; }
	const char* getName(){ return name->value.c_str(); }
	const char* getDescription(){ return description->value.c_str(); }
	
	bool isInManoeuvreList(){ return manoeuvreList != nullptr; }
	void setManoeuvreList(std::shared_ptr<ManoeuvreList> manoeuvreList_){ manoeuvreList = manoeuvreList_; }
	std::shared_ptr<ManoeuvreList> getManoeuvreList(){ return manoeuvreList; }
	
	void select();
	void deselect();
	bool isSelected();
	
	bool isValid(){ return b_valid; }
	void setValid(bool valid){ b_valid = valid; }
	
	std::shared_ptr<StringParameter> name = std::make_shared<StringParameter>("Default Name","Manoeuvre Name","Name",256);
	std::shared_ptr<StringParameter> description = std::make_shared<StringParameter>("Default Description","Manoeuvre Description","Description",512);
	std::shared_ptr<EnumeratorParameter<ManoeuvreType>> type = std::make_shared<EnumeratorParameter<ManoeuvreType>>(ManoeuvreType::KEY,"Manoeuvre Type","Type");
	std::shared_ptr<ManoeuvreList> manoeuvreList = nullptr;
	
	//—————————————— Animations —————————————————
	
public:
		
	bool hasAnimation(std::shared_ptr<Animatable> animatable);
	void addAnimation(std::shared_ptr<Animatable> animatable);
	void removeAnimation(std::shared_ptr<Animatable> animatable);
	void moveAnimation(int oldIndex, int newIndex);
	
	std::vector<std::shared_ptr<Animation>>& getAnimations(){ return animations; }
	
	void subscribeAllTracksToMachineParameter();
	void unsubscribeAllTracksFromMachineParameter();
	
	void updateAnimationSummary();
	void validateAllAnimations();
	
	//————————————— Curve Editor ————————————
public:
	void selectEditorCurve(std::shared_ptr<Animation> animation, std::shared_ptr<Motion::Curve> curve){
		selectedEditorAnimation = animation;
		selectedEditorCurve = curve;
	}
	
	void selectControlPoint(std::shared_ptr<Motion::ControlPoint> controlPoint){
		clearControlPointSelection();
		selectedControlPoints.push_back(controlPoint);
		controlPoint->b_selected = true;
	}
	
	void clearControlPointSelection(){
		for(auto controlPoint : selectedControlPoints) controlPoint->b_selected = false;
		selectedControlPoints.clear();
	}
	
	void clearEditorCurveSelection(){
		selectedEditorCurve = nullptr;
		selectedEditorAnimation = nullptr;
		clearControlPointSelection();
	}
	
	void deletedControlPointSelection(){
		for(auto controlPoint : selectedControlPoints){
			selectedEditorCurve->removePoint(controlPoint);
		}
	}
	
	bool isCurveSelectedInEditor(std::shared_ptr<Motion::Curve> curve) { return selectedEditorCurve == curve; }
	std::shared_ptr<Motion::Curve> getSelectedEditorCurve(){ return selectedEditorCurve; }
	std::shared_ptr<Animation> getSelectedEditorAnimation(){ return selectedEditorAnimation; }
	std::vector<std::shared_ptr<Motion::ControlPoint>>& getSelectedControlPoints(){ return selectedControlPoints; }
private:
	std::shared_ptr<Animation> selectedEditorAnimation = nullptr;
	std::shared_ptr<Motion::Curve> selectedEditorCurve = nullptr;
	std::vector<std::shared_ptr<Motion::ControlPoint>> selectedControlPoints = {};
	//—————————————— Playback —————————————————
	
public:

	bool canRapidToStart();
	bool canRapidToTarget();
	bool canRapidToPlaybackPosition();
	bool canStartPlayback();
	bool canPausePlayback();
	bool canStop();
	
	bool isAtStart();
	bool isAtTarget();
	bool isAtPlaybackPosition();
	
	void rapidToStart();
	void rapidToTarget();
	void rapidToPlaybackPosition();

	void startPlayback();
	void pausePlayback();
	void stop();
	
	void updatePlaybackState();
	bool hasActiveAnimations(){ return b_hasActiveAnimations; }
	std::vector<std::shared_ptr<Animation>> getActiveAnimations();
	
	double getSychronizedPlaybackPosition();
	double getRemainingPlaybackTime();
	bool canSetPlaybackPosition();
	void setSynchronizedPlaybackPosition(double seconds);
	
	bool hasDuration();
	void updateDuration();
	double getDuration();
	
	void getCurveRange(double& minX, double& maxX, double& minY, double& maxY);
	
	bool areAllMachinesEnabled();
	bool areNoMachinesEnabled();

private:
	
	bool b_hasActiveAnimations = false;
	double synchronizedPlaybackPosition;
	std::vector<std::shared_ptr<Animation>> animations;
	double duration_seconds = 0.0;
	bool b_valid = false;
	
	//—————————— User Interface ———————————
	
public:
	
	void listGui();
	void miniatureGui(glm::vec2 size_arg);
	
	void sheetEditor();
	void curveEditor();
	void spatialEditor();
	
	void requestCurveRefocus();
	bool shouldRefocusCurves();
};
