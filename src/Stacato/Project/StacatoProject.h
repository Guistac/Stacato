#pragma once

#include "Legato/Editor/LegatoProject.h"

#include "LayoutList.h"


class Plot;
class Manoeuvre;

 class StacatoProject : public Legato::Project{
	 
	 COMPONENT_IMPLEMENTATION(StacatoProject)
	 
 public:
	 std::vector<std::shared_ptr<Plot>>& getPlots(){ return plots; }
	 std::shared_ptr<Plot> getCurrentPlot(){ return currentPlot; }
	 void setCurrentPlot(std::shared_ptr<Plot> plot);
	 std::shared_ptr<Plot> createNewPlot();
	 void deletePlot(std::shared_ptr<Plot> plot);
	 void duplicatePlot(std::shared_ptr<Plot> plot);

	 void deleteAllPlots();
	 void addPlot(std::shared_ptr<Plot> plot);

	 void lockPlotEdit(){ b_plotEditLocked = true; }
	 void unlockPlotEdit(){ b_plotEditLocked = false; }
	 bool isPlotEditLocked(){ return b_plotEditLocked; }
	 
	 std::shared_ptr<LayoutList> getLayouts(){ return layouts; }
	 
protected:
	 
	 virtual void onConstruction() override;
	 virtual void copyFrom(Ptr<Legato::Component> source) override;
	 virtual bool onSerialization() override;
	 virtual bool onDeserialization() override;
	 virtual void onPostLoad() override;
	 
 private:
	 
	 std::shared_ptr<LayoutList> layouts;
	 
	 std::vector<std::shared_ptr<Plot>> plots;
	 std::shared_ptr<Plot> currentPlot;
	 bool b_plotEditLocked = true;
 };
