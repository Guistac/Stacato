#include "CommandZ.h"

#include <imgui.h>

#include "Project/Editor/CommandHistory.h"
#include "Project/Editor/Parameter.h"
#include "Project/Editor/List.h"

enum class NumberEnum{
	ONE,
	TWO,
	THREE,
	FOUR,
	FIVE
};

#define NumberEnumStrings \
{NumberEnum::ONE, 	.saveString = "ONE", 	.displayString = "One"},\
{NumberEnum::TWO, 	.saveString = "TWO", 	.displayString = "Two"},\
{NumberEnum::THREE, .saveString = "THREE", 	.displayString = "Three"},\
{NumberEnum::FOUR, 	.saveString = "FOUR", 	.displayString = "Four"},\
{NumberEnum::FIVE, 	.saveString = "FIVE", 	.displayString = "Five"}\

DEFINE_ENUMERATOR(NumberEnum, NumberEnumStrings)


struct Thing{
	int randomNumber;
};
std::vector<std::shared_ptr<Thing>> things;

class AddThingCommand : public UndoableCommand{
public:
	
	std::vector<std::shared_ptr<Thing>>* thingList;
	std::vector<std::shared_ptr<Thing>> addedThings;

	AddThingCommand(std::vector<std::shared_ptr<Thing>>* thingList_, std::vector<std::shared_ptr<Thing>> addedThings_) :
	UndoableCommand("Added " + std::to_string(addedThings.size()) + " Things"){
		thingList = thingList_;
		addedThings = addedThings_;
	}
	
	virtual void onExecute(){
		thingList->insert(thingList->end(), addedThings.begin(), addedThings.end());
	}
	
	virtual void onUndo(){
		for(int i = 0; i < addedThings.size(); i++){
			std::shared_ptr<Thing> deletedThing = addedThings[i];
			for(int j = 0; j < thingList->size(); j++){
				if(thingList->at(j) == deletedThing){
					thingList->erase(thingList->begin() + j);
					break;
				}
			}
		}
	}
};





