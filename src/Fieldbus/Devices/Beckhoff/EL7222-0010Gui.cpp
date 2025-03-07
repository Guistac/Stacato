#include "EL7222-0010.h"

void EL7222_0010::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL7222-0010")){
		if(ImGui::BeginTabBar("Drive")){
				
			if(ImGui::BeginTabItem("Actuator 1")){
				actuator1->gui();
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Actuator 2")){
				actuator2->gui();
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Drive")){
				
				ImGui::PushFont(Fonts::sansBold20);
				ImGui::Text("Utilities");
				ImGui::PopFont();
				
				if(ImGui::Button("Detect Motors and Configure Drives")) readMotorNameplatesAndConfigureDrive();				
				if(ImGui::Button("Download Diagnostics")) downloadCompleteDiagnostics();
				
				ImGui::Separator();
				
				ImGui::PushFont(Fonts::sansBold20);
				ImGui::Text("Digital Input Inversion");
				ImGui::PopFont();
				
				for(auto parameter : pinInversionParameters) parameter->gui(parameter->value ? Fonts::sansBold15 : Fonts::sansRegular15);
				
				ImGui::EndTabItem();
			}
				
			ImGui::EndTabBar();
		}
		ImGui::EndTabItem();
	}
}

void ELM7231_9016::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL7222-0010")){
		
		if(ImGui::Button("Detect Motor and Configure Drive")) readMotorNameplatesAndConfigureDrive();
		if(ImGui::Button("Download Diagnostics")) downloadCompleteDiagnostics();
		
		/*
		if(isStateOperational()){
			ImGui::BeginGroup();
			ImGui::Text("Outputs");
			for(int i = 0; i < identity->Obytes; i++){
				ImGui::Text("[%i]: %x", i, identity->outputs[i]);
			}
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();
			ImGui::Text("Inputs");
			for(int i = 0; i < identity->Ibytes; i++){
				ImGui::Text("[%i]: %x", i, identity->inputs[i]);
			}
			ImGui::EndGroup();
		}
		*/
		
		ImGui::Checkbox("STO", &b_sto);
		switch(fsoeConnection.getState()){
			case FsoeConnection::MasterState::RESET: ImGui::Text("RESET"); break;
			case FsoeConnection::MasterState::SESSION: ImGui::Text("SESSION"); break;
			case FsoeConnection::MasterState::CONNECTION: ImGui::Text("CONNECTION"); break;
			case FsoeConnection::MasterState::PARAMETER: ImGui::Text("PARAMETER"); break;
			case FsoeConnection::MasterState::DATA: ImGui::Text("DATA"); break;
		}
		ImGui::Text("failsafedata: %i", fsoeConnection.b_sendFailsafeData);
		ImGui::Text("Safe Inputs: %X %X", safeInputs[0], safeInputs[1]);
		ImGui::Text("Safe Outputs: %X %X", safeOutputs[0], safeOutputs[1]);

		if(isStateOperational()){
		ImGui::Text("Fsoe Inputs %x %x %x %x %x %x %x   %x %x %x",
			identity->inputs[19],
			identity->inputs[20],
			identity->inputs[21],
			identity->inputs[22],
			identity->inputs[23],
			identity->inputs[24],
			identity->inputs[25],
			identity->inputs[26],
			identity->inputs[27],
			identity->inputs[28]);


		ImGui::Text("Fsoe Outputs %x %x %x %x %x %x %x   %x %x %x",
			identity->outputs[13],
			identity->outputs[14],
			identity->outputs[15],
			identity->outputs[16],
			identity->outputs[17],
			identity->outputs[18],
			identity->outputs[19],
			identity->outputs[20],
			identity->outputs[21],
			identity->outputs[22]);
		}
		actuator->gui();
		
		ImGui::EndTabItem();
	}
}
