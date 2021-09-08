#include <pch.h>

#include "ioData.h"
#include "ioLink.h"

const char* ioData::dataTypeNames[TYPE_COUNT] = {
	"Boolean",
	"Integer",
	"Real"
};

//setting data (with data conversions)
void ioData::set(bool boolean) {
	switch (type) {
	case BOOLEAN_VALUE: booleanValue = boolean; break;
	case INTEGER_VALUE: integerValue = boolean; break;
	case REAL_VALUE: realValue = boolean; break;
	}
	b_hasNewValue = true;
}

void ioData::set(long long int integer) {
	switch (type) {
	case INTEGER_VALUE: integerValue = integer; break;
	case BOOLEAN_VALUE: booleanValue = integer > 0; break;
	case REAL_VALUE: realValue = integer; break;
	}
	b_hasNewValue = true;
}

void ioData::set(double real) {
	switch (type) {
	case REAL_VALUE: realValue = real; break;
	case BOOLEAN_VALUE: booleanValue = real > 0.0; break;
	case INTEGER_VALUE: integerValue = real; break;
	}
	b_hasNewValue = true;
}

//reading data (with data conversions)
bool ioData::getBoolean() {
	switch (type) {
	case BOOLEAN_VALUE: return booleanValue;
	case INTEGER_VALUE: return integerValue > 0;
	case REAL_VALUE: return realValue > 0;
	}
}
long long int ioData::getInteger() {
	switch (type) {
	case INTEGER_VALUE: return integerValue;
	case BOOLEAN_VALUE: return (long long int)booleanValue;
	case REAL_VALUE: return (long long int)realValue;
	}
}
double ioData::getReal() {
	switch (type) {
	case REAL_VALUE: return realValue;
	case BOOLEAN_VALUE: return (double)booleanValue;
	case INTEGER_VALUE: return (double)integerValue;
	}
}

const char* ioData::getValueString() {
	static char output[32];
	switch (type) {
	case BOOLEAN_VALUE: strcpy(output, booleanValue ? "True" : "False"); break;
	case INTEGER_VALUE: sprintf(output, "%i", integerValue); break;
	case REAL_VALUE: sprintf(output, "%.5f", realValue); break;
	}
	return (const char*)output;
}

void ioData::copyToLinked() {
	if (direction == DataDirection::NODE_OUTPUT) {
		for (auto link : ioLinks) {
			std::shared_ptr<ioData> other = link->getOutputData();
			switch (type) {
			case BOOLEAN_VALUE: other->set(getBoolean()); break;
			case INTEGER_VALUE: other->set(getInteger()); break;
			case REAL_VALUE: other->set(getReal()); break;
			}
		}
	}
}

void ioData::copyFromLinked() {
	if (direction == DataDirection::NODE_INPUT) {
		for (auto link : ioLinks) {
			std::shared_ptr<ioData> other = link->getInputData();
			switch (type) {
			case BOOLEAN_VALUE: set(other->getBoolean()); break;
			case INTEGER_VALUE: set(other->getInteger()); break;
			case REAL_VALUE: set(other->getReal()); break;
			}
		}
	}
}

std::vector<std::shared_ptr<ioNode>> ioData::getNodesLinkedAtOutputs() {
	std::vector<std::shared_ptr<ioNode>> linkedNodes;
	for (auto link : ioLinks) {
		linkedNodes.push_back(link->getOutputData()->getNode());
	}
	return linkedNodes;
}

std::vector<std::shared_ptr<ioNode>> ioData::getNodesLinkedAtInputs() {
	std::vector<std::shared_ptr<ioNode>> linkedNodes;
	for (auto link : ioLinks) {
		linkedNodes.push_back(link->getInputData()->getNode());
	}
	return linkedNodes;
}