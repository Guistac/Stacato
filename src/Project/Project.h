#pragma once

class Plot;

namespace Project {

	extern std::vector<std::shared_ptr<Plot>> plots;
	extern std::shared_ptr<Plot> currentPlot;
    extern char projectDirectory[];

	bool hasDefaultSavePath();

	bool load(const char* directory);
	bool reload();

	bool saveAs(const char* directory);
	bool save();

	void createNew();
	
	bool isEditingAllowed();

};

