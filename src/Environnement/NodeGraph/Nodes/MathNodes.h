#pragma once

#include "Environnement/NodeGraph/ioNode.h"

class AdditionNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Addition", AdditionNode)
	
	virtual void assignIoData() {
		addIoData(input);
		addIoData(output);
	}

	std::shared_ptr<ioData> input = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "input", true);
	std::shared_ptr<ioData> output = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "result");

	virtual void process(bool inputDataValid) {
		double sum = 0.0;
		if (input->isConnected()) {
			if (input->hasMultipleLinks()) {
				for (auto link : input->getLinks()) {
					sum += link->getInputData()->getReal();
				}
			}
			else {
				sum += input->getReal();
			}
		}
		output->set(sum);
	}
};

class SubtractionNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Subtraction", SubtractionNode)

		virtual void assignIoData() {
		addIoData(base);
		addIoData(sub0);
		addIoData(sub1);
		addIoData(sub2);
		addIoData(output);
	}

	std::shared_ptr<ioData> base = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "base");
	std::shared_ptr<ioData> sub0 = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "sub 0");
	std::shared_ptr<ioData> sub1 = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "sub 1");
	std::shared_ptr<ioData> sub2 = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "sub 2");
	std::shared_ptr<ioData> output = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "result");

	virtual void process(bool inputDataValid) {
		double sum = 0.0;
		if (base->isConnected()) sum += base->getReal();
		if (sub0->isConnected()) sum -= sub0->getReal();
		if (sub1->isConnected()) sum -= sub1->getReal();
		if (sub2->isConnected()) sum -= sub2->getReal();
		output->set(sum);
	}
};

class MultiplicationNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Multiplication", MultiplicationNode)

		virtual void assignIoData() {
		addIoData(term0);
		addIoData(term1);
		addIoData(term2);
		addIoData(term3);
		addIoData(output);
	}

	std::shared_ptr<ioData> term0 = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "term 0");
	std::shared_ptr<ioData> term1 = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "term 1");
	std::shared_ptr<ioData> term2 = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "term 2");
	std::shared_ptr<ioData> term3 = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "term 3");
	std::shared_ptr<ioData> output = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "result");

	virtual void process(bool inputDataValid) {
		double multoutput = 1.0;
		if (term0->isConnected()) multoutput *= term0->getReal();
		if (term1->isConnected()) multoutput *= term1->getReal();
		if (term2->isConnected()) multoutput *= term2->getReal();
		if (term3->isConnected()) multoutput *= term3->getReal();
		output->set(multoutput);
	}
};

class DivisionNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Division", DivisionNode)

		virtual void assignIoData() {
		addIoData(base);
		addIoData(div0);
		addIoData(div1);
		addIoData(div2);
		addIoData(output);
	}

	std::shared_ptr<ioData> base = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "base");
	std::shared_ptr<ioData> div0 = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "div 0");
	std::shared_ptr<ioData> div1 = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "div 1");
	std::shared_ptr<ioData> div2 = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "div 2");
	std::shared_ptr<ioData> output = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "result");

	virtual void process(bool inputDataValid) {
		double divOutput = 1.0;
		if (base->isConnected()) divOutput *= base->getReal();
		if (div0->isConnected()) divOutput /= div0->getReal();
		if (div1->isConnected()) divOutput /= div1->getReal();
		if (div2->isConnected()) divOutput /= div2->getReal();
		output->set(divOutput);
	}
};


class SinusNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Sinus", SinusNode)

	virtual void assignIoData() {
		addIoData(in);
		addIoData(out);
	}

	std::shared_ptr<ioData> in = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<ioData> out = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "out");

	virtual void process(bool inputDataValid) {
		out->set(std::sin(in->getReal()));
	}
};

class CosinusNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Cosinus", CosinusNode)

	virtual void assignIoData() {
		addIoData(in);
		addIoData(out);
	}

	std::shared_ptr<ioData> in = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<ioData> out = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "out");

	virtual void process(bool inputDataValid) {
		out->set(std::cos(in->getReal()));
	}
};