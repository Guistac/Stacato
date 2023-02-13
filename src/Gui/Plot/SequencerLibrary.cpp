#include <pch.h>
#include "SequencerLibrary.h"

#include <GLFW/glfw3.h>
#include <imgui_internal.h>

namespace SequencerLibrary{

Context* context = nullptr;
Style* style = nullptr;
ImDrawList* canvas = nullptr;

//size & absolute position of main sequencer elements
ImVec2 windowPos;
ImVec2 windowSize;
ImVec2 crossSize;
ImVec2 trackHeaderPos;
ImVec2 trackHeaderSize;
ImVec2 timelinePos;
ImVec2 timelineSize;
ImVec2 editorPos;
ImVec2 editorSize;
ImVec2 editorEndPos;

int i_trackGroupNestingLevel;		//how many track groups deep we are
float f_trackIndentation;			//current track indentation inside track groups
float f_currentTrackHeaderWidth;	//current track header width inside track groups
std::vector<TrackGroupInfo> trackGroups; //non retained list of submitted track groups, automatically cleared while submitting groups

float f_trackHeight; 			//current track height after vertical zoom
float f_currentTrackPositionY;	//increment position of tracks while submitting the track stack
float f_trackStackHeight;		//total height of track stack, used for vertical scrolling
std::vector<TrackInfo> tracks; 	//non retained list of tracks, used for placing regions after submitting the tracks
TrackInfo* hoveredTrack;		//currently hovered track, nullptr in none

std::vector<RegionInfo> regions;	//non retained list of regions
RegionInfo* currentHoveredRegion;	//used to keep track of hovered region during submission

bool b_mouseOverTrackList;	//hovering tracklist
bool b_mouseOverEditor;		//hovering editor
bool b_mouseOverTimeline;	//hovering timeline

//========================== CONTEXT ==============================

Context* createContext(){
	Context* newContext = new Context();
	context = newContext;
	setTimeMicrosecondsZeroCentered();
	return newContext;
}

bool hasContext(){ return context != nullptr; }
void setContext(Context* ctx){ context = ctx; }

Context* getContext(){
	return context;
}

void setTrackColor(ImVec4 color){
	context->style.trackColor = color;
}

void setRegionColor(ImVec4 color){
	context->style.regionColor = color;
}

//========================= CONVERSION ============================

long long int screenToTime(double screenX){
	return (screenX - editorPos.x - context->panX) / context->zoomX;
};

double timeToScreen(long long int timeX){
	return timeX * context->zoomX + editorPos.x + context->panX;
};

EditorSubUnitTime screenToSubUnitTime(double screenX){
	long long int timeUnits = screenToTime(screenX);
	double subTimeUnits = (screenX - timeToScreen(timeUnits)) / context->zoomX;
	return {timeUnits, subTimeUnits};
}

double subUnitTimeToScreen(EditorSubUnitTime& time){
	double roundedScreenPosition = timeToScreen(time.time);
	return roundedScreenPosition + time.subTimeUnit * context->zoomX;
}

long long int screenDistanceToDuration(double screenDx){
	return screenDx / context->zoomX;
}

double durationToScreenDistance(long long int duration){
	return duration * context->zoomX;
}

//=================================================================
//======================== SEQUENCER ==============================
//=================================================================

bool begin(const char* ID, ImVec2 size_arg){
	assert(hasContext());
	
	if(size_arg.x == 0.0 || size_arg.y == 0.0) size_arg = ImGui::GetContentRegionAvail();
	
	//main window containing the whole sequencer
	//completely disable scrolling in this window
	bool b_isOpen = ImGui::BeginChild(ID, size_arg, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	if(!b_isOpen){
		end(false);
		return false;
	}
	
	style = &context->style;
	
	//reset immediate state
	canvas = ImGui::GetWindowDrawList();
	windowPos = ImGui::GetWindowPos();
	windowSize = size_arg;
	crossSize = ImVec2(style->trackHeaderWidth, style->timelineHeight);
	trackHeaderPos = windowPos + ImVec2(0, style->timelineHeight);
	trackHeaderSize = ImVec2(style->trackHeaderWidth, windowSize.y - style->timelineHeight);
	timelinePos = windowPos + ImVec2(style->trackHeaderWidth, 0);
	timelineSize = ImVec2(windowSize.x - style->trackHeaderWidth, style->timelineHeight);
	editorPos = windowPos + crossSize;
	editorSize = windowSize - crossSize;
	editorEndPos = editorPos + editorSize;
	f_currentTrackPositionY = style->timelineHeight + context->panY;
	f_trackHeight = style->trackHeight * context->zoomY;
	f_trackStackHeight = 0;
	i_trackGroupNestingLevel = 0;
	f_trackIndentation = 0.0;
	f_currentTrackHeaderWidth = trackHeaderSize.x;
	tracks.clear();
	regions.clear();
	hoveredTrack = nullptr;
	currentHoveredRegion = nullptr;
	
	//draw main window backgrounds
	canvas->AddRectFilled(windowPos, editorPos, ImColor(style->crossBackgroundColor));
	canvas->AddRectFilled(trackHeaderPos, trackHeaderPos + trackHeaderSize, ImColor(style->trackContainerBackgroundColor));
	canvas->AddRectFilled(timelinePos, timelinePos + timelineSize, ImColor(style->timelineBackgroundColor));
	canvas->AddRectFilled(editorPos, editorEndPos, ImColor(style->editorBackgroundColor));
	
	
	/*
	//cross time
	char timeString[128];
	context->tickStringFunction(timeString, context->playbackTime, 1000);
	float textOffset = (crossSize.y - ImGui::GetFrameHeight()) / 2.0;
	ImGui::SetCursorPos(ImVec2(10,textOffset));
	ImGui::SetNextItemWidth(crossSize.x - 20);
	ImGui::PushStyleColor(ImGuiCol_Text, style->crossTimeStringColor);
	if(style->crossTimeStringFont) ImGui::PushFont(style->crossTimeStringFont);
	ImGui::InputText("##currenttime", timeString, 128);
	if(style->crossTimeStringFont) ImGui::PopFont();
	ImGui::PopStyleColor();
	*/
	 
	//get tick spacing for current zoom level
	long long int maxTickSpacingDuration = screenDistanceToDuration(style->maxScreenSpaceBetweenTicks);
	std::vector<TickSpacing>& tickSpacings = *context->tickSpacings;
	TickSpacing spacing = tickSpacings.back();
	for(int i = 0; i < tickSpacings.size(); i++){
		if (tickSpacings[i].major > maxTickSpacingDuration){
			spacing = tickSpacings[i];
			break;
		}
	}
	
	//clip rectangle for timeline & editor
	ImGui::PushClipRect(timelinePos, editorEndPos, false);
	
	//draw timeline ticks
	long long int screenStartTime = screenToTime(editorPos.x);
	long long int sreenEndTime = screenToTime(editorEndPos.x);
	long long int firstTickTime = ((screenStartTime / spacing.major) - 1) * spacing.major;
	long long int lastTickTime = ((sreenEndTime / spacing.major) + 1) * spacing.major;
	long long int currentMajorTickTime = firstTickTime;
	while(currentMajorTickTime < lastTickTime){
		double majorTickScreenPosX = timeToScreen(currentMajorTickTime);
		static char timeString[64];
		context->tickStringFunction(timeString, currentMajorTickTime, spacing.major);
		canvas->AddLine(ImVec2(majorTickScreenPosX, timelinePos.y),
						ImVec2(majorTickScreenPosX, editorPos.y),
						ImColor(style->majorTickColor));
		canvas->AddText(ImGui::GetFont(),
						ImGui::GetFontSize() * 0.85f,
						ImVec2(majorTickScreenPosX + 5.0f, timelinePos.y),
						ImColor(style->tickStringColor),
						timeString);
		canvas->AddLine(ImVec2(majorTickScreenPosX, editorPos.y),
						ImVec2(majorTickScreenPosX, editorEndPos.y),
						ImColor(style->majorTickEditorColor));
		long long int currentMinorTickTime = currentMajorTickTime + spacing.minor;
		long long int nextMajorTickTime = currentMajorTickTime + spacing.major;
		while(currentMinorTickTime < nextMajorTickTime){
			float minorTickScreenPosX = timeToScreen(currentMinorTickTime);
			canvas->AddLine(ImVec2(minorTickScreenPosX, timelinePos.y + ((float)style->timelineHeight * 0.5f)),
							ImVec2(minorTickScreenPosX, editorPos.y),
							ImColor(style->minorTickColor));
			currentMinorTickTime += spacing.minor;
		}
		currentMajorTickTime += spacing.major;
	}
	
	//timeline interaction element
	ImGui::SetCursorPos(ImVec2(crossSize.x, 0));
	ImGui::SetItemAllowOverlap();
	ImGui::InvisibleButton("##Timeline", timelineSize);
	b_mouseOverTimeline = ImGui::IsItemHovered();
	
	//pop timeline and editor clip rectangle
	ImGui::PopClipRect();
	
	//push track header and editor clip rectangle (popped by end())
	//ImGui::PushClipRect(trackHeaderPos, editorEndPos, false);
	
	//track header scrolling
	ImGui::SetCursorPos(ImVec2(0, style->timelineHeight));
	ImGui::SetItemAllowOverlap();
	ImGui::InvisibleButton("##TrackHeader", trackHeaderSize);
	b_mouseOverTrackList = ImGui::IsItemHovered();
	
	ImGuiID hoverID = ImGui::GetHoveredID();
	ImGuiID userID = ImGui::GetID("##TrackUserspace");
	bool same = (unsigned int)hoverID == (unsigned int)userID;

	if(b_mouseOverTrackList){
		context->panY += ImGui::GetIO().MouseWheel * 10.0;
		//context->zoomY *= 1.0 + ApplicationWindow::getMacOsTrackpadZoom();
		if(context->zoomY < 0.5) context->zoomY = 0.5;
		if(context->zoomY > 5.0) context->zoomY = 5.0;
	}
	
	//editor scrolling and zooming
	ImGui::SetCursorPos(ImVec2(trackHeaderSize.x, style->timelineHeight));
	ImGui::SetItemAllowOverlap();
	ImGui::InvisibleButton("##Editor", editorSize);
	b_mouseOverEditor = ImGui::IsItemHovered();
	if(b_mouseOverEditor || b_mouseOverTimeline){
		//handle zooming, taking care of maintaining position under mouse
		//even in sub-integer time division
		float zoomDelta = 0.0;//ApplicationWindow::getMacOsTrackpadZoom();
		if(zoomDelta){
			double screenPositionBeforeZoom = ImGui::GetMousePos().x;
			long long int screenTimeBeforeZoom = screenToTime(screenPositionBeforeZoom);
			double screenPositionBeforeZoomRounded = timeToScreen(screenTimeBeforeZoom);
			double screenTimeDecimal = (screenPositionBeforeZoom - screenPositionBeforeZoomRounded) / context->zoomX;
			context->zoomX *= 1.0 + zoomDelta;
			context->zoomX = std::min(context->zoomX, context->maxZoom);
			context->zoomX = std::max(context->zoomX, context->minZoom);
			double screenPositionTimeDecimal = screenTimeDecimal * context->zoomX;
			double screenPositionAfterZoom = timeToScreen(screenTimeBeforeZoom) + screenPositionTimeDecimal;
			context->panX -= screenPositionAfterZoom - screenPositionBeforeZoom;
		}
		//handle editor time scrolling
		context->panX += ImGui::GetIO().MouseWheelH * 10.0;
		context->panY += ImGui::GetIO().MouseWheel * 10.0;
		//limit zoom & pan to editor min & max time
		long long int maxScreenTime = screenToTime(editorEndPos.x);
		long long int minScreenTime = screenToTime(editorPos.x);
		if(maxScreenTime >= context->maxTime && minScreenTime <= context->minTime){
			context->zoomX = editorSize.x / (context->maxTime - context->minTime);
			context->panX = -context->minTime * context->zoomX;
		}
		else if(maxScreenTime > context->maxTime) context->panX = editorSize.x - context->maxTime * context->zoomX;
		else if(minScreenTime < context->minTime) context->panX = -context->minTime * context->zoomX;
	}
	
	return true;
}

void end(bool b_wasOpen){
	
	if(!b_wasOpen) {
		ImGui::EndChild(); //early out of main sequencer window
		return;
	}
	
	//add dark region below tracks if they don't fill the vertical space
	if(f_trackStackHeight < editorSize.y) canvas->AddRectFilled(ImVec2(windowPos.x, editorPos.y) + ImVec2(0, f_trackStackHeight),
																ImVec2(windowPos.x, editorPos.y) + ImVec2(windowSize.x, editorSize.y),
																ImColor(style->emptySpaceOverlayColor));
	
	//pop track header and editor clip rectangle
	//ImGui::PopClipRect();
	
	//push timeline & editor clipping rectangle
	ImGui::PushClipRect(timelinePos, editorEndPos, false);
	
	//timeline/editor separator line
	float lineHeight = editorPos.y;
	canvas->AddLine(ImVec2(editorPos.x, lineHeight + 1),
					ImVec2(editorEndPos.x, lineHeight + 1),
					ImColor(style->trackSeparatorLineShadow), 3.0);
	canvas->AddLine(ImVec2(editorPos.x, lineHeight),
					ImVec2(editorEndPos.x, lineHeight),
					ImColor(style->trackSeparatorLineHighlight), 1.0);
	
	//trackContainer/editor separator line
	float linePosX = editorPos.x;
	canvas->AddLine(ImVec2(linePosX + 1, windowPos.y),
					ImVec2(linePosX + 1, windowPos.y + windowSize.y),
					ImColor(style->trackSeparatorLineShadow), 3.0);
	canvas->AddLine(ImVec2(linePosX, windowPos.y),
					ImVec2(linePosX, windowPos.y + windowSize.y),
					ImColor(style->trackSeparatorLineHighlight), 1.0);
	
	//draw playhead
	double playheadPosX = timeToScreen(context->playbackTime);
	ImVec2 playheadTip(playheadPosX, timelinePos.y + timelineSize.y);
	if ((b_mouseOverTimeline && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) || context->b_movingPlayhead) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
	ImVec4 playheadGrabColor = 		style->playheadGrabColor;
	if(context->b_movingPlayhead) 	modifyBrightness(playheadGrabColor, -.2f);
	else if(b_mouseOverTimeline)		modifyBrightness(playheadGrabColor, .5f);
	ImVec2 playheadShape[5] = {
		playheadTip,
		playheadTip - ImVec2(-style->playheadSize.x * 0.5, style->playheadSize.x * 0.5),
		playheadTip - ImVec2(-style->playheadSize.x * 0.5, style->playheadSize.y),
		playheadTip - ImVec2(style->playheadSize.x * 0.5, style->playheadSize.y),
		playheadTip - ImVec2(style->playheadSize.x * 0.5, style->playheadSize.x * 0.5)
	};
	canvas->AddRectFilled(ImVec2(playheadPosX - 0.5, editorPos.y - style->playheadSize.y),
						  ImVec2(playheadPosX + 2.0, editorEndPos.y),
						  ImColor(style->playheadShadow));
	canvas->AddRectFilled(ImVec2(playheadPosX - 0.5, editorPos.y - style->playheadSize.y),
						  ImVec2(playheadPosX + 0.5, editorEndPos.y),
						  ImColor(style->playheadColor));
	canvas->AddConvexPolyFilled(playheadShape, 5, ImColor(playheadGrabColor));
	
	//pop timeline & editor clipping rectangle
	ImGui::PopClipRect();
	
	//limit vertical scrolling
	if(trackHeaderSize.y > f_trackStackHeight) context->panY = 0.0;
	else{
		float panLimit = trackHeaderSize.y - f_trackStackHeight;
		if(context->panY < panLimit) context->panY = panLimit;
		else if(context->panY > 0) context->panY = 0;
	}
	
	if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
		if(b_mouseOverTimeline) {
			//we clicked the timeline, set playhead position on timeline press
			context->b_movingPlayhead = true;
			context->playbackTime = screenToTime(ImGui::GetMousePos().x);
		}else if(b_mouseOverEditor && !currentHoveredRegion){
			//we clicked the editor background, remember the time under the mouse to do dragging later
			context->selectionRectangleStartPosY = ImGui::GetMousePos().y;
			context->selectionRectangleStartTime = screenToSubUnitTime(ImGui::GetMousePos().x);
		}else{
			//we clicked on a region, remember the clicked X position
			context->screenDragPositionX = ImGui::GetMousePos().x;
		}
	}
	
	//handle mouse dragging
	if(ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
	
		//handle timeline or playhead scrubbing
		if(context->b_movingPlayhead) context->playbackTime = screenToTime(ImGui::GetMousePos().x);
		
		//handle dragging of a region
		else if(context->draggedRegion){
			
			//handle dragging in X axis
			double screenDragPositionDelta = ImGui::GetMousePos().x - context->screenDragPositionX;
			long long int dragTimeDelta = screenDistanceToDuration(screenDragPositionDelta);
			context->screenDragPositionX += durationToScreenDistance(dragTimeDelta);
			
			std::vector<RegionInfo*> movedRegions = getMovedRegions();
			for(int i = 0; i < movedRegions.size(); i++){
				*movedRegions[i]->startTimePtr += dragTimeDelta;
			}
			
		
			//change region track
			if(hoveredTrack && hoveredTrack->id != *context->draggedRegion->trackId ){
				
				int draggedRegionTrackIndex = getTrackIndex(*context->draggedRegion->trackId);
				int hoveredTrackIndex = getTrackIndex(hoveredTrack->id);
				int trackIndexShift = hoveredTrackIndex - draggedRegionTrackIndex;
				
				std::vector<RegionInfo*> movedRegions = getMovedRegions();
				
				//if not regions are selected
				if(!isRegionSelected(context->draggedRegion->id)){
					//check if movement is valid before moving
					if(!hoveredTrack->b_collapsed &&
					   context->regionCompatibilityFunction(hoveredTrack->type, context->draggedRegion->type))
						*context->draggedRegion->trackId = hoveredTrack->id;
				}
				
				//if there is a region selection
				else{
					//check if all movements are valid before moveing
					bool b_allTracksAndRegionsCompatible = true;
					for(int i = 0; i < context->selectedRegionIds.size(); i++){
						RegionInfo* region = getRegion(context->selectedRegionIds[i]);
						int targetTrackIndex = getTrackIndex(*region->trackId) + trackIndexShift;
						TrackInfo* targetTrack = getTrackByIndex(targetTrackIndex);
						if(!targetTrack ||
						   targetTrack->b_collapsed ||
						   !context->regionCompatibilityFunction(targetTrack->type, context->draggedRegion->type)){
							b_allTracksAndRegionsCompatible = false;
							break;
						}
					}
					if(b_allTracksAndRegionsCompatible){
						for(int i = 0; i < context->selectedRegionIds.size(); i++){
							RegionInfo* region = getRegion(context->selectedRegionIds[i]);
							int targetTrackIndex = getTrackIndex(*region->trackId) + trackIndexShift;
							TrackInfo* targetTrack = getTrackByIndex(targetTrackIndex);
							*region->trackId = targetTrack->id;
						}
					}
				}
			}
		}
		
		//handle dragging of region selection rectangle
		else if(context->b_selectionRectangleActive || b_mouseOverEditor){
			
			context->b_selectionRectangleActive = true;
			
			EditorSubUnitTime selectionRectangleMaxTime = screenToSubUnitTime(ImGui::GetMousePos().x);
			double selectionRectangleMaxPosY = ImGui::GetMousePos().y;
			
			ImVec2 rectangleMin(subUnitTimeToScreen(context->selectionRectangleStartTime), context->selectionRectangleStartPosY);
			ImVec2 rectangleMax = ImGui::GetMousePos();
			
			//adjust rectangle coordinates to always be in the right order:
			//min = top/left | max = bottom/right
			float left, right, top, bottom;
			if(rectangleMin.x < rectangleMax.x){
				left = rectangleMin.x;
				right = rectangleMax.x;
			}else{
				left = rectangleMax.x;
				right = rectangleMin.x;
			}
			if(rectangleMin.y < rectangleMax.y){
				top = rectangleMin.y;
				bottom = rectangleMax.y;
			}else{
				top = rectangleMax.y;
				bottom = rectangleMin.y;
			}
			
			//limit to editor size for now
			top = std::max(top, editorPos.y);
			bottom = std::min(bottom, editorEndPos.y);
			left = std::max(left, editorPos.x);
			right = std::min(right, editorEndPos.x);
			
			ImRect selectionRectangle(ImVec2(left, top), ImVec2(right, bottom));
			
			//remove all canditates since we will check them all again right after
			removeAllRegionSelectionCandidates();
			
			//find regions that overlap the selection rectangle
			//add them to the retained context as selection canditates
			for(int i = 0; i < regions.size(); i++){
				RegionInfo* region = &regions[i];
				ImRect regionRectangle(region->rectMin, region->rectMax);
				if(selectionRectangle.Overlaps(regionRectangle) && !region->b_collapsed) addRegionSelectionCandidate(region->id);
			}
			
			//draw selection rectangle
			ImGui::PushClipRect(editorPos, editorEndPos, true);
			canvas->AddRectFilled(selectionRectangle.Min, selectionRectangle.Max, ImColor(style->selectionRectangleColor));
			canvas->AddRect(selectionRectangle.Min, selectionRectangle.Max, ImColor(style->selectionRectangleBorderColor));
			ImGui::PopClipRect();
			
		}
	}
	
	//key to add to selection
	bool b_selectionAddKeyDown = ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_SHIFT);
	
