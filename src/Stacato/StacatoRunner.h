#pragma once

class StacatoProject;

namespace Stacato::Runner{

	void run_async(std::shared_ptr<StacatoProject> project);
	void stop_async();
	void stop();
	void run(std::shared_ptr<StacatoProject> project);	

	bool isRunning();
	std::shared_ptr<StacatoProject> getRunningProject();

};
