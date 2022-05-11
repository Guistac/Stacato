#pragma once

#include <tinyxml2.h>
#include <imgui.h>
#include "CommandHistory.h"

class Parameter{
public:
	
	bool b_changed;
	std::string name;
	std::string saveString;
	std::string imguiID;
	
	Parameter(std::string name_, std::string saveString_ = ""){
		b_changed = false;
		setName(name_);
		setSaveString(saveString_);
	}
	
	void setName(std::string name_){
		name = name_;
		imguiID = "##" + name; //this avoid rendering the parameter name in imgui input field
	}
	void setSaveString(std::string saveString_){
		saveString = saveString_;
	}
	
	virtual void gui() = 0;
	virtual bool save(tinyxml2::XMLElement* xml) = 0;
	virtual bool load(tinyxml2::XMLElement* xml) = 0;
	
	bool changed(){
		if(b_changed){
			b_changed = false;
			return true;
		}else return false;
	}
	
	typedef void (*EditCallback)(void*);
	EditCallback editCallback = nullptr;
	void* editCallbackPayload = nullptr;
	
	void setEditCallback(EditCallback cb, void* cbPayload){
		editCallback = cb;
		editCallbackPayload = cbPayload;
	}
	
};



//===============================================================================
//=================================== NUMBERS ===================================
//===============================================================================

template<typename T>
class NumberParameter : public Parameter, public std::enable_shared_from_this<NumberParameter<T>>{
public:
	
	T displayValue;
	T value;
	T stepSmall;
	T stepLarge;
	const char* format = nullptr;
	
	NumberParameter(T value_, std::string name_, std::string saveString_ = "", T stepSmall_ = 0, T stepLarge_ = 0, const char* format_ = nullptr) : Parameter(name_, saveString_) {
		displayValue = value_;
		value = value_;
		format = format_;
		stepSmall = stepSmall_;
		stepLarge = stepLarge_;
	}
	
	void setStepSize(T stepSmall_, T stepLarge_){
		stepSmall = stepSmall_;
		stepLarge = stepLarge_;
	}
	
	void setFormat(const char* format_){
		format = format_;
	}
	
	void inputField();
	
	virtual void gui(){
		inputField();
		if(ImGui::IsItemDeactivatedAfterEdit() && value != displayValue){
			//=========Command Invoker=========
			std::shared_ptr<NumberParameter<T>> thisParameter = this->shared_from_this();
			CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter));
			//=================================
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml){
		xml->SetAttribute(saveString.c_str(), value);
		 return true;
	 }
	
	virtual bool load(tinyxml2::XMLElement* xml){
		using namespace tinyxml2;
		 double number;
		 XMLError result = xml->QueryDoubleAttribute(saveString.c_str(), &number);
		 if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", name);
		 value = number;
		 displayValue = number;
		 return true;
	 }
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<NumberParameter<T>> parameter;
		T newValue;
		T previousValue;
		EditCommand(std::shared_ptr<NumberParameter<T>> parameter_){
			parameter = parameter_;
			newValue = parameter->displayValue;
			previousValue = parameter->value;
			name = "Changed \'" + std::string(parameter->name) + "\' from " + std::to_string(previousValue) + " to " + std::to_string(newValue);
		}
		virtual void execute(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
			parameter->b_changed = true;
			if(parameter->editCallback) parameter->editCallback(parameter->editCallbackPayload);
		}
		virtual void undo(){
			parameter->value = previousValue;
			parameter->displayValue = previousValue;
			parameter->b_changed = true;
			if(parameter->editCallback) parameter->editCallback(parameter->editCallbackPayload);
		}
	};
	
};

template<>
inline void NumberParameter<float>::inputField(){
	ImGui::InputFloat(imguiID.c_str(), &displayValue, stepSmall, stepLarge, format);
}

template<>
inline void NumberParameter<double>::inputField(){
	ImGui::InputDouble(imguiID.c_str(), &displayValue, stepSmall, stepLarge, format);
}

template<>
inline void NumberParameter<uint8_t>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_U8, &displayValue, &stepSmall, &stepLarge, format);
}

template<>
inline void NumberParameter<int8_t>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_S8, &displayValue, &stepSmall, &stepLarge, format);
}

