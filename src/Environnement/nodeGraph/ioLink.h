#pragma once

class ioData;

class ioLink{
public:

	int getUniqueID() { return uniqueID; }

	std::shared_ptr<ioData> getOutputPin() { return outputData; }
	std::shared_ptr<ioData> getInputPin() { return inputData; }

private:

	friend class NodeGraph;
	friend class ioNode;
	friend class ioData;

	std::shared_ptr<ioData> outputData = nullptr;
	std::shared_ptr<ioData> inputData = nullptr;

	int uniqueID = -1;
};

