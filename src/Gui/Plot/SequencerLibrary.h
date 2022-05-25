#pragma once

#include <imgui.h>
#include <vector>

namespace Sequencer{
	
	typedef bool (*RegionTrackCompatFun)(int, int);
	typedef void (*TickStringFunction)(char*, long long int, long long int);

	struct RegionInfo;
	struct TrackInfo;
	struct TrackGroupInfo;
	struct Style;

	struct TickSpacing{
		long long int major;
		long long int minor;
	};

	struct EditorSubUnitTime{
		long long int time;
		double subTimeUnit;
	};

	struct Style{
		double 	trackHeaderWidth 				= 400.0;					//(pixels)
		double 	timelineHeight 					= 30.0;						//(pixels)
		ImVec4 	trackContainerBackgroundColor	{0.2f, 0.2f, 0.2f, 1.0f};
		ImVec4 	editorBackgroundColor			{0.25f, 0.25f, 0.25f, 1.0f};
		ImVec4 	emptySpaceOverlayColor			{0.0f, 0.0f, 0.0f, 0.3f};
		
		ImVec4 	selectionRectangleColor			{1.0f, 1.0f, 1.0f, 0.2f};
		ImVec4 	selectionRectangleBorderColor	{1.0f, 1.0f, 1.0f, 0.5f};
		
		double 	trackGroupHeight 				= 25.0;						//(pixels)
		double 	trackGroupIndentation 			= 15.0;						//(pixels)
		ImVec4 	trackGroupColor					{0.3f, 0.3f, 0.3f, 1.0f};
		ImVec4 	trackGroupNameColor				{1.0f, 1.0f, 1.0f, 1.0f};
		ImFont* trackGroupFont					= nullptr;
		float 	trackGroupBevelSize 			= 2.0;
		ImVec4 	trackGroupEditorColor			{0.0f, 0.0f, 0.0f, 0.6f};
		ImVec4 	trackGroupFoldIconColor			{1.0f, 1.0f, 1.0f, 1.0f};
		float 	trackGroupFolderIconPadding 	= 4.0;
		
		double 	trackHeight 					= 50.0; 					//(pixels)
		ImVec4 	trackColor						{0.5f, 0.3f, 0.2f, 1.0f};
		ImVec4 	trackNameColor					{1.0f, 1.0f, 1.0f, 1.0f};
		ImFont* trackNameFont					= nullptr;
		float 	trackBevelSize					= 2.0;
		ImVec4 	trackEditorHoverColor			{0.0f, 0.0f, 0.0f, 0.1f};
		ImVec4 	trackSeparatorLineHighlight		{0.5f, 0.5f, 0.5f, 1.0f};
		ImVec4 	trackSeparatorLineShadow		{0.0f, 0.0f, 0.0f, 0.5f};
		float 	trackUserSpaceWidth				= 200.0f;
		
		ImVec4 	regionColor						{0.2f, 0.7f, 0.3f, 1.0f};
		ImVec4 	regionNameColor					{0.0f, 0.0f, 0.0f, 0.8f};
		ImFont* regionNameFont					= nullptr;
		float 	regionBevelSize					= 2.0;
		float 	eventWidth						= 10.0;
		
		ImVec4 	crossBackgroundColor			{0.2f, 0.2f, 0.2f, 1.0f};
		ImVec4 	crossTimeStringColor			{1.0f, 1.0f, 1.0f, 1.0f};
		ImFont* crossTimeStringFont				= nullptr;
		
		double 	maxScreenSpaceBetweenTicks 		= 100.0; 					//(pixels) controls tick change when zooming
		ImVec2 	playheadSize					{20.0, 18.0};				//(pixels)
		ImVec4 	timelineBackgroundColor			{0.17f, 0.17f, 0.17f, 1.0f};
		ImVec4 	majorTickColor					{1.0f, 1.0f, 1.0f, 0.6f};
		ImVec4 	minorTickColor					{1.0f, 1.0f, 1.0f, 0.3f};
		ImVec4 	tickStringColor					{1.0f, 1.0f, 1.0f, 1.0f};
		ImFont* tickStringFont					= nullptr;
		ImVec4 	majorTickEditorColor			{0.1f, 0.1f, 0.1f, 1.0f};
		ImVec4 	playheadColor					{1.0f, 1.0f, 1.0f, 1.0f};
		ImVec4 	playheadShadow					{0.0f, 0.0f, 0.0f, 0.4f};
		ImVec4 	playheadGrabColor				{1.0f, 1.0f, 0.0f, 1.0f};
	};

	struct Context{
		long long int playbackTime = 0;	//(time units)
		long long int maxTime;			//(time units)
		long long int minTime;			//(time units)
		
		double zoomX = 0.0001; 	//(pixels per time unit)
		double panX = 100.0;	//(pixels) offset relative to zero time
		
		double zoomY = 1.0;
		double panY = 0.0;
		
		double maxZoom = 101.0;			//(pixels per time unit)
		double minZoom = 0.0000000001;	//(pixels per time unit)
		
		std::vector<TickSpacing>* tickSpacings;
		TickStringFunction tickStringFunction;
		
		double screenDragPositionX = 0.0;
		RegionInfo* hoveredRegion = nullptr;
		RegionInfo* draggedRegion = nullptr;
		
