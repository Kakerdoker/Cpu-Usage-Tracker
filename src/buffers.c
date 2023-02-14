#include <stdlib.h>
#include <threads.h>
#include <semaphore.h>
#include "global.h"

struct CpuInfo{
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
};

mtx_t cpuInfoMutex, cpuUsageMutex;
sem_t semUsageEmpty, semUsageFull;

struct CpuInfo* currentCpuInfoBuffer;
struct CpuInfo* previousCpuInfoBuffer;
float** cpuUsageBuffer;
const unsigned int cpuUsageBufferSize = 32;
int cpuUsageBufferIndex = 0;

void initializeMutexesAndSemaphores(){
    mtx_init(&cpuInfoMutex, mtx_plain);
    mtx_init(&cpuUsageMutex, mtx_plain);

    sem_init(&semUsageEmpty, 0, 32);
    sem_init(&semUsageFull, 0, 0);
}

void destroyMutexesAndSemaphores(){
    mtx_destroy(&cpuInfoMutex);
    mtx_destroy(&cpuUsageMutex);

    sem_destroy(&semUsageEmpty);
    sem_destroy(&semUsageFull);
}

void allocateBufferMemory(){
    currentCpuInfoBuffer = malloc(sizeof(struct CpuInfo)*cpuCoreAmount);
    previousCpuInfoBuffer = malloc(sizeof(struct CpuInfo)*cpuCoreAmount);

    cpuUsageBuffer = malloc(sizeof(float*)*32);
    for(int i = 0; i < cpuUsageBufferSize; i++){
        cpuUsageBuffer[i] = malloc(sizeof(float)*cpuCoreAmount);
    }
}

void collectBufferGarbage(){
    free(currentCpuInfoBuffer);
    free(previousCpuInfoBuffer);
    for(int i = 0; i < cpuUsageBufferSize; i++){
        free(cpuUsageBuffer[i]);
    }
    free(cpuUsageBuffer);
}