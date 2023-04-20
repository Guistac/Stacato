#pragma once

#include "Legato/Editor/Project.h"

#include "LayoutList.h"

class Plot;
class Manoeuvre;
class EnvironnementObject;

 class StacatoProject : public Project{
	 
	 DECLARE_PROTOTYPE_IMPLENTATION_METHODS(StacatoProject)
	 
 public:
	 
	 std::shared_ptr<EnvironnementObject> getEnvironnement(){ return environnement; }
	 
	 std::shared_ptr<Plot> getPlot() { return plot; }
	 void lockPlotEdit(){ b_plotEditLocked = true; }
	 void unlockPlotEdit(){ b_plotEditLocked = false; }
	 bool isPlotEditLocked(){ return b_plotEditLocked; }
	 
	 virtual bool canClose() override;
	 
	 std::shared_ptr<LayoutList> getLayouts(){ return layouts; }
	 
protected:
	 
	 virtual void onConstruction() override;
	 virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override;
	 virtual bool onWriteFile() override;
	 virtual bool onReadFile() override;
	 virtual void onOpen() override;
	 virtual void onClose() override;
	 
 private:
	 
	 std::shared_ptr<EnvironnementObject> environnement = nullptr;
	 
	 std::shared_ptr<Plot> plot;
	 bool b_plotEditLocked = true;
	 
	 std::shared_ptr<LayoutList> layouts;
 };
