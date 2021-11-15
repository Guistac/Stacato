#pragma once

class ParameterTrack;
class AnimatableParameter;
class Plot;

namespace tinyxml2 { class XMLElement; }

struct ManoeuvreType {
	enum class Type {
		KEY_POSITION,
		TIMED_MOVEMENT,
		MOVEMENT_SEQUENCE
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
	const char shortName[16];
};
std::vector<ManoeuvreType>& getManoeuvreTypes();
ManoeuvreType* getManoeuvreType(ManoeuvreType::Type t);
ManoeuvreType* getManoeuvreType(const char* saveName);



class Manoeuvre : public std::enable_shared_from_this<Manoeuvre> {
public:

	Manoeuvre(std::shared_ptr<Plot> p) : parentPlot(p) {}
	Manoeuvre(const Manoeuvre& original);

	char name[64] = "";
	char description[256] = "";
	std::shared_ptr<Plot> parentPlot = nullptr;

	void setType(ManoeuvreType::Type t);
	ManoeuvreType::Type type = ManoeuvreType::Type::KEY_POSITION;

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
	static void sequenceEditGui(const std::shared_ptr<Manoeuvre>& manoeuvre);
	static void playbackControlGui(const std::shared_ptr<Manoeuvre>& manoeuvre);
	static float getPlaybackControlGuiHeight(const std::shared_ptr<Manoeuvre>& manoeuvre);

	bool save(tinyxml2::XMLElement* manoeuvreXML);
	bool load(tinyxml2::XMLElement* manoeuvreXML);

};

