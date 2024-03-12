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
	void downloadDiagnostics();
	void downloadLatestDiagnosticsMessage();
	uint16_t latestDiagnosticsMessageTextID = 0x0;
	
	void readMotorNameplatesAndConfigureDrive();
	
	bool requestStateSafeOp();
	
};
