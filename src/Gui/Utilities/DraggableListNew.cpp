#include <pch.h>

#include <imgui.h>

#include "DraggableListNew.h"

namespace DraggableListNew{

	bool begin(const char* ID, ImVec2 size_arg){
		if(size_arg.x <= 0.0 || size_arg.y <= 0.0) size_arg = ImGui::GetContentRegionAvail();
		bool b_open = ImGui::BeginChild(ID, size_arg);
		if(!b_open) end(false);
		
		
		return b_open;
	}


	void end(bool b_wasOpen){
		ImGui::EndChild();
	}

	bool wasReordered(){ return false; }
	void getReorderedIndex(int& from, int& to){}

	bool beginItem(const char* ID, float height){
		assert(height > 0.0);
		
		bool b_open = ImGui::BeginChild(ID, ImVec2(ImGui::GetContentRegionAvail().x, height), true);
		if(!b_open) endItem(false);
		
		return b_open;
	}

	void endItem(bool b_wasOpen){
		ImGui::EndChild();
	}

	bool wasItemClicked(){}


};
