#pragma once

#include "Legato/Editor/Project.h"

#include "LayoutList.h"

namespace AnimationSystem{ class AnimatableRegistry; }

class Plot;
class Manoeuvre;
class EnvironnementObject;

 class StacatoProject : public Project{
	 
	 DECLARE_PROTOTYPE_IMPLENTATION_METHODS(StacatoProject)
	 
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
	 
	 virtual bool canClose() override;
	 
	 std::shared_ptr<LayoutList> getLayouts(){ return layouts; }
	 std::shared_ptr<AnimationSystem::AnimatableRegistry> getAnimatableRegistry(){ return animatableRegistry; }
	 std::shared_ptr<EnvironnementObject> getEnvironnement(){ return environnement; }
	 
protected:
	 
	 virtual void onConstruction() override;
	 virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override;
	 virtual bool onWriteFile() override;
	 virtual bool onReadFile() override;
	 virtual void onOpen() override;
	 virtual void onClose() override;
	 
 private:
	 
	 std::shared_ptr<LayoutList> layouts;
	 
	 std::vector<std::shared_ptr<Plot>> plots;
	 std::shared_ptr<Plot> currentPlot;
	 bool b_plotEditLocked = true;
	 
	 std::shared_ptr<EnvironnementObject> environnement = nullptr;
	 std::shared_ptr<AnimationSystem::AnimatableRegistry> animatableRegistry = nullptr;
 };
