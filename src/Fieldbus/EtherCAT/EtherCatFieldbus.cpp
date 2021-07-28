#include "EtherCatFieldbus.h"

#include <ethercat.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <bitset>


void EtherCatTest() {

    std::cout << "===== Beginning EtherCAT test program" << std::endl;

    ec_adaptert* nics = ec_find_adapters();

    NetworkInterfaceCard* networkInterfaceCards;
    size_t networkInterfaceCardCount;

    if (nics != nullptr) {
        networkInterfaceCardCount = 0;
        ec_adapter* nic = nics;
        while (nic != nullptr) {
            networkInterfaceCardCount++;
            nic = nic->next;
        }
        nic = nics;
        networkInterfaceCards = new NetworkInterfaceCard[networkInterfaceCardCount];
        int i = 0;
        while (nic != nullptr) {
            networkInterfaceCards[i] = { nic->desc, nic->name };
            i++;
            nic = nic->next;
        }
    }
    std::cout << "===== Available Network Interface Cards :" << std::endl;
    for (int i = 0; i < networkInterfaceCardCount; i++) std::cout << "  " << networkInterfaceCards[i].description << std::endl;
    
    NetworkInterfaceCard selectedNetworkInterfaceCard = networkInterfaceCards[0];

    EtherCAT_dev(selectedNetworkInterfaceCard);


}

