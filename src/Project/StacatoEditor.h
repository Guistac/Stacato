#pragma once

class StacatoProject;

namespace StacatoEditor{

	bool hasCurrentProject();
	std::shared_ptr<StacatoProject> getCurrentProject();

	void openProject(std::shared_ptr<StacatoProject> project);
	void closeCurrentProject();

};
