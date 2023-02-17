#include <pch.h>

#include "StacatoWorkspace.h"
#include "StacatoProject.h"
#include "Legato/Workspace.h"



namespace Stacato::Workspace{

std::shared_ptr<StacatoProject> currentProject;

bool onQuitRequest(){
	if(currentProject == nullptr) return true;
	return currentProject->canClose();
}

std::shared_ptr<File> openFile(std::filesystem::path path){

	if(!path.has_filename()){
		Logger::error("[Stacato] Could not open file : Path has no File Name");
		return nullptr;
	}
	
	if(!path.has_extension()){
		Logger::error("[Stacato] Could not open file : File has no extension");
		return nullptr;
	}
	
	std::string fileName = path.filename().string();
	std::string fileExtension = path.extension().string();
	
	if(fileExtension == ".stacato"){
		auto loadedProject = StacatoProject::createInstance();
		loadedProject->setFilePath(path);
		if(!loadedProject->readFile()){
			Logger::error("[Stacato] Failed to open stacato project {}", fileName);
			return nullptr;
		}
		openProject(loadedProject);
		return loadedProject;
	}
	
	Logger::error("[Stacato] Could not open file '{}' : Unsupported file type", fileName);
	return nullptr;
}

bool hasCurrentProject(){ return currentProject != nullptr; }

std::shared_ptr<StacatoProject> getCurrentProject(){ return currentProject; }

void closeCurrentProject(){
	currentProject = nullptr;
}

void openProject(std::shared_ptr<StacatoProject> project){
	currentProject = project;
}

void createNewProject(){
	if(hasCurrentProject() && !currentProject->canClose()) return;
	
	closeCurrentProject();
	auto newProject = StacatoProject::createInstance();
	openProject(newProject);
	::Workspace::addFile(newProject);
}

};
