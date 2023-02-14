#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "global.h"
#include "buffers.h"

float* avrgCpuUsage;

void initializePrinterVariables(){
    avrgCpuUsage = malloc(sizeof(float)*cpuCoreAmount);
}

void clearAvrgUsage(){
    for(int i = 0; i < cpuCoreAmount; i++){
        avrgCpuUsage[i] = 0;
    }
}

void averageTheSum(){
    for(int i = 0; i < cpuCoreAmount; i++){
        avrgCpuUsage[i] /= (float)cpuUsageBufferIndex;
    }
}

void calcAvrgFromUsageBuffer(){
    for(int buffer = 0; buffer < cpuUsageBufferIndex; buffer++){
        for(int cpu = 0; cpu < cpuCoreAmount; cpu++){
            avrgCpuUsage[cpu] += cpuUsageBuffer[buffer][cpu];
        }
        sem_trywait(&semUsageFull);//Decrement semaphore without unlocking it until it reaches 0. Maybe put it in it's own loop and method for better readability?
    }
    averageTheSum();
}

void printCpuUsageAverages(){
    char percantageBar[10];

    for(int i = 0; i < cpuCoreAmount; i++){
        int simplifiedPercent = (int)avrgCpuUsage[i]/10;
        for(int j = 0; j < 10; j++){
            percantageBar[j] = j < simplifiedPercent ? '#' : '-';
        }
        printf("CPU%i - [%s] - %.1f%%\n",i ,percantageBar, avrgCpuUsage[i]);
    }
}

//Takes the average from the entire usage buffer every second and then fully clears it and prints the averages.
void* printCpuUsagePercantages(){
    while(1){
        sleep(1);
        system("clear");

        sem_wait(&semUsageFull); //Lock semaphore and decrement it
        mtx_lock(&cpuUsageMutex); //Lock cpu usage buffer for analyzeCpuInfo()

        clearAvrgUsage();
        calcAvrgFromUsageBuffer();

        printCpuUsageAverages();

        cpuUsageBufferIndex = 0; //Reset index to zero

        mtx_unlock(&cpuUsageMutex);//Unlock cpu usage buffer
        sem_post(&semUsageEmpty); //Unlock semaphore and increment it
    }
    return NULL;
}