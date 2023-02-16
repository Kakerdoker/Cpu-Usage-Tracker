#include <stdlib.h>
#include <threads.h>
#include <semaphore.h>

#include "../inc/global.h"

struct CpuInfo{
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
};

mtx_t cpuInfoMutex, cpuUsageMutex;
sem_t semUsageEmpty, semUsageFull;

struct CpuInfo* currentCpuInfoBuffer;
struct CpuInfo* previousCpuInfoBuffer;
float* cpuUsageBuffer;

void initializeMutexes(){
    mtx_init(&cpuInfoMutex, mtx_plain);
    mtx_init(&cpuUsageMutex, mtx_plain);
}

void destroyMutexes(){
    mtx_destroy(&cpuInfoMutex);
    mtx_destroy(&cpuUsageMutex);
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