	//handle mouse release
	if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
	
		//if we were moving the playhead
		if(context->b_movingPlayhead) context->b_movingPlayhead = false;
		
		//we were dragging a region
		if(context->draggedRegion) context->draggedRegion = nullptr;
		
		//we were dragging the selection rectangle
		else if(context->b_selectionRectangleActive) {
		   context->b_selectionRectangleActive = false;
		   for(int i = 0; i < context->selectionCanditateRegionIds.size(); i++) selectRegion(context->selectionCanditateRegionIds[i]);
		   removeAllRegionSelectionCandidates();
		}
		
		//we clicked a region
		else if(currentHoveredRegion){
		   if(b_selectionAddKeyDown){
			   if(isRegionSelected(currentHoveredRegion->id)) deselectRegion(currentHoveredRegion->id);
			   else selectRegion(currentHoveredRegion->id);
		   }else {
			   deselectAllRegions();
			   selectRegion(currentHoveredRegion->id);
		   }
		}
		
		//we did not click a region
		else{
		   if(!b_selectionAddKeyDown && b_mouseOverEditor) deselectAllRegions();
		}
	}
	
	if(!ImGui::IsMouseDown(ImGuiMouseButton_Left)){
		//set single hovered region in retained state
		context->hoveredRegion = currentHoveredRegion;
	}
	
	//end main sequencer window
	ImGui::EndChild();
}

