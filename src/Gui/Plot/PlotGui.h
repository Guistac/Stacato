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
		static std::shared_ptr<ManoeuvreListWindow> get();
	};

	class TrackSheetEditorWindow : public Window{
	public:
		TrackSheetEditorWindow() : Window("Track Sheet Editor", true){}
		virtual void drawContent() override { trackSheetEditor(); };
		static std::shared_ptr<TrackSheetEditorWindow> get();
	};

	class CurveEditorWindow : public Window{
	public:
		CurveEditorWindow() : Window("Curve Editor", true){}
		virtual void drawContent() override { curveEditor(); };
		static std::shared_ptr<CurveEditorWindow> get();
	};

	class SpatialEditorWindow : public Window{
	public:
		SpatialEditorWindow() : Window("Spatial Editor", true){}
		virtual void drawContent() override { spatialEditor(); };
		static std::shared_ptr<SpatialEditorWindow> get();
	};

}
