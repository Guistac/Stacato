#pragma once

#include <ethercat.h>

#include "EtherCatData.h"

struct EtherCatPdoEntry {
	uint16_t index;
	uint8_t subindex;
	char name[64];

	uint8_t byteCount;
	uint8_t bitCount;
	int byteOffset;
	int bitOffset;

	void* dataPointer;
};

struct EtherCatPdoMappingModule {
	uint16_t index;
	std::vector<EtherCatPdoEntry> entries;
	size_t getEntryCount() { return entries.size(); }
	
	/*
	void addEntry(uint16_t idx, uint8_t sidx, uint8_t bitCount, const char* n, void* data) {
		EtherCatPdoEntry entry;
		entry.index = idx;
		entry.subindex = sidx;
		entry.dataPointer = data;
		entry.bitCount = bitCount;
		entry.byteCount = bitCount / 8;
		entries.push_back(entry);
	}
	*/
};

struct EtherCatPdoAssignement {
	std::vector<EtherCatPdoMappingModule> modules;
	int totalByteCount = 0;
	int totalBitCount = 0;
	size_t getModuleCount() { return modules.size(); }

	void clear() {
		modules.clear();
		totalBitCount = 0;
		totalByteCount = 0;
	}

	void addNewModule(uint16_t idx) { 
		EtherCatPdoMappingModule module;
		module.index = idx;
		modules.push_back(module);
	}
	
	void addEntry(uint16_t idx, uint8_t sidx, uint8_t bitCount, std::string n, void* data) {

		EtherCatPdoEntry entry;
		entry.index = idx;
		entry.subindex = sidx;
		entry.dataPointer = data;

		entry.bitCount = bitCount;
		entry.byteCount = bitCount / 8;
		entry.byteOffset = totalByteCount;
		entry.bitOffset = totalBitCount % 8;

		modules.back().entries.push_back(entry);
		strcpy(modules.back().entries.back().name, n.c_str());

		totalBitCount += bitCount;
		totalByteCount = totalBitCount / 8;
	}

	int getByteCount() {
		int output = 0;
		for (EtherCatPdoMappingModule& module : modules) {
			for (EtherCatPdoEntry& entry : module.entries) {
				output += entry.bitCount;
			}
		}
		return output;
	}

	bool mapToSyncManager(uint16_t slaveIndex, uint16_t syncManagerIndex, bool b_PDOconfig = true) {
		
		//disable sync manager
		if (!CanOpen::writeSDO_U8(syncManagerIndex, 0x0, 0x0, slaveIndex)) {
			return Logger::error("Failed to disable sync manager {:#x}", syncManagerIndex);
		}else Logger::trace("Disabled Sync Manager {:#x}", syncManagerIndex);
		
	
		if(b_PDOconfig){
		
			//for each mapping module
			for (int i = 0; i < modules.size(); i++) {
				
				EtherCatPdoMappingModule& pdoModule = modules[i];
				
				//disable the module
				if (!CanOpen::writeSDO_U8(pdoModule.index, 0x0, 0x0, slaveIndex)) {
					return Logger::error("Failed to disable PDO module {:#x} (is PDO Config supported ?)", pdoModule.index);
				}else Logger::trace("Disabled PDO module {:#x}", pdoModule.index);
				
				/*
				uint8_t entryCount = pdoModule.getEntryCount();
				if(!CanOpen::writeSDO_U8(pdoModule.index, 0x0, entryCount, slaveIndex)) {
					Logger::error("Failed to set Mapping Module Object {:#x} entry count ({})", pdoModule.index, entryCount);
				}else Logger::info("Set Mapping Module Object {:#x} entry count ({})", pdoModule.index, entryCount);
				*/
				 
				for (int j = 0; j < pdoModule.getEntryCount(); j++) {
					//concatenate the entry data
					EtherCatPdoEntry& entry = pdoModule.entries[j];
					uint32_t entryMapping = entry.index << 16 | entry.subindex << 8 | entry.bitCount;
					//write the entry to the module
					if (!CanOpen::writeSDO_U32(pdoModule.index, j + 1, entryMapping, slaveIndex)) {
						return Logger::error("Failed to configure PDO module object {:#x}:{:x} (is PDO Config supported ?)", pdoModule.index, j + 1);
					}else Logger::trace("Configured PDO module object {:#x}:{:x}", pdoModule.index, j + 1);
					
				}
				
				
				//enable the module by writing the entry count
				if (!CanOpen::writeSDO_U8(pdoModule.index, 0x0, pdoModule.getEntryCount(), slaveIndex)) {
					return Logger::error("Failed to reenable PDO module {:#x} (size: {}) (is PDO Config supported ?)", pdoModule.index, pdoModule.getEntryCount());
				}else Logger::trace("Reenabled PDO module {:#x} (size: {})", pdoModule.index, pdoModule.getEntryCount());
			}
			
		}
	
		for (int i = 0; i < modules.size(); i++) {
			uint16_t pdoMappingObjectIndex = modules[i].index;
			//assign each module to the sync manager
			if (!CanOpen::writeSDO_U16(syncManagerIndex, i + 1, modules[i].index, slaveIndex)) {
				return Logger::error("Failed to set PDO module {:#x} to sync manager {:#x}:{:x}", modules[i].index, syncManagerIndex, i + 1);
			}else Logger::trace("Set PDO module {:#x} to sync manager {:#x}:{:x}", modules[i].index, syncManagerIndex, i + 1);
			
		}
		
		//enable the sync manager by setting the number of modules
		uint8_t moduleCount = (uint8_t)getModuleCount();
		if (!CanOpen::writeSDO_U8(syncManagerIndex, 0x0, moduleCount, slaveIndex)) {
			return Logger::error("Failed to reenable sync manager {:#x} (size: {})", syncManagerIndex, moduleCount);
		}else Logger::trace("Reenabled sync manager {:#x} (size: {})", syncManagerIndex, moduleCount);
		
		return true;
	}
	
