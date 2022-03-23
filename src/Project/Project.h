#pragma once

class Plot;

namespace Project {

	extern std::vector<std::shared_ptr<Plot>> plots;
	extern std::shared_ptr<Plot> currentPlot;

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
	bool isNewProjectRequested();
	void confirmNewProjectRequest();
	void cancelNewProjectRequest();
	
	bool isEditingAllowed();
	void setPlotEdit(bool isAllowed);
	bool isPlotEditAllowed();

};

