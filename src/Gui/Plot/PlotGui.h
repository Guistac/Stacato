#pragma once

#include "Gui/ApplicationWindow/Window.h"

namespace PlotGui{

	void manoeuvreList();

	void trackSheetEditor();
	void curveEditor();
	void spatialEditor();


	class ManoeuvreListWindow : public Window{
	public:
		ManoeuvreListWindow() : Window("Manoeuvre List", true){}
		virtual void drawContent() override { manoeuvreList(); };
		SINGLETON_GET_METHOD(ManoeuvreListWindow)
	};

	class TrackSheetEditorWindow : public Window{
	public:
		TrackSheetEditorWindow() : Window("Track Sheet Editor", true){}
		virtual void drawContent() override { trackSheetEditor(); };
		SINGLETON_GET_METHOD(TrackSheetEditorWindow)
	};

	class CurveEditorWindow : public Window{
	public:
		CurveEditorWindow() : Window("Curve Editor", true){}
		virtual void drawContent() override { curveEditor(); };
		SINGLETON_GET_METHOD(CurveEditorWindow)
	};

	class SpatialEditorWindow : public Window{
	public:
		SpatialEditorWindow() : Window("Spatial Editor", true){}
		virtual void drawContent() override { spatialEditor(); };
		SINGLETON_GET_METHOD(SpatialEditorWindow)
	};

}
