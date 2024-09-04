#include <pch.h>

#include "Workspace.h"
#include <tinyxml2.h>

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
				saveLastLoadedFilePath(path);
				return true;
			}
		}
		return false;
	}

	void setFileOpenCallback(std::function<std::shared_ptr<File>(std::filesystem::path)> callback){
		openFileCallback = callback;
	}

	void saveLastLoadedFilePath(std::filesystem::path loadedFilePath){
		using namespace tinyxml2;
		XMLDocument loadedFileDocument;
		XMLElement* lastLoadedFileXML = loadedFileDocument.NewElement("LastLoadedFile");
		loadedFileDocument.InsertFirstChild(lastLoadedFileXML);

		if(loadedFilePath.is_relative()) loadedFilePath = std::filesystem::current_path() / loadedFilePath.string();

		lastLoadedFileXML->SetText(loadedFilePath.string().c_str());
		loadedFileDocument.SaveFile("LoadedFileMemory.xml");
	}

	bool getLastLoadedFilePath(std::filesystem::path& output){
		using namespace tinyxml2;
		XMLDocument loadedFileDocument;
		if(loadedFileDocument.LoadFile("LoadedFileMemory.xml") != XML_SUCCESS) return false;
		if(XMLElement* lastLoadedFileXML = loadedFileDocument.FirstChildElement("LastLoadedFile")){
			const char* path = lastLoadedFileXML->GetText();
			output = path;
			return true;
		}
		return false;
	}
   

};
