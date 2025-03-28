#include "LegatoDirectory.h"

void Legato::Directory::setPath(std::filesystem::path input){
	if(input.empty()){
		Logger::error("[Directory] path is empty");
		return;
	}
	directoryName = input;
}

std::filesystem::path Legato::Directory::getPath(){
	std::filesystem::path completePath = directoryName;
	Ptr<Directory> nextDir = parentDirectory;
	while(nextDir){
		completePath = nextDir->getDirectoryName() / completePath;
		nextDir = nextDir->parentDirectory;
	}
	return completePath;
}

bool Legato::Directory::serialize(){
	
	if(directoryName.empty()){
		Logger::error("[Directory] cannot serialize, directory name is empty");
		return false;
	}
	
	std::filesystem::path path = getPath();
	if(!std::filesystem::exists(path)){
		Logger::debug("[Directory] creating {}", path.string());
		std::filesystem::create_directory(path);
	}
	
	for(auto child : getChildren()){
		child->serialize();
	}
	
	return true;
}

bool Legato::Directory::deserialize(){
	
	if(directoryName.empty()){
		Logger::error("[Directory] cannot deserialize, directory name is empty");
		return false;
	}
	
	std::filesystem::path path = getPath();
	if(!std::filesystem::exists(path)){
		Logger::error("[Directory] Could not load, directory does not exists: {}", path.string());
		return false;
	}
	
	for(auto child : getChildren()){
		child->deserialize();
	}
}
