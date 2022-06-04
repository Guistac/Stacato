#pragma once

namespace DraggableListNew{

	bool begin(const char* ID, ImVec2 size_arg = ImVec2(0.0f, 0.0f));
	void end(bool b_wasOpen = true);

	bool wasReordered(int& fromIndex, int& toIndex);

	bool beginItem(const char* ID, float height);
	void endItem(bool b_wasOpen = true);

};





void dragListTest();
