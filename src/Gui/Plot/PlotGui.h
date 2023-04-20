#pragma once

#include "Legato/Gui/Window.h"
#include "Legato/Editor/Parameters.h"

class ManoeuvreList;

namespace PlotGui{

	void manoeuvreList();
	void trackSheetEditor();
	//void curveEditor();
	//void spatialEditor();


	class ManoeuvreListWindow : public Legato::Window{
	public:
		ManoeuvreListWindow() : Window("Manoeuvre List", true){}
		virtual void onDraw() override { manoeuvreList(); };
		SINGLETON_GET_METHOD(ManoeuvreListWindow)
	};

	class TrackSheetEditorWindow : public Legato::Window{
	public:
		TrackSheetEditorWindow() : Window("Track Sheet Editor", true){}
		virtual void onDraw() override {
			trackSheetEditor();
		};
		SINGLETON_GET_METHOD(TrackSheetEditorWindow)
	};

	class CurveEditorWindow : public Legato::Window{
	public:
		CurveEditorWindow() : Window("Curve Editor", true){}
		virtual void onDraw() override {
			//curveEditor();
		};
		SINGLETON_GET_METHOD(CurveEditorWindow)
	};

	class SpatialEditorWindow : public Legato::Window{
	public:
		SpatialEditorWindow() : Window("Spatial Editor", true){}
		virtual void onDraw() override {
			//spatialEditor();
		};
		SINGLETON_GET_METHOD(SpatialEditorWindow)
	};


	class ManoeuvreListCreationPopup : public Legato::Popup{
	public:
		ManoeuvreListCreationPopup() : Popup("New Manoeuvre List", true, true){}
		virtual void onDraw() override;
		static void open(std::shared_ptr<Plot> plot);
	private:
		std::shared_ptr<Plot> targetPlot;
		Legato::StringParam nameParameter = Legato::StringParameter::createInstance("", "ManoeuvreListName", "");
	};

	class ManoeuvreListEditPopup : public Legato::Popup{
	public:
		ManoeuvreListEditPopup() : Popup("Edit Manoeuvre List", true, true){}
		virtual void onDraw() override;
		static void open(std::shared_ptr<ManoeuvreList> manoeuvreList);
	private:
		std::shared_ptr<ManoeuvreList> editedManoeuvreList;
		Legato::StringParam nameParameter = Legato::StringParameter::createInstance("", "ManoeuvreListName", "");
	};

	class ManoeuvreListDeletePopup : public Legato::Popup{
	public:
		ManoeuvreListDeletePopup() : Popup("Delete Manoeuvre List", true, true) {}
		virtual void onDraw() override;
		static void open(std::shared_ptr<Plot> plot, std::shared_ptr<ManoeuvreList> manoeuvreList);
	private:
		std::shared_ptr<ManoeuvreList> deletedManoeuvreList;
		std::shared_ptr<Plot> targetPlot;
	};

}
