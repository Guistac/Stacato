#pragma once

class NodePin;
class NodeGraph;

class NodeLink : public std::enable_shared_from_this<NodeLink>{
public:

	int getUniqueID() { return uniqueID; }

	std::shared_ptr<NodePin> getOutputData() { return outputData; }
	std::shared_ptr<NodePin> getInputData() { return inputData; }

	std::shared_ptr<NodePin> outputData = nullptr;
	std::shared_ptr<NodePin> inputData = nullptr;

	int uniqueID = -1;
	std::shared_ptr<NodeGraph> nodeGraph = nullptr;
	
	void disconnect();
};