	bool mapToRxPdoSyncManager(uint16_t slaveIndex, bool b_PDOconfig = true){
		return mapToSyncManager(slaveIndex, 0x1C12, b_PDOconfig);
	}
	
	bool mapToTxPdoSyncManager(uint16_t slaveIndex, bool b_PDOconfig = true){
		return mapToSyncManager(slaveIndex, 0x1C13, b_PDOconfig);
	}

	void pullDataFrom(uint8_t* buffer) {
		for (int i = 0; i < getModuleCount(); i++) {
			EtherCatPdoMappingModule& module = modules[i];
			for (int j = 0; j < module.getEntryCount(); j++) {

				EtherCatPdoEntry& entry = module.entries[j];
				uint8_t* inBuffer = buffer + entry.byteOffset;

				if (entry.bitCount % 8 == 0 && entry.bitOffset == 0) {
					//if the data is only full bytes and is aligned to a bit start
					//cast the input buffer pointer to the correct size and copy it to the user data pointer
					if (entry.byteCount == 1) *((uint8_t*)entry.dataPointer) = *(uint8_t*)inBuffer;
					else if (entry.byteCount == 2) *((uint16_t*)entry.dataPointer) = *(uint16_t*)inBuffer;
					else if (entry.byteCount <= 4) *((uint32_t*)entry.dataPointer) = *(uint32_t*)inBuffer;
					else *((uint64_t*)entry.dataPointer) = *(uint64_t*)inBuffer;
				}
				else if (entry.bitCount == 1){
					bool value = *inBuffer & (0x1 << entry.bitOffset);
					*(bool*)entry.dataPointer = value;
				}else{
					//if the data is not full bytes and or is not aligned to a bit start
					//copy and shift the data to a byte start
					
					//TODO: this does not work, and is probably not necessary either
					/*
					unsigned long long data = *((unsigned long long*)inBuffer) << entry.bitOffset;
					unsigned long long mask;
					if (entry.bitCount < 64) mask = (0x1ULL << entry.bitCount) - 1;
					else mask = -1;
					data &= mask;
					for (int b = 0; b < entry.bitCount; b++) {
						bool bit = (data >> b) & 0x1ULL;
						if (bit) data |= 0x1ULL << b;
						else data &= ~(0x1ULL << b);
					}
					if (entry.byteCount == 1) *((uint8_t*)entry.dataPointer) = data;
					else if (entry.byteCount == 2) *((uint16_t*)entry.dataPointer) = data;
					else if (entry.byteCount <= 4) *((uint32_t*)entry.dataPointer) = data;
					else *((uint64_t*)entry.dataPointer) = data;
					 */
				}
			}
		}
	}


	void pushDataTo(uint8_t* buffer) {
		for (int i = 0; i < getModuleCount(); i++) {
			EtherCatPdoMappingModule& module = modules[i];
			for (int j = 0; j < module.getEntryCount(); j++) {

				EtherCatPdoEntry& entry = module.entries[j];
				uint8_t* outBuffer = buffer + entry.byteOffset;

				if (entry.bitCount % 8 == 0 && entry.bitOffset == 0) {
					//if the data size is only full bytes and aligned to a byte start
					//cast the user data pointer to the correct size and copy it to the output buffer pointer
					if (entry.byteCount == 1) *((uint8_t*)outBuffer) = *((uint8_t*)entry.dataPointer);
					else if (entry.byteCount == 2) {
						
						uint16_t dataValue = *(uint16_t*)entry.dataPointer;
						uint16_t& outValue = *(uint16_t*)outBuffer;
						outValue = dataValue;
						
						//*((uint16_t*)outBuffer) = *((uint16_t*)entry.dataPointer);
					}
					else if (entry.byteCount <= 4) *((uint32_t*)outBuffer) = *((uint32_t*)entry.dataPointer);
					else if (entry.byteCount <= 8) *((uint64_t*)outBuffer) = *((uint64_t*)entry.dataPointer);
				}
				else if(entry.bitCount == 1){
					bool value = *(bool*)entry.dataPointer;
					uint8_t& outputData = *outBuffer;
					if(value) outputData |= 0x1 << entry.bitOffset;
					else outputData &= ~(0x1 << entry.bitOffset);
					//if(value) *outBuffer |= 0x1 << entry.bitOffset;
					//else *outBuffer &= ~(0x1 << entry.bitOffset);
				}else{
					//else if the data is not full bytes and or is not aligned to a byte start
					//copy the data to the output buffer bit by bit
					
					//TODO: this does not work, and is probably not necessary either
					/*
					unsigned long long data;
					if (entry.byteCount <= 1) data = *((uint8_t*)entry.dataPointer);
					else if(entry.byteCount == 2) data = *((uint16_t*)entry.dataPointer);
					else if(entry.byteCount <= 4) data = *((uint32_t*)entry.dataPointer);
					else data = *((uint64_t*)entry.dataPointer);

					for (int b = 0; b < entry.bitCount; b++) {
						bool bit = data & 0x1 << b;
						int outputByteOffset = entry.byteOffset + b;
						if(bit) *((unsigned long long*)outBuffer) |= 0x1 << outputByteOffset;
						else *((unsigned long long*)outBuffer) &= ~(0x1 << outputByteOffset);
					}
					*/
				}
			}
		}
	}

};


