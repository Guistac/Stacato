#pragma once

#include "NodeGraph/Node.h"

class ConstantNode : public Node {
public:

	DEFINE_NODE(ConstantNode, "Constant", "Constant", Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> value = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "value", NodePin::Flags::ForceDataField);

	virtual void process() {}
	
};

void ConstantNode::initialize(){
	addIoData(value);
}


class AdditionNode : public Node {
public:

	DEFINE_NODE(AdditionNode, "Addition", "Addition", Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> input = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "input", NodePin::Flags::AcceptMultipleInputs);
	std::shared_ptr<NodePin> offset = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "offset", NodePin::Flags::DisablePin | NodePin::Flags::ForceDataField);
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "result", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> offsetPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void process() {
		double sum = 0.0;
		
		if(input->hasMultipleConnections()) {
			*inputPinValue = 0.0;
			for (auto pin : input->getConnectedPins()) *inputPinValue += pin->read<double>();
		}
		else if (input->isConnected()) input->copyConnectedPinValue();
		sum += *inputPinValue;
		
		sum += *offsetPinValue;
		*outputPinValue = sum;
	}
};

void AdditionNode::initialize(){
	input->assignData(inputPinValue);
	addIoData(input);
	offset->assignData(offsetPinValue);
	addIoData(offset);
	output->assignData(outputPinValue);
	addIoData(output);
}

class SubtractionNode : public Node {
public:

	DEFINE_NODE(SubtractionNode, "Subtraction", "Subtraction", Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> base = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "base", NodePin::Flags::AcceptMultipleInputs);
	std::shared_ptr<NodePin> sub = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "sub", NodePin::Flags::AcceptMultipleInputs);
	std::shared_ptr<NodePin> offset = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "offset", NodePin::Flags::DisablePin | NodePin::Flags::ForceDataField);
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "result", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> basePinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> subPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> offsetPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void process() {
		double sum = 0.0;
		
		if(base->hasMultipleConnections()) {
			*basePinValue = 0.0;
			for(auto pin : base->getConnectedPins()) *basePinValue += pin->read<double>();
		}
		else if(base->isConnected()) base->copyConnectedPinValue();
		sum += *basePinValue;
		
		if(sub->hasMultipleConnections()) {
			*subPinValue = 0.0;
			for(auto pin : sub->getConnectedPins()) *subPinValue += pin->read<double>();
		}else if(sub->isConnected()) sub->copyConnectedPinValue();
		sum -= *subPinValue;
		
		sum += *offsetPinValue;
		
		*outputPinValue = sum;
	}
};

void SubtractionNode::initialize(){
	base->assignData(basePinValue);
	addIoData(base);
	sub->assignData(subPinValue);
	addIoData(sub);
	offset->assignData(offsetPinValue);
	addIoData(offset);
	output->assignData(outputPinValue);
	addIoData(output);
}

class MultiplicationNode : public Node {
public:

	DEFINE_NODE(MultiplicationNode, "Multiplication", "Mutliplication", Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> input = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "input", NodePin::Flags::AcceptMultipleInputs);
	std::shared_ptr<NodePin> multiplier = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "mult", NodePin::Flags::DisablePin | NodePin::Flags::ForceDataField);
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "result", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> multiplierPinValue = std::make_shared<double>(1.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void process() {
		double out = 1.0;
		
		if(input->hasMultipleConnections()) {
			*inputPinValue = 1.0;
			for(auto pin : input->getConnectedPins()) *inputPinValue *= pin->read<double>();
		}else if(input->isConnected()) input->copyConnectedPinValue();
		out *= *inputPinValue;
		
		
		out *= *multiplierPinValue;
		
		*outputPinValue = out;
	}
};

void MultiplicationNode::initialize(){
	input->assignData(inputPinValue);
	addIoData(input);
	multiplier->assignData(multiplierPinValue);
	addIoData(multiplier);
	output->assignData(outputPinValue);
	addIoData(output);
}

class DivisionNode : public Node {
public:

	DEFINE_NODE(DivisionNode, "Division", "Division", Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> base = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "base", NodePin::Flags::AcceptMultipleInputs);
	std::shared_ptr<NodePin> div = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "div", NodePin::Flags::AcceptMultipleInputs);
	std::shared_ptr<NodePin> mult = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "mult", NodePin::Flags::DisablePin | NodePin::Flags::ForceDataField);
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "result", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> basePinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> divPinValue = std::make_shared<double>(1.0);
	std::shared_ptr<double> multPinValue = std::make_shared<double>(1.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	
	virtual void process() {
		double out = 1.0;
		
		if(base->hasMultipleConnections()){
			*basePinValue = 0.0;
			for(auto pin : base->getConnectedPins()) *basePinValue *= pin->read<double>();
		}else if(base->isConnected()) base->copyConnectedPinValue();
		out *= *basePinValue;
		
		if(div->hasMultipleConnections()){
			*divPinValue = 0.0;
			for(auto pin : div->getConnectedPins()) *divPinValue *= pin->read<double>();
		}else if(div->isConnected()) div->copyConnectedPinValue();
		out /= *divPinValue;
		
		out *= *multPinValue;
		
		*outputPinValue = out;
	}
};

void DivisionNode::initialize(){
	base->assignData(basePinValue);
	addIoData(base);
	div->assignData(divPinValue);
	addIoData(div);
	mult->assignData(multPinValue);
	addIoData(mult);
	output->assignData(outputPinValue);
	addIoData(output);
}

class ExponentNode : public Node {
public:

	DEFINE_NODE(ExponentNode, "Exponent", "Exponent", Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> base = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "base");
	std::shared_ptr<NodePin> exp = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "exp");
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "result", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> basePinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> expPinValue = std::make_shared<double>(1.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void process() {
		if(base->isConnected()) base->copyConnectedPinValue();
		
		if(exp->isConnected()) exp->copyConnectedPinValue();
		
		*outputPinValue = std::pow(*basePinValue, *expPinValue);
	}
};

