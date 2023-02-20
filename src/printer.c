#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../inc/printer.h"
#include "../inc/global.h"
#include "../inc/buffers.h"
#include "../inc/watchdog.h"
#include "../inc/threads.h"

static char percentageBar[10];

//What: Creates a bar that for every 10% usage inside the given core adds one # more up to a max of 9, it fill out the rest with - chars.
//What for: So the user can have some visual representation of the core's usage
char* makePercentageBar(const unsigned int core){
    int simplifiedPercent = (int)cpuUsageBuffer[core]/10;
    for(int i = 0; i < 9; i++){
        percentageBar[i] = i < simplifiedPercent ? '#' : '-';
    } 
    percentageBar[9] = '\0';
    return percentageBar;
}

//What: Prints to the screen the cpu usage of every core
//What for: So the user can see it
static void printCpuUsageAverages(void){
    for(unsigned int core = 0; core < cpuCoreAmount; core++){
        printf("CPU%u - [%s] - %.1f%%\n",core+1 ,makePercentageBar(core), cpuUsageBuffer[core]);
    }
}

//What: Calls printCpuUsageAverages() every (args) microseconds and uses mutexes to lock analyzerLoop
//What for: So it can be synchronized with analyzerLoop so the data won't change when it's being read
int printCpuUsagePercantages(void* args){
    unsigned int delay = *(unsigned int *)args;

    while(threadsActive){
        updateWatchdogBuffer(2);

        usleep(delay);
        system("clear");

        mtx_lock(&cpuUsageMutex); //Lock cpu usage buffer for analyzerLoop()

        printCpuUsageAverages();

        mtx_unlock(&cpuUsageMutex);

    }
    thrd_exit(0);
}