template<>
inline void NumberParameter<uint16_t>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_U16, &displayValue, &stepSmall, &stepLarge, format);
}

template<>
inline void NumberParameter<int16_t>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_S16, &displayValue, &stepSmall, &stepLarge, format);
}

template<>
inline void NumberParameter<uint32_t>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_U32, &displayValue, &stepSmall, &stepLarge, format);
}

template<>
inline void NumberParameter<int32_t>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_S32, &displayValue, &stepSmall, &stepLarge, format);
}

template<>
inline void NumberParameter<uint64_t>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_U64, &displayValue, &stepSmall, &stepLarge, format);
}

template<>
inline void NumberParameter<int64_t>::inputField(){
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_S64, &displayValue, &stepSmall, &stepLarge, format);
}




//===============================================================================
//=================================== VECTORS ===================================
//===============================================================================

template<typename T>
class VectorParameter : public Parameter, public std::enable_shared_from_this<VectorParameter<T>>{
public:
	
	T displayValue;
	T value;
	const char* format = nullptr;
	
	VectorParameter(T value_, std::string name_, std::string saveString_, const char* format_ = nullptr) : Parameter(name_, saveString_) {
		displayValue = value_;
		value = value_;
		format = format_;
	}
	
	void setFormat(const char* format_){
		format = format_;
	}
	
	void inputField();
	
	virtual void gui(){
		inputField();
		if(ImGui::IsItemDeactivatedAfterEdit() && value != displayValue){
			//=========Command Invoker=========
			auto thisParameter = this->shared_from_this();
			CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter));
			//=================================
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<VectorParameter<T>> parameter;
		T newValue;
		T previousValue;
		EditCommand(std::shared_ptr<VectorParameter<T>> parameter_){
			parameter = parameter_;
			newValue = parameter->displayValue;
			previousValue = parameter->value;
			name = "Changed \'" + std::string(parameter->name) + "\' from " + glm::to_string(previousValue) + " to " + glm::to_string(newValue);
		}
		virtual void execute(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
			parameter->b_changed = true;
			if(parameter->editCallback) parameter->editCallback(parameter->editCallbackPayload);
		}
		virtual void undo(){
			parameter->value = previousValue;
			parameter->displayValue = previousValue;
			parameter->b_changed = true;
			if(parameter->editCallback) parameter->editCallback(parameter->editCallbackPayload);
		}
	};
	
};

template<>
inline void VectorParameter<glm::vec2>::inputField(){
	ImGui::InputFloat2(imguiID.c_str(), &displayValue.x, format);
}

template<>
inline bool VectorParameter<glm::vec2>::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->InsertNewChildElement(saveString.c_str());
	element->SetAttribute("x", value.x);
	element->SetAttribute("y", value.y);
	return true;
}

template<>
inline bool VectorParameter<glm::vec2>::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->FirstChildElement(saveString.c_str());
	if(XML_SUCCESS != element->QueryAttribute("x", &value.x) ||
	   XML_SUCCESS != element->QueryAttribute("y", &value.y)){
		return Logger::warn("Could not load parameter {}", name);
	}
	displayValue = value;
	return true;
}




template<>
inline void VectorParameter<glm::vec3>::inputField(){
	ImGui::InputFloat3(imguiID.c_str(), &displayValue.x, format);
}

template<>
inline bool VectorParameter<glm::vec3>::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->InsertNewChildElement(saveString.c_str());
	element->SetAttribute("x", value.x);
	element->SetAttribute("y", value.y);
	element->SetAttribute("z", value.z);
	return true;
}

template<>
inline bool VectorParameter<glm::vec3>::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->FirstChildElement(saveString.c_str());
	if(XML_SUCCESS != element->QueryAttribute("x", &value.x) ||
	   XML_SUCCESS != element->QueryAttribute("y", &value.y) ||
	   XML_SUCCESS != element->QueryAttribute("z", &value.z)){
		return Logger::warn("Could not load parameter {}", name);
	}
	displayValue = value;
	return true;
}



template<>
inline void VectorParameter<glm::vec4>::inputField(){
	ImGui::InputFloat4(imguiID.c_str(), &displayValue.x, format);
}

