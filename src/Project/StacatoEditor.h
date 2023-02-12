#pragma once

#include "StacatoProject.h"

namespace StacatoEditor{

	bool hasCurrentProject();
	std::shared_ptr<StacatoProject> getCurrentProject();

	void openProject(std::shared_ptr<StacatoProject> project);
	void closeCurrentProject();

	void createNewProject();

};
