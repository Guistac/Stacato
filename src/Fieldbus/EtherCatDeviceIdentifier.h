#pragma once

#include <memory>

#include "Devices/SchneiderElectric/Lexium32.h"

std::shared_ptr<EtherCatSlave> getSlaveByName(const char* name);