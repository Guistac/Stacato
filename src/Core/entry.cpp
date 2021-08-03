
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

















