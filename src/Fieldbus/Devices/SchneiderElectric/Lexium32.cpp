#include "Lexium32.h"
#include "Fieldbus/EtherCatFieldbus.h"

#include <iostream>

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>


std::map<int, std::string> Lexium32::modelist = {
    {-6, "Manual/Auto Tuning"},
    {-5, "-5"},
    {-4, "-4"},
    {-3, "Motion Sequence"},
    {-2, "Electronic Gear"},
    {-1, "Jog"},
    {0, "0"},
    {1, "Profile Position"},
    {2, "2"},
    {3, "Profile Velocity"},
    {4, "Profile Torque"},
    {5, "5"},
    {6, "Homing"},
    {7, "Interpolated Position"},
    {8, "Cyclic Synchronous Position"},
    {9, "Cyclic Synchronous Velocity"},
    {10, "Cyclic Synchronous Torque"},
    {11, "11"}
};

bool Lexium32::startupConfiguration() {

    std::cout << "===== Begin Lexium32 Startup Configuration" << std::endl;

    std::cout << "    = Begin Setting Startup Parameters..." << std::endl;
    if (!setStartupParameters()) {
        std::cout << "    = Failed to set startup Parameters..." << std::endl;
        return false;
    }
    std::cout << "    = Successfully set startup parameters" << std::endl;

    std::cout << "    = Begin Lexium32 PDO assignement..." << std::endl;
    if (!assignPDOs()) {
        std::cout << "    = Lexium32 PDO assignement failed..." << std::endl;
        return false;
    }
    std::cout << "    = PDO assignement successfull !" << std::endl;

    int plotLength = 1000.0 * EtherCatFieldbus::metrics.scrollingBufferLength_seconds / EtherCatFieldbus::processInterval_milliseconds;
    positions.setMaxSize(plotLength);
    
    



    //set interrupt routine for cyclic synchronous position mode
    //interval should be the same as the frame cycle time, and offset should be zero
    //the frame cycle time is offset 50% from dc_sync time (which is a integer multiple of the interval time)
    //by setting the sync0 event at 0 offset, maximum time offset is garanteed between the sync event and the frame receive time
    uint32_t sync0Interval_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 1000000.0L;
    uint32_t sync0offset_nanoseconds = 0;
    ec_dcsync0(getSlaveIndex(), true, sync0Interval_nanoseconds, sync0offset_nanoseconds);
    //TODO: does this still apply with a lot of slaves ?
    //if propagation delays add up, might the last slaves have their sync event happen at the same time as their frame receive time?

    std::cout << "===== Finished Lexium32 Startup Configuration" << std::endl;

    return true;
}


