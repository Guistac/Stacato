
#include <thread>
#include <iostream>
#include "Gui/GuiWindow.h"

int main() {
	unsigned int cpuCoreCount = std::thread::hardware_concurrency();
	std::cout << cpuCoreCount << std::endl;

	GuiWindow::open(1920,1920);
	GuiWindow::refresh();
	GuiWindow::close();
}



//#define DCTEST
#ifdef DCTEST

#include <stdio.h>
#include <stdlib.h>
//#include <sys/time.h>
//#include <unistd.h>
//#include <sched.h>
#include <string.h>
//#include <sys/time.h>
#include <time.h>
//#include <pthread.h>
#include <math.h>

#include "ethercat.h"

#define NSEC_PER_SEC 1000000000

struct sched_param schedp;
char IOmap[4096];
//pthread_t thread1;
struct timeval tv, t1, t2;
int dorun = 0;
int deltat, tmax = 0;
int64 toff;
int DCdiff;
int os;
uint8 ob;
uint16 ob2;
//pthread_cond_t      cond = PTHREAD_COND_INITIALIZER;
//pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;
uint8* digout = 0;
int wcounter;



// add ns to timespec
void add_timespec(struct timespec* ts, int64 addtime)
{
    int64 sec, nsec;

    nsec = addtime % NSEC_PER_SEC;
    sec = (addtime - nsec) / NSEC_PER_SEC;
    ts->tv_sec += sec;
    ts->tv_nsec += nsec;
    if (ts->tv_nsec >= NSEC_PER_SEC)
    {
        nsec = ts->tv_nsec % NSEC_PER_SEC;
        ts->tv_sec += (ts->tv_nsec - nsec) / NSEC_PER_SEC;
        ts->tv_nsec = nsec;
    }
}

// PI calculation to get linux time synced to DC time 
void ec_sync(int64 reftime, int64 cycletime, int64* offsettime)
{
    static int64 integral = 0;
    int64 delta;
    // set linux sync point 50us later than DC sync, just as example
    delta = (reftime - 50000) % cycletime;
    if (delta > (cycletime / 2)) { delta = delta - cycletime; }
    if (delta > 0) { integral++; }
    if (delta < 0) { integral--; }
    *offsettime = -(delta / 100) - (integral / 20);
}

// RT EtherCAT thread
void ecatthread(void* ptr)
{
    struct timespec   ts;
    struct timeval    tp;
    int rc;
    int ht;
    int64 cycletime;

    //rc = pthread_mutex_lock(&mutex);
    //rc = gettimeofday(&tp, NULL);

    // Convert from timeval to timespec
    ts.tv_sec = tp.tv_sec;
    ht = (tp.tv_usec / 1000) + 1; // round to nearest ms
    ts.tv_nsec = ht * 1000000;
    cycletime = *(int*)ptr * 1000; //cycletime in ns
    toff = 0;
    dorun = 0;
    while (1)
    {
        // calculate next cycle start
        add_timespec(&ts, cycletime + toff);
        // wait to cycle start
        //rc = pthread_cond_timedwait(&cond, &mutex, &ts);
        if (dorun > 0)
        {
            //rc = gettimeofday(&tp, NULL);

            ec_send_processdata();

            wcounter = ec_receive_processdata(EC_TIMEOUTRET);

            //dorun++;
            // if we have some digital output, cycle
            //if (digout) *digout = (uint8)((dorun / 16) & 0xff);

            if (ec_slave[0].hasdc)
            {
                // calulate toff to get linux time and DC synced
                ec_sync(ec_DCtime, cycletime, &toff);
            }
        }
    }
}

#endif




#ifdef SyncSampleCode

#include "sys.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"

#include "ethercat.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Error Handler
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long ECatError(long errorCode, bool disconnect)
{
	int i;
	//Force all slaves to disarm immediately
	for (i = 0; i < gECatMaster.mSlaveCount; i++)
		gECatSlaves[i].mControlWord = 0;

	gECatMaster.mStatusBits |= ECAT_STATUS_ERROR;

	if (disconnect)
		ECatNetworkPurge(1000);

	return errorCode;
}

