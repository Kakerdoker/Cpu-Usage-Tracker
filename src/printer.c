#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../inc/global.h"
#include "../inc/buffers.h"
#include "../inc/watchdog.h"
#include "../inc/threads.h"

char percentageBar[9];

char* makePercentageBar(int core){
    int simplifiedPercent = (int)cpuUsageBuffer[core]/10;
    for(int i = 0; i < 9; i++){
        percentageBar[i] = i < simplifiedPercent ? '#' : '-';;
    } 
    return percentageBar;
}

void printCpuUsageAverages(){
    for(int core = 0; core < cpuCoreAmount; core++){
        printf("CPU%i - [%s] - %.1f%%\n",core+1 ,makePercentageBar(core), cpuUsageBuffer[core]);
    }
}

int printCpuUsagePercantages(){
    while(threadsActive){
        updateWatchdogBuffer(2);

        sleep(1);
        system("clear");

        mtx_lock(&cpuUsageMutex); //Lock cpu usage buffer for analyzeCpuInfo()

        printCpuUsageAverages();

        mtx_unlock(&cpuUsageMutex);

    }
    thrd_exit(0);
}