#include "EL7221-9014-MotionInterface.h"

void EL7221_9014::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL7221-9014")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}