//=================================================================
//========================= TRACK GROUP ===========================
//=================================================================

void beginTrackGroup(const char* name){
	ImGui::PushID(name);
	
	//retrieve collapsed state from the retained imgui state
	ImGuiWindow* window = ImGui::GetCurrentContext()->CurrentWindow;
	ImGuiStorage* storage = window->DC.StateStorage;
	ImGuiID id = window->GetID("##GroupFoldButton");
	
	TrackGroupInfo& parentGroup = trackGroups.back();
	trackGroups.push_back(TrackGroupInfo{
		.name = name,
		.b_collapsed = (bool)storage->GetInt(id),
		.b_parentCollapsed = !trackGroups.empty() && (parentGroup.b_collapsed || parentGroup.b_parentCollapsed)
	});
	TrackGroupInfo& currentGroup = trackGroups.back();
	
	//the group needs a y coordinate to draw regions at the group height when collapsed.
	//this coordinate is the current y position in the track stack.
	//if the parent group is collapsed, the current group will not be drawn.
	//But still we need a y coordinate to draw regions later.
	//In this case we copy the y coordinate from the parent group.
	if(currentGroup.b_parentCollapsed) {
		currentGroup.f_positionY = parentGroup.f_positionY;
		return;
	}
	
	currentGroup.f_positionY = f_currentTrackPositionY;
	
	if(currentGroup.b_collapsed){
		
		//if the group is collapsed
		//draw a horizontal track header that serves as a button to unfold the group
				
		currentGroup.b_offscreen = f_currentTrackPositionY + style->trackGroupHeight < timelineSize.y || f_currentTrackPositionY > windowSize.y;
	
		if(!currentGroup.b_offscreen){
			ImGui::SetCursorPos(ImVec2(f_trackIndentation, f_currentTrackPositionY));
			ImGui::SetItemAllowOverlap();
			ImGui::InvisibleButton("##GroupFoldButton", ImVec2(trackHeaderSize.x - f_trackIndentation, style->trackGroupHeight));
			if(ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
				storage->SetInt(id, false);
			}
			ImVec2 headerPosMin = ImGui::GetItemRectMin();
			ImVec2 headerPosMax = ImGui::GetItemRectMax();
			
			ImVec4 groupColor = 			style->trackGroupColor;
			if(ImGui::IsItemActive()) 		modifyBrightness(groupColor, -.1f);
			else if(ImGui::IsItemHovered()) modifyBrightness(groupColor, .1f);

			canvas->AddRectFilled(headerPosMin, headerPosMax, ImColor(groupColor));
			canvas->AddRectFilled(ImVec2(headerPosMin.x, headerPosMax.y - style->trackGroupBevelSize),
								  headerPosMax,
								  ImColor(modifiedBrightness(groupColor, -.15f)));
			canvas->AddRectFilled(headerPosMin,
								  ImVec2(headerPosMax.x, headerPosMin.y + style->trackGroupBevelSize),
								  ImColor(modifiedBrightness(groupColor, .15f)));
		
			ImVec2 containerPosMin(editorPos.x, headerPosMin.y);
			ImVec2 containerPosMax(editorEndPos.x, headerPosMax.y);
			
			canvas->AddRectFilled(containerPosMin, containerPosMax, ImColor(style->trackGroupEditorColor));
			
			float lineHeight = headerPosMax.y - 1.0f;
			canvas->AddLine(ImVec2(editorPos.x, lineHeight + 1), ImVec2(editorEndPos.x, lineHeight + 1), ImColor(style->trackSeparatorLineShadow), 3.0);
			canvas->AddLine(ImVec2(editorPos.x, lineHeight), ImVec2(editorEndPos.x, lineHeight), ImColor(style->trackSeparatorLineHighlight), 1.0);
		}
			
		f_currentTrackPositionY += style->trackGroupHeight;
		f_trackStackHeight += style->trackGroupHeight;
	}
	else{
		i_trackGroupNestingLevel++;
		f_trackIndentation = i_trackGroupNestingLevel * style->trackGroupIndentation;
		f_currentTrackHeaderWidth = trackHeaderSize.x - f_trackIndentation;
	}
}