template<>
inline bool VectorParameter<glm::vec4>::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->InsertNewChildElement(saveString.c_str());
	element->SetAttribute("x", value.x);
	element->SetAttribute("y", value.y);
	element->SetAttribute("z", value.z);
	element->SetAttribute("w", value.w);
	return true;
}

template<>
inline bool VectorParameter<glm::vec4>::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* element = xml->FirstChildElement(saveString.c_str());
	if(XML_SUCCESS != element->QueryAttribute("x", &value.x) ||
	   XML_SUCCESS != element->QueryAttribute("y", &value.y) ||
	   XML_SUCCESS != element->QueryAttribute("z", &value.z) ||
	   XML_SUCCESS != element->QueryAttribute("w", &value.w)){
		return Logger::warn("Could not load parameter {}", name);
	}
	displayValue = value;
	return true;
}




//===============================================================================
//=================================== BOOLEANS ==================================
//===============================================================================

class BooleanParameter : public Parameter, public std::enable_shared_from_this<BooleanParameter>{
public:
	
	bool displayValue;
	bool value;
	
	BooleanParameter(bool value_, std::string name_, std::string saveString_) : Parameter(name_, saveString_) {
		displayValue = value_;
		value = value_;
	}
	
	virtual void gui(){
		ImGui::Checkbox(imguiID.c_str(), &displayValue);
		if(ImGui::IsItemDeactivatedAfterEdit() && value != displayValue){
			//=========Command Invoker=========
			std::shared_ptr<BooleanParameter> thisParameter = shared_from_this();
			CommandHistory::pushAndExecute(std::make_shared<InvertCommand>(thisParameter));
			//=================================
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml){
		xml->SetAttribute(saveString.c_str(), value);
		return true;
	}
	
	virtual bool load(tinyxml2::XMLElement* xml){
		using namespace tinyxml2;
		XMLError result = xml->QueryBoolAttribute(saveString.c_str(), &value);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", name);
		displayValue = value;
		return true;
	}
	
	class InvertCommand : public Command{
	public:
		std::shared_ptr<BooleanParameter> parameter;
		InvertCommand(std::shared_ptr<BooleanParameter> parameter_){
			parameter = parameter_;
			name = "Inverted \'" + std::string(parameter->name) + "\'";
		}
		void invert(){
			parameter->value = !parameter->value;
			parameter->displayValue = parameter->value;
			parameter->b_changed = true;
		}
		virtual void execute(){
			invert();
			if(parameter->editCallback) parameter->editCallback(parameter->editCallbackPayload);
		}
		virtual void undo(){
			invert();
			if(parameter->editCallback) parameter->editCallback(parameter->editCallbackPayload);
		}
	};
	
};




//===============================================================================
//=================================== STRINGS ===================================
//===============================================================================

class StringParameter : public Parameter, public std::enable_shared_from_this<StringParameter>{
public:
	
	char* displayValue;
	std::string value;
	size_t bufferSize;
	
	StringParameter(std::string value_, std::string name_, std::string saveString_, size_t bufferSize_) : Parameter(name_, saveString_){
		value = value_;
		bufferSize = bufferSize_;
		displayValue = new char[bufferSize];
		strcpy(displayValue, value_.c_str());
	}
	
	void overwrite(const char* value_){
		strcpy(displayValue, value_);
		value = value_;
	}
	
	virtual void gui(){
		ImGui::InputText(imguiID.c_str(), displayValue, bufferSize);
		if(ImGui::IsItemDeactivatedAfterEdit() && strcmp(displayValue, value.c_str()) != 0){
			//=========Command Invoker=========
			std::shared_ptr<StringParameter> thisParameter = shared_from_this();
			CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter));
			//=================================
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml){
		xml->SetAttribute(saveString.c_str(), value.c_str());
		return true;
	}
	
