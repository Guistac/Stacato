#include "EtherCatFieldbus.h"

#include <ethercat.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <bitset>

std::vector<NetworkInterfaceCard>   EtherCatFieldbus::networkInterfaceCards;
NetworkInterfaceCard                EtherCatFieldbus::selectedNetworkInterfaceCard;
bool                                EtherCatFieldbus::b_networkScanned = false;
std::vector<EtherCatSlave>          EtherCatFieldbus::slaves;
uint8_t                             EtherCatFieldbus::ioMap[4096];
int                                 EtherCatFieldbus::ioMapSize = 0;
std::thread                         EtherCatFieldbus::etherCatRuntime;
bool                                EtherCatFieldbus::b_processRunning = false;
bool                                EtherCatFieldbus::b_ioMapConfigured = false;

void EtherCatFieldbus::updateNetworkInterfaceCardList() {
    std::cout << "===== Refreshing Network Interface Card List" << std::endl;
    networkInterfaceCards.clear();
    ec_adaptert* nics = ec_find_adapters();
    if (nics != nullptr) {
        while (nics != nullptr) {
            NetworkInterfaceCard nic;
            strcpy(nic.name, nics->name);
            strcpy(nic.description, nics->desc);
            networkInterfaceCards.push_back(std::move(nic));
            nics = nics->next;
        }
    }
    std::cout << "===== Found " << networkInterfaceCards.size() << " Network Interface Card" << (networkInterfaceCards.size() == 1 ? "" : "s") << std::endl;
    for (NetworkInterfaceCard& nic : networkInterfaceCards) {
        std::cout << "    = " << nic.description << " (ID: " << nic.name << ")" << std::endl;
    }
}

void EtherCatFieldbus::init(NetworkInterfaceCard& nic) {
    selectedNetworkInterfaceCard = std::move(nic);
    std::cout << "===== Initializing EtherCAT Fieldbus on Network Interface Card '" << selectedNetworkInterfaceCard.description << "'" << std::endl;
    int nicInitResult = ec_init(selectedNetworkInterfaceCard.name);
    if (nicInitResult > 0) std::cout << "===== Initialized network interface card !" << std::endl;
    else std::cout << "===== Failed to initialize network interface card ..." << std::endl;
}

void EtherCatFieldbus::scanNetwork() {
    slaves.clear();
    b_ioMapConfigured = false;
    //setup all slaves, get slave count and info in ec_slave, setup mailboxes, request state PRE-OP for all slaves
    int workingCounter = ec_config_init(FALSE); //what is usetable??
    for (int i = 1; i <= ec_slavecount; i++) {
        ec_slavet& slv = ec_slave[i];
        EtherCatSlave slave;
        slave.slave_ptr = &slv;
        strcpy(slave.name, slv.name);
        slave.index = i;
        slave.manualAddress = slv.aliasadr;
        slave.address = slv.configadr;
        sprintf(slave.displayName, "%s (Node %i #%i)", slave.name, slave.index, slave.manualAddress);
        slaves.push_back(std::move(slave));
    }
    if (workingCounter > 0) {
        b_networkScanned = true;
        std::cout << "===== Found and Configured " << ec_slavecount << " EtherCAT Slave" << ((ec_slavecount == 1) ? ": " : "s: ") << std::endl;
        for (EtherCatSlave& slave : slaves)
            std::cout << "    = Slave "
            << slave.index << " : '"
            << slave.name << "' Address: "
            << slave.address << " Manual Address: "
            << slave.manualAddress << std::endl;
    }
    else {
        b_networkScanned = false;
        std::cout << "===== No EtherCAT Slaves found..." << std::endl;
    }
}

