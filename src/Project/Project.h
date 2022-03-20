#pragma once

class Plot;

namespace Project {

	extern std::vector<std::shared_ptr<Plot>> plots;
	extern std::shared_ptr<Plot> currentPlot;
    
	extern char saveFilePath[];
	extern bool b_hasFilePath;
	extern bool b_hasModifications;

	void createNew();

	void loadStartup();
	bool load(const char* directory);
	bool reloadSaved();

	bool saveAs(const char* directory);
	bool save();
	
	
	bool isEditingAllowed();
	void setPlotEdit(bool isAllowed);
	bool isPlotEditAllowed();

};

