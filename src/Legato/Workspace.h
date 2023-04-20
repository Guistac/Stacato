#pragma once

#include "Editor/Project.h"

/*
 
 DESCRIPTION
 
 Workspace if the layer above the editor, it owns all projects
 the workspace handles loading files and opening projects
 it can have a single active project and switch between multiple projects
 There should be only a single workspace
 
 Workspace also should handle clipboard

 };
*/

namespace Workspace{

	const std::vector<std::shared_ptr<File>>& getFiles();
	bool hasFile(std::shared_ptr<File> file);
	void addFile(std::shared_ptr<File> file);
	void removeFile(std::shared_ptr<File> file);
	
	bool openFile(std::filesystem::path file);
	void setFileOpenCallback(std::function<std::shared_ptr<File>(std::filesystem::path)> callback);

	std::vector<std::shared_ptr<Legato::PrototypeBase>>& getClipboard();
	void pushToClipboard(std::shared_ptr<Legato::PrototypeBase> object);
	void pushToClipboard(std::vector<std::shared_ptr<Legato::PrototypeBase>> objects);
};

