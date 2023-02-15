#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "global.h"
#include "buffers.h"


void printCpuUsageAverages(){
    char percantageBar[10];

    for(int i = 0; i < cpuCoreAmount; i++){
        int simplifiedPercent = (int)cpuUsageBuffer[i]/10;
        for(int j = 0; j < 10; j++){
            percantageBar[j] = j < simplifiedPercent ? '#' : '-';
        }
        printf("CPU%i - [%s] - %.1f%%\n",i ,percantageBar, cpuUsageBuffer[i]);
    }
}

//Takes the average from the entire usage buffer every second and then fully clears it and prints the averages.
void* printCpuUsagePercantages(){
    while(1){
        sleep(1);
        system("clear");

        mtx_lock(&cpuUsageMutex); //Lock cpu usage buffer for analyzeCpuInfo()

        printCpuUsageAverages();

        mtx_unlock(&cpuUsageMutex);//Unlock cpu usage buffer

    }
    return NULL;
}