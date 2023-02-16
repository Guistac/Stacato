#pragma once

#include "Application/Gui/Window.h"

namespace PlotGui{

	void manoeuvreList();
	void trackSheetEditor();
	void curveEditor();
	void spatialEditor();


	class ManoeuvreListWindow : public Window{
	public:
		ManoeuvreListWindow() : Window("Manoeuvre List", true){}
		virtual void onDraw() override { manoeuvreList(); };
		SINGLETON_GET_METHOD(ManoeuvreListWindow)
	};

	class TrackSheetEditorWindow : public Window{
	public:
		TrackSheetEditorWindow() : Window("Track Sheet Editor", true){}
		virtual void onDraw() override { trackSheetEditor(); };
		SINGLETON_GET_METHOD(TrackSheetEditorWindow)
	};

	class CurveEditorWindow : public Window{
	public:
		CurveEditorWindow() : Window("Curve Editor", true){}
		virtual void onDraw() override { curveEditor(); };
		SINGLETON_GET_METHOD(CurveEditorWindow)
	};

	class SpatialEditorWindow : public Window{
	public:
		SpatialEditorWindow() : Window("Spatial Editor", true){}
		virtual void onDraw() override { spatialEditor(); };
		SINGLETON_GET_METHOD(SpatialEditorWindow)
	};


	class NewPlotPopup : public Popup{
	public:
		NewPlotPopup() : Popup("New Plot", true, true){}
		virtual void onOpen() override;
		virtual void onDraw() override;
		char newNameBuffer[256];
		SINGLETON_GET_METHOD(NewPlotPopup);
	};

	class PlotEditorPopup : public Popup{
	public:
		PlotEditorPopup(std::shared_ptr<Plot> editedPlot) : Popup("Edit Plot", true, true), plot(editedPlot){}
		virtual void onOpen() override;
		virtual void onDraw() override;
		std::shared_ptr<Plot> plot;
		char newNameBuffer[256];
		static void open(std::shared_ptr<Plot> plot){
			auto popup = std::make_shared<PlotEditorPopup>(plot);
			popup->Popup::open();
		}
	};

	class PlotDeletePopup : public Popup{
	public:
		PlotDeletePopup(std::shared_ptr<Plot> deletedPlot) : Popup("Delete Plot", true, true), plot(deletedPlot){}
		virtual void onDraw() override;
		std::shared_ptr<Plot> plot;
		static void open(std::shared_ptr<Plot> plot){
			auto popup = std::make_shared<PlotDeletePopup>(plot);
			popup->Popup::open();
		}
	};

}