	virtual bool load(tinyxml2::XMLElement* xml){
		using namespace tinyxml2;
		XMLError result = xml->QueryStringAttribute(saveString.c_str(), (const char**)&displayValue);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", name);
		strcpy(displayValue, value.c_str());
		return true;
	}
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<StringParameter> parameter;
		std::string newValue;
		std::string previousValue;
		EditCommand(std::shared_ptr<StringParameter> parameter_){
			parameter = parameter_;
			newValue = parameter->displayValue;
			previousValue = parameter->value;
			name = "Changed \'" + std::string(parameter->name) + "\' from \'" + previousValue + "\' to \'" + newValue + "\'";
		}
		virtual void execute(){
			parameter->value = newValue;
			strcpy(parameter->displayValue, newValue.c_str());
			parameter->b_changed = true;
			if(parameter->editCallback) parameter->editCallback(parameter->editCallbackPayload);
		}
		virtual void undo(){
			parameter->value = previousValue;
			strcpy(parameter->displayValue, previousValue.c_str());
			parameter->b_changed = true;
			if(parameter->editCallback) parameter->editCallback(parameter->editCallbackPayload);
		}
	};
	
};



//===============================================================================
//================================= ENUMERATORS =================================
//===============================================================================

template <typename T>
class EnumeratorParameter : public Parameter, public std::enable_shared_from_this<EnumeratorParameter<T>>{
public:
	T value;
	T displayValue;
	
	EnumeratorParameter(T value_, std::string name, std::string saveString_) : Parameter(name, saveString_){
		value = value_;
		displayValue = value_;
	}
	
	virtual void gui(){
		if(ImGui::BeginCombo(imguiID.c_str(), Enumerator::getDisplayString(displayValue))){
			for(auto& type : Enumerator::getTypes<T>()){
				if(ImGui::Selectable(type.displayString, type.enumerator == displayValue)){
					displayValue = type.enumerator;
					//=========Command Invoker=========
					std::shared_ptr<EnumeratorParameter<T>> thisParameter = this->shared_from_this();
					CommandHistory::pushAndExecute(std::make_shared<EditCommand>(thisParameter));
					//=================================
				}
			}
			ImGui::EndCombo();
		}
	}
	
	virtual bool save(tinyxml2::XMLElement* xml){
		xml->SetAttribute(saveString.c_str(), Enumerator::getSaveString(value));
	}
	
	virtual bool load(tinyxml2::XMLElement* xml){
		using namespace tinyxml2;
		const char * enumeratorSaveString;
		XMLError result = xml->QueryStringAttribute(saveString.c_str(), &enumeratorSaveString);
		if(result != XML_SUCCESS) return Logger::warn("Could not load parameter {}", name);
		if(!Enumerator::isValidSaveName<T>(enumeratorSaveString)) return Logger::warn("Could not load parameter {} : Invalid Enumerator Save String \'{}\'", name, enumeratorSaveString);
		value = Enumerator::getEnumeratorFromSaveString<T>(enumeratorSaveString);
		displayValue = value;
		return true;
	}
	
	class EditCommand : public Command{
	public:
		std::shared_ptr<EnumeratorParameter<T>> parameter;
		T oldValue;
		T newValue;
		EditCommand(std::shared_ptr<EnumeratorParameter<T>> parameter_){
			parameter = parameter_;
			oldValue = parameter->value;
			newValue = parameter->displayValue;
			name = "Edited \'" + std::string(parameter->name)
			+ "\' from \'" + std::string(Enumerator::getDisplayString(oldValue))
			+ "\' to \'" + std::string(Enumerator::getDisplayString(newValue)) + "\'";
		}
		virtual void execute(){
			parameter->value = newValue;
			parameter->displayValue = newValue;
			parameter->b_changed = true;
			if(parameter->editCallback) parameter->editCallback(parameter->editCallbackPayload);
		}
		virtual void undo(){
			parameter->value = oldValue;
			parameter->displayValue = oldValue;
			parameter->b_changed = true;
			if(parameter->editCallback) parameter->editCallback(parameter->editCallbackPayload);
		}
	};
	
};