void endTrackGroup(){
	TrackGroupInfo& groupInfo = trackGroups.back();
	
	if(!groupInfo.b_parentCollapsed){
		
		if(groupInfo.b_collapsed){
			
			if(!groupInfo.b_offscreen){
				ImVec2 textPosition(f_trackIndentation + style->trackGroupIndentation + 10.0,
									windowPos.y + groupInfo.f_positionY + (style->trackGroupHeight - ImGui::GetTextLineHeight()) / 2.0);
				canvas->AddText(textPosition, ImColor(style->trackGroupNameColor), groupInfo.name);
			}
			
		}else{

			i_trackGroupNestingLevel--;
			f_trackIndentation = i_trackGroupNestingLevel * style->trackGroupIndentation;
			f_currentTrackHeaderWidth = trackHeaderSize.x - f_trackIndentation;
			
			ImVec2 foldColumnButtonSize(style->trackGroupIndentation, f_currentTrackPositionY - groupInfo.f_positionY);
			groupInfo.b_offscreen = groupInfo.f_positionY + foldColumnButtonSize.y < timelineSize.y || groupInfo.f_positionY > windowSize.y;
			
			if(!groupInfo.b_offscreen){
			
				ImGui::SetCursorPos(ImVec2(f_trackIndentation, groupInfo.f_positionY));
				ImGui::InvisibleButton("##GroupFoldButton", foldColumnButtonSize);
				
				if(ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
					ImGuiWindow* window = ImGui::GetCurrentContext()->CurrentWindow;
					ImGuiStorage* storage = window->DC.StateStorage;
					ImGuiID id = window->GetID("##GroupFoldButton");
					storage->SetInt(id, true);
				}
				
				ImVec2 indentPosMin = ImGui::GetItemRectMin();
				ImVec2 indentPosMax = ImGui::GetItemRectMax();
				
				ImVec4 groupColor = style->trackGroupColor;
				if(ImGui::IsItemActive()) 		modifyBrightness(groupColor, -.1f);
				else if(ImGui::IsItemHovered()) modifyBrightness(groupColor, .1f);
				canvas->AddRectFilled(indentPosMin, indentPosMax, ImColor(groupColor));
				canvas->AddRectFilled(indentPosMin,
									  ImVec2(indentPosMax.x, indentPosMin.y + style->trackGroupBevelSize),
									  ImColor(modifiedBrightness(groupColor, .15f)));
				canvas->AddRectFilled(ImVec2(indentPosMax.x - style->trackGroupBevelSize, indentPosMin.y),
									  indentPosMax,
									  ImColor(modifiedBrightness(groupColor, -.15f)));
				canvas->AddRectFilled(ImVec2(indentPosMin.x, indentPosMax.y - style->trackGroupBevelSize),
									  ImVec2(indentPosMax.x, indentPosMax.y),
									  ImColor(modifiedBrightness(groupColor, -.15f)));
				
			}
		}
		
		if(!groupInfo.b_offscreen){
			ImVec2 foldIconPosMin = windowPos + ImVec2(f_trackIndentation, groupInfo.f_positionY + (style->trackGroupHeight - style->trackGroupIndentation) / 2.0);
			ImVec2 foldIconPosMax = foldIconPosMin + ImVec2(style->trackGroupIndentation, style->trackGroupIndentation);
			float trianglePadding = style->trackGroupFolderIconPadding;
			std::vector<ImVec2> trianglePoints;
			if(groupInfo.b_collapsed){
				trianglePoints = {
					ImVec2(foldIconPosMin.x + trianglePadding, foldIconPosMin.y + trianglePadding),
					ImVec2(foldIconPosMin.x + trianglePadding, foldIconPosMax.y - trianglePadding),
					ImVec2(foldIconPosMax.x - trianglePadding, (foldIconPosMin.y + foldIconPosMax.y) / 2.0)
				};
			}else{
				trianglePoints = {
					ImVec2(foldIconPosMin.x + trianglePadding, foldIconPosMin.y + trianglePadding),
					ImVec2(foldIconPosMax.x - trianglePadding, foldIconPosMin.y + trianglePadding),
					ImVec2((foldIconPosMin.x + foldIconPosMax.x) / 2.0, foldIconPosMax.y - trianglePadding)
				};
			}
			canvas->AddConvexPolyFilled(trianglePoints.data(), 3, ImColor(style->trackGroupFoldIconColor));
		}
	}
	
	trackGroups.pop_back();
	
	ImGui::PopID(); //pop group name id
}

//=================================================================
//============================ TRACK ==============================
//=================================================================

//these are used to finish drawing a multitrack
//we may need to have a MultiTrackInfo Structure which contains TrackInfo children
int currentTrackCount;
const char* currentTrackName;
const char** currentTrackNames;
ImVec2 currentTrackHeaderPosMin;
ImVec2 currentTrackHeaderPosMax;
float currentSubtrackIndentation;

ImVec2 currentTrackUserSpacePos;
ImVec2 currentTrackUserSpaceSize;
ImGuiID currentTrackHeaderID;
bool currentHeaderHovered;

bool beginTrack(int trackID, const char* name, int trackType){
	return beginMultiTrack(name, &trackID, nullptr, 1, &trackType);
}

void endTrack(bool b_wasOpen){
	endMultiTrack(b_wasOpen);
}

bool beginMultiTrack(const char* masterName, int* trackIDs, const char** names, int trackCount, int* trackTypes){
	
	bool b_noTracksOnScreen = true;
	float trackHeaderStartPosY = f_currentTrackPositionY;
	currentTrackName = masterName;
	currentTrackCount = trackCount;
	currentTrackNames = names;
	if(names != nullptr) currentSubtrackIndentation = ImGui::CalcTextSize(masterName).x + 10.0f;
	else currentSubtrackIndentation = 10.0f;
	bool b_parentGroupCollapsed = !trackGroups.empty() && (trackGroups.back().b_collapsed || trackGroups.back().b_parentCollapsed);
	
	for(int i = 0; i < trackCount; i++){
		int trackID = trackIDs[i];
		int type = trackTypes[i];
		
		//if the track is unfolded but off-screen, increment the track stack size and early out
		//don't register it so regions can't be drawn on it later
		if(!b_parentGroupCollapsed && (f_currentTrackPositionY + f_trackHeight < timelineSize.y || f_currentTrackPositionY > windowSize.y)){
			f_currentTrackPositionY += f_trackHeight;
			f_trackStackHeight += f_trackHeight;
			continue;
		}else b_noTracksOnScreen = false;
		
		//register the track so regions can be drawn on it
		tracks.push_back(TrackInfo{
			.id = trackID,
			.positionY = f_currentTrackPositionY,
			.sizeY = f_trackHeight,
			.type = type,
			.b_collapsed = b_parentGroupCollapsed
		});
		TrackInfo& track = tracks.back();
		
		//if the track is collapsed, adjust the size and early out of drawing the header and container
		if(track.b_collapsed){
			tracks.back().positionY = trackGroups.back().f_positionY;
			tracks.back().sizeY = style->trackGroupHeight;
			endMultiTrack(true, true);
			continue;
		}
		
		//declare track container (future drop target for external sources)
		ImGui::PushID(trackID);
		ImGui::SetCursorPos(ImVec2(trackHeaderSize.x, f_currentTrackPositionY));
		ImGui::SetItemAllowOverlap();
		ImGui::InvisibleButton("##DropTarget", ImVec2(editorSize.x, f_trackHeight));
		if(ImGui::IsItemHovered()) hoveredTrack = &tracks.back();
		ImGui::PopID();
		
		//draw track container
		ImVec2 trackContainerPosMin = ImGui::GetItemRectMin();
		ImVec2 trackContainerPosMax = ImGui::GetItemRectMax();
		if(ImGui::IsItemHovered()) canvas->AddRectFilled(trackContainerPosMin, trackContainerPosMax, ImColor(style->trackEditorHoverColor));
		if(i != trackCount) {
			float lineHeight = trackContainerPosMax.y - 1.0f;
			canvas->AddLine(ImVec2(editorPos.x, lineHeight), ImVec2(editorEndPos.x, lineHeight), ImColor(style->trackSeparatorLineShadow), 1.0);
		}
		
		//increment track stack
		f_currentTrackPositionY += f_trackHeight;
		f_trackStackHeight += f_trackHeight;
	}
	
	if(b_noTracksOnScreen || b_parentGroupCollapsed) {
		endMultiTrack(false, b_parentGroupCollapsed);
		return false;
	}
	
	ImGuiID hoveredId = ImGui::GetHoveredID();
	
	//track header
	ImGui::PushID(trackIDs[0]);
	ImGui::SetCursorPos(ImVec2(f_trackIndentation, trackHeaderStartPosY));
	ImGui::SetItemAllowOverlap();
	ImGui::InvisibleButton("##Header", ImVec2(f_currentTrackHeaderWidth, trackCount * f_trackHeight));
	ImGuiID headerID = ImGui::GetItemID();
	
	currentTrackHeaderID = ImGui::GetItemID();
	currentHeaderHovered = ImGui::IsItemHovered();
	bool b_hovered = ImGui::GetCurrentContext()->HoveredIdPreviousFrame == headerID;
	bool b_active = b_hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left);
	
	//draw track header
	currentTrackHeaderPosMin = ImGui::GetItemRectMin();
	currentTrackHeaderPosMax = ImGui::GetItemRectMax();
	ImVec4 trackColor = style->trackColor;
	if(b_active) 		modifyBrightness(trackColor, -.1f);
	else if(b_hovered) 	modifyBrightness(trackColor, .1f);
	canvas->AddRectFilled(currentTrackHeaderPosMin, currentTrackHeaderPosMax, ImColor(trackColor));
	canvas->AddRectFilled(ImVec2(currentTrackHeaderPosMin.x, currentTrackHeaderPosMax.y - style->trackBevelSize),
						  currentTrackHeaderPosMax,
						  ImColor(modifiedBrightness(trackColor, -.15f)));
	canvas->AddRectFilled(currentTrackHeaderPosMin,
						  ImVec2(currentTrackHeaderPosMax.x, currentTrackHeaderPosMin.y + style->trackBevelSize),
						  ImColor(modifiedBrightness(trackColor, .15f)));
	float userSpaceLineX = editorPos.x - style->trackUserSpaceWidth;
	canvas->AddLine(ImVec2(userSpaceLineX, currentTrackHeaderPosMin.y),
					ImVec2(userSpaceLineX, currentTrackHeaderPosMax.y),
					ImColor(0.f, 0.f, 0.f, .2f));
	
	float lineHeight = currentTrackHeaderPosMax.y - 1.0f;
	canvas->AddLine(ImVec2(editorPos.x, lineHeight + 1), ImVec2(editorEndPos.x, lineHeight + 1), ImColor(style->trackSeparatorLineShadow), 3.0);
	canvas->AddLine(ImVec2(editorPos.x, lineHeight), ImVec2(editorEndPos.x, lineHeight), ImColor(style->trackSeparatorLineHighlight), 1.0);
	
	ImGui::PushClipRect(currentTrackHeaderPosMin, currentTrackHeaderPosMax, true);
	currentTrackUserSpacePos = ImVec2(trackHeaderSize.x - style->trackUserSpaceWidth, trackHeaderStartPosY);
	currentTrackUserSpaceSize = ImVec2(style->trackUserSpaceWidth, trackCount * f_trackHeight);
	ImGui::SetCursorPos(currentTrackUserSpacePos);
	return true;
}