void Lexium32::process(bool inputDataValid) {

    int32_t outputPosition;
    if(b_inverted) outputPosition = positionCommand + (std::cos((double)counter / 1000.0) - 1.0) * 20000000;
    else outputPosition = positionCommand - (std::cos((double)counter / 1000.0) - 1.0) * 20000000;

    positions.addPoint(glm::vec2(counter, outputPosition));

    counter++;


    //============== READ INPUTS

    //TxPDO (inputs)
    //_DCOMstatus   (uint16_t)  2
    //_DCOMopmd_act (int8_t)    1
    //_p_act        (int32_t)   4
    //_p_dif        (int32_t)   4
    //_tq_act       (int32_t)   2
    //_LastError    (uint16_t)  2
    //_IO_act       (uint16_t)  2

    //don't read the input data if the received frame workingCounter did not match the expected one
    if (inputDataValid) {

        uint8_t* inByte = identity->inputs;

        //TxPDO Data
        uint16_t _DCOMstatus = inByte[0] | inByte[1] << 8;
        int8_t _DCOMopmd_act = inByte[2];
        position = inByte[3] | inByte[4] << 8 | inByte[5] << 16 | inByte[6] << 24;
        velocity = inByte[7] | inByte[8] << 8 | inByte[9] << 16 | inByte[10] << 24;
        torque = inByte[11] | inByte[12] << 8;
        lastErrorCode = inByte[13] | inByte[14] << 8;
        uint16_t _IO_act = inByte[15] | inByte[16] << 8;

        DI0 = _IO_act & 0x1;
        DI1 = _IO_act & 0x2;
        DI2 = _IO_act & 0x4;
        DI3 = _IO_act & 0x8;
        DI4 = _IO_act & 0x10;
        DI5 = _IO_act & 0x20;

        //state bits (0,1,2,3,5,6)
        bool readyToSwitchOn = _DCOMstatus & 0x1;
        bool switchedOn = _DCOMstatus & 0x2;
        bool operationEnabled = _DCOMstatus & 0x4;
        bool fault = _DCOMstatus & 0x8;
        bool quickStop = _DCOMstatus & 0x20;
        bool switchOnDisabled = _DCOMstatus & 0x40;
        //other DCOM bits
        motorVoltagePresent = _DCOMstatus & 0x10;
        class0error = _DCOMstatus & 0x80;
        halted = _DCOMstatus & 0x100;
        fieldbusControlActive = _DCOMstatus & 0x200;
        targetReached = _DCOMstatus & 0x400;  //Operating mode specifig information (b10) 
        internalLimitActive = _DCOMstatus & 0x800; //DS402intLim
        operatingModeSpecificFlag = _DCOMstatus & 0x1000;
        stoppedByError = _DCOMstatus & 0x2000;
        operatingModeFinished = _DCOMstatus & 0x4000; //Operating mode specific information (b12)
        validPositionReference = _DCOMstatus & 0x8000;

        //find the state using the state bits
        if (!readyToSwitchOn) {
            if (switchOnDisabled)       state = State::SwitchOnDisabled;
            else if (fault)             state = State::Fault;
            else                        state = State::NotReadyToSwitchOn;
        }
        else {
            if (fault)                  state = State::FaultReactionActive;
            else if (!quickStop)        state = State::QuickStopActive;
            else if (operationEnabled)  state = State::OperationEnabled;
            else if (switchedOn)        state = State::SwitchedOn;
            else                        state = State::ReadyToSwitchOn;
        }

        if (_DCOMopmd_act != mode) {
            mode = _DCOMopmd_act;
            modeChar = modelist[mode].c_str();
        }
    }

    //============== WRITE OUTPUTS

    //RxPDO (outputs)
    //DCOMcontrol   (uint16_t)  2
    //DCOMopmode    (int8_t)    1
    //PPp_target    (int32_t)   4
    //PVv_target    (int32_t)   4
    //PTtq_target   (int32_t)   2
    //IO_DQ_set     (uint16_t)  2

    uint8_t* outByte = identity->outputs;

    static bool faultResetOnPreviousCycle = false;

    //Drive State Commands
    if (disableVoltage) {
        disableVoltage = false;
        b_voltageEnabled = false;
        b_faultResetState = false;
    }
    if (enableVoltage) {
        enableVoltage = false;
        b_voltageEnabled = true;
        b_faultResetState = false;
    }
    if (shutdown) {
        shutdown = false;
        b_switchedOn = false;
        b_quickStopActive = true;
        b_voltageEnabled = true;
        b_faultResetState = false;
    }
    if (switchOn) {
        switchOn = false;
        b_operationEnabled = false;
        b_quickStopActive = true;
        b_voltageEnabled = true;
        b_switchedOn = true;
        b_faultResetState = false;
    }
    if (performQuickStop) {
        performQuickStop = false;
        b_quickStopActive = false;
        b_voltageEnabled = true;
        b_faultResetState = false;
    }
    if (performFaultReset && !faultResetOnPreviousCycle) {
        performFaultReset = false;
        b_faultResetState = true; //reset this to zero after execution !
        faultResetOnPreviousCycle = true;
    }
    if (enableOperation) {
        enableOperation = false;
        b_operationEnabled = true;
        b_quickStopActive = true;
        b_voltageEnabled = true;
        b_switchedOn = true;
        b_faultResetState = false;
        positions.clear();
    }
    if (disableOperation) {
        disableOperation = false;
        b_operationEnabled = false;
        b_quickStopActive = true;
        b_voltageEnabled = true;
        b_switchedOn = true;
        b_faultResetState = false;
    }

    //state control word
    uint16_t DCOMcontrol = 0x0000;
    if (b_switchedOn)       DCOMcontrol |= 0x1;
    if (b_voltageEnabled)   DCOMcontrol |= 0x2;
    if (b_quickStopActive)  DCOMcontrol |= 0x4;
    if (b_operationEnabled) DCOMcontrol |= 0x8;
    if (opModeSpec4)        DCOMcontrol |= 0x10;
    if (opModeSpec5)        DCOMcontrol |= 0x20;
    if (opModeSpec6)        DCOMcontrol |= 0x40;
    if (b_faultResetState)  DCOMcontrol |= 0x80;
    if (b_halted)           DCOMcontrol |= 0x100;
    if (opModeSpec9)        DCOMcontrol |= 0x200;
    //bits 10 to 15 have to be 0

    if (!b_faultResetState) faultResetOnPreviousCycle = false; //this prevents two fault reset bits to be sent after one another
    b_faultResetState = false; //reset bit after performing a fault reset

    int8_t DCOMopmode = modeCommand;

    uint16_t IO_DQ_set = 0;
    if (DQ0) IO_DQ_set |= 0x1;
    if (DQ1) IO_DQ_set |= 0x2;
    if (DQ2) IO_DQ_set |= 0x4;

    outByte[0] = (DCOMcontrol >> 0) & 0xFF;
    outByte[1] = (DCOMcontrol >> 8) & 0xFF;
    
    outByte[2] = DCOMopmode;

    outByte[3] = (outputPosition >> 0) & 0xFF;
    outByte[4] = (outputPosition >> 8) & 0xFF;
    outByte[5] = (outputPosition >> 16) & 0xFF;
    outByte[6] = (outputPosition >> 24) & 0xFF;

    outByte[7] = (velocityCommand >> 0) & 0xFF;
    outByte[8] = (velocityCommand >> 8) & 0xFF;
    outByte[9] = (velocityCommand >> 16) & 0xFF;
    outByte[10] = (velocityCommand >> 24) & 0xFF;

    outByte[11] = (torqueCommand >> 0) & 0xFF;
    outByte[12] = (torqueCommand >> 8) & 0xFF;

    outByte[13] = (IO_DQ_set >> 0) & 0xFF;
    outByte[14] = (IO_DQ_set >> 8) & 0xFF;

}


