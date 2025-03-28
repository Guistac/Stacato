#pragma once

#include "LegatoComponent.h"

namespace Legato{

	class File : public Component{
		COMPONENT_INTERFACE(File)
	public:
		
		void onConstruction() override;
		void setFileName(std::filesystem::path input);
		
		virtual bool serialize() override;
		virtual bool deserialize() override;
		
		std::filesystem::path getFileName(){ return fileName; }
		std::filesystem::path getPath();
		
	private:
		std::filesystem::path fileName;
	};



	class Directory : public Component{
		COMPONENT_IMPLEMENTATION(Directory)
		friend class Project;
	public:
		static Ptr<Directory> make(std::filesystem::path directoryName_){
			auto newInstance = make();
			newInstance->setPath(directoryName_);
			return newInstance;
		}
		void setPath(std::filesystem::path input);
		std::filesystem::path getDirectoryName(){ return directoryName; }
		std::filesystem::path getPath();
	protected:
		virtual bool serialize() override;
		virtual bool deserialize() override;
	private:
		std::filesystem::path directoryName;
	};

}