void endMultiTrack(bool b_wasOpen, bool b_enclosingGroupClosed){
	if(b_enclosingGroupClosed) return;
	if(!b_wasOpen) return;
	
	ImGui::PopClipRect();
	ImGui::PopID();
	
	float textOffset = (f_trackHeight - ImGui::GetTextLineHeight()) / 2.0;
	ImVec2 textPosition = ImVec2(trackHeaderPos.x + f_trackIndentation + 10.0, currentTrackHeaderPosMin.y + textOffset);
	
	ImGui::PushClipRect(currentTrackHeaderPosMin, currentTrackHeaderPosMax - ImVec2(style->trackUserSpaceWidth, 0), true);
	canvas->AddText(textPosition, ImColor(style->trackNameColor), currentTrackName);
	ImGui::PopClipRect();
	
	if(currentTrackNames != nullptr){
		float separatorLinePosX = trackHeaderPos.x + f_trackIndentation + currentSubtrackIndentation + 5.0f;
		canvas->AddLine(ImVec2(separatorLinePosX, currentTrackHeaderPosMin.y), ImVec2(separatorLinePosX, currentTrackHeaderPosMax.y), ImColor(0.0f, 0.0f, 0.0f, 0.3f));
		textPosition.x += currentSubtrackIndentation;
		ImGui::PushClipRect(currentTrackHeaderPosMin, currentTrackHeaderPosMax - ImVec2(style->trackUserSpaceWidth, 0), true);
		for(int i = 0; i < currentTrackCount; i++){
			canvas->AddText(textPosition, ImColor(style->trackNameColor), currentTrackNames[i]);
			textPosition.y += f_trackHeight;
		}
		ImGui::PopClipRect();
		for(int i = 1; i < currentTrackCount; i++){
			float lineHeight = currentTrackHeaderPosMin.y + i * f_trackHeight - 1.0f;
			canvas->AddLine(ImVec2(separatorLinePosX, lineHeight), ImVec2(currentTrackHeaderPosMax.x, lineHeight), ImColor(style->trackSeparatorLineShadow));
		}
	}
}

ImVec2 getTrackHeaderUserSpacePos(){
	return currentTrackUserSpacePos;
}

ImVec2 getTrackHeaderUserSpaceSize(){
	return currentTrackUserSpaceSize;
}

TrackInfo* getTrackByID(int trackID){
	for(int i = 0; i < tracks.size(); i++){
		if(tracks[i].id == trackID) return &tracks[i];
	}
	return nullptr;
}

TrackInfo* getTrackByIndex(int trackIndex){
	if(trackIndex < 0 || trackIndex >= tracks.size()) return nullptr;
	return &tracks[trackIndex];
}

int getTrackIndex(int trackID){
	for(int i = 0; i < tracks.size(); i++){
		if(tracks[i].id == trackID) return i;
	}
	return -1;
}





//=================================================================
//============================ REGION =============================
//=================================================================

bool beginRegion(int regionID, int& trackID, const char* name, long long int& startTime, long long int& length, int regionType, int linkedMasterRegionID){
	
	bool b_linked = linkedMasterRegionID >= 0;
	if(b_linked){
		regions.push_back(RegionInfo{
			.id = regionID,
			.trackId = &trackID,
			.startTimePtr = &startTime,
			.durationPtr = &length,
			.type = regionType,
			.name = name,
			.linkedRegionMasterId = linkedMasterRegionID,
			.b_hasLinkingMaster = linkedMasterRegionID >= 0,
			.b_isLinkingMaster = linkedMasterRegionID == regionID
		});
	}
	
	//find the corresponding track & get its position/height
	TrackInfo* track = getTrackByID(trackID);
	if(track == nullptr){
		//if no track found, early out of the region
		endRegion(false);
		return false;
	}
	
	//check if the region is completely offscreen in X to early out
	float screenPosMinX = timeToScreen(startTime);
	float screenPosMaxX = timeToScreen(startTime + length);
	if(screenPosMaxX < editorPos.x || screenPosMinX > editorEndPos.x){
		endRegion(false);
		return false;
	}
	
	//adjust drawing rectangle if the region is partially offscreen
	screenPosMinX = std::max(screenPosMinX, editorPos.x);
	screenPosMaxX = std::min(screenPosMaxX, editorEndPos.x);
	
	//as a safety, never start a region that has 0 size
	double screenWidth = screenPosMaxX - screenPosMinX;
	if(screenWidth <= 0.0){
		endRegion(false);
		return false;
	}
	
	//add the gui event receiver button
	ImGui::SetCursorPos(ImVec2(screenPosMinX - windowPos.x, track->positionY));
	ImGui::SetItemAllowOverlap();
	ImGui::PushID(regionID);
	if(track->b_collapsed) ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::InvisibleButton(name, ImVec2(screenWidth, track->sizeY));
	if(track->b_collapsed) ImGui::PopItemFlag();
	ImGui::PopID();
	
	if(!b_linked){
		regions.push_back(RegionInfo{
			.id = regionID,
			.trackId = &trackID,
			.startTimePtr = &startTime,
			.durationPtr = &length,
			.type = regionType,
			.name = name,
			.linkedRegionMasterId = linkedMasterRegionID,
			.b_hasLinkingMaster = linkedMasterRegionID >= 0,
			.b_isLinkingMaster = linkedMasterRegionID == regionID
		});
	}
	RegionInfo& region = regions.back();
	region.rectMin = ImGui::GetItemRectMin();
	region.rectMax = ImGui::GetItemRectMax();
	region.rectSize = ImGui::GetItemRectSize();
	region.selected = isRegionSelected(regionID);
	region.b_collapsed = track->b_collapsed;
	
	//deselect regions in collapsed groups
	if(region.selected && region.b_collapsed) deselectRegion(region.id);

	//check user interactions with the region
	if(ImGui::IsItemHovered()) currentHoveredRegion = &region;
	if(ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) context->draggedRegion = &region;
	
	//get region color
	ImVec4 regionColor = style->regionColor;
	if(region.b_collapsed) 													modifySaturationBrightness(regionColor, -.3f, -.3f);
	else if(region.selected) 												modifySaturationBrightness(regionColor, .5f, 0.1f);
	else if(ImGui::IsItemActive() || isRegionSelectionCanditate(regionID)) 	modifySaturationBrightness(regionColor, .2f, -.1f);
	else if(isRegionHovered(regionID)) 										modifyBrightness(regionColor, .1f);
	
	//push region clipping rectangle (popped in endRegion())
	ImGui::PushClipRect(region.rectMin, region.rectMax, true);
	
	//adjust style if selected
	float bevelSize = style->regionBevelSize * (region.selected ? 2.f : 1.f);
	float bevelBrightness = .15f * (region.selected ? 2.f : 1.f);
	
	//draw region rectangle with beveled edges
	canvas->AddRectFilled(region.rectMin,
						  region.rectMax,
						  ImColor(regionColor));
	canvas->AddRectFilled(region.rectMin + ImVec2(0, track->sizeY - bevelSize),
						  region.rectMax,
						  ImColor(modifiedBrightness(regionColor, -bevelBrightness)));
	canvas->AddRectFilled(region.rectMin,
						  region.rectMin + ImVec2(region.rectSize.x, bevelSize),
						  ImColor(modifiedBrightness(regionColor, bevelBrightness)));
	
	if(region.b_hasLinkingMaster){
		ImVec2 linkingIndicatorPoints[3] = {
			region.rectMin,
			ImVec2(region.rectMin.x + 8.0, region.rectMin.y),
			ImVec2(region.rectMin.x, region.rectMin.y + 8.0)
		};
		canvas->AddConvexPolyFilled(linkingIndicatorPoints, 3, ImColor(1.f, 1.f, 1.f, .7f));
	}
	
	return true;
}