int Omron_1S_setup(uint16 slave)
{
	int retval;
	uint16 u16val;

	uint16 map_1c12[2] = { 0x0001, 0x1701 };
	uint16 map_1c13[2] = { 0x0001, 0x1B01 };


	retval = 0;

	retval += ec_SDOwrite(slave, 0x1c12, 0x00, TRUE, sizeof(map_1c12), &map_1c12, EC_TIMEOUTSAFE);
	retval += ec_SDOwrite(slave, 0x1c13, 0x00, TRUE, sizeof(map_1c13), &map_1c13, EC_TIMEOUTSAFE);

	uint8 u8val = 8; // Set CSP Mode
	retval += ec_SDOwrite(slave, 0x3000, 0xF2, FALSE, sizeof(uint8), &u8val, EC_TIMEOUTSAFE);


	while (EcatError) printf("%s", ec_elist2string());

	printf("OMRON 1S slave %d set, retval = %d\n", slave, retval);

	return 1;
}


#define EC_TIMEOUTMON 500

BYTE IOmap[4096];
int expectedWKC;

volatile int wkc = 0;
volatile int rtcnt = 0;
volatile boolean RunPDO = false;
uint8 currentgroup = 0;

///////////////////////////////////////////////////////////////////
void PrintErrorCodes()
{
	int i;
	ec_readstate();
	for (i = 1; i <= ec_slavecount; i++)
	{
		if (ec_slave[i].state != EC_STATE_OPERATIONAL)
		{
			printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n",
				i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
		}
	}
}

#define CYCLE_TIME_us 1000
int64 InitialClockDiff_ns = 0, FirstSync0Time_ns, StartPDOTime_us;

bool PreciseDelay(DWORD Time_us);

//////////////////////////////////////////////////////////////////
/* RT thread for process data, just does IO transfer */
void Ethercat_RT_Thread(void* lpParam)
{

ReStart_RT:


	while (!RunPDO)
	{
		LATFINV = 1 << 13; //debug		
		vTaskDelay(2);
	}

	int64 CurrentTime = GetTime_usec();

	if (CurrentTime >= StartPDOTime_us)
	{
		printf("ERROR.......Time already passed %lld......\n", -StartPDOTime_us + CurrentTime);
		Nop();
		Nop();
	}
	else
	{
		PreciseDelay(StartPDOTime_us - CurrentTime);
	}

	CycleCntr = 0;

	while (RunPDO)
	{
		LATFINV = 1 << 13; //debug

		ec_send_processdata();
		wkc = ec_receive_processdata(EC_TIMEOUTRET);

		if (wkc != 12)
		{
			Nop();
			Nop();
			RunPDO = false;
		}

		rtcnt++;

		StartPDOTime_us += CYCLE_TIME_us;

		int64 Delay = StartPDOTime_us - GetTime_usec();

		PreciseDelay(Delay);

	}


	goto ReStart_RT;

}


