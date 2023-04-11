#pragma once

#include "Environnement/NodeGraph/Node.h"
#include <tinyxml2.h>

class ConstantNode : public Node {
public:

	DEFINE_NODE(ConstantNode, Node::Type::PROCESSOR, "Math")

	std::shared_ptr<double> constantValue = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> pin;
	
	virtual void inputProcess() override{};
	
	//saving & loading
	virtual bool load(tinyxml2::XMLElement* xml) override {
		double val;
		if(xml->QueryDoubleAttribute("ConstantValue", &val) != tinyxml2::XML_SUCCESS){
			Logger::warn("could not load constant value");
			return false;
		}
		*constantValue = val;
		return true;
	}
	virtual bool save(tinyxml2::XMLElement* xml) override {
		xml->SetAttribute("ConstantValue", *constantValue);
		return true;
	}
	
	virtual void onConstruction() override {
		Node::onConstruction();
		setName("Constant");
		pin = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "value", "value", NodePin::Flags::ForceDataField);
		pin->assignData(constantValue);
		addNodePin(pin);
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};


class AdditionNode : public Node {
public:

	DEFINE_NODE(AdditionNode, Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> input;
	std::shared_ptr<NodePin> offset;
	std::shared_ptr<NodePin> output;

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> offsetPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);

	virtual void inputProcess() override{
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
	
	virtual void onConstruction() override {
		Node::onConstruction();
		
		setName("Constant");
		
		input = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "input", "input",
										NodePin::Flags::AcceptMultipleInputs);
		offset = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "offset", "offset",
										 NodePin::Flags::DisablePin | NodePin::Flags::ForceDataField);
		output = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "result", "result",
										 NodePin::Flags::DisableDataField);
		
		input->assignData(inputPinValue);
		addNodePin(input);
		offset->assignData(offsetPinValue);
		addNodePin(offset);
		output->assignData(outputPinValue);
		addNodePin(output);
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};


class SubtractionNode : public Node {
public:

	DEFINE_NODE(SubtractionNode, Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> base;
	std::shared_ptr<NodePin> sub;
	std::shared_ptr<NodePin> offset;
	std::shared_ptr<NodePin> output;

	std::shared_ptr<double> basePinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> subPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> offsetPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void inputProcess() override{
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
	
	virtual void onConstruction() override {
		Node::onConstruction();
		
		setName("Substraction");
		
		base = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "base", "base",
									   NodePin::Flags::AcceptMultipleInputs);
		sub = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "sub", "sub",
									  NodePin::Flags::AcceptMultipleInputs);
		offset = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "offset", "offset",
										 NodePin::Flags::DisablePin | NodePin::Flags::ForceDataField);
		output = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "result", "result",
										 NodePin::Flags::DisableDataField);


		
		base->assignData(basePinValue);
		addNodePin(base);
		sub->assignData(subPinValue);
		addNodePin(sub);
		offset->assignData(offsetPinValue);
		addNodePin(offset);
		output->assignData(outputPinValue);
		addNodePin(output);
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};


class MultiplicationNode : public Node {
public:

	DEFINE_NODE(MultiplicationNode, Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> input;
	std::shared_ptr<NodePin> multiplier;
	std::shared_ptr<NodePin> output;

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> multiplierPinValue = std::make_shared<double>(1.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void inputProcess() override{
		double out = 1.0;

		if(input->hasMultipleConnections()) {
			*inputPinValue = 1.0;
			for(auto pin : input->getConnectedPins()) *inputPinValue *= pin->read<double>();
		}else if(input->isConnected()) input->copyConnectedPinValue();
		out *= *inputPinValue;


		out *= *multiplierPinValue;

		*outputPinValue = out;
	}
	
	virtual void onConstruction() override {
		Node::onConstruction();
		
		setName("Multiplication");
		
		input = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "input", "input",
										NodePin::Flags::AcceptMultipleInputs);
		multiplier = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "mult", "mult",
											 NodePin::Flags::DisablePin | NodePin::Flags::ForceDataField);
		output = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "result", "result",
										 NodePin::Flags::DisableDataField);

		
		input->assignData(inputPinValue);
		addNodePin(input);
		multiplier->assignData(multiplierPinValue);
		addNodePin(multiplier);
		output->assignData(outputPinValue);
		addNodePin(output);
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};



class DivisionNode : public Node {
public:

	DEFINE_NODE(DivisionNode, Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> base;
	std::shared_ptr<NodePin> div;
	std::shared_ptr<NodePin> mult;
	std::shared_ptr<NodePin> output;

	std::shared_ptr<double> basePinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> divPinValue = std::make_shared<double>(1.0);
	std::shared_ptr<double> multPinValue = std::make_shared<double>(1.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void inputProcess() override{
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
	
	virtual void onConstruction() override {
		Node::onConstruction();
		
		setName("Division");
		
		base = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "base", "base",
									   NodePin::Flags::AcceptMultipleInputs);
		div = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "div", "div",
									  NodePin::Flags::AcceptMultipleInputs);
		mult = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "mult", "mult",
									   NodePin::Flags::DisablePin | NodePin::Flags::ForceDataField);
		output = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "result", "result",
										 NodePin::Flags::DisableDataField);

		
		base->assignData(basePinValue);
		addNodePin(base);
		div->assignData(divPinValue);
		addNodePin(div);
		mult->assignData(multPinValue);
		addNodePin(mult);
		output->assignData(outputPinValue);
		addNodePin(output);
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};



