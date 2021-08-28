#pragma once

#include <ethercat.h>
#include <vector>

struct EtherCatPDOEntry {
	uint16_t index;
	uint8_t subindex;
	uint8_t byteCount;
};

struct EtherCatPDOModule {
	uint16_t index;
	std::vector<EtherCatPDOEntry> entries;
	size_t getEntryCount() { return entries.size(); }
};

struct EtherCatPDO {
	std::vector<EtherCatPDOModule> modules;
	size_t getModuleCount() { return modules.size(); }
};