void endRegion(bool b_wasOpen){
	if(!b_wasOpen) return;
	
	RegionInfo& region = regions.back();
	
	float textOffset = (region.rectSize.y - ImGui::GetTextLineHeight()) / 2.0;
	canvas->AddText(region.rectMin + ImVec2(10.0, textOffset),
					ImColor(style->regionNameColor),
					region.name);
	//pop region clipping rectangle
	ImGui::PopClipRect();
}

//============================ EVENT =============================

void event(int regionID, int& trackID, const char* name, long long int& time, int regionType, int linkedMasterRegionID){
	
	bool b_linked = linkedMasterRegionID >= 0;
	if(b_linked){
		regions.push_back(RegionInfo{
			.id = regionID,
			.trackId = &trackID,
			.startTimePtr = &time,
			.durationPtr = nullptr,
			.type = regionType,
			.name = name,
			.linkedRegionMasterId = linkedMasterRegionID,
			.b_hasLinkingMaster = linkedMasterRegionID >= 0,
			.b_isLinkingMaster = linkedMasterRegionID == regionID
		});
	}
	
	//find the corresponding track & get its position/height
	TrackInfo* track = getTrackByID(trackID);
	if(track == nullptr){
		//if no track found, early out of the region
		endRegion(false);
		return false;
	}
	
	//check if the region is completely offscreen in X to early out
	float screenPosMinX = timeToScreen(time);
	float screenPosMaxX = screenPosMinX + style->eventWidth;
	if(screenPosMaxX + 100.0 < editorPos.x || screenPosMinX > editorEndPos.x){ //TODO: the +100.0 is king of hacky, events should be be skipped if really offscreen
		endRegion(false);
		return false;
	}
	float screenWidth = screenPosMaxX - screenPosMinX;
	
	bool b_hovered = false;
	bool b_active = false;
	
	//add the gui event receiver button
	ImGui::SetCursorPos(ImVec2(screenPosMinX - windowPos.x, track->positionY));
	ImGui::SetItemAllowOverlap();
	ImGui::PushID(regionID);
	if(track->b_collapsed) ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::InvisibleButton(name, ImVec2(screenWidth, track->sizeY));
	ImVec2 stemPosMin = ImGui::GetItemRectMin();
	ImVec2 stemPosMax = ImGui::GetItemRectMax();
	ImVec2 stemSize = ImGui::GetItemRectSize();
	if(ImGui::IsItemHovered()) b_hovered = true;
	if(ImGui::IsItemActive()) b_active = true;
	float labelTextPadding = 3.f;
	ImVec2 textSize = ImGui::CalcTextSize(name);
	textSize.x = std::round(textSize.x);
	textSize.y = std::round(textSize.y);
	ImGui::SetCursorPos(ImVec2(screenPosMinX - windowPos.x, track->positionY));
	ImGui::SetItemAllowOverlap();
	ImVec2 labelSize = textSize + ImVec2(labelTextPadding, labelTextPadding * 2.f);
	labelSize.x += labelSize.y / 2.0;
	ImGui::InvisibleButton(name, labelSize);
	if(ImGui::IsItemHovered()) b_hovered = true;
	if(ImGui::IsItemActive()) b_active = true;
	ImVec2 labelPosMin = ImGui::GetItemRectMin();
	ImVec2 labelPosMax = ImGui::GetItemRectMax();
	if(track->b_collapsed) ImGui::PopItemFlag();
	ImGui::PopID();
	
	if(!b_linked){
		regions.push_back(RegionInfo{
			.id = regionID,
			.trackId = &trackID,
			.startTimePtr = &time,
			.durationPtr = nullptr,
			.type = regionType,
			.name = name,
			.linkedRegionMasterId = linkedMasterRegionID,
			.b_hasLinkingMaster = linkedMasterRegionID >= 0,
			.b_isLinkingMaster = linkedMasterRegionID == regionID
		});
	}
	RegionInfo& event = regions.back();
	event.rectMin = ImGui::GetItemRectMin();
	event.rectMax = ImGui::GetItemRectMax();
	event.rectSize = ImGui::GetItemRectSize();
	event.selected = isRegionSelected(regionID);
	event.b_collapsed = track->b_collapsed;
	
	//deselect events in collapsed groups
	if(event.selected && event.b_collapsed) deselectRegion(event.id);

	//check user interactions with the event
	if(b_hovered) currentHoveredRegion = &event;
	if(b_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) context->draggedRegion = &event;
	
	//get event color
	ImVec4 eventColor = style->regionColor;
	if(event.b_collapsed) 													modifySaturationBrightness(eventColor, -.3f, -.3f);
	else if(event.selected) 												modifySaturationBrightness(eventColor, .5f, 0.1f);
	else if(ImGui::IsItemActive() || isRegionSelectionCanditate(regionID)) 	modifySaturationBrightness(eventColor, .2f, -.1f);
	else if(isRegionHovered(regionID)) 										modifyBrightness(eventColor, .1f);
	
	float bevelSize = style->regionBevelSize * (event.selected ? 2.f : 1.f);
	float brightness = event.selected ? 2.f : 1.f;
	
	ImVec2 flagPoints[5] = {
		labelPosMin,
		ImVec2(labelPosMax.x - labelSize.y / 2.f, labelPosMax.y - labelSize.y),
		ImVec2(labelPosMax.x, labelPosMax.y - labelSize.y / 2.f),
		ImVec2(labelPosMax.x - labelSize.y / 2.f, labelPosMax.y),
		ImVec2(labelPosMin.x, labelPosMax.y)
	};
	ImVec2 flagHighlight[4] = {
		flagPoints[0],
		flagPoints[1],
		ImVec2(bevelSize + flagPoints[1].x, bevelSize + flagPoints[1].y),
		ImVec2(flagPoints[0].x, flagPoints[0].y + bevelSize)
	};
	ImVec2 flagShadow[4] = {
		flagPoints[4],
		flagPoints[3],
		flagPoints[3] + ImVec2(bevelSize, -bevelSize),
		flagPoints[4] + ImVec2(0.f, -bevelSize)
	};
	
	ImGui::PushClipRect(editorPos, editorEndPos, true);
	canvas->AddRectFilled(stemPosMin,
						  stemPosMax,
						  ImColor(modifiedBrightness(eventColor, -.25f)));
	canvas->AddConvexPolyFilled(flagPoints, 5, ImColor(eventColor));
	canvas->AddConvexPolyFilled(flagHighlight, 4, ImColor(modifiedBrightness(eventColor, .2f)));
	canvas->AddConvexPolyFilled(flagShadow, 4, ImColor(modifiedBrightness(eventColor, -.2f)));
	canvas->AddText(labelPosMin + ImVec2(1.f + labelTextPadding, labelTextPadding),
					ImColor(style->regionNameColor),
					name);
	canvas->AddRectFilled(event.rectMin,
						  ImVec2(event.rectMin.x + 1.f, event.rectMin.y + f_trackHeight),
						  ImColor(style->playheadColor));
	
	if(event.b_isLinkingMaster || event.b_hasLinkingMaster){
		ImVec2 linkingIndicatorPoints[3] = {
			event.rectMin,
			ImVec2(event.rectMin.x + 8.0, event.rectMin.y),
			ImVec2(event.rectMin.x, event.rectMin.y + 8.0)
		};
		canvas->AddConvexPolyFilled(linkingIndicatorPoints, 3, ImColor(1.f, 1.f, 1.f, .7f));
	}
	ImGui::PopClipRect();
}


void selectRegion(int regionID){
	if(!isRegionSelected(regionID)) context->selectedRegionIds.push_back(regionID);
}

void deselectRegion(int regionID){
	auto idIterator = std::find(context->selectedRegionIds.begin(), context->selectedRegionIds.end(), regionID);
	if(idIterator != context->selectedRegionIds.end()) context->selectedRegionIds.erase(idIterator);
}

void deselectAllRegions(){
	context->selectedRegionIds.clear();
}

bool isRegionSelected(int regionID){
	return std::find(context->selectedRegionIds.begin(), context->selectedRegionIds.end(), regionID) != context->selectedRegionIds.end();
}

RegionInfo* getRegion(int regionID){
	for(int i = 0; i < regions.size(); i++){
		if(regions[i].id == regionID) return &regions[i];
	}
	return nullptr;
}

int getSelectedRegionCount(){
	return context->selectedRegionIds.size();
}

bool isRegionHovered(int regionID){
	if(context->hoveredRegion) return context->hoveredRegion->id == regionID;
	return false;
}

std::vector<RegionInfo*> getMovedRegions(){
	std::vector<RegionInfo*> output;
	if(!isRegionSelected(context->draggedRegion->id)){
		if(context->draggedRegion->b_hasLinkingMaster) {
			RegionInfo* linkingMaster = getRegion(context->draggedRegion->linkedRegionMasterId);
			output = getLinkedRegions(linkingMaster);
		}
		else output.push_back(context->draggedRegion);
	}else{
		std::vector<int> selectionMastersIds;
		for(int i = 0; i < context->selectedRegionIds.size(); i++){
			RegionInfo* selectedRegion = getRegion(context->selectedRegionIds[i]);
			if(selectedRegion->b_hasLinkingMaster){
			   if(std::find(selectionMastersIds.begin(), selectionMastersIds.end(), selectedRegion->linkedRegionMasterId) == selectionMastersIds.end()){
				   selectionMastersIds.push_back(selectedRegion->linkedRegionMasterId);
			   }
			}else output.push_back(selectedRegion);
		}
		for(int i = 0; i < selectionMastersIds.size(); i++){
			RegionInfo* linkMasterRegion = getRegion(selectionMastersIds[i]);
			std::vector<RegionInfo*> linkedRegions = getLinkedRegions(linkMasterRegion);
			output.insert(output.end(), linkedRegions.begin(), linkedRegions.end());
		}
	}
	return output;
}

