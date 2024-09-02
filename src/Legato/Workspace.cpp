#include <pch.h>

#include "Workspace.h"

namespace Workspace{

	std::vector<std::shared_ptr<File>> files;

	const std::vector<std::shared_ptr<File>>& getFiles(){ return files; }

	void addFile(std::shared_ptr<File> file){
		files.push_back(file);
	}

	void removeFile(std::shared_ptr<File> file){
		for(size_t i = files.size() - 1; i >= 0; i--){
			if(file == files[i]){
				files.erase(files.begin() + i);
				break;
			}
		}
	}

	bool hasFile(std::shared_ptr<File> file){
		std::filesystem::path queriedPath = file->getFilePath();
		for(auto listedFile : files){
			if(listedFile->getFilePath() == queriedPath) return true;
		}
		return false;
	}

	std::function<std::shared_ptr<File>(std::filesystem::path)> openFileCallback;

	bool openFile(std::filesystem::path path){
		if(std::shared_ptr<File> openedFile = openFileCallback(path)){
			if(hasFile(openedFile)){
				//decide how we handle files that are already open
				Logger::warn("File is already open");
				return false;
			}else{
				addFile(openedFile);
				return true;
			}
		}
		return false;
	}

	void setFileOpenCallback(std::function<std::shared_ptr<File>(std::filesystem::path)> callback){
		openFileCallback = callback;
	}
   

};
