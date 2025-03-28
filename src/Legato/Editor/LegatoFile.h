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
		bool hasFileName(){ return !fileName.empty(); }
		
	protected:
		std::filesystem::path fileName;
	};

}
