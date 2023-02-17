#include <stdlib.h>
#include <threads.h>
#include <semaphore.h>
#include <time.h>
#include <stdio.h>

#include "../inc/global.h"

struct CpuInfo{
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
};

mtx_t cpuInfoMutex, cpuUsageMutex, watchdogUpdateMutex;

struct CpuInfo* currentCpuInfoBuffer;
struct CpuInfo* previousCpuInfoBuffer;

float* cpuUsageBuffer;

//Three updates for three different threads (excluding watchdog). To make sure that if one thread doesn't work, the others won't keep updating the watchdog.
time_t updateBuffer[3];
void updateWatchdogBuffer(int threadIndex){
    mtx_lock(&watchdogUpdateMutex);
    if(threadIndex < 3 && threadIndex >= 0){
        updateBuffer[threadIndex] = time(NULL);
    }
    else{
        //todo:error to logger
        printf("\nTried to update index out of range in buffers.c (%i)\n", threadIndex);
        exit(1);
    }
    mtx_unlock(&watchdogUpdateMutex);

}

void initializeMutexes(){
    mtx_init(&cpuInfoMutex, mtx_plain);
    mtx_init(&cpuUsageMutex, mtx_plain);
    mtx_init(&watchdogUpdateMutex, mtx_plain);   
}

void destroyMutexes(){
    mtx_destroy(&cpuInfoMutex);
    mtx_destroy(&cpuUsageMutex);
    mtx_destroy(&watchdogUpdateMutex);    
}

void allocateBufferMemory(){
    currentCpuInfoBuffer = malloc(sizeof(struct CpuInfo)*cpuCoreAmount);
    previousCpuInfoBuffer = malloc(sizeof(struct CpuInfo)*cpuCoreAmount);
    cpuUsageBuffer = malloc(sizeof(float)*cpuCoreAmount);
}

void collectBufferGarbage(){
    free(currentCpuInfoBuffer);
    free(previousCpuInfoBuffer);
    free(cpuUsageBuffer);
}