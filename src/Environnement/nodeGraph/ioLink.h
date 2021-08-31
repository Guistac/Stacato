#pragma once

class ioData;

class ioLink{
public:

	int getUniqueID() { return uniqueID; }

	ioData* getOutputPin() { return outputData; }
	ioData* getInputPin() { return inputData; }

private:

	friend class NodeGraph;
	friend class ioNode;
	friend class ioData;

	ioData* outputData = nullptr;
	ioData* inputData = nullptr;

	int uniqueID = -1;
};

