#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/EtherCatPDO.h"

#define DEFINE_MODULAR_DEVICE_MODULE(className, modelNameString, u32_identifier) \
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(className)\
public:\
	virtual std::string getClassName() override { return #className; }\
	virtual std::string getModelName() override { return modelNameString; };\
	virtual uint32_t getIdentifier() override { return u32_identifier; }\
	virtual void onSetIndex(int i) override;\
	virtual bool configureParameters() override;\
	virtual void addTxPdoMappingModule(EtherCatPdoAssignement& txPdoAssignement) override;\
	virtual void addRxPdoMappingModule(EtherCatPdoAssignement& rxPdoAssignement) override;\
	virtual void readInputs() override;\
	virtual void writeOutputs() override;\
	virtual void moduleGui() override;\

namespace EtherCAT::ModularDeviceProfile{

	class ModularDevice;

	class DeviceModule : public Legato::Component{
		//DECLARE_PROTOTYPE_INTERFACE_METHODS(DeviceModule)
		
	public:
		
		virtual bool onSerialization() override;
		
		virtual bool onDeserialization() override;
		
		virtual void onConstruction() override {
			Component::onConstruction();
		}
		
		virtual void onCopyFrom(std::shared_ptr<Prototype> source) override {
			Component::onCopyFrom(source);
		}

		//module u32 identifier
		virtual uint32_t getIdentifier() = 0;
		virtual std::string getModelName(){}
		virtual std::string getClassName(){}

		std::shared_ptr<ModularDevice> parentDevice = nullptr;
		void setParentDevice(std::shared_ptr<ModularDevice> parent){
			parentDevice = parent;
			onSetParentDevice(parent);
		}
		virtual void onSetParentDevice(std::shared_ptr<ModularDevice> parent) {}
		
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
	};



	class ModularDevice : public EtherCatDevice{
	public:
		
		virtual bool onSerialization() override;
		
		virtual bool onDeserialization() override;
		
		virtual void onConstruction() override;
		
		virtual void onCopyFrom(std::shared_ptr<Prototype> source) override {
			//EtherCatDevice::onCopyFrom(source);
		}
		
		//module list management
		std::vector<std::shared_ptr<DeviceModule>>& getModules(){ return moduleList->getEntries(); }
		void addModule(std::shared_ptr<DeviceModule> module);
		void removeModule(std::shared_ptr<DeviceModule> module);
		void reorderModule(int oldIndex, int newIndex);
		void moveModuleUp(std::shared_ptr<DeviceModule> module);
		void moveModuleDown(std::shared_ptr<DeviceModule> module);
		
		virtual void beforeModuleReordering(){}
		
		//module configuration
		bool discoverDeviceModules();
		DataTransferState moduleDiscoveryStatus = DataTransferState::NO_TRANSFER;
		bool configureModules();
		
		//module process data
		void readModuleInputs();
		void writeModuleOutputs();
		
		//module factory
		virtual std::vector<std::shared_ptr<DeviceModule>>& getModuleFactory() = 0;
		std::shared_ptr<DeviceModule> createModule(uint32_t identifier);
		std::shared_ptr<DeviceModule> createModule(const char* saveString);
		
		//module management gui
		void moduleManagerGui();
		std::shared_ptr<DeviceModule> selectedModule = nullptr;
		
	private:
		//modules and module management
		std::shared_ptr<Legato::ListComponent<DeviceModule>> moduleList;
	};


};
