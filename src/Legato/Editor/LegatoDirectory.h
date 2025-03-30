#pragma once

#include "LegatoFile.h"

namespace Legato{

	class Directory : public File{
		COMPONENT_IMPLEMENTATION(Directory)
		friend class Project;
	public:
		static Ptr<Directory> make(std::filesystem::path directoryName_){
			auto newInstance = make();
			newInstance->setPath(directoryName_);
			return newInstance;
		}
		virtual void setPath(std::filesystem::path folderName) override;
		Ptr<Directory> addDirectory(std::filesystem::path folderName);
	protected:
		virtual bool serialize() override;
		virtual bool deserialize() override;
	};

}
