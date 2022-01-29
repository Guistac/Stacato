#include <memory>
#include <vector>
#include <queue>
#include <deque>
#include <algorithm>
#include <thread>
#include <mutex>
#include <chrono>
#include <sstream>
#include <bitset>
#include <map>
#include <cmath>
#include <filesystem>
#include <ctime>
#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/glm.hpp>

#include "Utilities/Logger.h"
#include "Utilities/Timing.h"
#include "Utilities/Random.h"
#include "Utilities/Enumerator.h"

#define IMGUI_USER_CONFIG "Gui/ApplicationWindow/ImGuiCustomConfiguration.h"

#define BEGIN_DISABLE_IMGUI_ELEMENT	{ ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true); ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));	}

#define END_DISABLE_IMGUI_ELEMENT	{ ImGui::PopItemFlag(); ImGui::PopStyleColor();	}

#define clampValue(in,rangeA,rangeB) if(rangeA < rangeB) {					\
									if (in < rangeA) in = rangeA;		\
									else if(in > rangeB) in = rangeB;	\
								}										\
								else {									\
									if (in < rangeB) in = rangeB;		\
									else if(in > rangeA) in = rangeA;	\
								}										\
