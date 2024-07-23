#include "EsiReader.h"

#include "Gui/Utilities/FileDialog.h"
#include <imgui.h>
#include <tinyxml2.h>

#include "Gui/Assets/Fonts.h"

class DocumentNode{
public:
	
	std::string name;
	std::string content;
	
	std::vector<std::shared_ptr<DocumentNode>> nodes;
	
	void addNode(std::shared_ptr<DocumentNode> node){ nodes.push_back(node); }
	std::vector<std::shared_ptr<DocumentNode>>& getNodes(){ return nodes; }
	size_t getNodeCount(){ return nodes.size(); }
	bool hasChildren(){ return !nodes.empty(); }
	
	static std::shared_ptr<DocumentNode> make(const char* name, const char* content = nullptr){
		auto node = std::make_shared<DocumentNode>();
		node->name = name;
		if(content) node->content = content;
		return node;
	}
	
};

std::vector<FileDialog::FileTypeFilter> esiFileFilters = {
	FileDialog::FileTypeFilter("XML File", "xml"),
	FileDialog::FileTypeFilter("ESI File", "esi")
};


class EtherCatDataTypeInfo{
public:
	
	std::string name;
	
	static std::shared_ptr<EtherCatDataTypeInfo> load(tinyxml2::XMLElement* dataTypeXML){
		auto dataType = std::make_shared<EtherCatDataTypeInfo>();
		
		const char* n = dataTypeXML->FirstChildElement("Name")->GetText();
		if(n) dataType->name = n;
		
		return dataType;
	}
	
};

class EtherCatObjectInfo{
public:
	
	std::string name;
	std::string index;
	std::string type;
	std::string bitsize;
	
	static std::shared_ptr<EtherCatObjectInfo> load(tinyxml2::XMLElement* objectXML){
		auto object = std::make_shared<EtherCatObjectInfo>();
		
		const char* nm = objectXML->FirstChildElement("Name")->GetText();
		if(nm) object->name = nm;
		
		const char* idx = objectXML->FirstChildElement("Index")->GetText();
		if(idx) object->index = idx;
		
		const char* tp = objectXML->FirstChildElement("Type")->GetText();
		if(tp) object->type = tp;
		
		const char* bs = objectXML->FirstChildElement("BitSize")->GetText();
		if(tp) object->bitsize = bs;
		
		return object;
	}
};

class EtherCatDeviceInfo{
public:
	
	std::string name;
	std::string physics;
	std::vector<std::shared_ptr<EtherCatDataTypeInfo>> dataTypes;
	std::vector<std::shared_ptr<EtherCatObjectInfo>> objects;
	
	static std::shared_ptr<EtherCatDeviceInfo> load(tinyxml2::XMLElement* deviceXML){
		auto device = std::make_shared<EtherCatDeviceInfo>();
		using namespace tinyxml2;
		
		const char* n = deviceXML->FirstChildElement("Name")->GetText();
		if(n) device->name = n;
		
		const char* physics_;
		deviceXML->QueryStringAttribute("Physics", &physics_);
		if(physics_) device->physics = physics_;
		
		XMLElement* profileXML = deviceXML->FirstChildElement("Profile");
		XMLElement* dictionnaryXML = profileXML->FirstChildElement("Dictionary");
		
		XMLElement* dataTypesXML = dictionnaryXML->FirstChildElement("DataTypes");
		XMLElement* dataTypeXML = dataTypesXML->FirstChildElement("DataType");
		while(dataTypeXML){
			device->dataTypes.push_back(EtherCatDataTypeInfo::load(dataTypeXML));
			dataTypeXML = dataTypeXML->NextSiblingElement("DataType");
		}
		
		XMLElement* objectsXML = dictionnaryXML->FirstChildElement("Objects");
		XMLElement* objectXML = objectsXML->FirstChildElement("Object");
		while(objectXML){
			device->objects.push_back(EtherCatObjectInfo::load(objectXML));
			objectXML = objectXML->NextSiblingElement("Object");
		}
		
		return device;
	}
	
	void draw(){
		ImGui::Text("Physics %s", physics.c_str());
		if(ImGui::CollapsingHeader("Data Types")){
			ImGui::TreePush("DataT");
			for(auto& dataType : dataTypes) ImGui::Text("%s", dataType->name.c_str());
			ImGui::TreePop();
		}
		if(ImGui::CollapsingHeader("Objects")){
			ImGui::TreePush("Obj");
			for(auto& object : objects){
				ImGui::PushFont(Fonts::sansBold15);
				ImGui::Text("%s", object->name.c_str());
				ImGui::PopFont();
				ImGui::SameLine();
				ImGui::Text("%s  %s  %s", object->index.c_str(), object->type.c_str(), object->bitsize.c_str());
			}
			ImGui::TreePop();
		}
	}
	
};

