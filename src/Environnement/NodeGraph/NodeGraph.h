#pragma once

#include "Environnement/DeviceNode.h"
#include "NodePin.h"
#include "NodeLink.h"

namespace tinyxml2 { class XMLElement; }

namespace Environnement::NodeGraph{

	void addNode(std::shared_ptr<Node>);
	void removeNode(std::shared_ptr<Node>);

	bool isConnectionValid(std::shared_ptr<NodePin>, std::shared_ptr<NodePin>);
	std::shared_ptr<NodeLink> connect(std::shared_ptr<NodePin>, std::shared_ptr<NodePin>);
	void disconnect(std::shared_ptr<NodeLink>);

	std::shared_ptr<Node> getNode(int);
	std::shared_ptr<NodePin> getPin(int);
	std::shared_ptr<NodeLink> getLink(int);

	std::vector<std::shared_ptr<Node>>& getNodes();
	std::vector<std::shared_ptr<NodePin>>& getPins();
	std::vector<std::shared_ptr<NodeLink>>& getLinks();
	std::vector<std::shared_ptr<Node>>& getSelectedNodes();
	std::vector<std::shared_ptr<NodeLink>>& getSelectedLinks();


	enum class ProcessDirection{
		INPUT_PROCESS,
		OUTPUT_PROCESS
	};

	struct ProcessProgram{
		struct Instruction{
			std::shared_ptr<Node> processedNode;
			ProcessDirection processType;
			static std::shared_ptr<Instruction> make(std::shared_ptr<Node> node, ProcessDirection direction){
				auto instruction = std::make_shared<Instruction>();
				instruction->processedNode = node;
				instruction->processType = direction;
				return instruction;
			}
		};
		std::vector<std::shared_ptr<Instruction>> inputProcessInstructions;
		std::vector<std::shared_ptr<Instruction>> outputProcessInstructions;
		void log();
	};

	std::shared_ptr<ProcessProgram> compileProcessProgram(std::vector<std::shared_ptr<Node>>& startNodes);
	void executeInputProcess(std::shared_ptr<ProcessProgram> processProgram);
	void executeOutputProcess(std::shared_ptr<ProcessProgram> processProgram);

	bool load(tinyxml2::XMLElement* xml);
	bool save(tinyxml2::XMLElement* xml);

	void editorGui();
	void centerView();
	void showFlow();

	int getNewUniqueID();
	void startCountingUniqueIDsFrom(int largestUniqueID);
	
	bool& getShowOutputValues();
	bool& getWasJustLoaded();

	void reset();
	
}

