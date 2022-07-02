#include <pch.h>
#include "Fieldbus/Utilities/DS402.h"
#include "Gui/Assets/Colors.h"

namespace DS402 {

	glm::vec4 getColor(PowerState state){
		switch(state){
			case DS402::PowerState::NOT_READY_TO_SWITCH_ON:
			case DS402::PowerState::SWITCH_ON_DISABLED:
				return Colors::red;
			case DS402::PowerState::READY_TO_SWITCH_ON:
				return Colors::orange;
			case DS402::PowerState::SWITCHED_ON:
				return Colors::yellow;
			case DS402::PowerState::OPERATION_ENABLED:
				return Colors::green;
			case DS402::PowerState::QUICKSTOP_ACTIVE:
			case DS402::PowerState::FAULT_REACTION_ACTIVE:
			case DS402::PowerState::FAULT:
			case DS402::PowerState::UNKNOWN:
				return Colors::red;
		}
	}


}