std::vector<RegionInfo*> getLinkedRegions(RegionInfo* linkingMaster){
	std::vector<RegionInfo*> output;
	if(!linkingMaster->b_isLinkingMaster) return output;
	for(int i = 0; i < regions.size(); i++){
		if(regions[i].linkedRegionMasterId == linkingMaster->id)
			output.push_back(&regions[i]);
	}
	return output;
}




void addRegionSelectionCandidate(int regionID){
	if(!isRegionSelectionCanditate(regionID)) context->selectionCanditateRegionIds.push_back(regionID);
}

void removeRegionSelectionCanditate(int regionID){
	auto idIterator = std::find(context->selectionCanditateRegionIds.begin(), context->selectionCanditateRegionIds.end(), regionID);
	if(idIterator != context->selectionCanditateRegionIds.end()) context->selectionCanditateRegionIds.erase(idIterator);
}

void removeAllRegionSelectionCandidates(){
	context->selectionCanditateRegionIds.clear();
}

bool isRegionSelectionCanditate(int regionID){
	return std::find(context->selectionCanditateRegionIds.begin(), context->selectionCanditateRegionIds.end(), regionID) != context->selectionCanditateRegionIds.end();
}




//======================= TICKS =========================


TickStringFunction microsecondZeroCenteredString = [](char* output, long long int time_µs, long long int tickspacing_µs){
	
	static long long int microseconds_µs = 1;
	static long long int millisecond_µs = 1000;
	static long long int decisecond_µs = 100000;
	static long long int second_µs = 1000000;
	static long long int minute_µs = 60000000;
	static long long int hour_µs = 3600000000;
	static long long int day_µs = 86400000000;
	static long long int week_µs = 604800000000;

	enum class TimeUnit{
		MICROSECONDS,
		MILLISECONDS,
		DECISECONDS,
		SECONDS,
		MINUTES,
		HOURS,
		DAYS,
		WEEKS,
	};
	
	bool b_zero = time_µs == 0;
	bool b_negative = time_µs < 0;
	time_µs = std::abs(time_µs);
	int weeks = 0, days = 0, hours = 0, minutes = 0, seconds = 0, milliseconds = 0, microseconds = 0;
	
	TimeUnit timeUnitMax;
	TimeUnit timeUnitMin;
	if(tickspacing_µs < millisecond_µs) timeUnitMin = TimeUnit::MICROSECONDS;
	else if(tickspacing_µs < decisecond_µs) timeUnitMin = TimeUnit::MILLISECONDS;
	else if(tickspacing_µs < second_µs) timeUnitMin = TimeUnit::DECISECONDS;
	else if(tickspacing_µs < minute_µs) timeUnitMin = TimeUnit::SECONDS;
	else if(tickspacing_µs < hour_µs) timeUnitMin = TimeUnit::MINUTES;
	else if(tickspacing_µs < day_µs) timeUnitMin = TimeUnit::HOURS;
	else if(tickspacing_µs < week_µs) timeUnitMin = TimeUnit::DAYS;
	else timeUnitMin = TimeUnit::WEEKS;
	
	if(time_µs >= week_µs){
		weeks = time_µs / week_µs;
		time_µs -= weeks * week_µs;
	}
	if(time_µs >= day_µs){
		days = time_µs / day_µs;
		time_µs -= days * day_µs;
	}
	if(time_µs >= hour_µs){
		hours = time_µs / hour_µs;
		time_µs -= hours * hour_µs;
	}
	if(time_µs >= minute_µs){
		minutes = time_µs / minute_µs;
		time_µs -= minutes * minute_µs;
	}
	if(time_µs >= second_µs){
		seconds = time_µs / second_µs;
		time_µs -= seconds * second_µs;
	}
	if(time_µs >= millisecond_µs){
		milliseconds = time_µs / millisecond_µs;
		time_µs -= milliseconds * millisecond_µs;
	}
	microseconds = time_µs / microseconds_µs;
	
	if(b_zero) timeUnitMax = timeUnitMin;
	else if(weeks) timeUnitMax = TimeUnit::WEEKS;
	else if(days) timeUnitMax = TimeUnit::DAYS;
	else if(hours) timeUnitMax = TimeUnit::HOURS;
	else if(minutes) timeUnitMax = TimeUnit::MINUTES;
	else if(seconds) timeUnitMax = TimeUnit::SECONDS;
	else if(milliseconds) timeUnitMax = TimeUnit::MILLISECONDS;
	else timeUnitMax = TimeUnit::MICROSECONDS;
	
	int l = 0;
	output[0] = 0;
	if(b_negative) l+= sprintf(output + l, "-");
	switch(timeUnitMax){
		case TimeUnit::WEEKS:
			l += sprintf(output + l, "%iw", weeks);
			if(timeUnitMin == TimeUnit::WEEKS) break;
		case TimeUnit::DAYS:
			l += sprintf(output + l, "%id", days);
			if(timeUnitMin == TimeUnit::DAYS) break;
		case TimeUnit::HOURS:
			l += sprintf(output + l, "%ih", hours);
			if(timeUnitMin == TimeUnit::HOURS) break;
		case TimeUnit::MINUTES:
			l += sprintf(output + l, "%im", minutes);
			if(timeUnitMin == TimeUnit::MINUTES) break;
		default:
			break;
	}
	
	if(timeUnitMin == TimeUnit::SECONDS){
		l += sprintf(output + l, "%is", seconds);
	}else if(timeUnitMin == TimeUnit::DECISECONDS){
		float secondsf = seconds + milliseconds / 1000.0;
		l += sprintf(output + l, "%.1fs", secondsf);
	}else if(timeUnitMin == TimeUnit::MILLISECONDS){
		float secondsf = seconds + milliseconds / 1000.0;
		l += sprintf(output + l, "%.3fs", secondsf);
	}else if(timeUnitMin == TimeUnit::MICROSECONDS){
		if(timeUnitMax != TimeUnit::MICROSECONDS && timeUnitMax != TimeUnit::MILLISECONDS) l += sprintf(output + l, "%is", seconds);
		if(timeUnitMax != TimeUnit::MICROSECONDS) l += sprintf(output + l, "%ims", milliseconds);
		l += sprintf(output + l, "%iµs", microseconds);
	}
	
};

std::vector<TickSpacing> microsecondTickSpacings = {
	{1,					1 				},	//1 microseconds
	{5,					1 				},	//5 microseconds
	{10,				5 				},	//10 microseconds
	{50,				10 				},	//50 microseconds
	{100,				50 				},	//100 microseconds
	{500,				100 			},	//500 microseconds
	{1000,				500 			},	//1 millisecond
	{5000,				1000 			},	//5 milliseconds
	{10000,				5000 			},	//10 milliseconds
	{50000,				10000 			},	//50 milliseconds
	{100000,			50000 			},	//0.1 seconds
	{500000,			100000 			},	//0.5 seconds
	{1000000,			500000 			},	//1 second
	{5000000,			1000000 		},	//5 seconds
	{10000000,			5000000 		},	//10 seconds
	{60000000,			10000000 		},	//1 minute
	{300000000,			60000000 		},	//5 minutes
	{600000000,			120000000 		},	//10 minutes
	{3600000000,		600000000 		},	//1 hour
	{14400000000,		3600000000 		}, 	//4 hours
	{43200000000,		14400000000 	}, 	//12 hours
	{86400000000,		14400000000 	}, 	//1 day
	{604800000000,		86400000000 	}, 	//1 week
	{3024000000000,		604800000000 	}, 	//5 weeks
	{6048000000000,		1209600000000 	}, 	//10 weeks
	{30240000000000,	6048000000000 	}, 	//50 weeks
	{60480000000000,	12096000000000	} 	//100 weeks
};

void setTimeMicrosecondsZeroCentered(){
	context->tickSpacings = &microsecondTickSpacings;
	context->tickStringFunction = microsecondZeroCenteredString;
	context->maxTime = 86400000000;		//100 weeks in microseconds
	context->minTime = -86400000000;	//-100 weeks in microseconds
}