bool Lexium32::setStartupParameters() {

    //ramp settingts (useless in cyclic synchronous position mode for some reason...)
    uint32_t RAMP_v_acc_set = 600;
    if (!writeSDO(0x6083, 0x0, RAMP_v_acc_set)) return false;
    uint32_t RAMP_v_dec_set = 600;
    if (!writeSDO(0x6084, 0x0, RAMP_v_dec_set)) return false;

    //Cia DS402 mandatory Startup Settings (According to Lexium32 EtherCAT module documentation)
    uint16_t CompParSyncMot_set = 1;
    if (!writeSDO(0x3006, 0x3D, CompParSyncMot_set)) return false;
    uint16_t MOD_enable_set = 0;
    if (!writeSDO(0x3006, 0x38, MOD_enable_set)) return false;
    int16_t LIM_QStopReact_set = -1;
    if (!writeSDO(0x3006, 0x18, LIM_QStopReact_set)) return false;
    uint16_t IOsigRespOfPS_set = 1;
    if (!writeSDO(0x3006, 0x6, IOsigRespOfPS_set)) return false;
    int32_t ScalePOSdenom_set = 131072;
    if (!writeSDO(0x3006, 0x7, ScalePOSdenom_set)) return false;
    int32_t ScalePOSnum_set = 1;
    if (!writeSDO(0x3006, 0x8, ScalePOSnum_set)) return false;
    uint16_t CTRL1_KFPp_set = 1000;
    if (!writeSDO(0x3012, 0x6, CTRL1_KFPp_set)) return false;
    uint16_t CTRL2_KFPp_set = 1000;
    if (!writeSDO(0x3013, 0x6, CTRL2_KFPp_set)) return false;
    int8_t DCOMopmode_set = 8;
    if (!writeSDO(0x6060, 0x0, DCOMopmode_set)) return false;
    uint32_t ECATinpshifttime_set = 250000;;
    if (!writeSDO(0x1C33, 0x3, ECATinpshifttime_set)) return false;

    return true;
}

