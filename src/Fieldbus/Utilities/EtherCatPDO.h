#pragma once

#include <ethercat.h>

struct EtherCatPdoEntry {
	uint16_t index;
	uint8_t subindex;
	uint8_t byteCount;
	void* dataPointer;
	char name[64];
};

struct EtherCatPdoMappingModule {
	uint16_t index;
	std::vector<EtherCatPdoEntry> entries;
	size_t getEntryCount() { return entries.size(); }
};

struct EtherCatPdoAssignement {
	std::vector<EtherCatPdoMappingModule> modules;
	size_t getModuleCount() { return modules.size(); }

	void addNewModule(uint16_t idx) { 
		EtherCatPdoMappingModule module;
		module.index = idx;
		modules.push_back(module);
	}
	
	void addEntry(uint16_t idx, uint8_t sidx, uint8_t byteCount, const char* n, void* data) {
		EtherCatPdoEntry entry;
		entry.index = idx;
		entry.subindex = sidx;
		entry.dataPointer = data;
		entry.byteCount = byteCount;
		modules.back().entries.push_back(entry);
		strcpy(modules.back().entries.back().name, n);
	}

	int getByteCount() {
		int output = 0;
		for (EtherCatPdoMappingModule& module : modules) {
			for (EtherCatPdoEntry& entry : module.entries) {
				output += entry.byteCount;
			}
		}
	}

};


