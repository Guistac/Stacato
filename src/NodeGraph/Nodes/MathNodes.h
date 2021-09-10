#pragma once

#include "NodeGraph/ioNode.h"

class ConstantNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Constant", ConstantNode, "Math")

	virtual void assignIoData() {
		addIoData(value);
	}

	std::shared_ptr<ioData> value = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "value", ioDataFlags_ForceDataField);

	virtual void process(bool inputDataValid) {}
};

class AdditionNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Addition", AdditionNode, "Math")
	
	virtual void assignIoData() {
		addIoData(input);
		input->set(0.0);
		addIoData(offset);
		offset->set(0.0);
		addIoData(output);
		output->set(0.0);
	}

	std::shared_ptr<ioData> input = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "input", ioDataFlags_AcceptMultipleInputs);
	std::shared_ptr<ioData> offset = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "offset", ioDataFlags_DisablePin | ioDataFlags_ForceDataField);
	std::shared_ptr<ioData> output = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "result", ioDataFlags_DisableDataField);

	virtual void process(bool inputDataValid) {
		double sum = 0.0;
		for (auto link : input->getLinks()) sum += link->getInputData()->getReal();
		if(!input->isConnected()) sum += input->getReal();
		sum += offset->getReal();
		output->set(sum);
	}
};

class SubtractionNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Subtraction", SubtractionNode, "Math")

	virtual void assignIoData() {
		addIoData(base);
		base->set(0.0);
		addIoData(sub);
		sub->set(0.0);
		addIoData(offset);
		offset->set(0.0);
		addIoData(output);
		output->set(0.0);
	}

	std::shared_ptr<ioData> base = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "base", ioDataFlags_AcceptMultipleInputs);
	std::shared_ptr<ioData> sub = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "sub", ioDataFlags_AcceptMultipleInputs);
	std::shared_ptr<ioData> offset = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "offset", ioDataFlags_DisablePin | ioDataFlags_ForceDataField);
	std::shared_ptr<ioData> output = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "result", ioDataFlags_DisableDataField);

	virtual void process(bool inputDataValid) {
		double sum = 0.0;
		for (auto link : base->getLinks()) sum += link->getInputData()->getReal();
		if (!base->isConnected()) sum += base->getReal();
		for (auto link : sub->getLinks()) sum -= link->getInputData()->getReal();
		if (!base->isConnected()) sum -= sub->getReal();
		sum += offset->getReal();
		output->set(sum);
	}
};

class MultiplicationNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Multiplication", MultiplicationNode,"Math")

	virtual void assignIoData() {
		addIoData(input);
		input->set(0.0);
		addIoData(multiplier);
		multiplier->set(1.0);
		addIoData(output);
		output->set(0.0);
	}

	std::shared_ptr<ioData> input = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "input", ioDataFlags_AcceptMultipleInputs);
	std::shared_ptr<ioData> multiplier = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "mult", ioDataFlags_DisablePin | ioDataFlags_ForceDataField);
	std::shared_ptr<ioData> output = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "result", ioDataFlags_DisableDataField);

	virtual void process(bool inputDataValid) {
		double out = 1.0;
		for (auto link : input->getLinks()) out *= link->getInputData()->getReal();
		if (!multiplier->isConnected()) out *= multiplier->getReal();
		output->set(out);
	}
};

class DivisionNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Division", DivisionNode, "Math")

	virtual void assignIoData() {
		addIoData(base);
		base->set(0.0);
		addIoData(div);
		div->set(1.0);
		addIoData(mult);
		mult->set(1.0);
		addIoData(output);
		output->set(0.0);
	}

	std::shared_ptr<ioData> base = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "base", ioDataFlags_AcceptMultipleInputs);
	std::shared_ptr<ioData> div = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "div", ioDataFlags_AcceptMultipleInputs);
	std::shared_ptr<ioData> mult = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "mult", ioDataFlags_DisablePin | ioDataFlags_ForceDataField);
	std::shared_ptr<ioData> output = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "result", ioDataFlags_DisableDataField);

	virtual void process(bool inputDataValid) {
		double out = 1.0;
		for (auto link : base->getLinks()) out *= link->getInputData()->getReal();
		if (!base->isConnected()) out *= base->getReal();
		for (auto link : div->getLinks()) out /= link->getInputData()->getReal();
		if (!div->isConnected()) out /= div->getReal();
		out *= mult->getReal();
		output->set(out);
	}
};

class ExponentNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Exponent", ExponentNode, "Math")

	virtual void assignIoData() {
		addIoData(base);
		base->set(0.0);
		addIoData(exp);
		exp->set(1.0);
		addIoData(output);
		output->set(0.0);
	}

	std::shared_ptr<ioData> base = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "base");
	std::shared_ptr<ioData> exp = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "exp");
	std::shared_ptr<ioData> output = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "result", ioDataFlags_DisableDataField);

	virtual void process(bool inputDataValid) {
		double d_base, d_exp;
		if (base->isConnected()) d_base = base->getLinks().front()->getInputData()->getReal();
		else d_base = base->getReal();
		if (exp->isConnected()) d_exp = exp->getLinks().front()->getInputData()->getReal();
		else d_exp = exp->getReal();
		output->set(std::pow<double>(d_base, d_exp));
	}
};


class AbsoluteNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("AbsoluteNode", AbsoluteNode, "Math")

	virtual void assignIoData() {
		addIoData(input);
		input->set(0.0);
		addIoData(output);
		output->set(0.0);
	}

	std::shared_ptr<ioData> input = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<ioData> output = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "out", ioDataFlags_DisableDataField);

	virtual void process(bool inputDataValid) {
		if (input->isConnected()) output->set(std::abs(input->getLinks().front()->getInputData()->getReal()));
		else output->set(std::abs(input->getReal()));
	}
};






class SinusNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Sinus", SinusNode, "Math")

	virtual void assignIoData() {
		addIoData(in);
		in->set(0.0);
		addIoData(out);
		out->set(0.0);
	}

	std::shared_ptr<ioData> in = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<ioData> out = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "out", ioDataFlags_DisableDataField);

	virtual void process(bool inputDataValid) {
		if (in->isConnected()) out->set(std::sin(in->getLinks().front()->getInputData()->getReal()));
		else out->set(std::sin(in->getReal()));
	}
};

class CosinusNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Cosinus", CosinusNode, "Math")

	virtual void assignIoData() {
		addIoData(in);
		in->set(1.0);
		addIoData(out);
		out->set(0.0);
	}

	std::shared_ptr<ioData> in = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<ioData> out = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "out", ioDataFlags_DisableDataField);

	virtual void process(bool inputDataValid) {
		if (in->isConnected()) out->set(std::cos(in->getLinks().front()->getInputData()->getReal()));
		else out->set(std::cos(in->getReal()));
	}
};

class TangentNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Tangent", TangentNode, "Math")

	virtual void assignIoData() {
		addIoData(in);
		in->set(0.0);
		addIoData(out);
		out->set(0.0);
	}

	std::shared_ptr<ioData> in = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<ioData> out = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "out", ioDataFlags_DisableDataField);

	virtual void process(bool inputDataValid) {
		if (in->isConnected()) out->set(std::tan(in->getLinks().front()->getInputData()->getReal()));
		else out->set(std::tan(in->getReal()));
	}
};

class CotangentNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Cotangent", CotangentNode, "Math")

	virtual void assignIoData() {
		addIoData(in);
		in->set(3.1415926);
		addIoData(out);
		out->set(0.0);
	}

	std::shared_ptr<ioData> in = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<ioData> out = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "out", ioDataFlags_DisableDataField);

	virtual void process(bool inputDataValid) {
		if (in->isConnected()) out->set(1.0 / std::tan(in->getLinks().front()->getInputData()->getReal()));
		else out->set(1.0 / std::tan(in->getReal()));
	}
};












class BoolNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Bool", BoolNode, "Logic")

	virtual void assignIoData() {
		addIoData(in);
		in->set(false);
		addIoData(out);
		out->set(false);
	}

	std::shared_ptr<ioData> in = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<ioData> out = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "out", ioDataFlags_DisableDataField);

	virtual void process(bool inputDataValid) {
		if (in->isConnected()) out->set(in->getLinks().front()->getInputData()->getBoolean());
		else out->set(in->getBoolean());
	}
};

class NotNode : public ioNode{
public:

	DEFINE_PROCESSOR_NODE("Not", NotNode, "Logic")

	virtual void assignIoData() {
		addIoData(in);
		in->set(true);
		addIoData(out);
		out->set(false);
	}

	std::shared_ptr<ioData> in = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<ioData> out = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "out", ioDataFlags_DisableDataField);

	virtual void process(bool inputDataValid) {
		if (in->isConnected()) out->set(!in->getLinks().front()->getInputData()->getBoolean());
		else out->set(!in->getBoolean());
	}
};

class AndNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("And", AndNode, "Logic")

	virtual void assignIoData() {
		addIoData(in);
		in->set(false);
		addIoData(out);
		out->set(false);
	}

	std::shared_ptr<ioData> in = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "in", ioDataFlags_AcceptMultipleInputs);
	std::shared_ptr<ioData> out = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "out", ioDataFlags_DisableDataField);

	virtual void process(bool inputDataValid) {
		if (in->isConnected()) {
			for (auto link : in->getLinks()) {
				if (!link->getInputData()->getBoolean()) {
					out->set(false);
					return;
				}
			}
			out->set(true);
		}
		else out->set(false);
	}
};

class OrNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Or", OrNode, "Logic")

	virtual void assignIoData() {
		addIoData(in);
		in->set(false);
		addIoData(out);
		out->set(false);
	}

	std::shared_ptr<ioData> in = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "in", ioDataFlags_AcceptMultipleInputs);
	std::shared_ptr<ioData> out = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "out", ioDataFlags_DisableDataField);

	virtual void process(bool inputDataValid) {
		if (in->isConnected()) {
			for (auto link : in->getLinks()) {
				if (link->getInputData()->getBoolean()) {
					out->set(true);
					return;
				}
			}
			out->set(false);
		}
		else out->set(false);
	}
};

