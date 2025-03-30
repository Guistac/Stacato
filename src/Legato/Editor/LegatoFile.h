#pragma once

#include "LegatoComponent.h"

namespace Legato{

	class File : public Component{
		COMPONENT_INTERFACE(File)
	public:
		
		virtual void onConstruction() override;
		virtual bool serialize() override;
		virtual bool deserialize() override;
		
		virtual void setPath(std::filesystem::path fileName);
		bool hasPath(){ return !path.empty(); }
		std::filesystem::path getPath(){ return path; }
		std::filesystem::path getCompletePath();
		
	protected:
		std::filesystem::path path;
	};

}
