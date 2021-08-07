#pragma once

#include "ethercat.h"

//==========READING

bool readSDO(uint16_t slave, uint16_t index, uint8_t subindex, uint8_t& data, int attempts) {
	int size = 1;
	for (int i = 0; i < attempts; i++) if (ec_SDOread(slave, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}
bool readSDO(uint16_t slave, uint16_t index, uint8_t subindex, int8_t& data, int attempts) {
	int size = 1;
	for (int i = 0; i < attempts; i++) if (ec_SDOread(slave, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}
bool readSDO(uint16_t slave, uint16_t index, uint8_t subindex, uint16_t& data, int attempts) {
	int size = 2;
	for (int i = 0; i < attempts; i++) if (ec_SDOread(slave, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}
bool readSDO(uint16_t slave, uint16_t index, uint8_t subindex, int16_t& data, int attempts) {
	int size = 2;
	for (int i = 0; i < attempts; i++) if (ec_SDOread(slave, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}
bool readSDO(uint16_t slave, uint16_t index, uint8_t subindex, uint32_t& data, int attempts) {
	int size = 4;
	for (int i = 0; i < attempts; i++) if (ec_SDOread(slave, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}
bool readSDO(uint16_t slave, uint16_t index, uint8_t subindex, int32_t& data, int attempts) {
	int size = 4;
	for (int i = 0; i < attempts; i++) if (ec_SDOread(slave, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}
bool readSDO(uint16_t slave, uint16_t index, uint8_t subindex, uint64_t& data, int attempts) {
	int size = 8;
	for (int i = 0; i < attempts; i++) if (ec_SDOread(slave, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}
bool readSDO(uint16_t slave, uint16_t index, uint8_t subindex, int64_t& data, int attempts) {
	int size = 8;
	for (int i = 0; i < attempts; i++) if (ec_SDOread(slave, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}

//==========WRITING

bool writeSDO(uint16_t slave, uint16_t index, uint8_t subindex, uint8_t& data, int attempts) {
	for(int i = 0; i < attempts; i++) if (ec_SDOwrite(slave, index, subindex, false, 1, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}
bool writeSDO(uint16_t slave, uint16_t index, uint8_t subindex, int8_t data, int attempts) {
	for (int i = 0; i < attempts; i++) if (ec_SDOwrite(slave, index, subindex, false, 1, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}
bool writeSDO(uint16_t slave, uint16_t index, uint8_t subindex, uint16_t data, int attempts) {
	for (int i = 0; i < attempts; i++) if (ec_SDOwrite(slave, index, subindex, false, 2, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}
bool writeSDO(uint16_t slave, uint16_t index, uint8_t subindex, int16_t data, int attempts) {
	for (int i = 0; i < attempts; i++) if (ec_SDOwrite(slave, index, subindex, false, 2, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}
bool writeSDO(uint16_t slave, uint16_t index, uint8_t subindex, uint32_t data, int attempts) {
	for (int i = 0; i < attempts; i++) if (ec_SDOwrite(slave, index, subindex, false, 4, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}
bool writeSDO(uint16_t slave, uint16_t index, uint8_t subindex, int32_t data, int attempts) {
	for (int i = 0; i < attempts; i++) if (ec_SDOwrite(slave, index, subindex, false, 4, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}
bool writeSDO(uint16_t slave, uint16_t index, uint8_t subindex, uint64_t data, int attempts) {
	for (int i = 0; i < attempts; i++) if (ec_SDOwrite(slave, index, subindex, false, 8, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}
bool writeSDO(uint16_t slave, uint16_t index, uint8_t subindex, int64_t data, int attempts) {
	for (int i = 0; i < attempts; i++) if (ec_SDOwrite(slave, index, subindex, false, 8, &data, EC_TIMEOUTSAFE) == 1) return true;
	return false;
}

