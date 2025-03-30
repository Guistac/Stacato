#include "LegatoDirectory.h"

Ptr<Legato::Directory> Legato::Directory::addDirectory(std::filesystem::path folderName){
	auto newDirectory = Directory::make(folderName);
	addChild(newDirectory);
	return newDirectory;
}

void Legato::Directory::setPath(std::filesystem::path input){
	if(input.empty()){
		Logger::error("[Directory] path is empty");
		return;
	}
	else if(std::distance(input.begin(), input.end()) != 1){
		Logger::error("[Directory] '{}' path cannot contain multiple directories", input.string());
		return;
	}
	else path = input;
}

bool Legato::Directory::serialize(){
	
	if(path.empty()){
		Logger::error("[Directory] cannot serialize, directory name is empty");
		return false;
	}
	
	std::filesystem::path path = getCompletePath();
	if(!std::filesystem::exists(path)){
		Logger::debug("[Directory] creating {}", path.string());
		std::filesystem::create_directory(path);
	}
	
	bool b_success = true;
	for(auto child : getChildren()){
		b_success &= child->serialize();
	}
	
	return b_success;
}

bool Legato::Directory::deserialize(){
	
	if(path.empty()){
		Logger::error("[Directory] cannot deserialize, directory name is empty");
		return false;
	}
	
	std::filesystem::path path = getCompletePath();
	if(!std::filesystem::exists(path)){
		Logger::error("[Directory] Could not load, directory does not exists: {}", path.string());
		return false;
	}
	
	bool b_success = true;
	for(auto child : getChildren()){
		b_success &= child->deserialize();
	}
	
	return b_success;
}