class ESI_File{
public:
	
	std::string vendorString;
	std::string vendorCode;
	
	std::vector<std::shared_ptr<EtherCatDeviceInfo>> devices;
	
	
	static std::shared_ptr<ESI_File> load(const char* filePath){
		auto esi = std::make_shared<ESI_File>();
		using namespace tinyxml2;
		
		XMLDocument file;
		XMLError result = file.LoadFile(filePath);
		if(result != tinyxml2::XML_SUCCESS) return nullptr;
		XMLElement* etherCATinfo = file.FirstChildElement("EtherCATInfo");
		
		
		XMLElement* vendorXML = etherCATinfo->FirstChildElement("Vendor");
		if(!vendorXML) return nullptr;
		
		esi->vendorCode = vendorXML->FirstChildElement("Id")->GetText();
		esi->vendorString = vendorXML->FirstChildElement("Name")->GetText();
		
		
		XMLElement* descriptionsXML = etherCATinfo->FirstChildElement("Descriptions");
		if(!descriptionsXML) return nullptr;
		
		XMLElement* devicesXML = descriptionsXML->FirstChildElement("Devices");
		if(!devicesXML) return nullptr;
		
		XMLElement* deviceXML = devicesXML->FirstChildElement("Device");
		while(deviceXML){
			esi->devices.push_back(EtherCatDeviceInfo::load(deviceXML));
			deviceXML = deviceXML->NextSiblingElement("Device");
		}
		
		
		return esi;
	}
	
};





std::shared_ptr<DocumentNode> esiDocument;
std::shared_ptr<ESI_File> esiFile;

void exploreElement(tinyxml2::XMLElement* element, std::shared_ptr<DocumentNode> parentNode){
	using namespace tinyxml2;
	XMLElement* child = element->FirstChildElement();
	while(child){
		auto node = DocumentNode::make(child->Value(), child->GetText());
		parentNode->addNode(node);
		exploreElement(child, node);
		child = child->NextSiblingElement();
	}
}

void loadESI(const char* path){
	using namespace tinyxml2;
	XMLDocument file;
	XMLError result = file.LoadFile(path);
	if(result != XML_SUCCESS) {
		Logger::warn("could not open esi file");
		return;
	}
	XMLElement* etherCATinfo = file.FirstChildElement("EtherCATInfo");
	esiDocument = DocumentNode::make("ESI File");
	exploreElement(etherCATinfo, esiDocument);
	
	esiFile = ESI_File::load(path);
}



void displayNodeContent(std::shared_ptr<DocumentNode> node, bool topNode = true){
	if(!topNode) ImGui::TreePush("TopNode");
	if(!node->content.empty()) ImGui::TextWrapped("Content: %s", node->content.c_str());
	auto& childNodes = node->getNodes();
	for(int i = 0; i < childNodes.size(); i++){
		ImGui::PushID(i);
		auto childNode = childNodes[i];
		if(childNode->hasChildren()){
			if(ImGui::CollapsingHeader(childNodes[i]->name.c_str())){
				displayNodeContent(childNodes[i], false);
			}
		}else{
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("%s", childNode->name.c_str());
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::TextWrapped("%s", childNode->content.c_str());
		}
		ImGui::PopID();
	}
	if(!topNode) ImGui::TreePop();
}


void EsiReaderWindow::onDraw(){
	
	static FileDialog::FilePath filePath;
	if(ImGui::Button("Open ESI File")) {
		loadESI("/Users/leobecker/Desktop/ABB MicroFlex e190 Build 5904.6 (CoE).xml");
		//if(FileDialog::load(filePath, esiFileFilters)) loadESI(filePath.path);
	}
	
	if(ImGui::BeginTabBar("esiTabBar")){
		
		if(ImGui::BeginTabItem("ESI File")){
			ImGui::BeginChild("##Esi", ImGui::GetContentRegionAvail(), true);
			if(esiDocument) displayNodeContent(esiDocument);
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		
		if(esiFile){
			for(auto& device : esiFile->devices){
				if(ImGui::BeginTabItem(device->name.c_str())){
					device->draw();
					ImGui::EndTabItem();
				}
			}
		}
		
		ImGui::EndTabBar();
	}
	
}
