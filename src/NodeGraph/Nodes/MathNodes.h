#pragma once

#include "NodeGraph/Node.h"

class ConstantNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(ConstantNode, "Constant", "Constant", "Math")

	virtual void assignIoData() {
		addIoData(value);
	}

	std::shared_ptr<NodePin> value = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "value", NodePin::Flags::ForceDataField);

	virtual void process() {}
};

class AdditionNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(AdditionNode, "Addition", "Addition", "Math")
	
	virtual void assignIoData() {
		input->assignData(inputPinValue);
		addIoData(input);
		offset->assignData(offsetPinValue);
		addIoData(offset);
		output->assignData(outputPinValue);
		addIoData(output);
	}

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

class SubtractionNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(SubtractionNode, "Subtraction", "Subtraction", "Math")

	virtual void assignIoData() {
		base->assignData(basePinValue);
		addIoData(base);
		sub->assignData(subPinValue);
		addIoData(sub);
		offset->assignData(offsetPinValue);
		addIoData(offset);
		output->assignData(outputPinValue);
		addIoData(output);
	}

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

class MultiplicationNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(MultiplicationNode, "Multiplication", "Mutliplication", "Math")

	virtual void assignIoData() {
		input->assignData(inputPinValue);
		addIoData(input);
		multiplier->assignData(multiplierPinValue);
		addIoData(multiplier);
		output->assignData(outputPinValue);
		addIoData(output);
	}

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

class DivisionNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(DivisionNode, "Division", "Division", "Math")

	virtual void assignIoData() {
		base->assignData(basePinValue);
		addIoData(base);
		div->assignData(divPinValue);
		addIoData(div);
		mult->assignData(multPinValue);
		addIoData(mult);
		output->assignData(outputPinValue);
		addIoData(output);
	}

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

class ExponentNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(ExponentNode, "Exponent", "Exponent", "Math")

	virtual void assignIoData() {
		base->assignData(basePinValue);
		addIoData(base);
		exp->assignData(expPinValue);
		addIoData(exp);
		output->assignData(outputPinValue);
		addIoData(output);
	}

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


class AbsoluteNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(AbsoluteNode, "Absolute", "Absolute", "Math")

	virtual void assignIoData() {
		input->assignData(inputPinValue);
		addIoData(input);
		output->assignData(outputPinValue);
		addIoData(output);
	}

	std::shared_ptr<NodePin> input = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> output = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void process() {
		if(input->isConnected()) input->copyConnectedPinValue();
		
		*outputPinValue = std::abs(*inputPinValue);
	}
};






class SinusNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(SinusNode, "Sinus", "Sinus", "Trigonometry")

	virtual void assignIoData() {
		in->assignData(inputPinValue);
		addIoData(in);
		out->assignData(outputPinValue);
		addIoData(out);
	}

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);
	
	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);

	virtual void process() {
		if(in->isConnected()) in->copyConnectedPinValue();
		*outputPinValue = std::sin(*inputPinValue);
	}
};

class CosinusNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(CosinusNode, "Cosinus", "Cosinus", "Trigonometry")

	virtual void assignIoData() {
		in->assignData(inputPinValue);
		addIoData(in);
		out->assignData(outputPinValue);
		addIoData(out);
	}

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(1.0);
	
	virtual void process() {
		if(in->isConnected()) in->copyConnectedPinValue();
		*outputPinValue = std::cos(*inputPinValue);
	}
};

class TangentNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(TangentNode, "Tangent", "Tangent", "Trigonometry")

	virtual void assignIoData() {
		in->assignData(inputPinValue);
		addIoData(in);
		out->assignData(outputPinValue);
		addIoData(out);
	}

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void process() {
		if(in->isConnected()) in->copyConnectedPinValue();
		*outputPinValue = std::tan(*inputPinValue);
	}
};

class CotangentNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(CotangentNode, "Cotangent", "Cotangent", "Trigonometry")

	virtual void assignIoData() {
		in->assignData(inputPinValue);
		addIoData(in);
		out->assignData(outputPinValue);
		addIoData(out);
	}

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(std::numeric_limits<double>::infinity());
	
	virtual void process() {
		if(in->isConnected()) in->copyConnectedPinValue();
		*outputPinValue = 1.0 / std::sin(*inputPinValue);
	}
};












class BoolNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(BoolNode, "Bool", "Bool", "Logic")

	virtual void assignIoData() {
		in->assignData(inputPinValue);
		addIoData(in);
		out->assignData(outputPinValue);
		addIoData(out);
	}

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<bool> inputPinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> outputPinValue = std::make_shared<bool>(false);
	
	virtual void process() {
		if(in->isConnected()) in->copyConnectedPinValue();
		*outputPinValue = *inputPinValue;
	}
};

class NotNode : public Node{
public:

	DEFINE_PROCESSOR_NODE(NotNode, "Not", "Not", "Logic")

	virtual void assignIoData() {
		in->assignData(inputPinValue);
		addIoData(in);
		out->assignData(outputPinValue);
		addIoData(out);
	}

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);
	
	std::shared_ptr<bool> inputPinValue = std::make_shared<bool>(0.0);
	std::shared_ptr<bool> outputPinValue = std::make_shared<bool>(0.0);

	virtual void process() {
		if(in->isConnected()) in->copyConnectedPinValue();
		*outputPinValue = !*inputPinValue;
	}
};

class AndNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(AndNode, "And", "And", "Logic")

	virtual void assignIoData() {
		in->assignData(inputPinValue);
		addIoData(in);
		out->assignData(outputPinValue);
		addIoData(out);
	}

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

class OrNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(OrNode, "Or", "Or", "Logic")

	virtual void assignIoData() {
		in->assignData(inputPinValue);
		addIoData(in);
		out->assignData(outputPinValue);
		addIoData(out);
	}

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

