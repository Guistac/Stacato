#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/EtherCatPDO.h"

#define DEFINE_MODULAR_DEVICE_MODULE(className, saveName, displayName, u32_identifier) \
public:\
	virtual const char* getSaveName() override { return saveName; }\
	virtual const char* getDisplayName() override { return displayName; }\
	virtual uint32_t getIdentifier() override { return u32_identifier; }\
	\
	virtual std::shared_ptr<EtherCAT::ModularDeviceProfile::ChildModule> getNewInstance() override {\
	   auto newModule = std::make_shared<className>();\
	   newModule->onConstruction();\
	   return newModule;\
	}\
	virtual void onConstruction() override;\
	virtual void onSetIndex(int i) override;\
	\
	virtual bool configureParameters() override;\
	virtual void addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement) override;\
	virtual void addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement) override;\
	virtual void readInputs() override;\
	virtual void writeOutputs() override;\
	\
	virtual bool save(tinyxml2::XMLElement* xml) override;\
	virtual bool load(tinyxml2::XMLElement* xml) override;\
	\
	virtual void moduleGui() override;\


namespace EtherCAT::ModularDeviceProfile{

	class ChildModule;

	class ParentDevice : public EtherCatDevice{
	public:
		
		//modules and module management
		std::vector<std::shared_ptr<ChildModule>> modules = {};
		
		//module list management
		void addModule(std::shared_ptr<ChildModule> module);
		void removeModule(std::shared_ptr<ChildModule> module);
		void reorderModule(int oldIndex, int newIndex);
		void moveModuleUp(std::shared_ptr<ChildModule> module);
		void moveModuleDown(std::shared_ptr<ChildModule> module);
		
		virtual void beforeModuleReordering(){}
		
		//module configuration
		bool discoverChildModules();
		DataTransferState moduleDiscoveryStatus = DataTransferState::NO_TRANSFER;
		bool configureModules();
		
		//module process data
		void readModuleInputs();
		void writeModuleOutputs();
		
		bool saveModules(tinyxml2::XMLElement* xml);
		bool loadModules(tinyxml2::XMLElement* xml);
		
		//module factory
		virtual std::vector<ChildModule*>& getModuleFactory() = 0;
		std::shared_ptr<ChildModule> createModule(uint32_t identifier);
		std::shared_ptr<ChildModule> createModule(const char* saveString);
		
		//module management gui
		void moduleManagerGui();
		std::shared_ptr<ChildModule> selectedModule = nullptr;
	};


	class ChildModule : public std::enable_shared_from_this<ChildModule>{
	public:

		//module name as reported by the bus coupler
		virtual const char* getSaveName() = 0;
		//module name for gui display
		virtual const char* getDisplayName() = 0;
		//module u32 identifier
		virtual uint32_t getIdentifier() = 0;
		
		virtual std::shared_ptr<ChildModule> getNewInstance() = 0;
		virtual void onConstruction() = 0;

		std::shared_ptr<ParentDevice> parentDevice = nullptr;
		void setParentDevice(std::shared_ptr<ParentDevice> parent){
			parentDevice = parent;
			onSetParentDevice(parent);
		}
		virtual void onSetParentDevice(std::shared_ptr<ParentDevice> parent) {}
		
		int moduleIndex = -1;
		void setIndex(int i){
			moduleIndex = i;
			onSetIndex(i);
		}
		virtual void onSetIndex(int i) = 0;
		
		//configures parameters during fieldbus start
		virtual bool configureParameters() = 0;
		
		//configure PDOs
		virtual void addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement) = 0;
		virtual void addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement) = 0;
		
		//reads data from input data and assigns it to pins
		virtual void readInputs() = 0;
		int inputByteCount = 0;
		int inputBitCount = 0;
		
		//reads data from pins and output data
		virtual void writeOutputs() = 0;
		int outputByteCount = 0;
		int outputBitCount = 0;
		
		virtual void onConnection(){}
		virtual void onDisconnection(){}
		
		std::vector<std::shared_ptr<NodePin>> inputPins; //input pins are device outputs
		std::vector<std::shared_ptr<NodePin>> outputPins; //output pins are device inputs
		
		//gui stuff
		virtual void moduleGui() = 0;
		
		//saving and loading
		virtual bool save(tinyxml2::XMLElement* xml){ return true; }
		virtual bool load(tinyxml2::XMLElement* xml){ return true; }
	};


};
