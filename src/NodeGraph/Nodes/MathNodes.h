#pragma once

#include "NodeGraph/Node.h"

class ConstantNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(ConstantNode, "Constant", "Constant", "Math")

	virtual void assignIoData() {
		addIoData(value);
	}

	std::shared_ptr<NodePin> value = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "value", NodePinFlags_ForceDataField);

	virtual void process() {}
};

class AdditionNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(AdditionNode, "Addition", "Addition", "Math")
	
	virtual void assignIoData() {
		addIoData(input);
		input->set(0.0);
		addIoData(offset);
		offset->set(0.0);
		addIoData(output);
		output->set(0.0);
	}

	std::shared_ptr<NodePin> input = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "input", NodePinFlags_AcceptMultipleInputs);
	std::shared_ptr<NodePin> offset = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "offset", NodePinFlags_DisablePin | NodePinFlags_ForceDataField);
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "result", NodePinFlags_DisableDataField);

	virtual void process() {
		double sum = 0.0;
		for (auto link : input->getLinks()) sum += link->getInputData()->getReal();
		if(!input->isConnected()) sum += input->getReal();
		sum += offset->getReal();
		output->set(sum);
	}
};

class SubtractionNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(SubtractionNode, "Subtraction", "Subtraction", "Math")

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

	std::shared_ptr<NodePin> base = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "base", NodePinFlags_AcceptMultipleInputs);
	std::shared_ptr<NodePin> sub = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "sub", NodePinFlags_AcceptMultipleInputs);
	std::shared_ptr<NodePin> offset = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "offset", NodePinFlags_DisablePin | NodePinFlags_ForceDataField);
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "result", NodePinFlags_DisableDataField);

	virtual void process() {
		double sum = 0.0;
		for (auto link : base->getLinks()) sum += link->getInputData()->getReal();
		if (!base->isConnected()) sum += base->getReal();
		for (auto link : sub->getLinks()) sum -= link->getInputData()->getReal();
		if (!base->isConnected()) sum -= sub->getReal();
		sum += offset->getReal();
		output->set(sum);
	}
};

class MultiplicationNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(MultiplicationNode, "Multiplication", "Mutliplication", "Math")

	virtual void assignIoData() {
		addIoData(input);
		input->set(0.0);
		addIoData(multiplier);
		multiplier->set(1.0);
		addIoData(output);
		output->set(0.0);
	}

	std::shared_ptr<NodePin> input = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "input", NodePinFlags_AcceptMultipleInputs);
	std::shared_ptr<NodePin> multiplier = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "mult", NodePinFlags_DisablePin | NodePinFlags_ForceDataField);
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "result", NodePinFlags_DisableDataField);

	virtual void process() {
		double out = 1.0;
		for (auto link : input->getLinks()) out *= link->getInputData()->getReal();
		if (!multiplier->isConnected()) out *= multiplier->getReal();
		output->set(out);
	}
};

class DivisionNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(DivisionNode, "Division", "Division", "Math")

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

	std::shared_ptr<NodePin> base = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "base", NodePinFlags_AcceptMultipleInputs);
	std::shared_ptr<NodePin> div = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "div", NodePinFlags_AcceptMultipleInputs);
	std::shared_ptr<NodePin> mult = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "mult", NodePinFlags_DisablePin | NodePinFlags_ForceDataField);
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "result", NodePinFlags_DisableDataField);

	virtual void process() {
		double out = 1.0;
		for (auto link : base->getLinks()) out *= link->getInputData()->getReal();
		if (!base->isConnected()) out *= base->getReal();
		for (auto link : div->getLinks()) out /= link->getInputData()->getReal();
		if (!div->isConnected()) out /= div->getReal();
		out *= mult->getReal();
		output->set(out);
	}
};

class ExponentNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(ExponentNode, "Exponent", "Exponent", "Math")

	virtual void assignIoData() {
		addIoData(base);
		base->set(0.0);
		addIoData(exp);
		exp->set(1.0);
		addIoData(output);
		output->set(0.0);
	}

	std::shared_ptr<NodePin> base = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "base");
	std::shared_ptr<NodePin> exp = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "exp");
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "result", NodePinFlags_DisableDataField);

	virtual void process() {
		double d_base, d_exp;
		if (base->isConnected()) d_base = base->getLinks().front()->getInputData()->getReal();
		else d_base = base->getReal();
		if (exp->isConnected()) d_exp = exp->getLinks().front()->getInputData()->getReal();
		else d_exp = exp->getReal();
		//TODO: somehow this is not working on mac
		//output->set(std::pow<double>(d_base, d_exp));
	}
};


class AbsoluteNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(AbsoluteNode, "Absolute", "Absolute", "Math")

	virtual void assignIoData() {
		addIoData(input);
		input->set(0.0);
		addIoData(output);
		output->set(0.0);
	}

	std::shared_ptr<NodePin> input = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "out", NodePinFlags_DisableDataField);

	virtual void process() {
		if (input->isConnected()) output->set(std::abs(input->getLinks().front()->getInputData()->getReal()));
		else output->set(std::abs(input->getReal()));
	}
};






class SinusNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(SinusNode, "Sinus", "Sinus", "Trigonometry")

	virtual void assignIoData() {
		addIoData(in);
		in->set(0.0);
		addIoData(out);
		out->set(0.0);
	}

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "out", NodePinFlags_DisableDataField);

	virtual void process() {
		if (in->isConnected()) out->set(std::sin(in->getLinks().front()->getInputData()->getReal()));
		else out->set(std::sin(in->getReal()));
	}
};

class CosinusNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(CosinusNode, "Cosinus", "Cosinus", "Trigonometry")

	virtual void assignIoData() {
		addIoData(in);
		in->set(1.0);
		addIoData(out);
		out->set(0.0);
	}

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "out", NodePinFlags_DisableDataField);

	virtual void process() {
		if (in->isConnected()) out->set(std::cos(in->getLinks().front()->getInputData()->getReal()));
		else out->set(std::cos(in->getReal()));
	}
};

class TangentNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(TangentNode, "Tangent", "Tangent", "Trigonometry")

	virtual void assignIoData() {
		addIoData(in);
		in->set(0.0);
		addIoData(out);
		out->set(0.0);
	}

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "out", NodePinFlags_DisableDataField);

	virtual void process() {
		if (in->isConnected()) out->set(std::tan(in->getLinks().front()->getInputData()->getReal()));
		else out->set(std::tan(in->getReal()));
	}
};

class CotangentNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(CotangentNode, "Cotangent", "Cotangent", "Trigonometry")

	virtual void assignIoData() {
		addIoData(in);
		in->set(3.1415926);
		addIoData(out);
		out->set(0.0);
	}

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "out", NodePinFlags_DisableDataField);

	virtual void process() {
		if (in->isConnected()) out->set(1.0 / std::tan(in->getLinks().front()->getInputData()->getReal()));
		else out->set(1.0 / std::tan(in->getReal()));
	}
};












class BoolNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(BoolNode, "Bool", "Bool", "Logic")

	virtual void assignIoData() {
		addIoData(in);
		in->set(false);
		addIoData(out);
		out->set(false);
	}

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "out", NodePinFlags_DisableDataField);

	virtual void process() {
		if (in->isConnected()) out->set(in->getLinks().front()->getInputData()->getBoolean());
		else out->set(in->getBoolean());
	}
};

class NotNode : public Node{
public:

	DEFINE_PROCESSOR_NODE(NotNode, "Not", "Not", "Logic")

	virtual void assignIoData() {
		addIoData(in);
		in->set(true);
		addIoData(out);
		out->set(false);
	}

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "out", NodePinFlags_DisableDataField);

	virtual void process() {
		if (in->isConnected()) out->set(!in->getLinks().front()->getInputData()->getBoolean());
		else out->set(!in->getBoolean());
	}
};

class AndNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(AndNode, "And", "And", "Logic")

	virtual void assignIoData() {
		addIoData(in);
		in->set(false);
		addIoData(out);
		out->set(false);
	}

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "in", NodePinFlags_AcceptMultipleInputs);
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "out", NodePinFlags_DisableDataField);

	virtual void process() {
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

class OrNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(OrNode, "Or", "Or", "Logic")

	virtual void assignIoData() {
		addIoData(in);
		in->set(false);
		addIoData(out);
		out->set(false);
	}

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "in", NodePinFlags_AcceptMultipleInputs);
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "out", NodePinFlags_DisableDataField);

	virtual void process() {
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

