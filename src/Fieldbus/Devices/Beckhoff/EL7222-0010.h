#pragma once

#include "Submodules/EL722xActuator.h"

class EL7222_0010 : public EtherCatDevice{
public:
	DEFINE_ETHERCAT_DEVICE(EL7222_0010, "EL7222-0010", "EL7222-0010", "Beckhoff", "Servo Drives", 0x2, 0x50227569)

	class EL7222Gpio : public GpioInterface{
	public:
		EL7222Gpio(std::shared_ptr<EtherCatDevice> parentDevice) : etherCatDevice(parentDevice){}
		std::shared_ptr<EtherCatDevice> etherCatDevice;
		virtual std::string getName() override { return std::string(etherCatDevice->getName()) + " GPIO"; }
		virtual std::string getStatusString() override { return "No Status String Implemented..."; }
	};
	
	struct TxPdo{
		uint16_t infoData1_digitalInputs = 0x0;
	}txPdo;
	
	std::shared_ptr<EL722x_Actuator> actuator1;
	std::shared_ptr<EL722x_Actuator> actuator2;
	std::shared_ptr<EL7222Gpio> gpio;
	
	std::shared_ptr<NodePin> devicePin;
	std::vector<std::shared_ptr<bool>> digitalInputValues;
	std::vector<std::shared_ptr<NodePin>> digitalInputPins;
	
	std::vector<BoolParam> pinInversionParameters;
	
	std::string getDiagnosticsStringFromTextID(uint16_t textID);
	void downloadCompleteDiagnostics();
	void downloadLatestDiagnosticsMessage(uint16_t* output, bool* b_downloadFinished);
	
	void readMotorNameplatesAndConfigureDrive();
	
	bool requestStateSafeOp();
	
};

#include "Fieldbus/FsoeConnection.h"

class ELM7231_9016 : public EtherCatDevice{
	DEFINE_ETHERCAT_DEVICE(ELM7231_9016, "ELM7231-9016", "ELM7231-9016", "Beckhoff", "Servo Drives", 0x2, 0x502275F9)
	std::shared_ptr<EL722x_Actuator> actuator;
	std::shared_ptr<bool> sto = std::make_shared<bool>(true);
	
	uint8_t fsoeMasterFrame[7];
	uint8_t fslogicMaster[2] = {0,0};
	uint8_t fsoeSlaveFrame[7];
	uint8_t fsLogicSlave[2];
	uint8_t safeInputs[2];
	uint8_t safeOutputs[2];
	
	std::string getDiagnosticsStringFromTextID(uint16_t textID);
	void downloadCompleteDiagnostics();
	void readMotorNameplatesAndConfigureDrive();
	bool requestStateSafeOp();
	
	EtherCatPdoAssignement fsoeRxPdoAssignement;
	EtherCatPdoAssignement fsoeTxPdoAssignement;
	
	FsoeConnection fsoeConnection;
	
	uint8_t weird1;
	uint8_t weird2;
	
	bool b_sto = false;
	bool b_initialized = false;
	
	void fixFMMUs();
};
