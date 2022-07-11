#include <pch.h>
#include "EnvironnementScript.h"

#include "Scripting/Script.h"
#include "Scripting/EnvironnementLibrary.h"
#include "Scripting/LoggingLibrary.h"

#include "Environnement.h"

namespace Environnement{
namespace Script{


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
	   script.setLoadLibrairiesCallback([](lua_State* L){
		   Scripting::EnvironnementLibrary::openlib(L, true);
	   });
	   script.compileAndRun();
	   if(script.checkHasFunction("setup")) script.callFunction("setup");
	   Logger::info("Started Environnement Script");
   }

	void update(){
		if(script.isRunning()) script.callFunction("update");
	}

   void stop(){
	   //TODO: this sometimes crashes
	   if(script.checkHasFunction("exit")) script.callFunction("exit");
	   script.stop(); //TODO: this crashes sometimes
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

	void save(const char* filePath){
		script.save(filePath);
	}

	void load(const char* filePath){
		script.stop();
		script.load(filePath);
	}


};
};