class ExponentNode : public Node {
public:

	DEFINE_NODE(ExponentNode, Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> base;
	std::shared_ptr<NodePin> exp;
	std::shared_ptr<NodePin> output;

	std::shared_ptr<double> basePinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> expPinValue = std::make_shared<double>(1.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void inputProcess() override{
		if(base->isConnected()) base->copyConnectedPinValue();
		
		if(exp->isConnected()) exp->copyConnectedPinValue();
		
		*outputPinValue = std::pow(*basePinValue, *expPinValue);
	}
	
	virtual void onConstruction() override {
		Node::onConstruction();
		
		setName("Exponent");
		
		base = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "base", "base");
		exp = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "exp", "exp");
		output = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "result", "result", NodePin::Flags::DisableDataField);
		
		base->assignData(basePinValue);
		addNodePin(base);
		exp->assignData(expPinValue);
		addNodePin(exp);
		output->assignData(outputPinValue);
		addNodePin(output);
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};



class AbsoluteNode : public Node {
public:

	DEFINE_NODE(AbsoluteNode, Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> input;
	std::shared_ptr<NodePin> output;

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void inputProcess() override{
		if(input->isConnected()) input->copyConnectedPinValue();
			
			*outputPinValue = std::abs(*inputPinValue);
	}
	
	virtual void onConstruction() override {
		Node::onConstruction();
		
		setName("Absolute");
		
		input = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in", "in");
		output = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", "out", NodePin::Flags::DisableDataField);
		
		input->assignData(inputPinValue);
		addNodePin(input);
		output->assignData(outputPinValue);
		addNodePin(output);
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};





class SinusNode : public Node {
public:

	DEFINE_NODE(SinusNode, Node::Type::PROCESSOR, "Trigonometry")

	std::shared_ptr<NodePin> inputPin;
	std::shared_ptr<NodePin> outputPin;
	
	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);

	virtual void inputProcess() override{
		if(inputPin->isConnected()) inputPin->copyConnectedPinValue();
		*outputPinValue = std::sin(*inputPinValue);
	}
	
	virtual void onConstruction() override {
		Node::onConstruction();
		
		setName("Sinus");
		
		inputPin = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in", "in");
		outputPin = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", "out", NodePin::Flags::DisableDataField);
		
		inputPin->assignData(inputPinValue);
		addNodePin(inputPin);
		outputPin->assignData(outputPinValue);
		addNodePin(outputPin);
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};



class CosinusNode : public Node {
public:

	DEFINE_NODE(CosinusNode, Node::Type::PROCESSOR, "Trigonometry")

	std::shared_ptr<NodePin> inputPin;
	std::shared_ptr<NodePin> outputPin;

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(1.0);
	
	virtual void inputProcess() override{
		if(inputPin->isConnected()) inputPin->copyConnectedPinValue();
		*outputPinValue = std::cos(*inputPinValue);
	}
	
	virtual void onConstruction() override {
		Node::onConstruction();
		
		setName("Cosinus");
		
		inputPin = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in", "in");
		outputPin = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", "out", NodePin::Flags::DisableDataField);
		
		inputPin->assignData(inputPinValue);
		addNodePin(inputPin);
		outputPin->assignData(outputPinValue);
		addNodePin(outputPin);
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};


class TangentNode : public Node {
public:

	DEFINE_NODE(TangentNode, Node::Type::PROCESSOR, "Trigonometry")

	std::shared_ptr<NodePin> inputPin;
	std::shared_ptr<NodePin> outputPin;

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	virtual void inputProcess() override{
		if(inputPin->isConnected()) inputPin->copyConnectedPinValue();
		*outputPinValue = std::tan(*inputPinValue);
	}
	
	virtual void onConstruction() override {
		Node::onConstruction();
		
		setName("Tangent");
		
		inputPin = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in", "in");
		outputPin = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", "out", NodePin::Flags::DisableDataField);
		
		inputPin->assignData(inputPinValue);
		addNodePin(inputPin);
		outputPin->assignData(outputPinValue);
		addNodePin(outputPin);
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};


class CotangentNode : public Node {
public:

	DEFINE_NODE(CotangentNode, Node::Type::PROCESSOR, "Trigonometry")

	std::shared_ptr<NodePin> inputPin;
	std::shared_ptr<NodePin> outputPin;

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(std::numeric_limits<double>::infinity());
	
	virtual void inputProcess() override{
		if(inputPin->isConnected()) inputPin->copyConnectedPinValue();
		*outputPinValue = 1.0 / std::sin(*inputPinValue);
	}
	
	virtual void onConstruction() override {
		Node::onConstruction();
		
		setName("Cotangent");
		
		inputPin = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in", "in");
		outputPin = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", "out", NodePin::Flags::DisableDataField);
		
		inputPin->assignData(inputPinValue);
		addNodePin(inputPin);
		outputPin->assignData(outputPinValue);
		addNodePin(outputPin);
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};












class BoolNode : public Node {
public:

	DEFINE_NODE(BoolNode, Node::Type::PROCESSOR, "Logic")

	std::shared_ptr<NodePin> inputPin;
	std::shared_ptr<NodePin> outputPin;

	std::shared_ptr<bool> inputPinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> outputPinValue = std::make_shared<bool>(false);
	
	virtual void inputProcess() override{
		if(inputPin->isConnected()) {
			double realValue = inputPin->getConnectedPin()->read<double>();
			*outputPinValue = realValue > 0.0;
		}else *outputPinValue = false;
	}
	
	virtual void onConstruction() override {
		Node::onConstruction();
		
		setName("Bool");
		
		inputPin = NodePin::createInstance(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "in", "in");
		outputPin = NodePin::createInstance(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "out", "out", NodePin::Flags::DisableDataField);
		
		inputPin->assignData(inputPinValue);
		addNodePin(inputPin);
		outputPin->assignData(outputPinValue);
		addNodePin(outputPin);
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};


class NotNode : public Node{
public:

	DEFINE_NODE(NotNode, Node::Type::PROCESSOR, "Logic")

	std::shared_ptr<NodePin> inputPin;
	std::shared_ptr<NodePin> outputPin;
	
	std::shared_ptr<bool> inputPinValue = std::make_shared<bool>(0.0);
	std::shared_ptr<bool> outputPinValue = std::make_shared<bool>(0.0);

	virtual void inputProcess() override{
		if(inputPin->isConnected()) inputPin->copyConnectedPinValue();
		*outputPinValue = !*inputPinValue;
	}
	
	virtual void onConstruction() override {
		Node::onConstruction();
		
		setName("Not");
		
		inputPin = NodePin::createInstance(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "in", "in");
		outputPin = NodePin::createInstance(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "out", "out", NodePin::Flags::DisableDataField);
		
		inputPin->assignData(inputPinValue);
		addNodePin(inputPin);
		outputPin->assignData(outputPinValue);
		addNodePin(outputPin);
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};



class AndNode : public Node {
public:

	DEFINE_NODE(AndNode, Node::Type::PROCESSOR, "Logic")
	
	std::shared_ptr<NodePin> inputPin;
	std::shared_ptr<NodePin> outputPin;

	std::shared_ptr<bool> inputPinValue = std::make_shared<bool>(0.0);
	std::shared_ptr<bool> outputPinValue = std::make_shared<bool>(0.0);
	
	virtual void inputProcess() override{
		if(inputPin->hasMultipleConnections()){
			*inputPinValue = false;
			for(auto pin : inputPin->getConnectedPins()) {
				if(!pin->read<bool>()){
					*outputPinValue = false;
					return;
				}
			}
			*outputPinValue = true;
		}
	}
	
	virtual void onConstruction() override {
		Node::onConstruction();
		
		setName("And");
		
		inputPin = NodePin::createInstance(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "in", "in", NodePin::Flags::AcceptMultipleInputs);
		outputPin = NodePin::createInstance(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "out", "out", NodePin::Flags::DisableDataField);
		
		inputPin->assignData(inputPinValue);
		addNodePin(inputPin);
		outputPin->assignData(outputPinValue);
		addNodePin(outputPin);
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};




class OrNode : public Node {
public:

	DEFINE_NODE(OrNode, Node::Type::PROCESSOR, "Logic")

	std::shared_ptr<NodePin> inputPin;
	std::shared_ptr<NodePin> outputPin;

	std::shared_ptr<bool> inputPinValue = std::make_shared<bool>(0.0);
	std::shared_ptr<bool> outputPinValue = std::make_shared<bool>(0.0);
	
	virtual void inputProcess() override{
		if(inputPin->hasMultipleConnections()){
				*inputPinValue = false;
				for(auto pin : inputPin->getConnectedPins()) {
					if(pin->read<bool>()){
						*outputPinValue = true;
						return;
					}
				}
				*outputPinValue = false;
			}
	}
	
	virtual void onConstruction() override {
		Node::onConstruction();
		
		setName("Or");
		
		inputPin = NodePin::createInstance(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "in", "in", NodePin::Flags::AcceptMultipleInputs);
		outputPin = NodePin::createInstance(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "out", "out", NodePin::Flags::DisableDataField);
		
		inputPin->assignData(inputPinValue);
		addNodePin(inputPin);
		outputPin->assignData(outputPinValue);
		addNodePin(outputPin);
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};