////////////////////////////////////////////////////////////////
void DoLowPriorityECatTask()
{

	int i, j, oloop, iloop, chk, slc;

	RunPDO = FALSE;

	printf("EtherCAT Network Startup.....\n");

	// initialise SOEM
	if (ec_init(NULL))
	{
	restart_ECat:
		//make sure link is on
		while ((PORTA & (1 << 4)) == 0) //link LED 
		{
			Nop();
			Nop();
			Nop();
			vTaskDelay(1000);
		}

		extern long long gWastedCycles;
		gWastedCycles = 0; //reset waste cycle count so we can see how ecat is effecting the counter		

		printf("EtherCAT Link in %s\n", (PORTA & (1 << 4)) ? "ON" : "OFF");

		if (ec_config(FALSE, &IOmap) > 0)
		{

			printf("%d slaves found and configured.\n", ec_slavecount);

			if ((ec_slavecount > 1))
			{
				for (slc = 1; slc <= ec_slavecount; slc++)
				{
					//Omron_1S_setup(slc);

					// Omron 1SN01H and 1SN04H
					if ((ec_slave[slc].eep_man == 0x83) && (ec_slave[slc].eep_id == 0xAE || ec_slave[slc].eep_id == 0xB0))
					{
						printf("Found %s at position %d\n", ec_slave[slc].name, slc);
						// link slave specific setup to preop->safeop hook
						ec_slave[slc].PO2SOconfig = &Omron_1S_setup;
					}
				}
			}

			printf("Set SAFE OP State............\n");
			/* wait for all slaves to reach SAFE_OP state */
			ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);

			printf("Configure DC options for every DC capable slave found in the list\n");
			ec_configdc();

			int64 SlaveTime, Sync0start;
			extern ec_slavet	ec_slave[EC_MAXSLAVE];

			printf("Configure Sync0 Trigger time..............\n");
			int slave;
			for (slave = 1; slave <= ec_slavecount; slave++)
			{
				ec_dcsync0(slave, TRUE, CYCLE_TIME_us * 1000, 0, &SlaveTime, &Sync0start);

				if (slave == 1)
				{
					printf("Slave1 time = %lld %ld %d\n", SlaveTime, ec_slave[slave].DCcycle, ec_slave[slave].DCshift);
					FirstSync0Time_ns = Sync0start;

					InitialClockDiff_ns = MasterBaseTime_us * 1000 - SlaveTime;

					StartPDOTime_us = ((Sync0start + InitialClockDiff_ns) / 1000) - (CYCLE_TIME_us * 10) - (CYCLE_TIME_us * 0.9);

				}
			}

			oloop = ec_slave[0].Obytes;
			if ((oloop == 0) && (ec_slave[0].Obits > 0)) oloop = 1;
			if (oloop > 8) oloop = 8;
			iloop = ec_slave[0].Ibytes;
			if ((iloop == 0) && (ec_slave[0].Ibits > 0)) iloop = 1;
			if (iloop > 8) iloop = 8;

			expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;

			/* send one valid process data to make outputs in slaves happy*/
			ec_send_processdata();
			wkc = ec_receive_processdata(EC_TIMEOUTRET); //??

			printf("WKC value: %d, Expected %d\n", wkc, expectedWKC);
			RunPDO = TRUE;

			int64 Sync0StartTime = ((FirstSync0Time_ns + InitialClockDiff_ns) / 1000);
			while (GetTime_usec() < Sync0StartTime && wkc == expectedWKC)
			{
				vTaskDelay(2);
			}

			vTaskDelay(15);

			printf("Request OP State\n");

			ec_slave[0].state = EC_STATE_OPERATIONAL;
			/* request OP state for all slaves */

			ec_writestate(0);

			tmOpMode = GetTime_usec();

			/* wait for all slaves to reach OP state */
			ec_statecheck(0, EC_STATE_OPERATIONAL, 5 * EC_TIMEOUTSTATE);

			if (ec_slave[0].state == EC_STATE_OPERATIONAL)
			{
				printf("<><> OP State <><> ");

				/* cyclic loop, reads data from RT thread */
				while (1)
				{
					//printf("Clock Drift %lld ", ClockDrift);
					Sleep(1000);
					if (wkc != expectedWKC)
					{
						printf("ERROR: RTCnt %d wkc %d ExpWkc %d, waste %lld\n", rtcnt, wkc, expectedWKC, gWastedCycles);
						PrintErrorCodes();
						break;
					}
					else
					{
						U4TXREG = '#';
					}
				}

				RunPDO = FALSE;
				Sleep(10);
			}
			else
			{
				RunPDO = FALSE;
				printf("Not all slaves reached operational state.\n");
				PrintErrorCodes();
			}

			printf("\nRequest init state for all slaves\n");
			ec_slave[0].state = EC_STATE_INIT;
			/* request INIT state for all slaves */
			ec_writestate(0);
		}
		else
		{
			printf("No slaves found!\n");
		}

		goto restart_ECat;

	}
}








#endif