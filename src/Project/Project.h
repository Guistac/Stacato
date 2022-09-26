#pragma once

class Plot;
class Manoeuvre;

namespace Project {

	std::vector<std::shared_ptr<Plot>>& getPlots();
	std::shared_ptr<Plot> getCurrentPlot();
	void setCurrentPlot(std::shared_ptr<Plot> plot);
	std::shared_ptr<Plot> createNewPlot();
	void deletePlot(std::shared_ptr<Plot> plot);
	void duplicatePlot(std::shared_ptr<Plot> plot);

	void deleteAllPlots();
	void addPlot(std::shared_ptr<Plot> plot);

	void lockPlotEdit();
	void unlockPlotEdit();
	bool isPlotEditLocked();

	void pushManoeuvreToClipboard(std::shared_ptr<Manoeuvre> manoeuvre);
	std::shared_ptr<Manoeuvre> getClipboardManeouvre();

	//extern std::vector<std::shared_ptr<Plot>> plots;
	//extern std::shared_ptr<Plot> currentPlot;

	void createNew();
	void loadStartup();
	bool load(const char* directory);
	bool reloadSaved();

	bool hasFilePath();
	bool saveAs(const char* directory);
	bool save();

	void setModified();
	bool hasUnsavedModifications();

	bool canCloseImmediately();
	
	void confirmNewProjectRequest();
	
	bool isEditingAllowed();
	void setPlotEdit(bool isAllowed);
	bool isPlotEditAllowed();

};

