#pragma once

namespace DraggableListNew{

	bool begin(const char* ID, ImVec2 size_arg = ImVec2(0.0f, 0.0f));
	void end(bool b_wasOpen = true);

	bool wasReordered();
	void getReorderedIndex(int& from, int& to);


	bool beginItem(const char* ID, float height);
	void endItem(bool b_wasOpen = true);

	bool wasItemClicked();

};
