#pragma once

class ParameterTrack;
class AnimatableParameter;
class Plot;

namespace tinyxml2 { class XMLElement; }


class Manoeuvre : public std::enable_shared_from_this<Manoeuvre> {
public:

	enum class Type {
		KEY_POSITION,
		TIMED_MOVEMENT,
		MOVEMENT_SEQUENCE
	};
	
	Manoeuvre(std::shared_ptr<Plot> p) : parentPlot(p) {}
	Manoeuvre(const Manoeuvre& original);

	char name[64] = "";
	char description[256] = "";
	std::shared_ptr<Plot> parentPlot = nullptr;
	const char* getShortTypeString(){
		switch(type){
			case Type::KEY_POSITION: return "KEY";
			case Type::TIMED_MOVEMENT: return "TIM";
			case Type::MOVEMENT_SEQUENCE: return "SEQ";
		}
	}

	void setType(Type t);
	Type type = Type::KEY_POSITION;

	std::vector<std::shared_ptr<ParameterTrack>> tracks;
	void addTrack(std::shared_ptr<AnimatableParameter>& parameter);
	void removeTrack(std::shared_ptr<AnimatableParameter>& parameter);
	bool hasTrack(std::shared_ptr<AnimatableParameter>& parameter);
	std::shared_ptr<ParameterTrack> getTrack(std::shared_ptr<AnimatableParameter>& parameter);
	
	void refresh();
	bool b_valid = false;

	double getLength_seconds();
	double playbackStartTime_seconds = 0.0;
	double playbackPosition_seconds = 0.0;
	float getPlaybackProgress();
	bool b_isPaused = false;

	static void listGui(const std::shared_ptr<Manoeuvre>& manoeuvre);
	static void editGui(const std::shared_ptr<Manoeuvre>& manoeuvre);
    
    static void trackSheetGui(const std::shared_ptr<Manoeuvre>& manoeuvre);
    static void curveEditorGui(const std::shared_ptr<Manoeuvre>& manoeuvre);
    static void spatialEditorGui(const std::shared_ptr<Manoeuvre>& manoeuvre);
    
	static void sequenceEditGui(const std::shared_ptr<Manoeuvre>& manoeuvre);
	static void playbackControlGui(const std::shared_ptr<Manoeuvre>& manoeuvre);
	static float getPlaybackControlGuiHeight(const std::shared_ptr<Manoeuvre>& manoeuvre);

	bool save(tinyxml2::XMLElement* manoeuvreXML);
	bool load(tinyxml2::XMLElement* manoeuvreXML);

};

#define ManoeuvreTypeStrings \
	{Manoeuvre::Type::KEY_POSITION,		"Key Position",		"KeyPosition"},\
	{Manoeuvre::Type::TIMED_MOVEMENT,	"Timed Movement",	"TimedMovement"},\
	{Manoeuvre::Type::MOVEMENT_SEQUENCE,"Movement Sequence","MovementSequence"}\

DEFINE_ENUMERATOR(Manoeuvre::Type, ManoeuvreTypeStrings)