bool Lexium32::assignPDOs() {

    //variables applied to the zero subindex of an object dictionnary entry
    uint8_t zero = 0x00;
    uint8_t one = 0x01;

    //Sync Manager (SM2, SM3) registers that store the mapping objects (modules) which decribe PDO data
    uint16_t RxPDO = 0x1C12;
    uint16_t TxPDO = 0x1C13;

    //mapping object (module) to be stored in each pdo register
    uint16_t RxPDOmodule = 0x1603;
    uint16_t TxPDOmodule = 0x1A03;

    //turn the pdo off by writing a zero to the 0 index, set the mapping object at subindex 1, enable the pdo by writing a 1 (module count) to the index
    if (!writeSDO(RxPDO, 0x0, zero)) return false;
    if (!writeSDO(RxPDO, 0x1, RxPDOmodule)) return false;
    if (!writeSDO(RxPDO, 0x0, one)) return false;

    //do the same for the TxPDO but modify one parameter
    if (!writeSDO(TxPDO, 0x0, zero)) return false;

    //we are going to edit one parameter entry of the PDO module
    //disable TxPDO module before modifying it (set entry count to zero)
    if (!writeSDO(TxPDOmodule, 0x0, zero)) return false;              

    //replace default parameter 4 (_p_dif) by current velocity (_v_act 0x606C 0x00 int32_t)
    uint32_t TxPDOparameter4 = 0x606C0020;                            
    if (!writeSDO(TxPDOmodule, 0x4, TxPDOparameter4)) return false;

    //update parameter count at subindex 0 of pdo object
    uint8_t TxPDOparameterCount = 7;
    if (!writeSDO(TxPDOmodule, 0x0, TxPDOparameterCount)) return false;

    //assign pdo module object to sync manager
    if (!writeSDO(TxPDO, 0x1, TxPDOmodule)) return false;
    if (!writeSDO(TxPDO, 0x0, one)) return false;

    return true;
}