void testUndoHistory(){
	/*
	static std::shared_ptr<List<std::shared_ptr<Parameter>>> parameterList;
	
	static bool b_init = false;
	if(!b_init){
		b_init = true;
		
		std::vector<std::shared_ptr<Parameter>> parameters{
			NumberParameter<int>::make(1, "test")
			//std::make_shared<NumberParameter<int>>(1, "integer 0", "IntParam1", 1, 10, "%i added things"),
			
			std::make_shared<NumberParameter<int>>(2, "integer 1", "IntParam2"),
			std::make_shared<NumberParameter<int>>(3, "integer 2", "IntParam3", 1, 10),
			std::make_shared<NumberParameter<int>>(4, "integer 3", "IntParam4", 1, 10),
				
			//NumberParameter<int>::make(1, "integer 0", "IntParam1", 1, 10, "", )
			
			std::make_shared<NumberParameter<double>>(1.0, "parameter 0", "DoubleParam0", Units::Time::Week, 0.1, 1.0),
			std::make_shared<NumberParameter<double>>(2.0, "parameter 1", "DoubleParam1", Units::None::None, 0.1, 1.0, "%.3f lel"),
			std::make_shared<NumberParameter<double>>(3.0, "parameter 2", "DoubleParam2", Units::None::None, 0.1, 1.0, "%.9f"),
			std::make_shared<NumberParameter<double>>(4.0, "parameter 3", "DoubleParam3", Units::Mass::Kilogram, 0.1, 1.0, "Mass: %.3f"),
			std::make_shared<NumberParameter<double>>(5.0, "parameter 4", "DoubleParam4", Units::None::None, 0.1, 1.0, "%.3f"),
		 
			std::make_shared<BooleanParameter>(true, "boolean 0", "BoolParam0"),
			std::make_shared<BooleanParameter>(true, "boolean 1", "BoolParam1"),
			std::make_shared<BooleanParameter>(true, "boolean 2", "BoolParam2"),
			std::make_shared<BooleanParameter>(true, "boolean 3", "BoolParam3"),
			
			std::make_shared<StringParameter>("String One", 	"string 0", "StringParam0", 256),
			std::make_shared<StringParameter>("String Two", 	"string 1", "StringParam1", 256),
			std::make_shared<StringParameter>("String Three", 	"string 2", "StringParam2", 256),
			std::make_shared<StringParameter>("String Four", 	"string 3", "StringParam3", 256),
			
			std::make_shared<EnumeratorParameter<NumberEnum>>(NumberEnum::ONE, 		"Enum Parameter 1", "EnumParam0"),
			std::make_shared<EnumeratorParameter<NumberEnum>>(NumberEnum::TWO, 		"Enum Parameter 2", "EnumParam1"),
			std::make_shared<EnumeratorParameter<NumberEnum>>(NumberEnum::THREE, 	"Enum Parameter 3", "EnumParam2"),
			std::make_shared<EnumeratorParameter<NumberEnum>>(NumberEnum::FOUR, 	"Enum Parameter 4", "EnumParam3"),
			
			std::make_shared<VectorParameter<glm::vec2>>(glm::vec2(1.2, 3.4), 			"Vec2 Parameter 1", "Vec2Param0"),
			std::make_shared<VectorParameter<glm::vec3>>(glm::vec3(5.6, 7.8, 9.0), 		"Vec3 Parameter 2", "Vec2Param1"),
			std::make_shared<VectorParameter<glm::vec4>>(glm::vec4(0.0, 1.0, 2.0, 3.0), "Vec4 Parameter 3", "Vec2Param2")
			 
		};
		parameterList = std::make_shared<List<std::shared_ptr<Parameter>>>(parameters);
		
		parameters.front()->setEditCallback([](std::shared_ptr<Parameter> parameter){
			auto param = std::static_pointer_cast<NumberParameter<int>>(parameter);
			int num = param->value;
			
			std::vector<std::shared_ptr<Thing>> addedThings;
			for(int i = 0; i < num; i++){
				auto newThing = std::make_shared<Thing>();
				newThing->randomNumber = num * 10000 + Random::getNormalized() * 1000;
				addedThings.push_back(newThing);
			}
			std::vector<std::shared_ptr<Thing>>* list = &things;
			std::make_shared<AddThingCommand>(list, addedThings)->execute();
			
			if(num == 2){
				std::vector<std::shared_ptr<Thing>> moreThings;
				for(int i = 0; i < 3; i++){
					auto newThing = std::make_shared<Thing>();
					newThing->randomNumber = 333 * 10000 + Random::getNormalized() * 1000;
					moreThings.push_back(newThing);
				}
				std::vector<std::shared_ptr<Thing>>* list = &things;
				std::make_shared<AddThingCommand>(list, moreThings)->execute();
			}
		});
	}

	ImVec2 sizeHalf = ImGui::GetContentRegionAvail();
	sizeHalf.x /= 2.0;
	ImGui::BeginChild("ParameterList", sizeHalf);
	
	ImGui::Text("Parameter List :");
	if (ImGui::Button("Add Parameter")) ImGui::OpenPopup("AddParameterPopup");
	if (ImGui::BeginPopup("AddParameterPopup")) {
		if(ImGui::MenuItem("Double")){
			std::string name = "Double Parameter " + std::to_string(parameterList->size());
			parameterList->addElement(NumberParameter<double>::make(0.0, name, name));
		}
		if(ImGui::MenuItem("Float")){
			std::string name = "Float Parameter " + std::to_string(parameterList->size());
			parameterList->addElement(NumberParameter<float>::make(0.0f, name, name));
		}
		ImGui::Separator();
		if(ImGui::MenuItem("Integer")){
			std::string name = "Integer Parameter " + std::to_string(parameterList->size());
			parameterList->addElement(NumberParameter<int>::make(0, name, name));
		}
		if(ImGui::MenuItem("uint8_t")){
			std::string name = "uint8_t Parameter " + std::to_string(parameterList->size());
			parameterList->addElement(NumberParameter<uint8_t>::make(0, name, name));
		}
		if(ImGui::MenuItem("int8_t")){
			std::string name = "int8_t Parameter " + std::to_string(parameterList->size());
			parameterList->addElement(NumberParameter<int8_t>::make(0, name, name));
		}

		ImGui::Separator();
		if(ImGui::MenuItem("Boolean")){
			std::string name = "Boolean Parameter " + std::to_string(parameterList->size());
			parameterList->addElement(std::make_shared<BooleanParameter>(false, name, name));
		}
		
		
		ImGui::Separator();
		if(ImGui::MenuItem("String")){
			std::string name = "String Parameter " + std::to_string(parameterList->size());
			parameterList->addElement(std::make_shared<StringParameter>("", name, name, 256));
		}
		ImGui::Separator();
		
		
		if(ImGui::MenuItem("Enumerator")){
			std::string name = "Enumerator Parameter " + std::to_string(parameterList->size());
			parameterList->addElement(std::make_shared<EnumeratorParameter<NumberEnum>>(NumberEnum::ONE, name, name));
		}
		
		ImGui::EndPopup();
	}
	
	ImGui::Separator();
	
	if(parameterList->beginList()){
		for(int i = 0; i < parameterList->size(); i++){
			auto& parameter = parameterList->beginElement(i, true, true);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(200.0f);
			parameter->gui();
			parameterList->endElement();
		}
		
		parameterList->endList();
	}
	
	ImGui::EndChild();
	
	//----------------------------------------------------------------
	
	ImGui::SameLine();
	ImGui::BeginChild("History", ImGui::GetContentRegionAvail());
	ImGui::BeginDisabled(!CommandHistory::canUndo());
	if(ImGui::Button("Undo")) CommandHistory::undo();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!CommandHistory::canRedo());
	if(ImGui::Button("Redo")) CommandHistory::redo();
	ImGui::EndDisabled();
	
	if(ImGui::BeginListBox("##Things")){
		
		for(auto& thing : things){
			static char name[64];
			sprintf(name, "Thing: %i", thing->randomNumber);
			ImGui::Selectable(name);
		}
		
		ImGui::EndListBox();
	}
	
	if(ImGui::BeginListBox("##History", ImGui::GetContentRegionAvail())){
		int topIndex = CommandHistory::getUndoableCommandCount();
		std::vector<std::shared_ptr<UndoableCommand>>& history = CommandHistory::get();
		bool b_disabled = false;
		for(int i = 0; i < history.size(); i++){
			if(i == topIndex){
				ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(.5f, .5f, .5f, 1.f));
				b_disabled = true;
			}
			
			auto& command = history[i];
			
			if(command->getSideEffects().empty()) ImGui::Selectable(command->getName().c_str());
			else{
				if(ImGui::TreeNode(command->getName().c_str())){
					
					for(auto& sideEffect : command->getSideEffects()){
						ImGui::Selectable(sideEffect->getName().c_str());
					}
					
					ImGui::TreePop();
				}
			}
		}
		if(b_disabled) ImGui::PopStyleColor();
		
		ImGui::EndListBox();
	}
	
	ImGui::EndChild();
	*/
}


 



