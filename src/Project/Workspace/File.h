#pragma once


class File {
public:
	
	void setFilePath(std::filesystem::path filePath_){ filePath = filePath_; }
	
	std::filesystem::path getFilePath(){ return filePath; }
	
	bool hasFilePath(){ return !filePath.empty(); }
	
	bool writeFile(){
		if(filePath.empty()){
			Logger::warn("[File] could not write file : path is empty");
			return false;
		}
		return onWriteFile();
	}
	
	bool readFile(){
		if(filePath.empty()){
			Logger::warn("[File] could not read file : path is empty");
			return false;
		}
		if(!exists()){
			Logger::warn("[File] could not read file : path does not exist");
			return false;
		}
		return onReadFile();
	}
	
	bool exists(){
		return std::filesystem::exists(filePath);
	}
	
protected:
	
	std::filesystem::path filePath;
	
	virtual bool onWriteFile() = 0;
	virtual bool onReadFile() = 0;
};