void ExponentNode::initialize(){
	base->assignData(basePinValue);
	addIoData(base);
	exp->assignData(expPinValue);
	addIoData(exp);
	output->assignData(outputPinValue);
	addIoData(output);
}


class AbsoluteNode : public Node {
public:

	DEFINE_NODE(AbsoluteNode, "Absolute", "Absolute", Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> input = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void process() {
		if(input->isConnected()) input->copyConnectedPinValue();
		
		*outputPinValue = std::abs(*inputPinValue);
	}
};

void AbsoluteNode::initialize(){
	input->assignData(inputPinValue);
	addIoData(input);
	output->assignData(outputPinValue);
	addIoData(output);
}






class SinusNode : public Node {
public:

	DEFINE_NODE(SinusNode, "Sinus", "Sinus", Node::Type::PROCESSOR, "Trigonometry")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);
	
	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);

	virtual void process() {
		if(in->isConnected()) in->copyConnectedPinValue();
		*outputPinValue = std::sin(*inputPinValue);
	}
};

void SinusNode::initialize(){
	in->assignData(inputPinValue);
	addIoData(in);
	out->assignData(outputPinValue);
	addIoData(out);
}

class CosinusNode : public Node {
public:

	DEFINE_NODE(CosinusNode, "Cosinus", "Cosinus", Node::Type::PROCESSOR, "Trigonometry")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(1.0);
	
	virtual void process() {
		if(in->isConnected()) in->copyConnectedPinValue();
		*outputPinValue = std::cos(*inputPinValue);
	}
};

void CosinusNode::initialize(){
	in->assignData(inputPinValue);
	addIoData(in);
	out->assignData(outputPinValue);
	addIoData(out);
}

class TangentNode : public Node {
public:

	DEFINE_NODE(TangentNode, "Tangent", "Tangent", Node::Type::PROCESSOR, "Trigonometry")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void process() {
		if(in->isConnected()) in->copyConnectedPinValue();
		*outputPinValue = std::tan(*inputPinValue);
	}
};

void TangentNode::initialize(){
	in->assignData(inputPinValue);
	addIoData(in);
	out->assignData(outputPinValue);
	addIoData(out);
}

class CotangentNode : public Node {
public:

	DEFINE_NODE(CotangentNode, "Cotangent", "Cotangent", Node::Type::PROCESSOR, "Trigonometry")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(std::numeric_limits<double>::infinity());
	
	virtual void process() {
		if(in->isConnected()) in->copyConnectedPinValue();
		*outputPinValue = 1.0 / std::sin(*inputPinValue);
	}
};

void CotangentNode::initialize(){
	in->assignData(inputPinValue);
	addIoData(in);
	out->assignData(outputPinValue);
	addIoData(out);
}












class BoolNode : public Node {
public:

	DEFINE_NODE(BoolNode, "Bool", "Bool", Node::Type::PROCESSOR, "Logic")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<bool> inputPinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> outputPinValue = std::make_shared<bool>(false);
	
	virtual void process() {
		if(in->isConnected()) in->copyConnectedPinValue();
		*outputPinValue = *inputPinValue;
	}
};

void BoolNode::initialize(){
	in->assignData(inputPinValue);
	addIoData(in);
	out->assignData(outputPinValue);
	addIoData(out);
}

class NotNode : public Node{
public:

	DEFINE_NODE(NotNode, "Not", "Not", Node::Type::PROCESSOR, "Logic")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);
	
	std::shared_ptr<bool> inputPinValue = std::make_shared<bool>(0.0);
	std::shared_ptr<bool> outputPinValue = std::make_shared<bool>(0.0);

	virtual void process() {
		if(in->isConnected()) in->copyConnectedPinValue();
		*outputPinValue = !*inputPinValue;
	}
};

void NotNode::initialize(){
	in->assignData(inputPinValue);
	addIoData(in);
	out->assignData(outputPinValue);
	addIoData(out);
}

class AndNode : public Node {
public:

	DEFINE_NODE(AndNode, "And", "And", Node::Type::PROCESSOR, "Logic")
	
	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "in", NodePin::Flags::AcceptMultipleInputs);
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<bool> inputPinValue = std::make_shared<bool>(0.0);
	std::shared_ptr<bool> outputPinValue = std::make_shared<bool>(0.0);
	
	virtual void process() {
		if(in->hasMultipleConnections()){
			*inputPinValue = false;
			for(auto pin : in->getConnectedPins()) {
				if(!pin->read<bool>()){
					*outputPinValue = false;
					return;
				}
			}
			*outputPinValue = true;
		}
	}
};

void AndNode::initialize(){
	in->assignData(inputPinValue);
	addIoData(in);
	out->assignData(outputPinValue);
	addIoData(out);
}

class OrNode : public Node {
public:

	DEFINE_NODE(OrNode, "Or", "Or", Node::Type::PROCESSOR, "Logic")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "in", NodePin::Flags::AcceptMultipleInputs);
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<bool> inputPinValue = std::make_shared<bool>(0.0);
	std::shared_ptr<bool> outputPinValue = std::make_shared<bool>(0.0);
	
	virtual void process() {
		if(in->hasMultipleConnections()){
			*inputPinValue = false;
			for(auto pin : in->getConnectedPins()) {
				if(pin->read<bool>()){
					*outputPinValue = true;
					return;
				}
			}
			*outputPinValue = false;
		}
	}
};

void OrNode::initialize(){
	in->assignData(inputPinValue);
	addIoData(in);
	out->assignData(outputPinValue);
	addIoData(out);
}

