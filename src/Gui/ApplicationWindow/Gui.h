#pragma once

namespace Gui{

	void draw();
	void menuBar();
	void toolbar(float height);

	void quitApplicationModal();
	void aboutModal();
	void unlockEnvironnementEditorModal(); //this needs to go to another translation unit

}
