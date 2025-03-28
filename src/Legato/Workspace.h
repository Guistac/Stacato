#pragma once

//#include "Editor/Project.h"
#include "Editor/LegatoFile.h"

/*
 
 DESCRIPTION
 
 Workspace is the layer above the editor, it owns all projects
 the workspace handles loading files and opening projects
 it can have a single active project and switch between multiple projects
 There should be only a single workspace
 
 Workspace also should handle clipboard

 };
*/

namespace Workspace{

	const std::vector<std::shared_ptr<Legato::File>>& getFiles();
	bool hasFile(std::shared_ptr<Legato::File> file);
	void addFile(std::shared_ptr<Legato::File> file);
	void removeFile(std::shared_ptr<Legato::File> file);
	
	bool openFile(std::filesystem::path file);
	void setFileOpenCallback(std::function<std::shared_ptr<Legato::File>(std::filesystem::path)> callback);

	//std::vector<std::shared_ptr<PrototypeBase>>& getClipboard();
	//void pushToClipboard(std::shared_ptr<PrototypeBase> object);
	//void pushToClipboard(std::vector<std::shared_ptr<PrototypeBase>> objects);

	void saveLastLoadedFilePath(std::filesystem::path loadedFilePath);
	bool getLastLoadedFilePath(std::filesystem::path& output);
};