void EtherCAT_dev(NetworkInterfaceCard nic) {

    //initialize network interface card
    int nicInitResult = ec_init(nic.name);
    if (nicInitResult > 0) std::cout << "===== Initialized network interface card : " << nic.description << std::endl;
    else std::cout << "===== Failed to initialize network interface card " << nic.description << std::endl;

    //setup all slaves, get slave count and info in ec_slave, setup mailboxes, request state PRE-OP for all slaves
    int workingCounter = ec_config_init(FALSE);
    if (workingCounter > 0) std::cout << "===== Found and Configured " << ec_slavecount << ((ec_slavecount == 1) ? " EtherCAT Slave :" : " EtherCAT Slaves :") << std::endl;
    else std::cout << "===== No EtherCAT Slaves found..." << std::endl;
    if (ec_slavecount == 0) goto end_test;

    for (int i = 1; i <= ec_slavecount; i++) {
        ec_slavet& slave = ec_slave[i];
        std::cout << "  Slave " << i << " : '" << slave.name << "'" << std::endl;
        std::cout << "          AliasAddress: " << slave.aliasadr << " ConfigAddress: " << slave.configadr << std::endl;
    }
    
    bool remapTxoPDO = true;

    if (remapTxoPDO) {
        std::cout << "===== Remapping TxPDO Parameters..." << std::endl;
        //remap used TxPDO to two arbitray parameters of the drive (iMax and vMax)

        uint16_t TxPDOmodule = 0x1A03;
        uint8_t zero = 0;
        uint8_t TxPDOparameterCount = 2;
        uint32_t TxPDOparameter1 = 0x30110C10; //parameter 3011, index 0C, size 10 (16 bits)
        uint32_t TxPDOparameter2 = 0x30111020; //parameter 3011, index 10, size 20 (32 bits)
        int wc = 0;

        //disable pdo by setting the sub-index size to zero
        wc += ec_SDOwrite(1, TxPDOmodule, 0x0, false, 1, &zero, EC_TIMEOUTSAFE);
        //add first parameter at first index of PDO
        wc += ec_SDOwrite(1, TxPDOmodule, 0x1, false, 4, &TxPDOparameter1, EC_TIMEOUTSAFE);
        //add second parameter at second index of PDO
        wc += ec_SDOwrite(1, TxPDOmodule, 0x2, false, 4, &TxPDOparameter2, EC_TIMEOUTSAFE);
        //enable pdo by setting the index equal to the number of parameters in the pdo
        wc += ec_SDOwrite(1, TxPDOmodule, 0x0, false, 1, &TxPDOparameterCount, EC_TIMEOUTSAFE);

        if (wc == TxPDOparameterCount + 2) std::cout << "===== Successfully set custom pdo mapping !" << std::endl;
        else std::cout << "===== Failed to set custom pdo mapping..." << std::endl;
    }

    //asign RxPDO and TxPDO here:
    ec_slave[1].PO2SOconfigx = [](ecx_contextt* context, uint16_t slave) -> int {
        std::cout << "===== Begin PDO assignement..." << std::endl;
        int wc = 0;
        uint8_t PDOoff = 0x00;
        uint8_t PDOon = 0x01;
        //Sync Manager (SM2, SM3) registers that store the mapping objects (modules) which decribe PDO data
        uint16_t RxPDO = 0x1C12;
        uint16_t TxPDO = 0x1C13;
        //mapping object (module) to be stored in each pdo register
        uint16_t RxPDOmodule = 0x1603;
        uint16_t TxPDOmodule = 0x1A03;
        //turn the pdo off by writing a zero to the 0 index, set the mapping object at subindex 1, enable the pdo by writing a 1 (module count) to the index
        wc += ec_SDOwrite(slave, RxPDO, 0x0, false, 1, &PDOoff, EC_TIMEOUTSAFE);
        wc += ec_SDOwrite(slave, RxPDO, 0x1, false, 2, &RxPDOmodule, EC_TIMEOUTSAFE);
        wc += ec_SDOwrite(slave, RxPDO, 0x0, false, 1, &PDOon, EC_TIMEOUTSAFE);
        //do the same for the TxPDO
        wc += ec_SDOwrite(slave, TxPDO, 0x0, false, 1, &PDOoff, EC_TIMEOUTSAFE);
        wc += ec_SDOwrite(slave, TxPDO, 0x1, false, 2, &TxPDOmodule, EC_TIMEOUTSAFE);
        wc += ec_SDOwrite(slave, TxPDO, 0x0, false, 1, &PDOon, EC_TIMEOUTSAFE);
        if (wc == 6) {
            std::cout << "===== PDO assignement successfull !" << std::endl;
            return 1;
        }
        else {
            std::cout << "===== PDO assignement failed..." << std::endl;
            return 0;
        }
    };
    
    //build ioMap for PDO data, configure FMMU and SyncManager, request SAFE-OP state for all slaves
    std::cout << "===== Begin Building I/O Map..." << std::endl;
    char ioMap[4096];
    int ioMapSize;
    ioMapSize = ec_config_map(ioMap);
    std::cout << "===== Finished Building I/O Map (Size : " << ioMapSize << " bytes)" << std::endl;

    for (int i = 1; i <= ec_slavecount; i++) {
        ec_slavet& slave = ec_slave[i];
        std::cout << "   [" << i  << "] '"<< slave.name << "' " << slave.Ibytes + slave.Obytes << " bytes (" << slave.Ibits + slave.Obits << " bits)" << std::endl;
        std::cout << "          Inputs: " << slave.Ibytes << " bytes (" << slave.Ibits << " bits)" << std::endl;
        std::cout << "          Outputs: " << slave.Obytes << " bytes (" << slave.Obits << " bits)" << std::endl;
    }

    std::cout << "===== Configuring Distributed Clocks" << std::endl;
    ec_configdc();

    std::cout << "===== Setting All Slaves to Operational state..." << std::endl;
    /* Act on slave 0 (a virtual slave used for broadcasting) */
    ec_slavet* broadcastSlave = &ec_slave[0];
    broadcastSlave->state = EC_STATE_OPERATIONAL;
    ec_writestate(0);

    //wait for all slaves to reach OP state
    uint16_t slaveState = ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);
    if(slaveState == EC_STATE_OPERATIONAL) std::cout << "===== All slaves are operational !" << std::endl;
    else std::cout << "===== Not all slaves are operational ..." << std::endl;

    std::cout << "===== Begin Cyclic Process Data Echange !" << std::endl;

    int lineLength = 0;
    int transmissions = 0;
    int transmissionSuccesses = 0;
    int transmissionFailures = 0;
    int maxTransmissions = 10000;
    std::chrono::system_clock::time_point begin = std::chrono::system_clock::now();
    while (true) {
        ec_send_processdata();
        workingCounter = ec_receive_processdata(EC_TIMEOUTRET * 10);
        //TODO: detect difference between transmission corruption and timeout
        if (workingCounter == 3) {
            std::cout << ".";
            transmissionSuccesses++;
        }
        else {
            std::cout << "#";
            transmissionFailures++;
        }
        lineLength++;
        if (lineLength > 80) {
            lineLength = 0;
            std::cout << std::endl;
        }
        transmissions++;
        if (transmissions == maxTransmissions) break;

        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    long long durationMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - begin).count();
    double durationSeconds = (double)durationMicroseconds / 1000000.0L;
    double cycleFrequency = (double)maxTransmissions / durationSeconds;

    float transmissionSuccessRate = 100.0f * (float)transmissionSuccesses / (float)maxTransmissions;
    std::cout << std::endl;
    std::cout << "===== Ended Cycle Process Data Exchange After " << maxTransmissions << " cycles !" << std::endl;
    std::cout << "===== Sucessfull transmissions: " << transmissionSuccesses << "   Failures: " << transmissionFailures << std::endl;
    std::cout << "===== Success Rate: " << transmissionSuccessRate << "%" << std::endl;
    std::cout << "===== Cycle Frequency: " << cycleFrequency << "Hz" << std::endl;
    std::cout << "===== Ending EtherCAT test program" << std::endl;

    //ec_statecheck(slave,state,timeout) checks the status of the slave and returns the state
    //ec_readstate() reads state of all slaves and stores it in ec_slave[]
    //ec_writestate() sets the state of all slaves as it was set in ec_slave[]

    //ec_B--  broadcast addressing 
    //ec_AP-  position addressing  
    //ec_FP-  node addressing      
    //ec_L--  logical addressing   
    //are these commands event needed for regular use ?

    /*
    std::cout << "slave 1: " << slave->name
        << "\n activeports: " << slave->activeports
        << "\n aliasadr " << slave->aliasadr
        << "\n configadr " << slave->configadr
        << "\n configindex " << slave->configindex
        << "\n alstatuscode " << ec_ALstatuscode2string(slave->ALstatuscode)
        << "\n entryport " << slave->entryport
        << "\n itype " << slave->Itype
        << "\n state " << slave->state
        << "\n topology " << slave->topology
        << "\n ibytes " << slave->Ibytes
        << "\n ibits " << slave->Ibits
        << "\n obytes " << slave->Obytes
        << "\n obits " << slave->Obits
        << std::endl;
    */

    /*
    while (true) {

        static uint16_t DCOM = 0;
        static uint32_t target = 0;
        static uint16_t IO = 0;

        //DCOM = 1 << 6;
        target++;

        IO = 0;
        if ((target + 0) % 60 < 30) IO |= 1;
        if ((target + 15) % 60 < 30) IO |= 2;
        if ((target + 30) % 60 < 30) IO |= 4;

        uint8_t* ioMapOut = slave->outputs;
        ioMapOut[0] = (DCOM >> 0) & 0xFF;
        ioMapOut[1] = (DCOM >> 8) & 0xFF;
        ioMapOut[2] = (target >> 0) & 0xFF;
        ioMapOut[3] = (target >> 8) & 0xFF;
        ioMapOut[4] = (target >> 16) & 0xFF;
        ioMapOut[5] = (target >> 24) & 0xFF;
        ioMapOut[6] = (IO >> 0) & 0xFF;
        ioMapOut[7] = (IO >> 8) & 0xFF;

        ec_send_processdata();
        workingCounter = ec_receive_processdata(EC_TIMEOUTRET);

        if (workingCounter == EC_NOFRAME) {
            std::cout << "timeout" << std::endl;
            continue;
        }

        if (workingCounter != 3) {
            std::cout << "Wrong Working Counter" << std::endl;
            continue;
        }

        uint8_t* inByte = slave->inputs;
        uint16_t _DCOMstatus = inByte[0] | inByte[1] << 8;
        uint32_t _p_act = inByte[2] | inByte[3] << 8 | inByte[4] << 16 | inByte[5] << 24;
        uint16_t _LastError = inByte[6] | inByte[7] << 8;
        uint16_t _IO_act = inByte[8] | inByte[9] << 8;

        std::bitset<16> DCOMbits(_DCOMstatus);
        std::bitset<16> IObits(_IO_act);
        std::cout << "===Inputs=== OpState: " << DCOMbits << " ActualPosition: " << _p_act << " LastError: " << _LastError << " IO-inputs: " << IObits;

        uint8_t* outByte = slave->outputs;
        uint16_t _DCOMcontrol = outByte[0] | outByte[1] << 8;
        uint32_t PPp_target = outByte[2] | outByte[3] << 8 | outByte[4] << 16 | outByte[5] << 24;
        uint16_t IO_DQ_set = outByte[6] | outByte[7] << 8;

        std::bitset<16> DCOMcBits(_DCOMcontrol);
        std::bitset<16> IO_DQbits(IO_DQ_set);
        std::cout << "  ===Outputs=== OpStateCommand: " << DCOMcBits << " PositionTarget: " << PPp_target << " IO-ouputs " << IO_DQbits << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    */
end_test:
    ec_close();
}








const char* getStateString(uint16_t state) {
    switch (state) {
    case EC_STATE_NONE: return "NO STATE";
    case EC_STATE_INIT: return "INIT";
    case EC_STATE_PRE_OP: return "PRE-OPERATIONAL";
    case EC_STATE_BOOT: return "BOOT";
    case EC_STATE_SAFE_OP: return "SAFE-OPERATIONAL";
    case EC_STATE_OPERATIONAL: return "OPERATIONAL";
    case EC_STATE_ACK: return "STATE-ACK / ERROR";
    default: return "UNKNOWN STATE";
    }
}