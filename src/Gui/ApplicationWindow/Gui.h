#pragma once

namespace Gui{

	void draw();
	
	void menuBar();
	void toolbar(float height);
	void popups();
	
	void quitApplicationPopup();
	
	void openAboutPopup();
	bool isAboutPopupOpenRequested();
	void aboutPopup();

	void resetDefaultLayout();
	bool shouldResetDefaultLayout();
	void finishResetDefaultLayout();

}
