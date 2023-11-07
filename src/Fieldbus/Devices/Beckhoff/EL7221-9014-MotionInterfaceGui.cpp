#include "EL7221-9014-MotionInterface.h"

void EL7221_9014::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL7221-9014")){
		
		ImGui::Text("Motor Connected: %i", b_motorConnected);
		
		ImGui::Separator();
		
		ImGui::Text("readyToSwitchOn: %i", statusWord.readyToSwitchOn);
		ImGui::Text("switchedOn: %i", statusWord.switchedOn);
		ImGui::Text("operationEnabled: %i", statusWord.operationEnabled);
		ImGui::Text("fault: %i", statusWord.fault);
		ImGui::Text("quickstop: %i", statusWord.quickstop);
		ImGui::Text("switchOnDisabled: %i", statusWord.switchOnDisabled);
		ImGui::Text("warning: %i", statusWord.warning);
		ImGui::Text("TxPdoToggle: %i", statusWord.TxPdoToggle);
		ImGui::Text("internalLimitActive: %i", statusWord.internalLimitActive);
		ImGui::Text("commandValueFollowed: %i", statusWord.commandValueFollowed);
		
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
		ImGui::Text("pdo velocity target: %i", rxPdo.targetVelocity);
		
		double pos = double(txPdo.fbPosition) / double(motorSettings.positionResolution_rev);
		ImGui::Text("%.3f rev", pos);
		
		if(ImGui::Button("Configure Drive")) configureDrive();
		
		if(ImGui::Button("Set State Init")) {
			identity->state = EC_STATE_INIT;
			ec_writestate(getSlaveIndex());
		}
		if(ImGui::Button("Set State PreOp")) {
			identity->state = EC_STATE_PRE_OP;
			ec_writestate(getSlaveIndex());
		}
		
		
		ImGui::InputScalar("##offset", ImGuiDataType_U32, &offsetInputField);
		ImGui::SameLine();
		if(ImGui::Button("Set Encoder Offset")) writeEncoderPositionOffset(offsetInputField);
		if(ImGui::Button("Zero Encoder")) writeEncoderPositionOffset(txPdo.fbPosition);
		
		ImGui::Text("%s", actuator->getStatusString().c_str());
		
		ImGui::EndTabItem();
	}
}

