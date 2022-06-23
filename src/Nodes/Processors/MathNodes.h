#pragma once

#include "Environnement/Node.h"

class ConstantNode : public Node {
public:

	DEFINE_NODE(ConstantNode, "Constant", "Constant", Node::Type::PROCESSOR, "Math")

	std::shared_ptr<double> constantValue = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> pin = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "value", NodePin::Flags::ForceDataField);
	
	virtual void process(){}
	
};

void ConstantNode::initialize(){
	pin->assignData(constantValue);
	addNodePin(pin);
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

	virtual void process();
	
};

void AdditionNode::initialize(){
	input->assignData(inputPinValue);
	addNodePin(input);
	offset->assignData(offsetPinValue);
	addNodePin(offset);
	output->assignData(outputPinValue);
	addNodePin(output);
}

void AdditionNode::process() {
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
	
	virtual void process();
	
};

void SubtractionNode::initialize(){
	base->assignData(basePinValue);
	addNodePin(base);
	sub->assignData(subPinValue);
	addNodePin(sub);
	offset->assignData(offsetPinValue);
	addNodePin(offset);
	output->assignData(outputPinValue);
	addNodePin(output);
}

void SubtractionNode::process() {
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

class MultiplicationNode : public Node {
public:

	DEFINE_NODE(MultiplicationNode, "Multiplication", "Mutliplication", Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> input = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "input", NodePin::Flags::AcceptMultipleInputs);
	std::shared_ptr<NodePin> multiplier = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "mult", NodePin::Flags::DisablePin | NodePin::Flags::ForceDataField);
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "result", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> multiplierPinValue = std::make_shared<double>(1.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void process();
	
};

void MultiplicationNode::initialize(){
	input->assignData(inputPinValue);
	addNodePin(input);
	multiplier->assignData(multiplierPinValue);
	addNodePin(multiplier);
	output->assignData(outputPinValue);
	addNodePin(output);
}

void MultiplicationNode::process() {
	double out = 1.0;
	
	if(input->hasMultipleConnections()) {
		*inputPinValue = 1.0;
		for(auto pin : input->getConnectedPins()) *inputPinValue *= pin->read<double>();
	}else if(input->isConnected()) input->copyConnectedPinValue();
	out *= *inputPinValue;
	
	
	out *= *multiplierPinValue;
	
	*outputPinValue = out;
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
	
	virtual void process();
	
};

void DivisionNode::initialize(){
	base->assignData(basePinValue);
	addNodePin(base);
	div->assignData(divPinValue);
	addNodePin(div);
	mult->assignData(multPinValue);
	addNodePin(mult);
	output->assignData(outputPinValue);
	addNodePin(output);
}

void DivisionNode::process() {
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

class ExponentNode : public Node {
public:

	DEFINE_NODE(ExponentNode, "Exponent", "Exponent", Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> base = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "base");
	std::shared_ptr<NodePin> exp = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "exp");
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "result", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> basePinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> expPinValue = std::make_shared<double>(1.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void process();
	
};

void ExponentNode::initialize(){
	base->assignData(basePinValue);
	addNodePin(base);
	exp->assignData(expPinValue);
	addNodePin(exp);
	output->assignData(outputPinValue);
	addNodePin(output);
}

void ExponentNode::process() {
	if(base->isConnected()) base->copyConnectedPinValue();
	
	if(exp->isConnected()) exp->copyConnectedPinValue();
	
	*outputPinValue = std::pow(*basePinValue, *expPinValue);
}


class AbsoluteNode : public Node {
public:

	DEFINE_NODE(AbsoluteNode, "Absolute", "Absolute", Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> input = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void process();
	
};

void AbsoluteNode::initialize(){
	input->assignData(inputPinValue);
	addNodePin(input);
	output->assignData(outputPinValue);
	addNodePin(output);
}

void AbsoluteNode::process() {
	if(input->isConnected()) input->copyConnectedPinValue();
	
	*outputPinValue = std::abs(*inputPinValue);
}






class SinusNode : public Node {
public:

	DEFINE_NODE(SinusNode, "Sinus", "Sinus", Node::Type::PROCESSOR, "Trigonometry")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);
	
	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);

	virtual void process();
	
};

void SinusNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void SinusNode::process() {
	if(in->isConnected()) in->copyConnectedPinValue();
	*outputPinValue = std::sin(*inputPinValue);
}


class CosinusNode : public Node {
public:

	DEFINE_NODE(CosinusNode, "Cosinus", "Cosinus", Node::Type::PROCESSOR, "Trigonometry")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(1.0);
	
	virtual void process();
	
};

void CosinusNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void CosinusNode::process() {
	if(in->isConnected()) in->copyConnectedPinValue();
	*outputPinValue = std::cos(*inputPinValue);
}

class TangentNode : public Node {
public:

	DEFINE_NODE(TangentNode, "Tangent", "Tangent", Node::Type::PROCESSOR, "Trigonometry")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void process();
	
};

void TangentNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void TangentNode::process() {
	if(in->isConnected()) in->copyConnectedPinValue();
	*outputPinValue = std::tan(*inputPinValue);
}

class CotangentNode : public Node {
public:

	DEFINE_NODE(CotangentNode, "Cotangent", "Cotangent", Node::Type::PROCESSOR, "Trigonometry")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(std::numeric_limits<double>::infinity());
	
	virtual void process();
	
};

void CotangentNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void CotangentNode::process() {
	if(in->isConnected()) in->copyConnectedPinValue();
	*outputPinValue = 1.0 / std::sin(*inputPinValue);
}












class BoolNode : public Node {
public:

	DEFINE_NODE(BoolNode, "Bool", "Bool", Node::Type::PROCESSOR, "Logic")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<bool> inputPinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> outputPinValue = std::make_shared<bool>(false);
	
	virtual void process();
	
};

void BoolNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void BoolNode::process() {
	if(in->isConnected()) {
		double realValue = in->getConnectedPin()->read<double>();
		*outputPinValue = realValue > 0.0;
	}else *outputPinValue = false;
}


class NotNode : public Node{
public:

	DEFINE_NODE(NotNode, "Not", "Not", Node::Type::PROCESSOR, "Logic")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);
	
	std::shared_ptr<bool> inputPinValue = std::make_shared<bool>(0.0);
	std::shared_ptr<bool> outputPinValue = std::make_shared<bool>(0.0);

	virtual void process();
	
};

void NotNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void NotNode::process() {
	if(in->isConnected()) in->copyConnectedPinValue();
	*outputPinValue = !*inputPinValue;
}



class AndNode : public Node {
public:

	DEFINE_NODE(AndNode, "And", "And", Node::Type::PROCESSOR, "Logic")
	
	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "in", NodePin::Flags::AcceptMultipleInputs);
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<bool> inputPinValue = std::make_shared<bool>(0.0);
	std::shared_ptr<bool> outputPinValue = std::make_shared<bool>(0.0);
	
	virtual void process();
	
};

void AndNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void AndNode::process() {
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



class OrNode : public Node {
public:

	DEFINE_NODE(OrNode, "Or", "Or", Node::Type::PROCESSOR, "Logic")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "in", NodePin::Flags::AcceptMultipleInputs);
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<bool> inputPinValue = std::make_shared<bool>(0.0);
	std::shared_ptr<bool> outputPinValue = std::make_shared<bool>(0.0);
	
	virtual void process();
	
};

void OrNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void OrNode::process() {
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

