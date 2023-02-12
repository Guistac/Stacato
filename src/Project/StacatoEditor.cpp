#include <pch.h>

#include "StacatoEditor.h"




namespace StacatoEditor{

std::shared_ptr<StacatoProject> currentProject;

bool hasCurrentProject(){ return currentProject != nullptr; }

std::shared_ptr<StacatoProject> getCurrentProject(){ return currentProject; }

void openProject(std::shared_ptr<StacatoProject> project){
	
}

void closeCurrentProject(){
	
}

};