TickStringFunction nansecondZeroCenteredString = [](char* output, long long int time_ns, long long int tickspacing_ns){
	
	static long long int nanosecond_ns = 1;
	static long long int microsecond_µs = 1000;
	static long long int millisecond_µs = 1000000;
	static long long int decisecond_µs = 100000000;
	static long long int second_µs = 1000000000;
	static long long int minute_µs = 60000000000;
	static long long int hour_µs = 3600000000000;
	static long long int day_µs = 86400000000000;
	static long long int week_µs = 604800000000000;

	enum class TimeUnit{
		NANOSECONDS,
		MICROSECONDS,
		MILLISECONDS,
		DECISECONDS,
		SECONDS,
		MINUTES,
		HOURS,
		DAYS,
		WEEKS,
	};
	
	bool b_zero = time_ns == 0;
	bool b_negative = time_ns < 0;
	time_ns = std::abs(time_ns);
	int weeks = 0, days = 0, hours = 0, minutes = 0, seconds = 0, milliseconds = 0, microseconds = 0, nanoseconds = 0;
	
	TimeUnit timeUnitMax;
	TimeUnit timeUnitMin;
	if(tickspacing_ns < microsecond_µs) timeUnitMin = TimeUnit::NANOSECONDS;
	else if(tickspacing_ns < millisecond_µs) timeUnitMin = TimeUnit::MICROSECONDS;
	else if(tickspacing_ns < decisecond_µs) timeUnitMin = TimeUnit::MILLISECONDS;
	else if(tickspacing_ns < second_µs) timeUnitMin = TimeUnit::DECISECONDS;
	else if(tickspacing_ns < minute_µs) timeUnitMin = TimeUnit::SECONDS;
	else if(tickspacing_ns < hour_µs) timeUnitMin = TimeUnit::MINUTES;
	else if(tickspacing_ns < day_µs) timeUnitMin = TimeUnit::HOURS;
	else if(tickspacing_ns < week_µs) timeUnitMin = TimeUnit::DAYS;
	else timeUnitMin = TimeUnit::WEEKS;
	
	if(time_ns >= week_µs){
		weeks = time_ns / week_µs;
		time_ns -= weeks * week_µs;
	}
	if(time_ns >= day_µs){
		days = time_ns / day_µs;
		time_ns -= days * day_µs;
	}
	if(time_ns >= hour_µs){
		hours = time_ns / hour_µs;
		time_ns -= hours * hour_µs;
	}
	if(time_ns >= minute_µs){
		minutes = time_ns / minute_µs;
		time_ns -= minutes * minute_µs;
	}
	if(time_ns >= second_µs){
		seconds = time_ns / second_µs;
		time_ns -= seconds * second_µs;
	}
	if(time_ns >= millisecond_µs){
		milliseconds = time_ns / millisecond_µs;
		time_ns -= milliseconds * millisecond_µs;
	}
	if(time_ns >= microsecond_µs){
		microseconds = time_ns / microsecond_µs;
		time_ns -= microseconds * microsecond_µs;
	}
	nanoseconds = time_ns / nanosecond_ns;
	
	if(b_zero) timeUnitMax = timeUnitMin;
	else if(weeks) timeUnitMax = TimeUnit::WEEKS;
	else if(days) timeUnitMax = TimeUnit::DAYS;
	else if(hours) timeUnitMax = TimeUnit::HOURS;
	else if(minutes) timeUnitMax = TimeUnit::MINUTES;
	else if(seconds) timeUnitMax = TimeUnit::SECONDS;
	else if(milliseconds) timeUnitMax = TimeUnit::MILLISECONDS;
	else if(microseconds) timeUnitMax = TimeUnit::MICROSECONDS;
	else timeUnitMax = TimeUnit::NANOSECONDS;
	
	int l = 0;
	output[0] = 0;
	if(b_negative) l+= sprintf(output + l, "-");
	switch(timeUnitMax){
		case TimeUnit::WEEKS:
			l += sprintf(output + l, "%iw", weeks);
			if(timeUnitMin == TimeUnit::WEEKS) break;
		case TimeUnit::DAYS:
			l += sprintf(output + l, "%id", days);
			if(timeUnitMin == TimeUnit::DAYS) break;
		case TimeUnit::HOURS:
			l += sprintf(output + l, "%ih", hours);
			if(timeUnitMin == TimeUnit::HOURS) break;
		case TimeUnit::MINUTES:
			l += sprintf(output + l, "%im", minutes);
			if(timeUnitMin == TimeUnit::MINUTES) break;
		default:
			break;
	}
	
	if(timeUnitMin == TimeUnit::SECONDS){
		l += sprintf(output + l, "%is", seconds);
	}else if(timeUnitMin == TimeUnit::DECISECONDS){
		float secondsf = seconds + milliseconds / 1000.0;
		l += sprintf(output + l, "%.1fs", secondsf);
	}else if(timeUnitMin == TimeUnit::MILLISECONDS){
		float secondsf = seconds + milliseconds / 1000.0;
		l += sprintf(output + l, "%.3fs", secondsf);
	}else if(timeUnitMin == TimeUnit::MICROSECONDS){
		if(timeUnitMax != TimeUnit::MICROSECONDS && timeUnitMax != TimeUnit::MILLISECONDS) l += sprintf(output + l, "%is", seconds);
		if(timeUnitMax != TimeUnit::MICROSECONDS) l += sprintf(output + l, "%ims", milliseconds);
		l += sprintf(output + l, "%iµs", microseconds);
	}else if(timeUnitMin == TimeUnit::NANOSECONDS){
		if(timeUnitMax != TimeUnit::NANOSECONDS && timeUnitMax != TimeUnit::MICROSECONDS && timeUnitMax != TimeUnit::MILLISECONDS)
			l += sprintf(output + l, "%is", seconds);
		if(timeUnitMax != TimeUnit::NANOSECONDS && timeUnitMax != TimeUnit::MICROSECONDS)
			l += sprintf(output + l, "%ims", milliseconds);
		if(timeUnitMax != TimeUnit::NANOSECONDS)
			l += sprintf(output + l, "%iµs", microseconds);
		l += sprintf(output + l, "%ins", nanoseconds);
	}
	
};

std::vector<TickSpacing> nanosecondTickSpacings = {
	{1,					1 					},	//1 nanosecond
	{5,					1 					},	//5 nanoseconds
	{10,				5 					},	//10 nanoseconds
	{50,				10 					},	//50 nanoseconds
	{100,				50 					},	//100 nanoseconds
	{500,				100 				},	//500 nanoseconds
	{1000,				500 				},	//1 microseconds
	{5000,				1000 				},	//5 microseconds
	{10000,				5000 				},	//10 microseconds
	{50000,				10000 				},	//50 microseconds
	{100000,			50000 				},	//100 microseconds
	{500000,			100000 				},	//500 microseconds
	{1000000,			500000 				},	//1 millisecond
	{5000000,			1000000 			},	//5 milliseconds
	{10000000,			5000000 			},	//10 milliseconds
	{50000000,			10000000 			},	//50 milliseconds
	{100000000,			50000000 			},	//0.1 seconds
	{500000000,			100000000 			},	//0.5 seconds
	{1000000000,		500000000 			},	//1 second
	{5000000000,		1000000000 			},	//5 seconds
	{10000000000,		5000000000 			},	//10 seconds
	{60000000000,		10000000000 		},	//1 minute
	{300000000000,		60000000000 		},	//5 minutes
	{600000000000,		120000000000 		},	//10 minutes
	{3600000000000,		600000000000 		},	//1 hour
	{14400000000000,	3600000000000 		}, 	//4 hours
	{43200000000000,	14400000000000 		}, 	//12 hours
	{86400000000000,	14400000000000 		}, 	//1 day
	{604800000000000,	86400000000000 		}, 	//1 week
	{3024000000000000,	604800000000000 	}, 	//5 weeks
	{6048000000000000,	1209600000000000 	}, 	//10 weeks
	{30240000000000000,	6048000000000000 	}, 	//50 weeks
	{60480000000000000,	12096000000000000	} 	//100 weeks
};

void setTimeNanosecondsZeroCentered(){
	context->tickSpacings = &nanosecondTickSpacings;
	context->tickStringFunction = nansecondZeroCenteredString;
	context->maxTime = 86400000000000;	//1 day in microseconds
	context->minTime = -86400000000000;	//-1 day in microseconds
}

void normalizeColor(ImVec4& color){
	color.x = std::max(std::min(color.x, 1.0f), 0.0f);
	color.y = std::max(std::min(color.y, 1.0f), 0.0f);
	color.z = std::max(std::min(color.z, 1.0f), 0.0f);
	color.w = std::max(std::min(color.w, 1.0f), 0.0f);
}

void modifySaturation(ImVec4& color, float amount){
	amount = 1.0 + amount;
	float P = sqrt(pow(color.x, 2.0f) * 0.299f +
				   pow(color.y, 2.0f) * .587f +
				   pow(color.z, 2.0f) * .114f);
	color.x = P + (color.x - P) * amount;
	color.y = P + (color.y - P) * amount;
	color.z = P + (color.z - P) * amount;
	normalizeColor(color);
}

void modifyBrightness(ImVec4& color, float amount){
	color.x += amount;
	color.y += amount;
	color.z += amount;
	normalizeColor(color);
}

void modifySaturationBrightness(ImVec4& color, float saturationChange, float brightnessChange){
	modifySaturation(color, saturationChange);
	modifyBrightness(color, brightnessChange);
}

ImVec4 modifiedSaturation(ImVec4 color, float amount){
	modifySaturation(color, amount);
	return color;
}
ImVec4 modifiedBrightness(ImVec4 color, float amount){
	modifyBrightness(color, amount);
	return color;
}
ImVec4 modifiedSaturationBrightness(ImVec4 color, float saturationChange, float brightnessChange){
	modifySaturationBrightness(color, saturationChange, brightnessChange);
	return color;
}

}
