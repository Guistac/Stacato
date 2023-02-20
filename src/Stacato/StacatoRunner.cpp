#include <pch.h>
#include "StacatoRunner.h"

#include "Project/StacatoProject.h"

namespace Stacato::Runner{

	std::shared_ptr<StacatoProject> runningProject;
	std::thread runnerThread;
	bool b_isRunning = false;
	std::mutex runnerMutex;

	void projectRuntime(){
		
		while(b_isRunning){
			
		}
	}

	void run_async(std::shared_ptr<StacatoProject> project){
		std::lock_guard<std::mutex> lock(runnerMutex);
		if(b_isRunning) {
			Logger::warn("[Stacato Runner] Cannot run project {} : another project is still running", project->getName());
			return false;
		}
		runningProject = project;
		b_isRunning = true;
		runnerThread = std::thread(projectRuntime);
		return true;
	}

	void stop(){
		if(b_isRunning) {
			Logger::warn("[Stacato Runner] Cannot stop running : no project is running");
			return;
		}
		b_isRunning = false;
		if(runnerThread.joinable()) runnerThread.join();
	}

	void stop_async(){
		std::thread([](){
			stop();
		});
	}

	bool isRunning();

	std::shared_ptr<StacatoProject> getRunningProject(){
		std::lock_guard<std::mutex> lock(runnerMutex);
		return runningProject;
	}

};
