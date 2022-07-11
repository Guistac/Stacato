#include <pch.h>
#include "EnvironnementScript.h"

#include "Scripting/Script.h"
#include "Scripting/EnvironnementLibrary.h"
#include "Scripting/LoggingLibrary.h"

#include "Environnement.h"

namespace Environnement{
namespace Script{

	std::string directory;

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
		   
		   std::string path = directory;
		   path.append("?.lua");
		   
		   lua_getglobal( L, "package" );
		   lua_getfield( L, -1, "path" ); // get field "path" from table at top of stack (-1)
		   std::string cur_path = lua_tostring( L, -1 ); // grab path string from top of stack
		   cur_path.append( ";" ); // do your path magic here
		   cur_path.append( path );
		   lua_pop( L, 1 ); // get rid of the string on the stack we just pushed on line 5
		   lua_pushstring( L, cur_path.c_str() ); // push the new one
		   lua_setfield( L, -2, "path" ); // set the field "path" in table at -2 with value at top of stack
		   lua_pop( L, 1 ); // get rid of package table from top of stack

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
		std::filesystem::path fileSystemPath = filePath;
		directory = fileSystemPath.remove_filename().string();
	}


};
};
