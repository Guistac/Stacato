#pragma once

#include "Environnement/NodeGraph/Node.h"
#include <tinyxml2.h>

class ConstantNode : public Node {
public:

	DEFINE_NODE(ConstantNode, "Constant", "Constant", Node::Type::PROCESSOR, "Math")

	std::shared_ptr<double> constantValue = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> pin = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "value", NodePin::Flags::ForceDataField);
	
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

	virtual void inputProcess() override;
	
	//saving & loading
	virtual bool load(tinyxml2::XMLElement* xml) override {
		double val;
		if(xml->QueryDoubleAttribute("Offset", &val) != tinyxml2::XML_SUCCESS){
			Logger::warn("could not load constant value");
			return false;
		}
		*offsetPinValue = val;
		return true;
	}
	virtual bool save(tinyxml2::XMLElement* xml) override {
		xml->SetAttribute("Offset", *offsetPinValue);
		return true;
	}
	
};

void AdditionNode::initialize(){
	input->assignData(inputPinValue);
	addNodePin(input);
	offset->assignData(offsetPinValue);
	addNodePin(offset);
	output->assignData(outputPinValue);
	addNodePin(output);
}

void AdditionNode::inputProcess() {
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
	
	virtual void inputProcess() override;
	
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

void SubtractionNode::inputProcess() {
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
	
	virtual void inputProcess() override;
	
	//saving & loading
	virtual bool load(tinyxml2::XMLElement* xml) override {
		double val;
		if(xml->QueryDoubleAttribute("Multiplier", &val) != tinyxml2::XML_SUCCESS){
			Logger::warn("could not load constant value");
			return false;
		}
		*multiplierPinValue = val;
		return true;
	}
	virtual bool save(tinyxml2::XMLElement* xml) override {
		xml->SetAttribute("Multiplier", *multiplierPinValue);
		return true;
	}
	
};

void MultiplicationNode::initialize(){
	input->assignData(inputPinValue);
	addNodePin(input);
	multiplier->assignData(multiplierPinValue);
	addNodePin(multiplier);
	output->assignData(outputPinValue);
	addNodePin(output);
}

void MultiplicationNode::inputProcess() {
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
	
	virtual void inputProcess() override;
	
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

void DivisionNode::inputProcess() {
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
	
	virtual void inputProcess() override;
	
};

void ExponentNode::initialize(){
	base->assignData(basePinValue);
	addNodePin(base);
	exp->assignData(expPinValue);
	addNodePin(exp);
	output->assignData(outputPinValue);
	addNodePin(output);
}

void ExponentNode::inputProcess() {
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
	
	virtual void inputProcess() override;
	
};

void AbsoluteNode::initialize(){
	input->assignData(inputPinValue);
	addNodePin(input);
	output->assignData(outputPinValue);
	addNodePin(output);
}

void AbsoluteNode::inputProcess() {
	if(input->isConnected()) input->copyConnectedPinValue();
	
	*outputPinValue = std::abs(*inputPinValue);
}



class ModuloNode : public Node {
public:

	DEFINE_NODE(ModuloNode, "Modulo", "Modulo", Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in", NodePin::Flags::None);
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	NumberParam<double> rangeLower = NumberParameter<double>::make(-1.0, "Lower Range", "LowerRange", "%.3f");
	NumberParam<double> rangeUpper = NumberParameter<double>::make(1.0, "Upper Range", "UpperRange", "%.3f");

	void nodeSpecificGui() override {
		if (ImGui::BeginTabItem("Plot")) {
			rangeLower->gui(Fonts::sansBold15);
			rangeUpper->gui(Fonts::sansBold15);
			ImGui::EndTabItem();
		}
	}

	void inputProcess() override {
		if(in->isConnected()) in->copyConnectedPinValue();
		double input = *inputPinValue;
		double low = rangeLower->value;
		double high = rangeUpper->value;
		if(low > high) std::swap(low, high);
		else if(low == high) {
			*outputPinValue = low;
			return;
		}
		double range = high - low;
		double output = fmod(input, range);
		if(output > high) output -= range;
		if(output < low) output += range;
		*outputPinValue = output;
	}
	
	bool load(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		rangeLower->load(xml);
		rangeUpper->load(xml);
		return true;
	}

	bool save(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		rangeLower->save(xml);
		rangeUpper->save(xml);
		return true;
	}
	
};


void ModuloNode::initialize() {
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}


inline double map(double val, double inlow, double inhigh, double outlow, double outhigh) {
	return (val - inlow) * (outhigh - outlow) / (inhigh - inlow) + outlow;
}

class JoystickNode : public Node{
public:

	DEFINE_NODE(JoystickNode, "Joystick", "Joystick", Node::Type::PROCESSOR, "Math")

	std::shared_ptr<NodePin> inNeg = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in-", NodePin::Flags::None);
	std::shared_ptr<NodePin> inPos = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in+", NodePin::Flags::None);
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);

	std::shared_ptr<double> inputNegPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> inputPosPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);
	
	NumberParam<double> deadZone = NumberParameter<double>::make(0.01, "deadzone", "deadzone", "%.3f");
	NumberParam<double> maxRange = NumberParameter<double>::make(1.0, "max range", "maxrange", "%.3f");
	double raw = 0.0;
	
	void nodeSpecificGui() override {
		if (ImGui::BeginTabItem("Joystick")) {
			deadZone->gui(Fonts::sansBold15);
			maxRange->gui(Fonts::sansBold15);
			ImGui::Text("raw=%.4f", raw);
			ImGui::Text("out=%.4f", *outputPinValue);
			ImGui::EndTabItem();
		}
	}

	void inputProcess() override {
		if(inNeg->isConnected()) inNeg->copyConnectedPinValue();
		if(inPos->isConnected()) inPos->copyConnectedPinValue();
		
		raw = *inputPosPinValue - *inputNegPinValue;
		double output;
		if(raw > deadZone->value){
			output = map(raw, deadZone->value, maxRange->value, 0.0, maxRange->value);
			if(output > maxRange->value) output = maxRange->value;
		}
		else if(raw < -deadZone->value){
			output = map(raw, -deadZone->value, -maxRange->value, 0.0, -maxRange->value);
			if(output < -maxRange->value) output = -maxRange->value;
		}
		else output = 0.0;
		
		*outputPinValue = output;
	}
	
	bool load(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		deadZone->load(xml);
		maxRange->load(xml);
		return true;
	}

	bool save(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		deadZone->save(xml);
		maxRange->save(xml);
		return true;
	}
};

void JoystickNode::initialize() {
	inNeg->assignData(inputNegPinValue);
	inPos->assignData(inputPosPinValue);
	out->assignData(outputPinValue);
	addNodePin(inNeg);
	addNodePin(inPos);
	addNodePin(out);
}






class SinusNode : public Node {
public:

	DEFINE_NODE(SinusNode, "Sinus", "Sinus", Node::Type::PROCESSOR, "Trigonometry")

	std::shared_ptr<NodePin> in = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "in");
	std::shared_ptr<NodePin> out = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "out", NodePin::Flags::DisableDataField);
	
	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);

	virtual void inputProcess() override;
	
};

void SinusNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void SinusNode::inputProcess() {
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
	
	virtual void inputProcess() override;
	
};

void CosinusNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void CosinusNode::inputProcess() {
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
	
	virtual void inputProcess() override;
	
};

void TangentNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void TangentNode::inputProcess() {
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
	
	virtual void inputProcess() override;
	
};

void CotangentNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void CotangentNode::inputProcess() {
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
	
	virtual void inputProcess() override;
	
};

void BoolNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void BoolNode::inputProcess() {
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

	virtual void inputProcess() override;
	
};

void NotNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void NotNode::inputProcess() {
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
	
	virtual void inputProcess() override;
	
};

void AndNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void AndNode::inputProcess() {
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
	
	virtual void inputProcess() override;
	
};

void OrNode::initialize(){
	in->assignData(inputPinValue);
	addNodePin(in);
	out->assignData(outputPinValue);
	addNodePin(out);
}

void OrNode::inputProcess() {
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