/*




class EnumStruct_Base{
public:
	char displayString[64];
	char saveString[64];
};

class Enum_Base{
public:
	
	virtual std::vector<EnumStruct_Base>& getTypes() = 0;
	
	EnumStruct_Base& getStructureFromSaveString(const char* saveString){
		
	}
	
};




template<typename T>
class EnumStruct_Derived : public EnumStruct_Base{
public:
	T enumerator;
};


template<typename T>
class Enum_Derived : public Enum_Base{
public:
	
	virtual std::vector<EnumStruct_Base>& getTypes(){
		static std::vector<EnumStruct_Base> baseTypes;
		return baseTypes;
	}
	
	void init(std::vector<EnumStruct_Derived<T>> derivedTypes){
		for(auto& derivedType : derivedTypes){
			getTypes().push_back(derivedType);
		}
	}
	
	
	
	
	
};



//#define DEFINE_NEW_ENUM(TypeName, TypeStructures)\
//template class Enum_Derived::<TypeName>\

enum class Numbers{
	ONE,
	TWO,
	THREE
};

template class EnumStruct_Derived<Numbers>;
template class Enum_Derived<Numbers>;

void initTest(){
	Enum_Derived<Numbers> test;

	test.init({
		{.enumerator = Numbers::ONE, .displayString = "one", .saveString = "two"}
	});
}

*/









#include "Machine/AnimatableParameterValue.h"
#include "Motion/Curve/Curve.h"
#include "Motion/MotionTypes.h"

class Machine;
class ParameterTrack;
namespace tinyxml2 { class XMLElement; }



class ParameterGroup_B;
template<typename T>
class AnimatableParameter_B;

class MachineParameter_B : public std::enable_shared_from_this<MachineParameter_B>{
public:
	MachineParameter_B(const char* name_){ strcpy(name, name_); }
	
	virtual bool isParentGroup() = 0;
	bool hasParentGroup() { return parentParameterGroup != nullptr; }
	void setParentGroup(std::shared_ptr<ParameterGroup_B> parent){ parentParameterGroup = parent; }
	
	const char* getName(){ return name; }
	void setMachine(std::shared_ptr<Machine> machine_){ machine = machine_; }
	std::shared_ptr<Machine> getMachine(){ return machine; }
	
	static std::shared_ptr<ParameterGroup_B> castToGroup(std::shared_ptr<MachineParameter_B> input){
		return std::dynamic_pointer_cast<ParameterGroup_B>(input);
	}
	
	template<typename T>
	static std::shared_ptr<AnimatableParameter_B<T>> castToAnimatable(std::shared_ptr<MachineParameter_B> input){
		return std::dynamic_pointer_cast<AnimatableParameter_B<T>>(input);
	}

private:
	char name[256];
	std::shared_ptr<Machine> machine;
	std::shared_ptr<ParameterGroup_B> parentParameterGroup = nullptr;
};


class ParameterGroup_B : public MachineParameter_B{
public:
	
	ParameterGroup_B(const char* name, std::vector<std::shared_ptr<MachineParameter_B>> children) : MachineParameter_B(name), childParameters(children){
		for(auto& childParameter : childParameters){
			auto thisGroup = std::dynamic_pointer_cast<ParameterGroup_B>(shared_from_this());
			childParameter->setParentGroup(thisGroup);
		}
	}
	
	virtual bool isParentGroup(){ return true; }
	
	std::vector<std::shared_ptr<MachineParameter_B>>& getChildren(){ return childParameters; }

private:
	std::vector<std::shared_ptr<MachineParameter_B>> childParameters;
};


template<typename T>
class AnimatableParameter_B : public MachineParameter_B{
public:
	
	AnimatableParameter_B(const char* name) : MachineParameter_B(name) {}
	
	virtual std::vector<Motion::InterpolationType>& getCompatibleInterpolationTypes(){
		static std::vector<Motion::InterpolationType> test;
		return test;
	}

	virtual bool isParentGroup(){ return false; }
	
	bool hasParameterTrack() { return actualParameterTrack != nullptr; }
	T getActiveTrackParameterValue(){
		//TODO: IMPORTANT !!!
		//return actualParameterTrack->getParameterValueAtPlaybackTime();
		return {};
	}
	
private:
	std::shared_ptr<ParameterTrack> actualParameterTrack = nullptr;
};


static void test(){
	std::shared_ptr<AnimatableParameter_B<bool>> test = std::make_shared<AnimatableParameter_B<bool>>("test");
	std::shared_ptr<MachineParameter_B> base;
	base = test;
	
	MachineParameter_B::castToAnimatable<bool>(base);
	
	
}






