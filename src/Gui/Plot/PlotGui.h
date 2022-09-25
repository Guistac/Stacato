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


	class NewPlotPopup : public Popup{
	public:
		NewPlotPopup() : Popup("New Plot", true, true){}
		//virtual void onPopupOpen() override;
		virtual void drawContent() override;
		SINGLETON_GET_METHOD(NewPlotPopup);
		char newNameBuffer[256];
	};

	class PlotEditorPopup : public Popup{
	public:
		PlotEditorPopup(std::shared_ptr<Plot> editedPlot) : Popup("Edit Plot", true, true), plot(editedPlot){}
		//virtual void onPopupOpen() override;
		virtual void drawContent() override;
		SINGLETON_GET_METHOD(PlotEditorPopup);
		std::shared_ptr<Plot> plot;
		char newNameBuffer[256];
	};

	class PlotDeletePopup : public Popup{
	public:
		PlotDeletePopup(std::shared_ptr<Plot> deletedPlot) : Popup("Delete Plot", true, true), plot(deletedPlot){}
		virtual void drawContent() override;
		SINGLETON_GET_METHOD(PlotDeletePopup);
		std::shared_ptr<Plot> plot;
	};

}
