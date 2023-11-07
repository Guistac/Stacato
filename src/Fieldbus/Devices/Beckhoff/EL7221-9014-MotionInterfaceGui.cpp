#include "EL7221-9014-MotionInterface.h"

void EL7221_9014::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL7221-9014")){
		
		bool readyToSwitchOn		= txPdo.statusWord & (0x1 << 0);
		bool switchedOn				= txPdo.statusWord & (0x1 << 1);
		bool operationEnabled		= txPdo.statusWord & (0x1 << 2);
		bool fault 					= txPdo.statusWord & (0x1 << 3);
		bool quickstop 				= txPdo.statusWord & (0x1 << 5);
		bool switchOnDisabled 		= txPdo.statusWord & (0x1 << 6);
		bool warning 				= txPdo.statusWord & (0x1 << 7);
		bool TxPdoToggle 			= txPdo.statusWord & (0x1 << 10);
		bool internalLimitActive	= txPdo.statusWord & (0x1 << 11);
		bool commandValueFollowed	= txPdo.statusWord & (0x1 << 12);
		ImGui::Text("readyToSwitchOn: %i", readyToSwitchOn);
		ImGui::Text("switchedOn: %i", switchedOn);
		ImGui::Text("operationEnabled: %i", operationEnabled);
		ImGui::Text("fault: %i", fault);
		ImGui::Text("quickstop: %i", quickstop);
		ImGui::Text("switchOnDisabled: %i", switchOnDisabled);
		ImGui::Text("warning: %i", warning);
		ImGui::Text("TxPdoToggle: %i", TxPdoToggle);
		ImGui::Text("internalLimitActive: %i", internalLimitActive);
		ImGui::Text("commandValueFollowed: %i", commandValueFollowed);
		
		ImGui::Separator();
		
		ImGui::Text("adcError: %i", driverErrors.adc);
		ImGui::Text("overcurrentError: %i", driverErrors.overcurrent);
		ImGui::Text("undervoltageError: %i", driverErrors.undervoltage);
		ImGui::Text("overvoltageError: %i", driverErrors.overvoltage);
		ImGui::Text("overtemperatureError: %i", driverErrors.overtemperature);
		ImGui::Text("i2tAmplifierError: %i", driverErrors.i2tAmplifier);
		ImGui::Text("i2tMotorError: %i", driverErrors.i2tMotor);
		ImGui::Text("encoderError: %i", driverErrors.encoder);
		ImGui::Text("watchdogError: %i", driverErrors.watchdog);
	
		ImGui::Separator();
		
		
		ImGui::Text("undervoltageWarning: %i", driverWarnings.undervoltage);
		ImGui::Text("overvoltageWarning: %i", driverWarnings.overvoltage);
		ImGui::Text("overtemperatureWarning: %i", driverWarnings.overtemperature);
		ImGui::Text("i2tAmplifierWarning: %i", driverWarnings.i2tAmplifier);
		ImGui::Text("i2tMotorWarning: %i", driverWarnings.i2tMotor);
		ImGui::Text("encoderWarning: %i", driverWarnings.encoder);
	
		
		ImGui::Separator();
		
		ImGui::Checkbox("Enable", &b_enableRequest);
		if(ImGui::Button("Fault Reset")) b_faultResetRequest = true;
		
		double minVel = -motorSettings.speedLimitation_rps;
		double maxVel = motorSettings.speedLimitation_rps;
		ImGui::SliderScalar("Target Velocity", ImGuiDataType_Double, &velocityRequest_rps, &minVel, &maxVel);
		if(ImGui::IsItemDeactivatedAfterEdit()) velocityRequest_rps = 0x0;
		
		if(ImGui::Button("Configure Drive")) configureDrive();
		
		ImGui::EndTabItem();
	}
}