void EtherCatFieldbus::configureSlaves() {
    int workingCounter = 0;

    for (EtherCatSlave& slave : slaves) {
        if (strcmp(slave.name, "LXM32M EtherCAT") == 0) {
            //asign RxPDO and TxPDO here:
            slave.slave_ptr->PO2SOconfigx = [](ecx_contextt* context, uint16_t slave) -> int {
                /*
                std::cout << "===== Remapping TxPDO Parameters..." << std::endl;
                //remap used TxPDO to two arbitray parameters of the drive (iMax and vMax)
                uint16_t TxPDOmodule = 0x1A03;
                uint8_t zero = 0;
                uint8_t TxPDOparameterCount = 2;
                uint32_t TxPDOparameter1 = 0x30110C10; //parameter 3011, index 0C, size 10 (16 bits)
                uint32_t TxPDOparameter2 = 0x30111020; //parameter 3011, index 10, size 20 (32 bits)
                //disable pdo by setting the sub-index size to zero
                workingCounter += ec_SDOwrite(1, TxPDOmodule, 0x0, false, 1, &zero, EC_TIMEOUTSAFE);
                //add first parameter at first index of PDO
                workingCounter += ec_SDOwrite(1, TxPDOmodule, 0x1, false, 4, &TxPDOparameter1, EC_TIMEOUTSAFE);
                //add second parameter at second index of PDO
                workingCounter += ec_SDOwrite(1, TxPDOmodule, 0x2, false, 4, &TxPDOparameter2, EC_TIMEOUTSAFE);
                //enable pdo by setting the index equal to the number of parameters in the pdo
                workingCounter += ec_SDOwrite(1, TxPDOmodule, 0x0, false, 1, &TxPDOparameterCount, EC_TIMEOUTSAFE);
                if (workingCounter == TxPDOparameterCount + 2) std::cout << "===== Successfully set custom pdo mapping !" << std::endl;
                else std::cout << "===== Failed to set custom pdo mapping..." << std::endl;
                */

                std::cout << "===== Begin PDO assignement..." << std::endl;
                int workingCounter = 0;
                uint8_t PDOoff = 0x00;
                uint8_t PDOon = 0x01;
                //Sync Manager (SM2, SM3) registers that store the mapping objects (modules) which decribe PDO data
                uint16_t RxPDO = 0x1C12;
                uint16_t TxPDO = 0x1C13;
                //mapping object (module) to be stored in each pdo register
                uint16_t RxPDOmodule = 0x1603;
                uint16_t TxPDOmodule = 0x1A03;
                //turn the pdo off by writing a zero to the 0 index, set the mapping object at subindex 1, enable the pdo by writing a 1 (module count) to the index
                workingCounter += ec_SDOwrite(slave, RxPDO, 0x0, false, 1, &PDOoff, EC_TIMEOUTSAFE);
                workingCounter += ec_SDOwrite(slave, RxPDO, 0x1, false, 2, &RxPDOmodule, EC_TIMEOUTSAFE);
                workingCounter += ec_SDOwrite(slave, RxPDO, 0x0, false, 1, &PDOon, EC_TIMEOUTSAFE);
                //do the same for the TxPDO
                workingCounter += ec_SDOwrite(slave, TxPDO, 0x0, false, 1, &PDOoff, EC_TIMEOUTSAFE);
                workingCounter += ec_SDOwrite(slave, TxPDO, 0x1, false, 2, &TxPDOmodule, EC_TIMEOUTSAFE);
                workingCounter += ec_SDOwrite(slave, TxPDO, 0x0, false, 1, &PDOon, EC_TIMEOUTSAFE);
                if (workingCounter == 6) {
                    std::cout << "===== PDO assignement successfull !" << std::endl;
                    return 1;
                }
                else {
                    std::cout << "===== PDO assignement failed..." << std::endl;
                    return 0;
                }
            };
        }
    }
    

    //build ioMap for PDO data, configure FMMU and SyncManager, request SAFE-OP state for all slaves
    std::cout << "===== Begin Building I/O Map..." << std::endl;
    ioMapSize = ec_config_map(ioMap);
    std::cout << "===== Finished Building I/O Map (Size : " << ioMapSize << " bytes)" << std::endl;
    
    if (ioMapSize > 0) b_ioMapConfigured = true;

    for (EtherCatSlave& slave : slaves) {
        slave.b_configured = true;
    }

    for (EtherCatSlave& slave : slaves) {
        std::cout << "   [" << slave.index << "] '" << slave.name << "' " << slave.slave_ptr->Ibytes + slave.slave_ptr->Obytes << " bytes (" << slave.slave_ptr->Ibits + slave.slave_ptr->Obits << " bits)" << std::endl;
        std::cout << "          Inputs: " << slave.slave_ptr->Ibytes << " bytes (" << slave.slave_ptr->Ibits << " bits)" << std::endl;
        std::cout << "          Outputs: " << slave.slave_ptr->Obytes << " bytes (" << slave.slave_ptr->Obits << " bits)" << std::endl;
    }

    /*
    std::cout << "===== Configuring Distributed Clocks" << std::endl;
    bool distributedClockConfigurationResult = ec_configdc();
    if (distributedClockConfigurationResult) std::cout << "===== Finished Configuring Distributed Clocks" << std::endl;
    else std::cout << "===== Could not configure distributed clocks ..." << std::endl;
    */
}

void EtherCatFieldbus::terminate() {
    std::cout << "===== Closing EtherCAT Network Interface Card" << std::endl;
    ec_close();
}

void EtherCatFieldbus::startCyclicExchange() {
    /*
    if (!b_processRunning) {

        b_processRunning = true;

        etherCatRuntime = std::thread([]() {
        */
            
            std::cout << "===== Setting All Slaves to Operational state..." << std::endl;
            // Act on slave 0 (a virtual slave used for broadcasting)
            ec_slavet* broadcastSlave = &ec_slave[0];
            broadcastSlave->state = EC_STATE_OPERATIONAL;
            ec_writestate(0);

            //wait for all slaves to reach OP state
            uint16_t slaveState = ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);
            if (slaveState == EC_STATE_OPERATIONAL) std::cout << "===== All slaves are operational !" << std::endl;
            
            std::cout << "===== Begin Cyclic Process Data Echange !" << std::endl;

            int workingCounter;

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


            b_processRunning = false;
            /*
        });
    }
    */
}
void EtherCatFieldbus::stopCyclicExchange() {
    b_processRunning = false;
}

void EtherCAT_dev(NetworkInterfaceCard nic) {
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