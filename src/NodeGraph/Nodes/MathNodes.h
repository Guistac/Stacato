#pragma once

#include "NodeGraph/ioNode.h"

class AdditionNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Addition", AdditionNode, "Math")
	
	virtual void assignIoData() {
		addIoData(input);
		addIoData(output);
	}

	std::shared_ptr<ioData> input = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "input", true);
	std::shared_ptr<ioData> output = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "result");

	virtual void process(bool inputDataValid) {
		double sum = 0.0;
		for (auto link : input->getLinks()) sum += link->getInputData()->getReal();
		output->set(sum);
	}
};

class SubtractionNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Subtraction", SubtractionNode, "Math")

		virtual void assignIoData() {
		addIoData(base);
		addIoData(sub);
		addIoData(output);
	}

	std::shared_ptr<ioData> base = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "base", true);
	std::shared_ptr<ioData> sub = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "sub", true);
	std::shared_ptr<ioData> output = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "result");

	virtual void process(bool inputDataValid) {
		double sum = 0.0;
		for (auto link : base->getLinks()) sum += link->getInputData()->getReal();
		for (auto link : sub->getLinks()) sum -= link->getInputData()->getReal();
		output->set(sum);
	}
};

class MultiplicationNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Multiplication", MultiplicationNode,"Math")

		virtual void assignIoData() {
		addIoData(input);
		addIoData(output);
	}

	std::shared_ptr<ioData> input = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "input", true);
	std::shared_ptr<ioData> output = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "result");

	virtual void process(bool inputDataValid) {
		double out = 1.0;
		for (auto link : input->getLinks()) out *= link->getInputData()->getReal();
		output->set(out);
	}
};

class DivisionNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Division", DivisionNode, "Math")

		virtual void assignIoData() {
		addIoData(base);
		addIoData(div);
		addIoData(output);
	}

	std::shared_ptr<ioData> base = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "base", true);
	std::shared_ptr<ioData> div = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "div 0", true);
	std::shared_ptr<ioData> output = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "result");

	virtual void process(bool inputDataValid) {
		double out = 1.0;
		for (auto link : base->getLinks()) out *= link->getInputData()->getReal();
		for (auto link : div->getLinks()) out /= link->getInputData()->getReal();
		output->set(out);
	}
};


class SinusNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Sinus", SinusNode, "Math")

	virtual void assignIoData() {
		addIoData(in);
		addIoData(out);
	}

	std::shared_ptr<ioData> in = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<ioData> out = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "out");

	virtual void process(bool inputDataValid) {
		double output = 0.0;
		for (auto link : in->getLinks()) output = std::sin(link->getInputData()->getReal());
		out->set(output);
	}
};

class CosinusNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Cosinus", CosinusNode, "Math")

	virtual void assignIoData() {
		addIoData(in);
		addIoData(out);
	}

	std::shared_ptr<ioData> in = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<ioData> out = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "out");

	virtual void process(bool inputDataValid) {
		double output = 0.0;
		for (auto link : in->getLinks()) output = std::cos(link->getInputData()->getReal());
		out->set(output);
	}
};

class NotNode : public ioNode{
public:

	DEFINE_PROCESSOR_NODE("Not", NotNode, "Logic")

	virtual void assignIoData() {
		addIoData(in);
		addIoData(out);
	}

	std::shared_ptr<ioData> in = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<ioData> out = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "out");

	virtual void process(bool inputDataValid) {
		bool output = false;
		if (in->isConnected()) {
			in->set(in->getLinks().front()->getInputData()->getBoolean());
			output = !in->getBoolean();
		}
		out->set(output);
	}
};

class AndNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("And", AndNode, "Logic")

	virtual void assignIoData() {
		addIoData(in);
		addIoData(out);
	}

	std::shared_ptr<ioData> in = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "in", true);
	std::shared_ptr<ioData> out = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "out");

	virtual void process(bool inputDataValid) {
		bool output = true;
		for (auto link : in->getLinks()) {
			if (!link->getInputData()->getBoolean()) {
				output = false;
				break;
			}
		}
		out->set(output);
	}
};

class OrNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Or", OrNode, "Logic")

	virtual void assignIoData() {
		addIoData(in);
		addIoData(out);
	}

	std::shared_ptr<ioData> in = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "in", true);
	std::shared_ptr<ioData> out = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "out");

	virtual void process(bool inputDataValid) {
		bool output = false;
		for (auto link : in->getLinks()) {
			if (link->getInputData()->getBoolean()) {
				output = true;
				break;
			}
		}
		out->set(output);
	}
};