		bool b_selectionRectangleActive = false;
		EditorSubUnitTime selectionRectangleStartTime;
		double selectionRectangleStartPosY;
		bool b_movingPlayhead = false;
		
		std::vector<int> selectedRegionIds;
		std::vector<int> selectionCanditateRegionIds;
		
		RegionTrackCompatFun regionCompatibilityFunction = [](int, int) -> bool{ return true; };
		
		Style style;
	};

	Context* createContext();
	void setContext(Context* ctx);
	bool hasContext();
	Context* getContext();

	void setRegionColor(ImVec4 color);
	void setTrackColor(ImVec4 color);

	void setTimeMicrosecondsZeroCentered();
	void setTimeNanosecondsZeroCentered();

	//====== Sequencer

	bool begin(const char* ID, ImVec2 size_arg = ImVec2(.0f,.0f));
	void end(bool b_wasOpen = true);

	//====== Track Group

	struct TrackGroupInfo{
		const char* name;
		float f_positionY;
		bool b_collapsed;
		bool b_parentCollapsed;
		bool b_offscreen;
	};
	void beginTrackGroup(const char* name);
	void endTrackGroup();
	
	//======= Track

	bool beginTrack(int trackID, const char* name, int trackType = -1);
	void endTrack(bool b_wasOpen = true);

	bool beginMultiTrack(const char* masterName, int* trackIDs, const char** names, int trackCount, int* trackTypes = nullptr);
	void endMultiTrack(bool b_wasOpen = true, bool b_enclosingGroupClosed = false);

	ImVec2 getTrackHeaderUserSpacePos();
	ImVec2 getTrackHeaderUserSpaceSize();

	bool TrackButton(const char* text, ImVec2 size_arg);

	struct TrackInfo{
		int id;
		float positionY;
		float sizeY;
		int type;
		bool b_collapsed;
	};
	TrackInfo* getTrackByID(int trackID);
	TrackInfo* getTrackByIndex(int trackIndex);
	int getTrackIndex(int trackID);

	//====== Region / Event

	bool beginRegion(int regionID, int& trackID, const char* name, long long int& startTime, long long int& length, int regionType = -1, int linkedMasterRegionID = -1);
	void endRegion(bool b_wasOpen = true);

	void event(int regionID, int& trackID, const char* name, long long int& time, int regionType = -1, int linkedMasterRegionID = -1);

	struct RegionInfo{
		int id;
		int* trackId;
		long long int* startTimePtr;
		long long int* durationPtr; //null if is event
		ImVec2 rectMin;
		ImVec2 rectMax;
		ImVec2 rectSize;
		const char* name;
		bool selected;
		int type;
		bool b_collapsed;
		int linkedRegionMasterId;
		bool b_hasLinkingMaster;
		bool b_isLinkingMaster;
	};
	RegionInfo* getRegion(int regionID);

	void selectRegion(int regionID);
	void deselectRegion(int regionID);
	void deselectAllRegions();
	bool isRegionSelected(int regionID);
	int getSelectedRegionCount();
	bool isRegionHovered(int regionID);

	std::vector<RegionInfo*> getMovedRegions();
	std::vector<RegionInfo*> getLinkedRegions(RegionInfo*);

	void addRegionSelectionCandidate(int regionID);
	void removeRegionSelectionCanditate(int regionID);
	void removeAllRegionSelectionCandidates();
	bool isRegionSelectionCanditate(int regionID);

	//====== Screen/Time Conversions
	
	long long int screenToTime(double screenX);
	double timeToScreen(long long int time);

	EditorSubUnitTime screenToSubUnitTime(double screenX);
	double subUnitTimeToScreen(EditorSubUnitTime& time);

	long long int screenDistanceToDuration(double screenDx);
	double durationToScreenDistance(long long int time);

	//====== Color Conversions

	void modifySaturation(ImVec4& color, float amount);
	void modifyBrightness(ImVec4& color, float amount);
	void modifySaturationBrightness(ImVec4& color, float saturationChange, float brightnessChange);

	ImVec4 modifiedSaturation(ImVec4 color, float amount);
	ImVec4 modifiedBrightness(ImVec4 color, float amount);
	ImVec4 modifiedSaturationBrightness(ImVec4 color, float saturationChange, float brightnessChange);
}


inline ImVec2 operator+(const ImVec2& vec, const ImVec2 value) { return ImVec2{ vec.x + value.x, vec.y + value.y }; }
inline ImVec2 operator-(const ImVec2& vec, const ImVec2 value) { return ImVec2{ vec.x - value.x, vec.y - value.y }; }

inline ImVec2 operator+(const ImVec2& vec, const float value) { return ImVec2{ vec.x + value, vec.y + value }; }
inline ImVec2 operator-(const ImVec2& vec, const float value) { return ImVec2{ vec.x - value, vec.y - value }; }
inline ImVec2 operator*(const ImVec2& vec, const float value) { return ImVec2{ vec.x * value, vec.y * value }; }
inline ImVec2 operator/(const ImVec2& vec, const float value) { return ImVec2{ vec.x / value, vec.y / value }; }

inline bool operator==(const ImVec2& vec, const float value) { return vec.x == value || vec.y == value; }

inline ImVec4 operator*(const ImVec4& vec, const float mult) { return ImVec4{ vec.x * mult, vec.y * mult, vec.z * mult, vec.w }; }