void Lexium32::gui() {

    ImGui::Text("Manual Address: %i  Assigned Address: %i", getManualAddress(), getAssignedAddress());
    ImGui::Text("state: %i   ALstatuscode: %i", identity->state, identity->ALstatuscode);
    ImGui::Text("ptype: %i  topology: %i  activeports: %i  consumedports: %i", identity->ptype, identity->topology, identity->activeports, identity->consumedports);
    ImGui::Text("parent: %i  parentport: %i  entryport: %i  ebuscurrent: %i", identity->parent, identity->parentport, identity->entryport, identity->Ebuscurrent);
    
    if (!b_mapped) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4, 0.4, 0.4, 1.0));
    }

    ImGui::Text("Input Bytes: %i (%i bits)", identity->Ibytes, identity->Ibits);
    ImGui::Text("Output Bytes: %i (%i bits)", identity->Obytes, identity->Obits);


    ImGui::Text("hasDC: %i  DCactive: %i", identity->hasdc, identity->DCactive);
    ImGui::Text("pdelay: %i  DCnext: %i  DCprevious: %i  DCcycle: %i  DCshift: %i", identity->pdelay, identity->DCnext, identity->DCprevious, identity->DCcycle, identity->DCshift);
    ImGui::Text("DCrtA: %i  DcrtB: %i  DCrtC: %i  DCrtD: %i", identity->DCrtA, identity->DCrtB, identity->DCrtC, identity->DCrtD);

    ImGui::Separator();

    if (ImGui::Button("Enable Voltage")) enableVoltage = true;
    ImGui::SameLine();
    if (ImGui::Button("Disable Voltage")) disableVoltage = true;

    if (ImGui::Button("Switch On")) switchOn = true;
    ImGui::SameLine();
    if (ImGui::Button("Shut Down")) shutdown = true;

    if (ImGui::Button("Enable Operation")) { 
        enableOperation = true;
        positionCommand = position;
        counter = 0;
    }
    ImGui::SameLine();
    if (ImGui::Button("Disable Operation")) disableOperation = true;

    if (ImGui::Button("Fault Reset")) performFaultReset = true;
    ImGui::SameLine();
    if (ImGui::Button("Quick Stop")) performQuickStop = true;

    const char* stateChar;
    switch (state) {
    case State::Fault:					stateChar = "Fault"; break;
    case State::FaultReactionActive:	stateChar = "Fault Reaction Active"; break;
    case State::NotReadyToSwitchOn:		stateChar = "Not Ready To Switch On"; break;
    case State::OperationEnabled:		stateChar = "Operation Enabled"; break;
    case State::QuickStopActive:		stateChar = "Quick Stop Active"; break;
    case State::ReadyToSwitchOn:		stateChar = "Ready To Switch On"; break;
    case State::SwitchedOn:				stateChar = "Switched On"; break;
    case State::SwitchOnDisabled:		stateChar = "Switch On Disabled"; break;
    }
    ImGui::Text("State: %s", stateChar);
    if (state == State::Fault) {
        ImGui::SameLine();
        ImGui::Text("(%X)", lastErrorCode);
    }

    ImGui::Separator();

    ImGui::Text("Position: %i", position);
    ImGui::Text("Velocity: %i", velocity);
    ImGui::Text("Torque: %i", torque);

    ImGui::Text("Digital Inputs: %i %i %i %i %i %i", DI0, DI1, DI2, DI3, DI4, DI5);

    ImGui::Separator();

    ImGui::Text("Current Operating Mode: %s (%i)", modeChar, mode);

    if (ImGui::BeginCombo("Mode Switch", modelist[modeCommand].c_str(), ImGuiComboFlags_HeightLargest)) {
        for (const auto& [key, value] : modelist) {
            if (key == 0) continue;
            if (ImGui::Selectable(value.c_str(), false)) modeCommand = key;

        }
        ImGui::EndCombo();
    }

    ImPlot::SetNextPlotLimitsX(positions.newest().x - 1000.0, positions.newest().x, ImGuiCond_Always);
    ImPlot::SetNextPlotFormatY("%g ticks");
    ImPlot::FitNextPlotAxes(false, true);

    if (ImPlot::BeginPlot("positions", NULL, NULL, ImVec2(-1, 600))) {
        ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 2.0);
        ImPlot::PlotLine("Position", &positions.front().x, &positions.front().y, positions.size(), positions.offset(), positions.stride());
        ImPlot::EndPlot();
    }

    ImGui::Checkbox("Invert", &b_inverted);

    int displayPosition = position;
    int displayVelocity = velocity;
    int displayTorque = torque;

    float velocityFraction = (((double)displayVelocity / 7000.0) + 1.0) / 2.0;
    ImGui::ProgressBar(velocityFraction, ImVec2(0, 0), "velocity");


    ImGui::Checkbox("DQ0", &DQ0);
    ImGui::SameLine();
    ImGui::Checkbox("DQ1", &DQ1);
    ImGui::SameLine();
    ImGui::Checkbox("DQ2", &DQ2);

    if (ImGui::Button("<<--")) {
        jog = true;
        direction = false;
        fast = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("<--")) {
        jog = true;
        direction = false;
        fast = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("0")) {
        stop = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("-->")) {
        jog = true;
        direction = true;
        fast = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("-->>")) {
        jog = true;
        direction = true;
        fast = true;
    }

    ImGui::Separator();

    ImGui::Text("motorVoltagePresent : %i", motorVoltagePresent);
    ImGui::Text("class0error : %i", class0error);
    ImGui::Text("halted : %i", halted);
    ImGui::Text("fieldbusControlActive : %i", fieldbusControlActive);
    ImGui::Text("targetReached : %i", targetReached);
    ImGui::Text("internalLimitActive : %i", internalLimitActive);
    ImGui::Text("operatingModeSpecificFlag : %i", operatingModeSpecificFlag);
    ImGui::Text("stoppedByError : %i", stoppedByError);
    ImGui::Text("operatingModeFinished : %i", operatingModeFinished);
    ImGui::Text("validPositionReference : %i", validPositionReference);

    if (!b_mapped) {
        ImGui::PopItemFlag();
        ImGui::PopStyleColor();
    }

}



/*
if (ImGui::Button("Fault Reset All")) for (ECatServoDrive& drive : EtherCatFieldbus::servoDrives) drive.performFaultReset = true;
ImGui::SameLine();
if (ImGui::Button("Enable All")) for (ECatServoDrive& drive : EtherCatFieldbus::servoDrives) { drive.enableOperation = true; drive.positionCommand = drive.position; drive.counter = 0; }
ImGui::SameLine();
if(ImGui::Button("Disable All")) for (ECatServoDrive& drive : EtherCatFieldbus::servoDrives) drive.disableOperation = true;
*/