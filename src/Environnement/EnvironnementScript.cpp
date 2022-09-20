#include <pch.h>
#include "EnvironnementScript.h"

#include "Scripting/Script.h"
#include "Scripting/EnvironnementLibrary.h"
#include "Scripting/LoggingLibrary.h"

#include "Environnement.h"

namespace Environnement{
namespace Script{

    std::mutex mutex;

	std::string scriptFolder;

	LuaScript script("Environnement Script");

	void editor(glm::vec2 size){
		script.editor(size);
	}

	bool canCompile(){
		return !Environnement::isRunning();
	}

	void compile(){
		if(!canCompile()) return;
	   script.stop();
	   script.compile();
	   script.stop();
   }
   
   void start(){
	   script.stop();
	   mutex.lock();
	   script.setLoadLibrairiesCallback([](lua_State* L){
		   Scripting::EnvironnementLibrary::openlib(L, true);
		   
		   std::string folderIncludePath = scriptFolder + "/?.lua";
		   std::string parentFolderIncludePath = std::filesystem::path(scriptFolder).parent_path().string() + "/?.lua";
		   std::string lua_path;
		   lua_path.append(folderIncludePath);
		   lua_path.append(";");
		   lua_path.append(parentFolderIncludePath);
		   
		   lua_getglobal(L, "package"); 			//get the package table
		   lua_pushstring( L, lua_path.c_str() ); 	//push the new path string
		   lua_setfield( L, -2, "path" ); 			//set the new path string as index "path" of the table at -2
		   lua_pop( L, 1 ); 						//pop the package table from top of stack

	   });
	   script.compileAndRun();
	   if(script.checkHasFunction("setup")) script.callFunction("setup");
	   mutex.unlock();
	   Logger::info("Started Environnement Script");
   }

	void update(){
		mutex.lock();
		if(script.isRunning()) script.callFunction("update");
		mutex.unlock();
	}

   void stop(){
	   mutex.lock();
	   if(script.checkHasFunction("exit")) script.callFunction("exit");
	   script.stop();
	   mutex.unlock();
	   Logger::info("Stopped Environnement Script");
   }

	bool isRunning(){
		return script.isRunning();
	}

	void reset(){
		std::string defaultScript =
			"\n"
			"---------------------------------------\n"
			"----Default Environnement Script----\n"
			"---------------------------------------\n\n\n"
			"--Initialize stuff here :\n\n"
			"function setup()\n"
			"end\n\n\n"
			"--Perform Environnement Logic here :\n\n"
			"function update()\n"
			"end\n\n\n"
			"--Clean Up stuff here :\n\n"
			"function exit()\n"
			"end\n";
		script.load(defaultScript);
	}

	void save(std::string folderPath){
		std::string path = folderPath + "/main.lua";
		script.save(path.c_str());
	}

	void load(std::string folderPath){
		std::string path = folderPath + "/main.lua";
		script.stop();
		script.load(path.c_str());
		scriptFolder = folderPath;
	}


};
};
