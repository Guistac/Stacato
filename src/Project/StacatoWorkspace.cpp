#include <pch.h>

#include "Stacato.h"
#include "Workspace/Workspace.h"
#include "StacatoProject.h"

 namespace Stacato::Application{
 
 
 bool openFile(std::filesystem::path path){
 
	 if(!path.has_filename()){
		 Logger::error("[Stacato Workspace] Could not open file : File is a folder");
		 return false;
	 }
	 
	 if(!path.has_extension()){
		 Logger::error("[Stacato Workspace] Could not open file : File has no extension");
		 return false;
	 }
	 
	 std::string fileName = path.filename().string();
	 std::string fileExtension = path.extension().string();
	 
	 if(fileExtension == ".stacato"){
		 auto loadedProject = StacatoProject::createInstance();
		 loadedProject->setFilePath(path);
		 if(!loadedProject->readFile()){
			 Logger::error("[Stacato Workspace] Failed to open stacato project {}", fileName);
			 return false;
		 }
		 Workspace::openProject(nullptr);
		 return true;
	 }
	 
	 return false;
 }
 
 
 };
 
